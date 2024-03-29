using namespace std;
#include "BoatRunner.hpp"

#if defined(_WIN64)
#define boatSpeed 0.3f
#define rockSpeed 0.4f
#define maxAcceleration 3.0f
#else
#define boatSpeed 0.35f
#define rockSpeed 0.7f
#define maxAcceleration 3.5f
#endif

#define ESC "\033[;"
#define RED "31m"
#define GREEN "32m"
#define YELLOW "33m"
#define BLUE "34m"
#define PURPLE "35m"
#define RESET "\033[m"

static const int minRockNum = 10;
static const int maxRockNum = 15;

static const glm::mat4 I = glm::mat4(1.1f); // Identity matrix
static const glm::vec3 origin = glm::vec3(0, 0, 0);
static const glm::vec3 xAxis = glm::vec3(1, 0, 0); // x axis
static const glm::vec3 yAxis = glm::vec3(0, 1, 0); // y axis
static const glm::vec3 zAxis = glm::vec3(0, 0, 1); // z axis

static const glm::vec3 sbScalingFactor = glm::vec3(12.0f);
static const glm::vec3 boatScalingFactor = glm::vec3(0.3f);
static const glm::vec3 oceanScalingFactor = glm::vec3(50.0f);
static const glm::vec3 skyScalingFactor = glm::vec3(30.0f);
static const float minRockScalingFactor = 0.25f;
static const float maxRockScalingFactor = 0.35f;
static const float boatMotionDisplacement = 0.05f;

static const float FoV = glm::radians(60.0f);
static const float nearPlane = 0.1f;
static const float farPlane = 100.0f;
static const float maxDepth = -10.0f;
static const float leftBound = 10.0f;
static const float rightBound = -10.0f;
static const float forwardBound = 4.0f;
static const float backwardBound = -2.0f;
static const float rockGenDelta = farPlane * 2;
static const float boatWidth = 2.5f;
static const float boatLength = 4.5f;
static const float boatHeight = 1.5f;
static const float rockHeight = 1.0f;
static const float rock1Width = 2.5f;
static const float rock2Width = 1.8f;
static const float rock1Length = 2.5f;
static const float rock2Length = 1.8f;

static const glm::vec3 camDelta = glm::vec3(0.0f, 1.5f, 0.0f);
static const glm::vec3 camPosDisplacement = glm::vec3(0.0f, 2.5f, -4.0f);
static const glm::vec3 initialBoatPosition = origin;
static const glm::vec3 initialOceanPosition = origin;
static const glm::vec3 initialCameraPosition = initialBoatPosition + camPosDisplacement;

enum gameState
{
    PLAY,
    GAME_OVER
};

enum gameDifficulty
{
    EASY,
    HARD
};

struct globalUniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
};

struct SkyBoxUniformBufferObject
{
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

struct SkyBoxData
{
    Pipeline P;
    DescriptorSetLayout DSL;
    DescriptorSetSkyBox DS;
};

class Ocean
{
protected:
    Model model;
    Texture texture;
    DescriptorSet DS;

public:
    void init(BaseProject *br, DescriptorSetLayout DSLobj)
    {
        model.init(br, MODEL_PATH + "/Ocean.obj");
        texture.init(br, TEXTURE_PATH + "/Ocean.png");
        DS.init(br, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &texture}});
    }

    void cleanup()
    {
        model.cleanup();
        texture.cleanup();
        DS.cleanup();
    }

    Model getModel()
    {
        return model;
    }

    DescriptorSet getDS()
    {
        return DS;
    }
};

class Boat
{
protected:
    Model model;
    Texture texture;
    DescriptorSet DS;
    glm::vec3 pos;
    float height;
    float width;
    float speedFactor;

public:
    void init(BaseProject *br, DescriptorSetLayout DSLobj)
    {
        model.init(br, MODEL_PATH + "/Boat.obj");
        texture.init(br, TEXTURE_PATH + "/Boat.bmp");
        DS.init(br, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &texture}});

        speedFactor = boatSpeed;
        height = boatHeight;
        width = boatWidth;
        reset();
        std::cout << ESC << GREEN << "Boat initialized" << RESET << std::endl;
    }

    void reset()
    {
        pos = initialBoatPosition;
    }

    void cleanup()
    {
        model.cleanup();
        texture.cleanup();
        DS.cleanup();
    }

    void moveLeft()
    {
        // z because we need to take into account boat rotation
        pos.x += speedFactor;
    }

    void moveRight()
    {
        pos.x -= speedFactor;
    }

    void moveForward()
    {
        pos.z += speedFactor * 0.33f;
    }

    void moveBackward()
    {
        pos.z -= speedFactor * 0.33f;
    }

    void jump()
    {
        pos.y += speedFactor * 0.66f;
    }

    void fall()
    {
        pos.y -= speedFactor * 0.66f;
    }

    Model getModel()
    {
        return model;
    }

    DescriptorSet getDS()
    {
        return DS;
    }

    float getHeight()
    {
        return height;
    }

    float getWidth()
    {
        return width;
    }

    glm::vec3 getPos()
    {
        return pos;
    }

    void setPos(glm::vec3 newPos)
    {
        pos = newPos;
    }

    void printPos()
    {
        printf("Boat Position: (%.1f, %.1f, %.1f)\n", pos.x, pos.y, pos.z);
    }
};

class Rock
{
protected:
    DescriptorSet DS;
    int id;
    int type;
    float speedFactor;
    glm::vec3 scalingFactor;
    glm::vec3 pos;
    float height;
    float width;
    float rotationFactor;

public:
    void init(BaseProject *br, DescriptorSetLayout DSLobj, int newId, int newType, Model newModel, Texture newTexture)
    {
        DS.init(br, &DSLobj, {{0, UNIFORM, sizeof(UniformBufferObject), nullptr}, {1, TEXTURE, 0, &newTexture}});

        id = newId;
        type = newType;
        scalingFactor = glm::vec3(glm::linearRand(minRockScalingFactor, maxRockScalingFactor));
        speedFactor = rockSpeed;
        reset();
        std::cout << ESC << GREEN << "Rock " << id << " [type: " << type << "] initialized" << RESET << std::endl;
    }

    void reset()
    {
        // Randomly generated position according to a normal distribution
        pos = glm::vec3(glm::linearRand(rightBound, leftBound), 0, glm::linearRand(farPlane, farPlane + rockGenDelta));
        // same for rotation
        rotationFactor = glm::linearRand(0.0f, 360.0f);
    }

    void cleanup()
    {
        DS.cleanup();
    }

    void moveForward(float accelerationFactor)
    {
        pos.z -= speedFactor + accelerationFactor;
    }

    DescriptorSet getDS()
    {
        return DS;
    }

    float getHeight()
    {
        return height;
    }

    float getWidth()
    {
        return width;
    }

    glm::vec3 getPos()
    {
        return pos;
    }

    float getRot()
    {
        return rotationFactor;
    }

    int getId()
    {
        return id;
    }

    glm::vec3 getScalingFactor()
    {
        return scalingFactor;
    }

    int getType()
    {
        return type;
    }

    float getSpeedFactor()
    {
        return speedFactor;
    }

    void printPos()
    {
        printf("Rock %d Position: (%.1f, %.1f, %.1f)\n", id, pos.x, pos.y, pos.z);
    }
};

class BoatRunner : public BaseProject
{
protected:
    // Here you list all the Vulkan objects you need:

    // Descriptor Layouts [what will be passed to the shaders]
    DescriptorSetLayout DSLglobal;
    DescriptorSetLayout DSLobj;

    // Pipelines [Shader couples]
    Pipeline P1;

    // Descriptors, models and textures (values assigned to the uniforms)

    DescriptorSet DS_global;
    SkyBoxData skybox;
    Ocean ocean;
    Boat boat;

    int rockCount;
    Model rockModels[2];
    Texture rockTextures[2];
    vector<Rock> rocks;

    glm::vec3 cameraPosition;
    /*	debugging purposes
     *	glm::vec3 oldBoatPos = initialBoatPosition; */

    // game logic related variables
    float score;
    float highScore;
    float accFactor;
    gameState state;
    gameDifficulty difficulty;

    // application parameters:
    // Window size, titile and initial background
    void setWindowParameters()
    {
        // 4:3 aspect ratio
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "Boat Runner";
        initialBackgroundColor = {0.0f, 0.5f, 0.8f, 1.0f};

        /* Number of rocks is generated casually here
         *  according to a normal distribution, the number
         *  of rocks determines the difficulty of the game
         */
        rockCount = rand() % (maxRockNum - minRockNum + 1) + minRockNum;

        // Descriptor pool sizes
        uniformBlocksInPool = rockCount + 4; // ocean, skybox, boat, rocks
        texturesInPool = rockCount + 3;
        setsInPool = rockCount + 4;
    }

    // Here you load and setup all your Vulkan objects
    void localInit()
    {
        // Descriptor Layouts [what will be passed to the shaders]
        // Objects DescriptorSetLayout
        DSLobj.init(this, {// this array contains the binding:
                           // first  element : the binding number
                           // second element : the time of element (buffer or texture)
                           // third  element : the pipeline stage where it will be used
                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});
        // Global (camera) DescriptorSetLayout
        DSLglobal.init(this, {
                                 {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                             });

        // P1: global pipeline, used for each object with the only exception of the skybox
        // Pipelines [Shader couples]
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P1.init(this, VERTEX_SHADER, FRAGMENT_SHADER, {&DSLglobal, &DSLobj}, VK_COMPARE_OP_LESS_OR_EQUAL);

        // Models, textures and Descriptors (values assigned to the uniforms)

        // Skybox DescriptorSetLayout, Pipeline and DescriptorSet
        skybox.DSL.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
                               {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});
        skybox.P.init(this, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", {&skybox.DSL}, VK_COMPARE_OP_LESS_OR_EQUAL);
        skybox.DS.init(this, &skybox.DSL, {{0, UNIFORM, sizeof(SkyBoxUniformBufferObject), nullptr}, {1, TEXTURE, 0, &(SkyBox.TD)}});

        // Ocean Model, Texture and DescriptorSet
        ocean.init(this, DSLobj);
        // Same for boat
        boat.init(this, DSLobj);

        // As for rocks we just initialize the models and textures
        rockModels[0].init(this, MODEL_PATH + ROCK_MODELS_PATH[0]);
        rockModels[1].init(this, MODEL_PATH + ROCK_MODELS_PATH[1]);
        rockTextures[0].init(this, TEXTURE_PATH + ROCK_TEXTURES_PATH[0]);
        rockTextures[1].init(this, TEXTURE_PATH + ROCK_TEXTURES_PATH[1]);

        // and then a DescriptorSet for each rock we want to render
        int rockSelection;
        for (int i = 0; i < rockCount; i++)
        {
            rockSelection = rand() % 2;
            Rock rock;
            rock.init(this, DSLobj, i, rockSelection, rockModels[rockSelection], rockTextures[rockSelection]);
            rocks.push_back(rock);
        }

        // Global DescriptorSet, for camera
        DS_global.init(this, &DSLglobal, {{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}});

        // game logic related code
        highScore = (float)readScore("highscore.dat"); // reading old highscore from file
        // detecting game difficulty basing on the casually generated number of rocks
        string difficultyString;
        std::cout << "rocks count: " << rockCount << std::endl;
        if (rockCount <= (maxRockNum - minRockNum) / 2)
        {
            difficulty = EASY;
            difficultyString = "Easy";
        }
        else
        {
            difficulty = HARD;
            difficultyString = "Hard";
        }
        std::cout << "BoatRunner initialized..." << std::endl;
        std::cout << "GAME OUTPUT ["
                  << "Highest score: " << highScore << " / Difficulty: " << difficultyString << "]:" << std::endl;
    }

    // Here you destroy all the objects you created!
    void localCleanup()
    {
        // clean stdout
        std::cout << "\n"
                  << std::endl;

        skybox.DS.cleanup();
        skybox.P.cleanup();
        skybox.DSL.cleanup();

        ocean.cleanup();
        boat.cleanup();

        rockModels[0].cleanup();
        rockModels[1].cleanup();

        rockTextures[0].cleanup();
        rockTextures[1].cleanup();

        for (auto &r : rocks)
        {
            r.cleanup();
        }

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
        // Skybox
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skybox.P.graphicsPipeline);

        VkBuffer SBVertexBuffers[] = {SkyBox.MD.vertexBuffer};
        VkDeviceSize SBOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, SBVertexBuffers, SBOffsets);
        vkCmdBindIndexBuffer(commandBuffer, SkyBox.MD.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                skybox.P.pipelineLayout, 0, 1, &skybox.DS.descriptorSets[currentImage],
                                0, nullptr);

        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(SkyBox.MD.indices.size()), 1, 0, 0, 0);

        // Global Pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.graphicsPipeline);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
                                0, nullptr);

        // Ocean
        VkBuffer oceanVertexBuffers[] = {ocean.getModel().vertexBuffer};
        VkDeviceSize oceanOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, oceanVertexBuffers, oceanOffsets);

        vkCmdBindIndexBuffer(commandBuffer, ocean.getModel().indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &ocean.getDS().descriptorSets[currentImage],
                                0, nullptr);

        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(ocean.getModel().indices.size()), 1, 0, 0, 0);

        // Boat
        VkBuffer boatVertexBuffers[] = {boat.getModel().vertexBuffer};
        // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
        VkDeviceSize boatOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, boatVertexBuffers, boatOffsets);

        vkCmdBindIndexBuffer(commandBuffer, boat.getModel().indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &boat.getDS().descriptorSets[currentImage],
                                0, nullptr);

        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(boat.getModel().indices.size()), 1, 0, 0, 0);

        // Rocks

        // type 1
        VkBuffer rockType1VertexBuffers[] = {rockModels[0].vertexBuffer};
        VkDeviceSize rockType1offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, rockType1VertexBuffers, rockType1offsets);
        vkCmdBindIndexBuffer(commandBuffer, rockModels[0].indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        for (auto &r : rocks)
        {
            if (r.getType() != 0)
            {
                continue;
            }
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    P1.pipelineLayout, 1, 1, &r.getDS().descriptorSets[currentImage],
                                    0, nullptr);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(rockModels[0].indices.size()), 1, 0, 0, 0);
        }

        // type 2
        VkBuffer rockType2VertexBuffers[] = {rockModels[1].vertexBuffer};
        VkDeviceSize rockType2offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, rockType2VertexBuffers, rockType2offsets);
        vkCmdBindIndexBuffer(commandBuffer, rockModels[1].indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        for (auto &r : rocks)
        {
            if (r.getType() != 1)
            {
                continue;
            }
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    P1.pipelineLayout, 1, 1, &r.getDS().descriptorSets[currentImage],
                                    0, nullptr);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(rockModels[1].indices.size()), 1, 0, 0, 0);
        }
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        static float lastTime = 0.0f;

        if (lastTime == 0.0f)
        {
            printMenu();
            initGame();
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float deltaT = time - lastTime;
        lastTime = time;

        // progressive acceleration with max threeshold
        if (accFactor >= maxAcceleration)
        {
            accFactor = maxAcceleration;
        }
        else
        {
            accFactor += log10(log10(lastTime / 10000 + 10));
        }

        updatePosition(accFactor, time);
        if (state == PLAY)
        {
            score += deltaT;
            // Updating score in console while running
            std::cout << ESC << PURPLE << "score: " << score << RESET << '\r' << std::flush;

            // object position checking
            checkBoatBoundaries();
            detectCollisions();
        }

        SkyBoxUniformBufferObject subo{};
        globalUniformBufferObject gubo{};
        UniformBufferObject ubo{};

        void *data;

        gubo.view = glm::lookAt(scaleVector(boat.getPos(), boatMotionDisplacement) + camPosDisplacement, scaleVector(boat.getPos(), boatMotionDisplacement) + camDelta, yAxis);
        gubo.proj = glm::perspective(FoV, swapChainExtent.width / (float)swapChainExtent.height, nearPlane, farPlane);
        gubo.proj[1][1] *= -1;

        // First we draw the skybox, then the camera position
        subo.mMat = I;
        subo.nMat = I;
        subo.mvpMat = gubo.proj * glm::lookAt(scaleVector(initialBoatPosition, 0.1f) + camPosDisplacement, scaleVector(initialBoatPosition, 0.1f) + camDelta, yAxis);
        subo.mvpMat = glm::scale(subo.mvpMat, sbScalingFactor);

        vkMapMemory(device, skybox.DS.uniformBuffersMemory[0][currentImage], 0, sizeof(subo), 0, &data);
        memcpy(data, &subo, sizeof(subo));
        vkUnmapMemory(device, skybox.DS.uniformBuffersMemory[0][currentImage]);

        // Now we can proceed with the camera position
        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

        // Boat
        ubo.model = I;
        ubo.model = glm::scale(ubo.model, boatScalingFactor);                   // scale the model
        ubo.model = glm::rotate(ubo.model, glm::radians(sin(2 * time)), xAxis); // boat oscillation
        ubo.model = glm::rotate(ubo.model, glm::radians(sin(2 * time)), zAxis); // ocean oscillation
        ubo.model = glm::translate(ubo.model, boat.getPos());                   // translating boat according to players input
        ubo.model = glm::translate(ubo.model, glm::vec3(0, -0.8f, 0));          // translating the boat down in the water

        vkMapMemory(device, boat.getDS().uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, boat.getDS().uniformBuffersMemory[0][currentImage]);

        // Ocean
        ubo.model = I;
        ubo.model = glm::scale(ubo.model, oceanScalingFactor);                     // scale the model
        ubo.model = glm::rotate(ubo.model, glm::radians(0.5f * sin(time)), zAxis); // ocean oscillation
        ubo.model = glm::translate(ubo.model, glm::vec3(0, -0.005f, 0));           // translating the ocean down so that it is always under the boat
        ubo.model = glm::scale(ubo.model, glm::vec3(1, 0.5f, 1));                  // making it shorter in height so that it doesn't cover the boat

        vkMapMemory(device, ocean.getDS().uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, ocean.getDS().uniformBuffersMemory[0][currentImage]);

        // Rocks
        for (auto &r : rocks)
        {
            ubo.model = I;
            ubo.model = glm::scale(ubo.model, r.getScalingFactor()); // randomly generated size accourding to a normal distribution
            ubo.model = glm::translate(ubo.model, r.getPos());       // adjusting position according to game logic
            ubo.model = glm::rotate(ubo.model, r.getRot(), yAxis);   // randomly generated rotation accourding to a normal distribution

            vkMapMemory(device, r.getDS().uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, r.getDS().uniformBuffersMemory[0][currentImage]);
        }
    }

    // Here we handle object motion
    void updatePosition(float accelerationFactor, float time)
    {
        static bool isJumping = false;
        static float jumpTime;

        // game restart logic
        if (glfwGetKey(window, GLFW_KEY_R) && state == GAME_OVER)
        {
            std::cout << "Restarting the game..." << std::endl;
            initGame();
        }

        if (state == GAME_OVER)
        {
            return;
        }

        for (auto &r : rocks)
        {
            // Speed increases with time (up to a certain limit given by maxAcceleration)
            r.moveForward(accelerationFactor);
            // When rocks get behind the boat they are moved
            // again in the field of view far from the boat
            // to give the illusion of an infinite amount of rocks
            if (r.getPos().z < maxDepth)
            {
                r.reset();
            }
        }

        // Boat motion
        if (glfwGetKey(window, GLFW_KEY_A))
        {
            boat.moveLeft();
        }
        if (glfwGetKey(window, GLFW_KEY_D))
        {
            boat.moveRight();
        }
        if (glfwGetKey(window, GLFW_KEY_W))
        {
            boat.moveForward();
        }
        if (glfwGetKey(window, GLFW_KEY_S))
        {
            boat.moveBackward();
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE))
        {
            if (!isJumping)
            {
                isJumping = true;
                jumpTime = time;
            }
        }
        if (isJumping)
        {
            if (time - jumpTime < 0.2f)
            {
                boat.jump();
            }
            else if ((time - jumpTime) > 0.25f && (time - jumpTime) < 0.45f)
            {
                boat.fall();
            }
            else if (time - jumpTime > 0.45f)
            {
                boat.setPos(glm::vec3(boat.getPos().x, 0.0f, boat.getPos().z));
                isJumping = false;
            }
        }
    }

    // check for collisions beteween boat and rocks
    // if yes, game is stopped and every object goes back
    // to its original position thanks to the initGame function
    void detectCollisions()
    {
        /* debugging purposes
         * if(!glm::all(glm::equal(boat.getPos(), oldBoatPos))) {
         * 	boat.printPos();
         * 	oldBoatPos = boat.getPos();
         * } */

        for (auto &r : rocks)
        {
            /* debugging purposes
             * if(r.getType() == 0) {
             * 	r.printPos();
             * } */

            // if some rocks ends up with the same inside the area
            // defined by the boat coordinates plus some margin
            // then we have a collision
            if (r.getPos().x <= boat.getPos().x + boatLength / 2 && r.getPos().x >= boat.getPos().x - boatLength / 2)
            {
                if (r.getPos().z <= boat.getPos().z + boatWidth / 2 && r.getPos().z >= boat.getPos().z - boatWidth / 2)
                {
                    if (rockHeight >= boat.getPos().y - boatHeight / 3)
                    {
                        /* debugging purposes
                         * printf("Collided in (%.1f, %.1f, %.1f) with rock %d. Boat position: (%.1f, %.1f, %.1f).\n", r.getPos().x, r.getPos().y, r.getPos().z, r.getId(), boat.getPos().x, boat.getPos().y, boat.getPos().z); */
                        std::cout << ESC << RED << "Final score: " << score << RESET << std::endl;
                        std::cout << "Press R to restart." << std::endl;

                        // setting high score if necessary
                        if (score > highScore)
                        {
                            highScore = score;
                            std::cout << ESC << GREEN << "New High Score: " << highScore << RESET << std::endl;

                            if (writeScore("highscore.dat", highScore))
                            {
                                std::cout << "High Score written to file." << std::endl;
                            }
                            else
                            {
                                std::cout << "Failed to write High Score to file." << std::endl;
                            }
                        }
                        // here we just change the state of the game,
                        // the actual reset of initial game conditions
                        // is done by the initGame function called by updatePosition
                        state = GAME_OVER;
                    }
                }
            }
        }
    }

    // checking that boat position stays between a given range
    void checkBoatBoundaries()
    {
        if (boat.getPos().x > leftBound)
        {
            boat.setPos(glm::vec3(leftBound, boat.getPos().y, boat.getPos().z));
        }
        if (boat.getPos().x < rightBound)
        {
            boat.setPos(glm::vec3(rightBound, boat.getPos().y, boat.getPos().z));
        }
        if (boat.getPos().z < backwardBound)
        {
            boat.setPos(glm::vec3(boat.getPos().x, boat.getPos().y, backwardBound));
        }
        if (boat.getPos().z > forwardBound)
        {
            boat.setPos(glm::vec3(boat.getPos().x, boat.getPos().y, forwardBound));
        }
    }

    void initGame()
    {
        // Reset boat, rocks and game logic variables
        score = 0.0f;
        accFactor = 0.0f;
        state = PLAY;

        cameraPosition = initialCameraPosition;
        boat.reset();
        for (auto &r : rocks)
        {
            r.reset();
        }
    }

    bool writeScore(string fname, float score)
    {
        ofstream wf(fname, ios::out | ios::binary);
        if (!wf)
        {
            cout << "Cannot open file!" << endl;
            return false;
        }
        wf.write((char *)&score, sizeof(score));
        wf.close();
        if (!wf.good())
        {
            cout << "Cannot write high score file..." << endl;
            return false;
        }
        return true;
    }

    float readScore(string fname)
    {
        float score;

        ifstream rf(fname, ios::out | ios::binary);
        if (!rf || !rf.good())
        {
            cout << "Cannot read score file. Creating a new one..." << endl;
            writeScore(fname, 0.0f);
        }
        else
        {
            rf.read((char *)&score, sizeof(float));
        }
        rf.close();

        return score;
    }

    void printMenu()
    {
        std::cout << "Press W to move forward." << std::endl;
        std::cout << "Press S to move backward." << std::endl;
        std::cout << "Press A to move left." << std::endl;
        std::cout << "Press D to move right." << std::endl;
        std::cout << "Press SPACE to jump." << std::endl;
        std::cout << "Press R to restart the game at gameover." << std::endl;
    }

    glm::vec3 scaleVector(glm::vec3 v, float s)
    {
        return glm::vec3(v.x * s, v.y * s, v.z * s);
    }
};

int main()
{
    BoatRunner app;
    srand(time(0));
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