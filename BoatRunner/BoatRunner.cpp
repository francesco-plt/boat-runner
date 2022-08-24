using namespace std;

#include "BoatRunner.hpp"

#include <cmath>
#include <glm/gtx/euler_angles.hpp>
#include <string>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Global variables

// Asset paths
static const string MODEL_PATH = "models";
static const string TEXTURE_PATH = "textures";
static const string SHADERS_PATH = "shaders";

// Position limits
static const float fieldWidth = 30;
static const float leftBorder = fieldWidth / 2;
static const float rightBorder = (-1) * (fieldWidth / 2);
static const float boatWidth = 2.5;

// Camera displacement w.r.t. boat
static const float camDelta = 7.0f;

// Game logic constants (not used for now)
enum status : int { pause = 0, play = 1 };

// Other variables
static const glm::vec3 origin = glm::vec3(0, 0, 0);
static const glm::mat4 I = glm::mat4(1.0f);           // Identity matrix
static const glm::vec3 xAxis = glm::vec3(1, 0, 0);    // x axis
static const glm::vec3 yAxis = glm::vec3(0, 1, 0);    // y axis
static const glm::vec3 zAxis = glm::vec3(0, 0, 1);    // z axis
static const int whRes = 1000;                        // window horizontal resolution
static const float moveSpeed = 0.01;                  // boat motion speed multiplier
static const float rotSpeed = glm::radians(1.0f);     // camera rotation speed multiplier
static const glm::vec3 boatScalingFactor =            // scaling factor for boat model
    glm::vec3(0.0003f);
static const glm::vec3 initBoatPos = origin;                    // initial boat position
static const glm::vec3 initCamPos = origin + glm::vec3(0, 0, camDelta); // initial camera position

// direction vectors
unordered_map<string, glm::vec4> dirs({
    {"east", glm::vec4(1, 0, 0, 1)},
    {"west", glm::vec4(-1, 0, 0, 1)},
    {"north", glm::vec4(0, 0, -1, 1)},
    {"south", glm::vec4(0, 0, 1, 1)}
});

// Buffer object structure
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class BoatRunner : public BaseProject {
    protected:

    // Descriptor Layouts
    DescriptorSetLayout DSLglobal;
    DescriptorSetLayout DSLobj;

    // Pipelines
    Pipeline P1;

    // Models, Textures and Descriptor sets (values assigned to uniforms)

    int dsCount = 3;    // boat, rock1, rock2 (plus global set)

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


    // Starting position variables

    // Boat in the origin for simplicity
    glm::vec3 boatPos = initBoatPos;

    // Camera a little bit above and behind
    // the boat to be able to see it
    glm::vec3 camPos = initCamPos;
    // And camera pointing at the boat
    glm::vec3 camDir = glm::normalize(camPos - boatPos);

    // Debug variables
    glm::vec3 camLogPos;
    glm::vec3 camLogDir;
    glm::vec3 boatLogPos;

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
            {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}, {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}}
        );

        DSLglobal.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}});

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that
        // will be used in this pipeline. The first element will be set 0, and so
        // on..
        P1.init(this, SHADERS_PATH + "/vert.spv", SHADERS_PATH + "/frag.spv", {&DSLglobal, &DSLobj});

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

        DS_global.init(this, &DSLglobal, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}});
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
                VK_PIPELINE_BIND_POINT_GRAPHICS,         // pipeline bind point
                P1.pipelineLayout,                       // layout
                0,                                       // first set
                1,                                       // descriptor set count
                &DS_global.descriptorSets[currentImage], // pDescriptorSets
                0,                                       // dynamic offset count
                nullptr                                  // pDynamicOffsets
        );

        // Boat descriptor set bindings
        VkBuffer vertexBuffersBoat[] = {M_Boat.vertexBuffer};
        VkDeviceSize offsetsBoat[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersBoat, offsetsBoat);
        vkCmdBindIndexBuffer(commandBuffer, M_Boat.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1,
            &DS_Boat.descriptorSets[currentImage], 0, nullptr
        );
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Boat.indices.size()), 1, 0, 0, 0);

        // Rock1 descriptor set bindings
        VkBuffer vertexBuffersRock1[] = {M_Rock1.vertexBuffer};
        VkDeviceSize offsetsRock1[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock1, offsetsRock1);
        vkCmdBindIndexBuffer(commandBuffer, M_Rock1.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1,
            &DS_Rock1.descriptorSets[currentImage], 0, nullptr
        );
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Rock1.indices.size()), 1, 0, 0, 0);

        // Rock2 descriptor set bindings
        VkBuffer vertexBuffersRock2[] = {M_Rock2.vertexBuffer};
        VkDeviceSize offsetsRock2[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock2, offsetsRock2);
        vkCmdBindIndexBuffer(commandBuffer, M_Rock2.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1,
            &DS_Rock2.descriptorSets[currentImage], 0, nullptr
        );
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Rock2.indices.size()), 1, 0, 0, 0);
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your
    // application.
    void updateUniformBuffer(uint32_t currentImage) {
        
        // Positions updates
        updatePosition(window);

        // Checking that new values are legal
        // boatPos = ensurePosLimits(boatPos);
        camPos = ensurePosLimits(camPos);

        // Printing the new values for debug purposes
        debugCoord(camPos, camLogPos, "Camera Position");
        debugCoord(boatPos, boatLogPos, "Boat Position");
        debugCoord(camDir, camLogDir, "Boat Position");

        // Buffers update
        UniformBufferObject ubo{};
        void *data;

        ubo.model = I;
        ubo.view = LookAtMat(camPos, camPos + camDir);
        ubo.proj = computePerspective();
        ubo.proj[1][1] *= -1;

        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

        // 2. Boat
        ubo.model = I;
        glm::translate(ubo.model, boatPos);
        ubo.model = glm::scale(ubo.model, boatScalingFactor);

        vkMapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage]);

        /* Note: random position values for now */
        // 3. Rock1

        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.5f, 0.0f));
        ubo.model = glm::mat4(1);

        vkMapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage]);

        // 4. Rock2

        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 2.0f, -1.0f));

        vkMapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage]);

        camLogPos = camPos;
        camLogDir = camDir;
        boatLogPos = boatPos;
    }

    void updatePosition(GLFWwindow *window) {
        static auto startTime = chrono::high_resolution_clock::now();
        auto currentTime = chrono::high_resolution_clock::now();
        float time = chrono::duration<float, chrono::seconds::period>(currentTime - startTime).count();
        float lastTime = 0.0f;
        float deltaT = time - lastTime;
        lastTime = time;

        glm::vec3 posDelta = glm::vec3(0.0f);

        if (glfwGetKey(window, GLFW_KEY_LEFT)) {
            camDir.y += rotSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT))
        {
            camDir.y -= rotSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_UP))
        {
            camDir.x += rotSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN))
        {
            camDir.x -= rotSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_A)) {
            posDelta = computePos("west", camDir.y, deltaT);
        }

        if (glfwGetKey(window, GLFW_KEY_D)) {
            posDelta = computePos("east", camDir.y, deltaT);
        }

        if (glfwGetKey(window, GLFW_KEY_W)) {
            posDelta = computePos("north", camDir.y, deltaT);
        }

        if (glfwGetKey(window, GLFW_KEY_S)) {
            posDelta = computePos("south", camDir.y, deltaT);
        }
        
        // boatPos += posDelta;
        camPos += posDelta;
    }

    glm::vec3 ensurePosLimits(glm::vec3 pos) {
        if (pos.x + boatWidth > leftBorder) {
            pos.x = leftBorder - boatWidth;
        } else if (pos.x - boatWidth < rightBorder) {
            pos.x = rightBorder + boatWidth;
        }
        return pos;
    }

    /* ---------------------------------------------------------------------------------------------- */
    // What follows is just helper functions + cleanup code

    // given a direction and a speed, compute the new position offset
    glm::vec3 computePos(string direction, float dirValue, float deltaT) {
        glm::vec4 dir = dirs[direction];
        return moveSpeed * glm::vec3(glm::rotate(I, dirValue, yAxis) * dir);
    }

    // to avoid redundant code while computing ubo.proj
    glm::mat4 computePerspective() {
        return glm::perspective(
            glm::radians(45.0f),
            swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f
        );
    }

    // Pos -> Position of the camera (c)
    // aim -> Posizion of the target (a)
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

    void debugCoord(glm::vec3 newPos, glm::vec3 oldPos, string id) {
        if(newPos != oldPos) {
            cout << "Variable " << id << " updated: (" <<
                oldPos.x << ", " << oldPos.y << ", " << oldPos.z << ") -> ("
                << newPos.x << ", " << newPos.y << ", " << newPos.z << ")" << endl;
        }
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
    return EXIT_SUCCESS;
}