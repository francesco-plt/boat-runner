// This has been adapted from the Vulkan tutorial

#include "museum_hpp.hpp"

/*const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";*/

// The uniform buffer object used in this example
struct globalUniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
};

// MAIN !
class MyProject : public BaseProject
{
protected:
    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSLglobal;
    DescriptorSetLayout DSLobj;

    // Pipelines [Shader couples]
    Pipeline P1;

    // Models, textures and Descriptors (values assigned to the uniforms)
    Model M_MuseumTri;
    Texture T_MuseumTri;
    DescriptorSet DS_MuseumTri; // instance DSLobj
    // floor
    Model M_Floor;
    Texture T_Floor;
    DescriptorSet DS_Floor; // instance DSLobj
    // painting 1
    Model M_Square;
    Texture T_Square;
    DescriptorSet DS_Square; // instance DSLobj
    // painting 2
    Model M_Square2;
    Texture T_Square2;
    DescriptorSet DS_Square2; // instance DSLobj
    // painting 3
    Model M_Square3;
    Texture T_Square3;
    DescriptorSet DS_Square3; // instance DSLobj
    // painting 4
    Model M_Square4;
    Texture T_Square4;
    DescriptorSet DS_Square4; // instance DSLobj
    // painting 5
    Model M_Square5;
    Texture T_Square5;
    DescriptorSet DS_Square5; // instance DSLobj
    // painting 6
    Model M_Square6;
    Texture T_Square6;
    DescriptorSet DS_Square6; // instance DSLobj
    // painting 7
    Model M_Square7;
    Texture T_Square7;
    DescriptorSet DS_Square7; // instance DSLobj
    // painting 8
    Model M_Square8;
    Texture T_Square8;
    DescriptorSet DS_Square8; // instance DSLobj
    // painting 9
    Model M_Square9;
    Texture T_Square9;
    DescriptorSet DS_Square9; // instance DSLobj
    // painting 10
    Model M_Square10;
    Texture T_Square10;
    DescriptorSet DS_Square10; // instance DSLobj
    // painting 11
    Model M_Square11;
    Texture T_Square11;
    DescriptorSet DS_Square11; // instance DSLobj

    Model M_Text;
    Texture T_Text;
    DescriptorSet DS_Text;

    Model M_Text2;
    Texture T_Text2;
    DescriptorSet DS_Text2;

    Model M_Text3;
    Texture T_Text3;
    DescriptorSet DS_Text3;

    Model M_Text4;
    Texture T_Text4;
    DescriptorSet DS_Text4;

    Model M_Text5;
    Texture T_Text5;
    DescriptorSet DS_Text5;

    Model M_Text6;
    Texture T_Text6;
    DescriptorSet DS_Text6;

    Model M_Text7;
    Texture T_Text7;
    DescriptorSet DS_Text7;

    Model M_Text8;
    Texture T_Text8;
    DescriptorSet DS_Text8;

    Model M_Text9;
    Texture T_Text9;
    DescriptorSet DS_Text9;

    Model M_Text10;
    Texture T_Text10;
    DescriptorSet DS_Text10;

    Model M_Text11;
    Texture T_Text11;
    DescriptorSet DS_Text11;

    Model M_Sculpture1;
    Texture T_Sculpture1;
    DescriptorSet DS_Sculpture1; // instance DSLobj

    DescriptorSet DS_global;

    // Here you set the main application parameters
    void setWindowParameters()
    {
        // window size, titile and initial background
        windowWidth = 2000;
        windowHeight = 1500;
        windowTitle = "Museum Project";
        initialBackgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = 26;
        texturesInPool = 25;
        setsInPool = 26;
    }

    // Here you load and setup all your Vulkan objects
    void localInit()
    {
        // Descriptor Layouts [what will be passed to the shaders]
        DSLobj.init(this, {// this array contains the binding:
                           // first  element : the binding number
                           // second element : the time of element (buffer or texture)
                           // third  element : the pipeline stage where it will be used
                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});

        DSLglobal.init(this, {
                                 {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                             });

        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj});

        // Models, textures and Descriptors (values assigned to the uniforms)
        M_MuseumTri.init(this, "models/museumTri.obj");
        T_MuseumTri.init(this, "textures/wall.jpg");
        DS_MuseumTri.init(this, &DSLobj, {// the second parameter, is a pointer to the Uniform Set Layout of this set
                                          // the last parameter is an array, with one element per binding of the set.
                                          // first  elmenet : the binding number
                                          // second element : UNIFORM or TEXTURE (an enum) depending on the type
                                          // third  element : only for UNIFORMs, the size of the corresponding C++ object
                                          // fourth element : only for TEXTUREs, the pointer to the corresponding texture object
                                          {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                          {1, TEXTURE, 0, &T_MuseumTri}});

        M_Floor.init(this, "models/floor.obj");
        T_Floor.init(this, "textures/parquet.jpg");
        DS_Floor.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Floor}});

        M_Square.init(this, "models/Square.obj");
        T_Square.init(this, "textures/VanGogh_self.jpg");
        DS_Square.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square}});

        M_Square2.init(this, "models/Square.obj");
        T_Square2.init(this, "textures/Manet_Dejeuner.jpg");
        DS_Square2.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square2}});

        M_Square3.init(this, "models/Square.obj");
        T_Square3.init(this, "textures/Matisse_theDance.jpg");
        DS_Square3.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square3}});

        M_Square4.init(this, "models/Square.obj");
        T_Square4.init(this, "textures/Monet-Sunrise.jpg");
        DS_Square4.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square4}});

        M_Square5.init(this, "models/Square.obj");
        T_Square5.init(this, "textures/Munch_Scream.jpg");
        DS_Square5.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square5}});

        M_Square6.init(this, "models/Rectangle.obj");
        T_Square6.init(this, "textures/Picasso_Guernica.jpg");
        DS_Square6.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square6}});

        M_Square7.init(this, "models/Square.obj");
        T_Square7.init(this, "textures/pisarro_boulevard_monmarte.jpg");
        DS_Square7.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square7}});

        M_Square8.init(this, "models/Square.obj");
        T_Square8.init(this, "textures/Seurat_a_sunday.png");
        DS_Square8.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square8}});

        M_Square9.init(this, "models/Square.obj");
        T_Square9.init(this, "textures/starringNight.jpg");
        DS_Square9.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square9}});

        M_Square10.init(this, "models/Square.obj");
        T_Square10.init(this, "textures/theBathers_Cezanne.jpg");
        DS_Square10.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square10}});

        M_Square11.init(this, "models/Square.obj");
        T_Square11.init(this, "textures/Volpedo_FourthEstate.jpg");
        DS_Square11.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Square11}});

        M_Text.init(this, "models/Rectangle.obj");
        T_Text.init(this, "textures/Picasso.png");
        DS_Text.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text}});

        M_Text2.init(this, "models/Rectangle.obj");
        T_Text2.init(this, "textures/Munch.png");
        DS_Text2.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text2}});

        M_Text3.init(this, "models/Rectangle.obj");
        T_Text3.init(this, "textures/Seurat.png");
        DS_Text3.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text3}});

        M_Text4.init(this, "models/Rectangle.obj");
        T_Text4.init(this, "textures/Cezanne.png");
        DS_Text4.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text4}});

        M_Text5.init(this, "models/Rectangle.obj");
        T_Text5.init(this, "textures/Manet.png");
        DS_Text5.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text5}});

        M_Text6.init(this, "models/Rectangle.obj");
        T_Text6.init(this, "textures/Matisse.png");
        DS_Text6.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text6}});

        M_Text7.init(this, "models/Rectangle.obj");
        T_Text7.init(this, "textures/Monet.png");
        DS_Text7.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text7}});

        M_Text8.init(this, "models/Rectangle.obj");
        T_Text8.init(this, "textures/selfvangogh.png");
        DS_Text8.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text8}});

        M_Text9.init(this, "models/Rectangle.obj");
        T_Text9.init(this, "textures/starry_vangogh.png");
        DS_Text9.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text9}});

        M_Text10.init(this, "models/Rectangle.obj");
        T_Text10.init(this, "textures/volpedo.png");
        DS_Text10.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text10}});

        M_Text11.init(this, "models/Rectangle.obj");
        T_Text11.init(this, "textures/Pisarro.png");
        DS_Text11.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Text11}});

        M_Sculpture1.init(this, "models/MOAI.obj");
        T_Sculpture1.init(this, "textures/Concrete_Roughness.jpg");
        DS_Sculpture1.init(this, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &T_Sculpture1}});

        DS_global.init(this, &DSLglobal, {
                                             {0, UNIFORM, sizeof(globalUniformBufferObject), nullptr},
                                         });
    }

    // Here you destroy all the objects you created!
    void localCleanup()
    {
        DS_MuseumTri.cleanup();
        T_MuseumTri.cleanup();
        M_MuseumTri.cleanup();

        DS_Floor.cleanup();
        T_Floor.cleanup();
        M_Floor.cleanup();

        DS_Square.cleanup();
        T_Square.cleanup();
        M_Square.cleanup();

        DS_Square2.cleanup();
        T_Square2.cleanup();
        M_Square2.cleanup();

        DS_Square3.cleanup();
        T_Square3.cleanup();
        M_Square3.cleanup();

        DS_Square4.cleanup();
        T_Square4.cleanup();
        M_Square4.cleanup();

        DS_Square5.cleanup();
        T_Square5.cleanup();
        M_Square5.cleanup();

        DS_Square6.cleanup();
        T_Square6.cleanup();
        M_Square6.cleanup();

        DS_Square7.cleanup();
        T_Square7.cleanup();
        M_Square7.cleanup();

        DS_Square8.cleanup();
        T_Square8.cleanup();
        M_Square8.cleanup();

        DS_Square9.cleanup();
        T_Square9.cleanup();
        M_Square9.cleanup();

        DS_Square10.cleanup();
        T_Square10.cleanup();
        M_Square10.cleanup();

        DS_Square11.cleanup();
        T_Square11.cleanup();
        M_Square11.cleanup();

        DS_Text.cleanup();
        T_Text.cleanup();
        M_Text.cleanup();

        DS_Text2.cleanup();
        T_Text2.cleanup();
        M_Text2.cleanup();

        DS_Text3.cleanup();
        T_Text3.cleanup();
        M_Text3.cleanup();

        DS_Text4.cleanup();
        T_Text4.cleanup();
        M_Text4.cleanup();

        DS_Text5.cleanup();
        T_Text5.cleanup();
        M_Text5.cleanup();

        DS_Text6.cleanup();
        T_Text6.cleanup();
        M_Text6.cleanup();

        DS_Text7.cleanup();
        T_Text7.cleanup();
        M_Text7.cleanup();

        DS_Text8.cleanup();
        T_Text8.cleanup();
        M_Text8.cleanup();

        DS_Text9.cleanup();
        T_Text9.cleanup();
        M_Text9.cleanup();

        DS_Text10.cleanup();
        T_Text10.cleanup();
        M_Text10.cleanup();

        DS_Text11.cleanup();
        T_Text11.cleanup();
        M_Text11.cleanup();

        M_Sculpture1.cleanup();
        T_Sculpture1.cleanup();
        DS_Sculpture1.cleanup();

        DS_global.cleanup();

        P1.cleanup();
        DSLglobal.cleanup();
        DSLobj.cleanup();
    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage)
    {

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          P1.graphicsPipeline);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
                                0, nullptr);

        VkBuffer vertexBuffersMTri[] = {M_MuseumTri.vertexBuffer};
        // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
        VkDeviceSize offsetsMTri[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersMTri, offsetsMTri);
        // property .indexBuffer of models, contains the VkBuffer handle to its index buffer
        vkCmdBindIndexBuffer(commandBuffer, M_MuseumTri.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);

        // property .pipelineLayout of a pipeline contains its layout.
        // property .descriptorSets of a descriptor set contains its elements.
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_MuseumTri.descriptorSets[currentImage],
                                0, nullptr);

        // property .indices.size() of models, contains the number of triangles * 3 of the mesh.
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_MuseumTri.indices.size()), 1, 0, 0, 0);

        // Draw commands for the floor
        VkBuffer vertexBuffersFloor[] = {M_Floor.vertexBuffer};
        VkDeviceSize offsetsFloor[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersFloor, offsetsFloor);
        vkCmdBindIndexBuffer(commandBuffer, M_Floor.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Floor.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Floor.indices.size()), 1, 0, 0, 0);

        // Draw commands for the paintings
        VkBuffer vertexBuffersSquare[] = {M_Square.vertexBuffer};
        VkDeviceSize offsetsSquare[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare, offsetsSquare);
        vkCmdBindIndexBuffer(commandBuffer, M_Square.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare2[] = {M_Square2.vertexBuffer};
        VkDeviceSize offsetsSquare2[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare2, offsetsSquare2);
        vkCmdBindIndexBuffer(commandBuffer, M_Square2.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square2.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square2.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare3[] = {M_Square3.vertexBuffer};
        VkDeviceSize offsetsSquare3[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare3, offsetsSquare3);
        vkCmdBindIndexBuffer(commandBuffer, M_Square3.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square3.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square3.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare4[] = {M_Square4.vertexBuffer};
        VkDeviceSize offsetsSquare4[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare4, offsetsSquare4);
        vkCmdBindIndexBuffer(commandBuffer, M_Square4.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square4.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square4.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare5[] = {M_Square5.vertexBuffer};
        VkDeviceSize offsetsSquare5[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare5, offsetsSquare5);
        vkCmdBindIndexBuffer(commandBuffer, M_Square5.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square5.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square5.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare6[] = {M_Square6.vertexBuffer};
        VkDeviceSize offsetsSquare6[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare6, offsetsSquare6);
        vkCmdBindIndexBuffer(commandBuffer, M_Square6.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square6.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square6.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare7[] = {M_Square7.vertexBuffer};
        VkDeviceSize offsetsSquare7[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare7, offsetsSquare7);
        vkCmdBindIndexBuffer(commandBuffer, M_Square7.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square7.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square7.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare8[] = {M_Square8.vertexBuffer};
        VkDeviceSize offsetsSquare8[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare8, offsetsSquare8);
        vkCmdBindIndexBuffer(commandBuffer, M_Square8.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square8.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square8.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare9[] = {M_Square9.vertexBuffer};
        VkDeviceSize offsetsSquare9[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare9, offsetsSquare9);
        vkCmdBindIndexBuffer(commandBuffer, M_Square9.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square9.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square9.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare10[] = {M_Square10.vertexBuffer};
        VkDeviceSize offsetsSquare10[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare10, offsetsSquare10);
        vkCmdBindIndexBuffer(commandBuffer, M_Square10.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square10.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square10.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSquare11[] = {M_Square11.vertexBuffer};
        VkDeviceSize offsetsSquare11[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSquare11, offsetsSquare11);
        vkCmdBindIndexBuffer(commandBuffer, M_Square11.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Square11.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Square11.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText[] = {M_Text.vertexBuffer};
        VkDeviceSize offsetsText[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText, offsetsText);
        vkCmdBindIndexBuffer(commandBuffer, M_Text.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText2[] = {M_Text2.vertexBuffer};
        VkDeviceSize offsetsText2[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText2, offsetsText2);
        vkCmdBindIndexBuffer(commandBuffer, M_Text2.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text2.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text2.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText3[] = {M_Text3.vertexBuffer};
        VkDeviceSize offsetsText3[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText3, offsetsText3);
        vkCmdBindIndexBuffer(commandBuffer, M_Text3.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text3.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text3.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText4[] = {M_Text4.vertexBuffer};
        VkDeviceSize offsetsText4[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText4, offsetsText4);
        vkCmdBindIndexBuffer(commandBuffer, M_Text4.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text4.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text4.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText5[] = {M_Text5.vertexBuffer};
        VkDeviceSize offsetsText5[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText5, offsetsText5);
        vkCmdBindIndexBuffer(commandBuffer, M_Text5.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text5.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text5.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText6[] = {M_Text6.vertexBuffer};
        VkDeviceSize offsetsText6[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText6, offsetsText6);
        vkCmdBindIndexBuffer(commandBuffer, M_Text6.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text6.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text6.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText7[] = {M_Text7.vertexBuffer};
        VkDeviceSize offsetsText7[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText7, offsetsText7);
        vkCmdBindIndexBuffer(commandBuffer, M_Text7.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text7.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text7.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText8[] = {M_Text8.vertexBuffer};
        VkDeviceSize offsetsText8[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText8, offsetsText8);
        vkCmdBindIndexBuffer(commandBuffer, M_Text8.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text8.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text8.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText9[] = {M_Text9.vertexBuffer};
        VkDeviceSize offsetsText9[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText9, offsetsText9);
        vkCmdBindIndexBuffer(commandBuffer, M_Text9.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text9.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text9.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText10[] = {M_Text10.vertexBuffer};
        VkDeviceSize offsetsText10[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText10, offsetsText10);
        vkCmdBindIndexBuffer(commandBuffer, M_Text10.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text10.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text10.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersText11[] = {M_Text11.vertexBuffer};
        VkDeviceSize offsetsText11[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersText11, offsetsText11);
        vkCmdBindIndexBuffer(commandBuffer, M_Text11.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Text11.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Text11.indices.size()), 1, 0, 0, 0);

        VkBuffer vertexBuffersSculpture1[] = {M_Sculpture1.vertexBuffer};
        VkDeviceSize offsetsSculpture1[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSculpture1, offsetsSculpture1);
        vkCmdBindIndexBuffer(commandBuffer, M_Sculpture1.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Sculpture1.descriptorSets[currentImage],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Sculpture1.indices.size()), 1, 0, 0, 0);
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
            CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
                                                         glm::vec3(0.0f, 1.0f, 0.0f)) *
                                             glm::vec4(1, 0, 0, 1));
        }
        if (glfwGetKey(window, GLFW_KEY_D))
        {
            CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
                                                         glm::vec3(0.0f, 1.0f, 0.0f)) *
                                             glm::vec4(1, 0, 0, 1));
        }
        if (glfwGetKey(window, GLFW_KEY_S))
        {
            CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
                                                         glm::vec3(0.0f, 1.0f, 0.0f)) *
                                             glm::vec4(0, 0, 1, 1));
        }
        if (glfwGetKey(window, GLFW_KEY_W))
        {
            CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
                                                         glm::vec3(0.0f, 1.0f, 0.0f)) *
                                             glm::vec4(0, 0, 1, 1));
        }

        static int state = 0; // 0 - no text
                              // 1 - text about art appears

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

        gubo.proj = glm::perspective(glm::radians(90.0f),
                                     swapChainExtent.width / (float)swapChainExtent.height,
                                     0.1f, 10.0f);
        gubo.proj[1][1] *= -1;

        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

        // For the museum walls:
        ubo.model = glm::mat4(1.0f);
        vkMapMemory(device, DS_MuseumTri.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_MuseumTri.uniformBuffersMemory[0][currentImage]);

        // For the museum floor:
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.04422f, 0.0f));
        vkMapMemory(device, DS_Floor.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Floor.uniformBuffersMemory[0][currentImage]);

        // For the museum paintings:
        // VanGogh
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.2f, 1.9f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square.uniformBuffersMemory[0][currentImage]);
        // VanGogh text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.4f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text8.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text8.uniformBuffersMemory[0][currentImage]);

        // Manet
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.13f, 1.9f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square2.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square2.uniformBuffersMemory[0][currentImage]);
        // Manet text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.3f, 1.4f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text5.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text5.uniformBuffersMemory[0][currentImage]);

        // Matisse
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.3f, 1.9f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square3.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square3.uniformBuffersMemory[0][currentImage]);
        // Matisse text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.1f, 1.4f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text6.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text6.uniformBuffersMemory[0][currentImage]);

        // Monet
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.1f, 1.9f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square4.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square4.uniformBuffersMemory[0][currentImage]);
        // Monet text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.9f, 1.4f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text7.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text7.uniformBuffersMemory[0][currentImage]);

        // Munch
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.4f, 1.9f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square5.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square5.uniformBuffersMemory[0][currentImage]);
        // Munch text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.6f, 1.4f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text2.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text2.uniformBuffersMemory[0][currentImage]);
        // Picasso
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.15f, 1.9f, -0.85f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.5f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square6.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square6.uniformBuffersMemory[0][currentImage]);
        // Picasso text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.41f, 1.23f, -0.85f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text.uniformBuffersMemory[0][currentImage]);
        // Pisarro
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.1f, 1.9f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square7.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square7.uniformBuffersMemory[0][currentImage]);
        // Pisarro text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.3f, 1.4f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text11.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text11.uniformBuffersMemory[0][currentImage]);

        // Seurat
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.3f, 1.9f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square8.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square8.uniformBuffersMemory[0][currentImage]);
        // Seurat text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.5f, 1.4f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text3.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text3.uniformBuffersMemory[0][currentImage]);

        // VanGogh-night
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2f, 1.9f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square9.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square9.uniformBuffersMemory[0][currentImage]);
        // VanGogh-night text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.4f, -0.86f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text9.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text9.uniformBuffersMemory[0][currentImage]);

        // Cezanne
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.3f, 1.9f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.3f));
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square10.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square10.uniformBuffersMemory[0][currentImage]);
        // Cezanne text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 1.4f, 3.099f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text4.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text4.uniformBuffersMemory[0][currentImage]);

        // Volpedo
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, 2.2f, 3.08f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::scale(ubo.model, glm::vec3(0.5f));
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Square11.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Square11.uniformBuffersMemory[0][currentImage]);
        // Volpedo text
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, 1.5f, 3.08f));
        ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (state == 0)
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.0f));
        }
        else
        {
            ubo.model = glm::scale(ubo.model, glm::vec3(0.1f));
        }
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vkMapMemory(device, DS_Text10.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Text10.uniformBuffersMemory[0][currentImage]);

        // MOAI Sculpture
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.05f, 0.6f));
        ubo.model = glm::scale(ubo.model, glm::vec3(2.5f));
        vkMapMemory(device, DS_Sculpture1.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Sculpture1.uniformBuffersMemory[0][currentImage]);
    }
};

// This is the main: probably you do not need to touch this!
int main()
{
    MyProject app;

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