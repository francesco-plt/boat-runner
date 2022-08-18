#include "BoatRunner.hpp"
#include <string>

const std::string MODEL_DIR = "models";
const std::string TEXTURE_PATH = "textures";

struct globalUniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
};

class BoatRunner : public BaseProject
{
protected:
    // Vulkan objects

    // Descriptor Layouts
    DescriptorSetLayout DSLglobal;
    DescriptorSetLayout DSLobj;

    // Pipelines
    Pipeline P1;

    // Models, Textures and Descriptors (values assigned to uniforms)
    int mCount = 3;

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

    // Camera
    // glm::mat3 CamDir = glm::mat3(1.0f);
    // glm::vec3 CamPos = glm::vec3(9.83158f, 14.6194f, -15.6189f);
    // glm::vec3 CamAng = glm::vec3(-0.300168f, 2.41573f, 0.0702695f);
    // Game
    // Game game;

    // application parameters
    void setWindowParameters()
    {
        // Window size, titile and initial background
        windowWidth = 1080;
        windowHeight = 1920;
        windowTitle = "Boat Runner";
        initialBackgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = mCount + 1;
        texturesInPool = mCount;
        setsInPool = mCount + 1;
    }

    // Load and setup of Vulkan objects
    void localInit()
    {

        // Descriptor Layouts [what will be passed to the shaders]
        DSLobj.init(this, {
            // this array contains the binding:
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
        P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj});

        // Models, textures and Descriptors (values assigned to the uniforms)
        M_Boat.init(this, MODEL_DIR + "/Boat.obj");
        T_Boat.init(this, TEXTURE_PATH + "/Boat.bmp");
        DS_Boat.init(this, &DSLobj, {// the second parameter, is a pointer to the Uniform Set Layout of this set
                                     // the last parameter is an array, with one element per binding of the set.
                                     // first  elmenet : the binding number
                                     // second element : UNIFORM or TEXTURE (an enum) depending on the type
                                     // third  element : only for UNIFORMs, the size of the corresponding C++ object
                                     // fourth element : only for TEXTUREs, the pointer to the corresponding texture object
                                     {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                     {1, TEXTURE, 0, &T_Boat}});

        M_Rock1.init(this, MODEL_DIR + "/Rock1.obj");
        T_Rock1.init(this, TEXTURE_PATH + "/Rock1.png");
        DS_Rock1.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Rock1}});

        M_Rock1.init(this, MODEL_DIR + "/Rock2.obj");
        T_Rock1.init(this, TEXTURE_PATH + "/Rock2.jpg");
        DS_Rock1.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Rock2}});

        DS_global.init(this, &DSLglobal, {{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}});
    }

    // Here you destroy all the objects you created!
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

        P1.cleanup();
        DSLglobal.cleanup();
        DSLobj.cleanup();
    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage)
    {

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.graphicsPipeline);

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
            0, nullptr);

        VkBuffer vertexBuffers1[] = {M_Boat.vertexBuffer};
        // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
        VkDeviceSize offsets1[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers1, offsets1);
        // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
        vkCmdBindIndexBuffer(commandBuffer, M_Boat.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        // property .pipelineLayout of a pipeline contains its layout.
        // property .descriptorSets of a descriptor set contains its elements.
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 0, 1, &DS_Boat.descriptorSets[currentImage],
            0, nullptr);

        // property .indices.size() of models, contains the number of triangles * 3 of the mesh.
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Boat.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffers2[] = {M_Rock1.vertexBuffer};
        VkDeviceSize offsets2[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
        vkCmdBindIndexBuffer(commandBuffer, M_Rock1.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 0, 1, &DS_Rock1.descriptorSets[currentImage],
            0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Rock1.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffers3[] = {M_Rock2.vertexBuffer};
        VkDeviceSize offsets3[] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers3, offsets3);
        vkCmdBindIndexBuffer(commandBuffer, M_Rock2.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            P1.pipelineLayout, 0, 1, &DS_Rock2.descriptorSets[currentImage],
            0, nullptr);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Rock2.indices.size()), 1, 0, 0, 0);
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        static float debounce = time;

        const float ROT_SPEED = glm::radians(1.0f);
        const float MOVE_SPEED = 0.01f;

        if (glfwGetKey(window, GLFW_KEY_LEFT))
        {
            CamAng.y += ROT_SPEED * 1.0f;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT))
        {
            CamAng.y -= ROT_SPEED * 1.0f;
        }

        if (glfwGetKey(window, GLFW_KEY_UP))
        {
            CamAng.x += ROT_SPEED * 1.0f;
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN))
        {
            CamAng.x -= ROT_SPEED * 1.0f;
        }

        if (glfwGetKey(window, GLFW_KEY_A))
        {
            CamPos -= MOVE_SPEED * glm::vec3(
                glm::rotate(glm::mat4(1.0f), CamAng.y,
                glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1));
        }
        if (glfwGetKey(window, GLFW_KEY_D))
        {
            CamPos += MOVE_SPEED * glm::vec3(
                glm::rotate(glm::mat4(1.0f), CamAng.y,
                glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1));
        }
        if (glfwGetKey(window, GLFW_KEY_S))
        {
            CamPos += MOVE_SPEED * glm::vec3(
                glm::rotate(glm::mat4(1.0f), CamAng.y,
                glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1));
        }
        if (glfwGetKey(window, GLFW_KEY_W))
        {
            CamPos -= MOVE_SPEED * glm::vec3(
                glm::rotate(glm::mat4(1.0f),
                CamAng.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1));
        }

        glm::mat3 CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.y, glm::vec3(0.0f, 1.0f, 0.0f))) *
                           glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.x, glm::vec3(1.0f, 0.0f, 0.0f))) *
                           glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.z, glm::vec3(0.0f, 0.0f, 1.0f)));

        UniformBufferObject ubo{};
        globalUniformBufferObject gubo{};

        void *data;

        gubo.view =
            glm::rotate(glm::mat4(1.0), -CamAng.z, glm::vec3(0, 0, 1)) *
            glm::rotate(glm::mat4(1.0), -CamAng.x, glm::vec3(1, 0, 0)) *
            glm::rotate(glm::mat4(1.0), -CamAng.y, glm::vec3(0, 1, 0)) *
            glm::translate(glm::mat4(1.0), glm::vec3(-CamPos.x, -CamPos.y, -CamPos.z));
        ;

        gubo.proj = glm::perspective(
            glm::radians(90.0f),
            swapChainExtent.width / (float)swapChainExtent.height,
            0.1f, 10.0f);
        
        gubo.proj[1][1] *= -1;

        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);
    }
};

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