#include "BoatRunner.hpp"
#include <string>
#include <unordered_map>
#include <glm/gtx/euler_angles.hpp>

const std::string MODEL_PATH = "models";
const std::string TEXTURE_PATH = "textures";
const std::string SHADERS_PATH = "shaders";

struct globalUniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
};

class BoatRunner : public BaseProject {
// Vulkan objects
protected:

    // Descriptor Layouts
    DescriptorSetLayout DSLglobal;
    DescriptorSetLayout DSLobj;

    // Pipelines
    Pipeline P1;

    // Models, Textures and Descriptors (values assigned to uniforms)
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

    // glm::vec3 CamAng = glm::vec3(0.0f, 95.68f, 0.0f);
    // glm::vec3 CamPos = glm::vec3(0.0f, 1.0f, 1.0f);
    // glm::vec3 BoatPos = glm::vec3(0.0f, 0.0f, 0.0f);
    // float boatOffset = -0.1f;
    // glm::vec3 BoatPos = glm::vec3(1.65f, 1.5f + boatOffset, -0.1f + boatOffset);

    // application parameters
    void setWindowParameters()
    {
        // Window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "Boat Runner";
        initialBackgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = dsCount + 1;
        texturesInPool = dsCount;
        setsInPool = dsCount + 1;
    }

    // Load and setup of Vulkan objects
    void localInit()
    {

        // Descriptor Layouts [what will be passed to the shaders]
        DSLobj.init(this, {
            // this array contains the bindings:
            // first  element : the binding number
            // second element : the time of element (buffer or texture)
            // third  element : the pipeline stage where it will be used
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
        });

        DSLglobal.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}});

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P1.init(this, SHADERS_PATH + "/vert.spv", SHADERS_PATH + "/frag.spv", {&DSLglobal, &DSLobj});

        // Models, textures and Descriptors (values assigned to the uniforms)
        M_Boat.init(this, MODEL_PATH + "/Boat.obj");
        M_Rock1.init(this, MODEL_PATH + "/Rock1.obj");
        M_Rock2.init(this, MODEL_PATH + "/Rock2.obj");

        T_Boat.init(this, TEXTURE_PATH + "/Boat.bmp");
        T_Rock1.init(this, TEXTURE_PATH + "/Rock1.png");
        T_Rock2.init(this, TEXTURE_PATH + "/Rock2.jpg");

        DS_Boat.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Boat}});
        DS_Rock1.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Rock1}});
        DS_Rock2.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Rock2}});

        DS_global.init(this, &DSLglobal, {{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}});
    }

    // pretty self explanatory
    void localCleanup()
    {
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
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage)
    {

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.graphicsPipeline);

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
            0, nullptr
        );

        VkBuffer vertexBuffersBoat[] = {M_Boat.vertexBuffer};
        VkDeviceSize offsetsBoat[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersBoat, offsetsBoat);
        vkCmdBindIndexBuffer(commandBuffer, M_Boat.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1, &DS_Boat.descriptorSets[currentImage],
            0, nullptr
        );
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Boat.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersRock1[] = {M_Rock1.vertexBuffer};
        VkDeviceSize offsetsRock1[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock1, offsetsRock1);
        vkCmdBindIndexBuffer(commandBuffer, M_Rock1.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1, &DS_Rock1.descriptorSets[currentImage],
            0, nullptr
        );
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Rock1.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersRock2[] = {M_Rock2.vertexBuffer};
        VkDeviceSize offsetsRock2[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock2, offsetsRock2);
        vkCmdBindIndexBuffer(commandBuffer, M_Rock2.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 1, 1, &DS_Rock2.descriptorSets[currentImage],
            0, nullptr
        );
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Rock2.indices.size()), 1, 0, 0, 0);
    }

    // function to create world matrix from assignment 7
    glm::mat4 eulerWM(glm::vec3 pos, glm::vec3 YPR)
    {
        glm::mat4 MEa = glm::eulerAngleYXZ(glm::radians(YPR.y), glm::radians(YPR.x), glm::radians(YPR.z));
        return glm::translate(glm::mat4(1.0), pos) * MEa;
    }

    // TODO
    // Uniform buffers are updated here, aka application logic
    // for now those are placeholder settings
    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
		static float lastTime = 0.0f;
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
					(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;
					
		const float ROT_SPEED = glm::radians(60.0f);
		const float MOVE_SPEED = 0.1f;

		static float debounce = time;

        static int state = 0;

        std::unordered_map<std::string, glm::vec3> CamOrientation;
        CamOrientation["east"] = glm::vec3(1, 0, 0);
        CamOrientation["west"] = glm::vec3(-1, 0, 0);

        float yawAngle = 90.0f;

        glm::vec3 pos;
        glm::mat4 BoatPosition;

        // WASD input control
        // we want to move the boat and to modify the camera position
        // according to the boat position

        // west
        if (glfwGetKey(window, GLFW_KEY_A))
        {
            pos += MOVE_SPEED * CamOrientation["west"] * deltaT;
            // BoatPos = eulerWM(pos, glm::vec3(0.0f, yawAngle, 0.0f));
            // BoatPos -= MOVE_SPEED * glm::vec3(
            //     glm::rotate(glm::mat4(1.0f),
            //     CamAng.y,
            //     glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)
            // );
        }
        // east
        if (glfwGetKey(window, GLFW_KEY_D))
        {
            pos += MOVE_SPEED * CamOrientation["east"] * deltaT;
            // BoatPos = eulerWM(pos, glm::vec3(0.0f, yawAngle, 0.0f));

            // BoatPos += MOVE_SPEED * glm::vec3(
            //     glm::rotate(glm::mat4(1.0f),
            //     CamAng.y,
            //     glm::vec3(0.0f, 1.0f, 0.0f)) *
            //     glm::vec4(1, 0, 0, 1)
            // );
        
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE))
        {
            if (time - debounce > 0.33)
            {
                debounce = time;

                if (state == 0)
                {
                    state = 1;
                }
                else
                {
                    state = 0;
                }
            }
        }

        // glm::mat3 CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.y, glm::vec3(0.0f, 1.0f, 0.0f))) *
        //                    glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.x, glm::vec3(1.0f, 0.0f, 0.0f))) *
        //                    glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.z, glm::vec3(0.0f, 0.0f, 1.0f)));

        glm::mat4 CamMat = glm::mat4(1);
		glm::vec3 CamPos = glm::vec3(0);
        BoatPosition = eulerWM(pos, glm::vec3(0.0f, yawAngle, 0.0f));
		CamPos = glm::vec3(BoatPosition * glm::vec4(0,0,0,1));
        // now we move the camera up of a given factor
        CamPos = CamPos + glm::vec3(0, 1.5f, 3.0f);

        UniformBufferObject ubo{};
        globalUniformBufferObject gubo{};

        gubo.view =
            // glm::rotate(glm::mat4(1.0), -CamAng.z, glm::vec3(0, 0, 1)) *
            // glm::rotate(glm::mat4(1.0), -CamAng.x, glm::vec3(1, 0, 0)) *
            // glm::rotate(glm::mat4(1.0), -CamAng.y, glm::vec3(0, 1, 0)) *
            glm::translate(glm::mat4(1.0), glm::vec3(-CamPos.x, -CamPos.y, -CamPos.z));
        ;

        gubo.proj = glm::perspective(glm::radians(90.0f),
                                     swapChainExtent.width / (float)swapChainExtent.height,
                                     0.1f, 10.0f);
        gubo.proj[1][1] *= -1;
		
		void* data;
        glm::vec3 initial;

        // global descriptor set
        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

        // Boat
        ubo.model = glm::mat4(1.0f);
        // ubo.model = 
        //     glm::rotate(glm::mat4(1.0), -CamAng.z, glm::vec3(0, 0, 1)) *
        //     glm::rotate(glm::mat4(1.0), -CamAng.x, glm::vec3(1, 0, 0)) *
        //     glm::rotate(glm::mat4(1.0), -CamAng.y, glm::vec3(0, 1, 0)) *
        //     glm::translate(glm::mat4(1.0), glm::vec3(-BoatPos.x, -BoatPos.y, -BoatPos.z));
        // ;
        // now we want to scale properly the boat and to rotate it of 180 degrees
        ubo.model = glm::scale(ubo.model, glm::vec3(0.003f));
        ubo.model = BoatPosition * ubo.model;

        vkMapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage]);

        // Rock1
        ubo.model = glm::mat4(1.0f);
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -1.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.15f));
        vkMapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage]);

        // Rock2
        ubo.model = glm::mat4(1.0f);
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 1.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.15f));
        vkMapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage]);
    }
};

// simple main to load
// the project class
int main()
{
    BoatRunner app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}