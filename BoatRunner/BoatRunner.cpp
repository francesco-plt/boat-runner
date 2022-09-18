using namespace std;

#include "BoatRunner.hpp"

#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/epsilon.hpp>

#define rocksNum 30
#define limitNorth -160
#define limitSouth 30
#define limitZ 50
#define deltaNorth 60
#define boatWidth 20
#define boatLength 20

// Asset paths
static const string MODEL_PATH = "models";
static const string TEXTURE_PATH = "textures";
static const string FRAGMENT_SHADER = "shaders/frag.spv";
static const string VERTEX_SHADER = "shaders/vert.spv";

// Other variables
static const glm::mat4 I = glm::mat4(1.0f);           // Identity matrix
static const glm::vec3 xAxis = glm::vec3(1, 0, 0);    // x axis
static const glm::vec3 yAxis = glm::vec3(0, 1, 0);    // y axis
static const glm::vec3 zAxis = glm::vec3(0, 0, 1);    // z axis

static const float speedFactorConstant = 30.0f;
static const float fwdSpeedFactorConstant = 0.1f;
static const glm::vec3 boatScalingFactor = glm::vec3(0.005f);
static const glm::vec3 rock1scalingFactor = glm::vec3(0.3f);
static const glm::vec3 rock2scalingFactor = glm::vec3(0.3f);
static const float FoV = glm::radians(90.0f);

static const glm::vec3 cameraPosition = glm::vec3(0.0f, 2.5f, -4.0f);
static const glm::vec3 cameraDirection = glm::vec3(0.0f, 1.5f, 0.0f);
static const glm::vec3 initialBoatPosition = glm::vec3(0.0f, 0.0f, 0.0f);

struct globalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};

struct RockStruct {
	DescriptorSet DS_Rock;
	glm::vec3 pos;
	float rot;
	int id;
};

class BoatRunner : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLglobal;
	DescriptorSetLayout DSLobj;

	// Pipelines [Shader couples]
	Pipeline P1;

	int dsCount = 1;    // boat, rock1, rock2 (plus global set)

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M_Boat;
	Texture T_Boat;
	DescriptorSet DS_Boat;	// instance DSLobj

	Model M_Rock1;
	Texture T_Rock1;

	Model M_Rock2;
	Texture T_Rock2;

	vector<RockStruct> rocks1;
	vector<RockStruct> rocks2;
	
	DescriptorSet DS_global;

	glm::vec3 boatPosition;
	glm::vec3 oldBoatPos = initialBoatPosition;

	float boatSpeedFactor;
	float rockSpeedFactor;
	
	// application parameters:
    // Window size, titile and initial background
    void setWindowParameters() {
        // 4:3 aspect ratio
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "Boat Runner";
        initialBackgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = dsCount + rocksNum * 2 + 1;
        texturesInPool = dsCount + rocksNum * 2;
        setsInPool = dsCount + rocksNum * 2 + 1;
    }
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLobj.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
		});

		DSLglobal.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
		});

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, VERTEX_SHADER, FRAGMENT_SHADER, {&DSLglobal, &DSLobj});

		// Models, textures and Descriptors (values assigned to the uniforms)
		M_Boat.init(this, MODEL_PATH + "/Boat.obj");
		T_Boat.init(this, TEXTURE_PATH + "/Boat.bmp");
		DS_Boat.init(this, &DSLobj, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_Boat}
		});

		M_Rock1.init(this, MODEL_PATH + "/Rock1.obj");
		T_Rock1.init(this, TEXTURE_PATH + "/Rock1.png");

		for(int i = 0; i < rocksNum; i++) {
			RockStruct rock;
			rock.DS_Rock.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_Rock1}
			});
			rock.pos = glm::vec3(glm::linearRand(limitNorth - deltaNorth, limitNorth + deltaNorth), 0, glm::linearRand(-limitZ, limitZ));
			rock.id = i;
			rocks1.push_back(rock);
		}

		M_Rock2.init(this, MODEL_PATH + "/Rock2.obj");
		T_Rock2.init(this, TEXTURE_PATH + "/Rock2.jpg");

		for(int i = 0; i < rocksNum; i++) {
			RockStruct rock;
			rock.DS_Rock.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_Rock2}
			});
			rock.pos = glm::vec3(glm::linearRand(limitNorth - deltaNorth, limitNorth + deltaNorth), 0, glm::linearRand(-limitZ, limitZ));
			rock.id = rocksNum + i;
			rocks2.push_back(rock);
		}

		DS_global.init(this, &DSLglobal, {
					{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}
		});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS_Boat.cleanup();
		T_Boat.cleanup();
		M_Boat.cleanup();

		T_Rock1.cleanup();
		M_Rock1.cleanup();
		
		M_Rock2.cleanup();
		T_Rock2.cleanup();

		for(int i = 0; i < rocksNum; i++) {
			rocks1[i].DS_Rock.cleanup();
		}

		for(int i = 0; i < rocksNum; i++) {
			rocks2[i].DS_Rock.cleanup();
		}

		DS_global.cleanup();

		P1.cleanup();
		DSLglobal.cleanup();
		DSLobj.cleanup();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
				
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.graphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
						0, nullptr);

				
		VkBuffer vertexBuffers[] = {M_Boat.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, M_Boat.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);

		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &DS_Boat.descriptorSets[currentImage],
						0, nullptr);
						
		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M_Boat.indices.size()), 1, 0, 0, 0);

		VkBuffer vertexBuffers2[] = {M_Rock1.vertexBuffer};
		VkDeviceSize offsets2[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
		vkCmdBindIndexBuffer(commandBuffer, M_Rock1.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);

		VkBuffer vertexBuffers3[] = {M_Rock2.vertexBuffer};
		VkDeviceSize offsets3[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers3, offsets3);
		vkCmdBindIndexBuffer(commandBuffer, M_Rock2.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);
		
		for(int i = 0; i < rocksNum; i++) {
			vkCmdBindDescriptorSets(commandBuffer,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							P1.pipelineLayout, 1, 1, &rocks1[i].DS_Rock.descriptorSets[currentImage],
							0, nullptr);
			vkCmdDrawIndexed(commandBuffer,
						static_cast<uint32_t>(M_Rock1.indices.size()), 1, 0, 0, 0);
						
			vkCmdBindDescriptorSets(commandBuffer,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							P1.pipelineLayout, 1, 1, &rocks2[i].DS_Rock.descriptorSets[currentImage],
							0, nullptr);
			vkCmdDrawIndexed(commandBuffer,
						static_cast<uint32_t>(M_Rock2.indices.size()), 1, 0, 0, 0);
		}
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
        static float lastTime = 0.0f;

		if(lastTime == 0.0f) {
			initGame();
		}
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period> (currentTime - startTime).count();
        float deltaT = time - lastTime;
        lastTime = time;

		updatePosition(deltaT);
		detectCollisions();
					
		globalUniformBufferObject gubo{};
		UniformBufferObject ubo{};
		
		void* data;

		gubo.view = glm::lookAt(cameraPosition, cameraDirection, glm::vec3(0, 0, 1));
		gubo.proj = glm::perspective(FoV, swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
		gubo.proj[1][1] *= -1;

		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);
		

		// Boat
		ubo.model = I;
		ubo.model = glm::scale(ubo.model, boatScalingFactor);	// scale the model
		ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), yAxis);	// align the model to the camera
		ubo.model = glm::rotate(ubo.model, glm::radians(sin(2 * time)), xAxis);	// boat oscillation
		ubo.model = glm::translate(ubo.model, boatPosition);	// translating boat according to players input
		
		vkMapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage]);

		for(int i = 0; i < rocksNum; i++) {
			ubo.model = I;
			ubo.model = glm::scale(ubo.model, rock1scalingFactor);
			ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0, 1, 0));
			ubo.model = glm::translate(ubo.model, rocks1[i].pos);
			ubo.model = glm::rotate(ubo.model, rocks1[i].rot, yAxis);

			vkMapMemory(device, rocks1[i].DS_Rock.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, rocks1[i].DS_Rock.uniformBuffersMemory[0][currentImage]);

			ubo.model = I;
			ubo.model = glm::scale(ubo.model, rock2scalingFactor);
			ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0, 1, 0));
			ubo.model = glm::translate(ubo.model, rocks2[i].pos);
			ubo.model = glm::rotate(ubo.model, rocks2[i].rot, yAxis);

			vkMapMemory(device, rocks2[i].DS_Rock.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, rocks2[i].DS_Rock.uniformBuffersMemory[0][currentImage]);
		}
	}

	void updatePosition(float deltaT) {
		for(int i = 0; i < rocksNum; i++) {
			rocks1[i].pos.x += rockSpeedFactor;
			if(rocks1[i].pos.x > limitSouth) {
				rocks1[i].pos.x = glm::linearRand(limitNorth - deltaNorth, limitNorth + deltaNorth);
				rocks1[i].pos.z = glm::linearRand(-limitZ, limitZ);
				rocks1[i].rot = glm::radians(glm::linearRand(0.0f, 1.0f));
			}
			rocks2[i].pos.x += rockSpeedFactor;
			if(rocks2[i].pos.x > limitSouth) {
				rocks2[i].pos.x = glm::linearRand(limitNorth - deltaNorth, limitNorth + deltaNorth);
				rocks2[i].pos.z = glm::linearRand(-limitZ, limitZ);
				rocks2[i].rot = glm::radians(glm::linearRand(0.0f, 360.0f));
			}
		}

		if (glfwGetKey(window, GLFW_KEY_A)) {
			boatPosition.z += boatSpeedFactor;
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			boatPosition.z -= boatSpeedFactor;
		}
	}

	void detectCollisions() {
		if(!glm::all(glm::equal(boatPosition, oldBoatPos))) {
			printf("Boat Position: (%.1f, %.1f, %.1f)\n", boatPosition.x, boatPosition.y, boatPosition.z);
			oldBoatPos = boatPosition;
		}
		for(int i = 0; i < rocksNum; i++) {
			if(rocks1[i].pos.x <= boatPosition.x + boatLength / 2 && rocks1[i].pos.x >= boatPosition.x - boatLength / 2) {
				if(rocks1[i].pos.z <= boatPosition.z + boatWidth / 2 && rocks1[i].pos.z >= boatPosition.z - boatWidth / 2) {
					printf("Collided in (%.1f, %.1f, %.1f) with rock %d.\nRestarting the game\n", rocks1[i].pos.x, rocks1[i].pos.y, rocks1[i].pos.z, rocks1[i].id);
					initGame();
				}
			}
			if(rocks2[i].pos.x <= boatPosition.x + boatLength / 2 && rocks2[i].pos.x >= boatPosition.x - boatLength / 2) {
				if(rocks2[i].pos.z <= boatPosition.z + boatWidth / 2 && rocks2[i].pos.z >= boatPosition.z - boatWidth / 2) {
					printf("Collided in (%.1f, %.1f, %.1f) with rock %d.\nRestarting the game\n", rocks2[i].pos.x, rocks2[i].pos.y, rocks2[i].pos.z, rocks2[i].id);
					initGame();
				}
			}
		}
	}

	void initGame() {
		boatPosition = initialBoatPosition;
		boatSpeedFactor = speedFactorConstant;
		rockSpeedFactor = fwdSpeedFactorConstant;
		for(int i = 0; i < rocksNum; i++) {
			rocks1[i].pos.x = glm::linearRand(limitNorth - deltaNorth, limitNorth + deltaNorth);
			rocks2[i].pos.x = glm::linearRand(limitNorth - deltaNorth, limitNorth + deltaNorth);
		}
	}
};

// This is the main: probably you do not need to touch this!
int main() {
    BoatRunner app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}