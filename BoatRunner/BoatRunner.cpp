using namespace std;
#include <cmath>
#include <glm/gtx/euler_angles.hpp>
#include <string>
#include <unordered_map>

#include "BoatRunner.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

static const string MODEL_PATH = "models";
static const string TEXTURE_PATH = "textures";
static const string SHADERS_PATH = "shaders";

// Position limits
static const float fieldWidth = 30;
static const float leftBorder = fieldWidth / 2;
static const float rightBorder = (-1) * (fieldWidth / 2);
static const float boatWidth = 2.5;

// Camera displacement w.r.t. boat
static const float deltaHeight = 4.0f;

// Game logic constants
enum status : int { pause = 0, play = 1 };

// Other constants
static const glm::mat4 I = glm::mat4(1.0f);         // Identity matrix
static const glm::vec3 xAxis = glm::vec3(1, 0, 0);  // x axis
static const glm::vec3 yAxis = glm::vec3(0, 1, 0);  // y axis
static const glm::vec3 zAxis = glm::vec3(0, 0, 1);  // z axis
static const int whRes = 1000;  // window horizontal resolution

unordered_map<string, glm::vec4> dirs({{"east", glm::vec4(1, 0, 0, 1)},
                                       {"west", glm::vec4(-1, 0, 0, 1)},
                                       {"north", glm::vec4(0, 0, -1, 1)},
                                       {"south", glm::vec4(0, 0, 1, 1)}});

unordered_map<string, float> yaws({{"east", 0.0f},
                                   {"north-east", 45.0f},
                                   {"north", 90.0f},
                                   {"north-west", 135.0f},
                                   {"west", 180.0f},
                                   {"south-west", 225.0f},
                                   {"south", 270.0f},
                                   {"south-east", 315.0f}});

// Camera buffer object
struct globalUniformBufferObject {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

// objects (boat, rocks) buffer object
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
};

class BoatRunner : public BaseProject {
 protected:
  // Descriptor Layouts
  DescriptorSetLayout DSLglobal;
  DescriptorSetLayout DSLobj;

  // Pipelines
  Pipeline P1;

  // Models, Textures and Descriptors (values assigned to uniforms)
  int dsCount = 3;  // boat, rock1, rock2 (plus global set)

  Model M_Boat;
  Texture T_Boat;
  DescriptorSet DS_Boat;

  Model M_Rock1;
  Texture T_Rock1;
  DescriptorSet DS_Rock1;

  Model M_Rock2;
  Texture T_Rock2;
  DescriptorSet DS_Rock2;

  DescriptorSet DS_global;

  // Position variables
  glm::vec3 boatPos = glm::vec3(0, 0, 0);
  glm::vec3 camPos =
      boatPos + glm::vec3(0.0f, deltaHeight / 2, deltaHeight / 2);

  // application parameters:
  // Window size, titile and initial background
  void setWindowParameters() {
    // 4:3 aspect ratio
    windowWidth = whRes;
    windowHeight = floor(whRes / 4 * 3);
    windowTitle = "Boat Runner";
    initialBackgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};

    // Descriptor pool sizes
    uniformBlocksInPool = dsCount + 1;
    texturesInPool = dsCount;
    setsInPool = dsCount + 1;
  }

  // Load and setup of Vulkan objects
  void localInit() {
    // Descriptor Layouts [what will be passed to the shaders]
    DSLobj.init(
        this,
        {// this array contains the bindings:
         // first  element : the binding number
         // second element : the time of element (buffer or texture)
         // third  element : the pipeline stage where it will be used
         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
         {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT}});

    DSLglobal.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                           VK_SHADER_STAGE_ALL_GRAPHICS}});

    // Pipelines [Shader couples]
    // The last array, is a vector of pointer to the layouts of the sets that
    // will be used in this pipeline. The first element will be set 0, and so
    // on..
    P1.init(this, SHADERS_PATH + "/vert.spv", SHADERS_PATH + "/frag.spv",
            {&DSLglobal, &DSLobj});

    cout << "Pipeline created"
         << "\n";

    // Models, textures and Descriptors (values assigned to the uniforms)
    M_Boat.init(this, MODEL_PATH + "/Boat.obj");
    M_Rock1.init(this, MODEL_PATH + "/Rock1.obj");
    M_Rock2.init(this, MODEL_PATH + "/Rock2.obj");

    T_Boat.init(this, TEXTURE_PATH + "/Boat.bmp");
    T_Rock1.init(this, TEXTURE_PATH + "/Rock1.png");
    T_Rock2.init(this, TEXTURE_PATH + "/Rock2.jpg");

    DS_Boat.init(this, &DSLobj,
                 {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                  {1, TEXTURE, 0, &T_Boat}});
    DS_Rock1.init(this, &DSLobj,
                  {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                   {1, TEXTURE, 0, &T_Rock1}});
    DS_Rock2.init(this, &DSLobj,
                  {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                   {1, TEXTURE, 0, &T_Rock2}});

    DS_global.init(this, &DSLglobal,
                   {{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}});
  }

  // pretty self explanatory
  void localCleanup() {
    DS_Boat.cleanup();
    T_Boat.cleanup();
    M_Boat.cleanup();

    DS_Rock1.cleanup();
    T_Rock1.cleanup();
    M_Rock1.cleanup();

    DS_Rock2.cleanup();
    T_Rock2.cleanup();
    M_Rock2.cleanup();

    DS_global.cleanup();

    P1.cleanup();
    DSLglobal.cleanup();
    DSLobj.cleanup();
  }

  // Creation of the command buffer:
  // Objects (+ buffers, textures) --> GPU
  void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
    // Pipeline binding
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      P1.graphicsPipeline);

    // Global descriptor set binding
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,          // pipeline bind point
        P1.pipelineLayout,                        // layout
        0,                                        // first set
        1,                                        // descriptor set count
        &DS_global.descriptorSets[currentImage],  // pDescriptorSets
        0,                                        // dynamic offset count
        nullptr                                   // pDynamicOffsets
    );

    // Boat descriptor set bindings
    VkBuffer vertexBuffersBoat[] = {M_Boat.vertexBuffer};
    VkDeviceSize offsetsBoat[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersBoat, offsetsBoat);
    vkCmdBindIndexBuffer(commandBuffer, M_Boat.indexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            P1.pipelineLayout, 1, 1,
                            &DS_Boat.descriptorSets[currentImage], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer,
                     static_cast<uint32_t>(M_Boat.indices.size()), 1, 0, 0, 0);

    // Rock1 descriptor set bindings
    VkBuffer vertexBuffersRock1[] = {M_Rock1.vertexBuffer};
    VkDeviceSize offsetsRock1[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock1,
                           offsetsRock1);
    vkCmdBindIndexBuffer(commandBuffer, M_Rock1.indexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            P1.pipelineLayout, 1, 1,
                            &DS_Rock1.descriptorSets[currentImage], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer,
                     static_cast<uint32_t>(M_Rock1.indices.size()), 1, 0, 0, 0);

    // Rock2 descriptor set bindings
    VkBuffer vertexBuffersRock2[] = {M_Rock2.vertexBuffer};
    VkDeviceSize offsetsRock2[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock2,
                           offsetsRock2);
    vkCmdBindIndexBuffer(commandBuffer, M_Rock2.indexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            P1.pipelineLayout, 1, 1,
                            &DS_Rock2.descriptorSets[currentImage], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer,
                     static_cast<uint32_t>(M_Rock2.indices.size()), 1, 0, 0, 0);
  }

  // Here is where you update the uniforms.
  // Very likely this will be where you will be writing the logic of your
  // application.
  void updateUniformBuffer(uint32_t currentImage) {
    globalUniformBufferObject gubo{};
    UniformBufferObject ubo{};
    void *data;

    // Boat and camera position update
    glm::vec3 posDelta = updatePosition(window);
    boatPos += posDelta;
    cameraPos += posDelta;

    // Uniform buffer objects update (Camera, Boat, Rock1, Rock2)

    // 1. Camera

    gubo.view = glm::mat4(1);
    // gubo.view = LookAtMat(camPos, boatPos);
    gubo.view = glm::translate(ubo.model, camPos);
    gubo.proj = computePerspective();
    gubo.proj[1][1] *= -1;

    vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
                sizeof(gubo), 0, &data);
    memcpy(data, &gubo, sizeof(gubo));
    vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

    // 2. Boat

    ubo.model = glm::mat4(1);
    // First we want to scale it
    ubo.model = glm::scale(ubo.model, glm::vec3(0.003f));
    // Then we want to translate it according to its updated position
    ubo.model = glm::translate(ubo.model, boatPos);

    vkMapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage], 0,
                sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage]);

    // // 3. Rock1

    // ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.04422f,
    // 0.0f)); ubo.model = glm::mat4(1);

    // vkMapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage], 0,
    //             sizeof(ubo), 0, &data);
    // memcpy(data, &ubo, sizeof(ubo));
    // vkUnmapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage]);

    // // 4. Rock2

    // ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.2f, 1.9f,
    // -0.86f));

    // vkMapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage], 0,
    //             sizeof(ubo), 0, &data);
    // memcpy(data, &ubo, sizeof(ubo));
    // vkUnmapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage]);
  }

  glm::vec3 updatePosition(GLFWwindow *window) {
    static auto startTime = chrono::high_resolution_clock::now();
    auto currentTime = chrono::high_resolution_clock::now();
    float time = chrono::duration<float, chrono::seconds::period>(currentTime -
                                                                  startTime)
                     .count();
    float lastTime = 0.0f;
    float deltaT = time - lastTime;
    lastTime = time;

    const float ROT_SPEED = glm::radians(0.01f);
    const float MOVE_SPEED = 0.01;


    if (glfwGetKey(window, GLFW_KEY_A)) {
      return computePos(MOVE_SPEED, "west", deltaT);
    }

    if (glfwGetKey(window, GLFW_KEY_D)) {
      return computePos(MOVE_SPEED, "east", deltaT);
    }

    if (glfwGetKey(window, GLFW_KEY_W)) {
      return computePos(MOVE_SPEED, "north", deltaT);
    }

    if (glfwGetKey(window, GLFW_KEY_S)) {
      return computePos(MOVE_SPEED, "south", deltaT);
    }

    // Checking that new values are legal
    ensurePosLimits(boatPos);
    ensurePosLimits(camPos);

    return;
  }

  void ensurePosLimits(glm::vec3 pos) {
    if (pos.x + boatWidth > leftBorder) {
      pos.x = leftBorder - boatWidth;
    } else if (pos.x - boatWidth < rightBorder) {
      pos.x = rightBorder + boatWidth;
    } else {
      return;
    }
  }

  /* ----------------------------------------------------------------------------------------------
   */
  // What follows is just helper functions

  glm::vec3 computePos(float speed, string direction, float deltaT) {
    glm::vec4 dir = dirs[direction];
    return speed * glm::vec3(glm::rotate(I, 0.0f, yAxis) * dir) * deltaT;
  }

  glm::mat4 computePerspective() {
    return glm::perspective(
        glm::radians(45.0f),
        swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
  }

  // Pos    -> Position of the camera (c)
  // aim    -> Posizion of the target (a)
  glm::mat4 LookAtMat(glm::vec3 Pos, glm::vec3 aim) {
    // glm::lookAt() functions: creates a
    // Look-at matrix starting from three glm::vec3 vectors
    // representing respectively the center of the camera, the point it
    // targets, and it up-vector
    return glm::lookAt(Pos, aim, yAxis);
  }

  glm::mat4 LookInDirMat(glm::vec3 Pos, glm::vec3 Angs) {
    return glm::rotate(glm::mat4(1.0), -Angs.z, glm::vec3(0, 0, 1)) *
           glm::rotate(glm::mat4(1.0), -Angs.y, glm::vec3(1, 0, 0)) *
           glm::rotate(glm::mat4(1.0), -Angs.x, glm::vec3(0, 1, 0)) *
           glm::translate(glm::mat4(1.0), -Pos);
  }
};

// simple main to load
// the project class
int main() {
  BoatRunner app;

  try {
    app.run();
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return EXIT_FAILURE;
  }

  cout << "Exiting..." << endl;
  return EXIT_SUCCESS;
}