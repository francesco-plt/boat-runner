using namespace std;

#include "BoatRunner.hpp"

#include <cmath>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/epsilon.hpp>
#include <string>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>

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
static const glm::vec3 rock1scalingFactor = glm::vec3(0.09f);
static const glm::vec3 rock2scalingFactor = glm::vec3(0.1f);
static const float FoV = glm::radians(120.0f);

static const glm::vec3 cameraPosition = glm::vec3(0.0f, 2.0f, -2.0f);
static const glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
static const glm::vec3 initialBoatPosition = glm::vec3(0.0f, 0.0f, 0.0f);
static const glm::vec3 initialRock1Position = glm::vec3(0.0f, 0.0f, -15.0f);
static const glm::vec3 initialRock2Position = glm::vec3(0.0f, 0.0f, 10.0f);


struct globalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};

class BoatRunner : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLglobal;
	DescriptorSetLayout DSLobj;

	// Pipelines [Shader couples]
	Pipeline P1;

	int dsCount = 3;    // boat, rock1, rock2 (plus global set)

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M_Boat;
	Texture T_Boat;
	DescriptorSet DS_Boat;	// instance DSLobj

	Model M_Rock1;
	Texture T_Rock1;
	DescriptorSet DS_Rock1;	// instance DSLobj

	Model M_Rock2;
	Texture T_Rock2;
	DescriptorSet DS_Rock2;	// instance DSLobj
	
	DescriptorSet DS_global;

	glm::vec3 boatPosition;
	glm::vec3 rock1Position;
	glm::vec3 rock2Position;

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
        uniformBlocksInPool = dsCount + 1;
        texturesInPool = dsCount;
        setsInPool = dsCount + 1;
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
		DS_Rock1.init(this, &DSLobj, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_Rock1}
		});


		M_Rock2.init(this, MODEL_PATH + "/Rock2.obj");
		T_Rock2.init(this, TEXTURE_PATH + "/Rock2.jpg");
		DS_Rock2.init(this, &DSLobj, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_Rock2}
		});

		DS_global.init(this, &DSLglobal, {
					{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}
		});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS_Boat.cleanup();
		T_Boat.cleanup();
		M_Boat.cleanup();

		DS_Rock1.cleanup();
		T_Rock1.cleanup();
		M_Rock1.cleanup();
		
		DS_Rock2.cleanup();
		M_Rock2.cleanup();
		T_Rock2.cleanup();

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
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &DS_Rock1.descriptorSets[currentImage],
						0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M_Rock1.indices.size()), 1, 0, 0, 0);



		VkBuffer vertexBuffers3[] = {M_Rock2.vertexBuffer};
		VkDeviceSize offsets3[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers3, offsets3);
		vkCmdBindIndexBuffer(commandBuffer, M_Rock2.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &DS_Rock2.descriptorSets[currentImage],
						0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M_Rock2.indices.size()), 1, 0, 0, 0);
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

		gubo.view = glm::lookAt(cameraPosition, cameraDirection, yAxis);
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

		// Rock 1
		ubo.model = I;
		ubo.model = glm::scale(ubo.model, rock1scalingFactor);
		ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), yAxis);	// align the model to the camera
		ubo.model = glm::translate(ubo.model, rock1Position);

		vkMapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage]);

		// Rock 2
		ubo.model = I;
		ubo.model = glm::scale(ubo.model, rock2scalingFactor);
		ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), yAxis);	// align the model to the camera
		ubo.model = glm::translate(ubo.model, rock2Position);

		vkMapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage]);
	}

	void updatePosition(float deltaT) {

		// rock1Position.x += rockSpeedFactor;
		// rock2Position.x += rockSpeedFactor;

		if (glfwGetKey(window, GLFW_KEY_A)) {
			// boatPosition.z += boatSpeedFactor;
			boatPosition.z += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			boatPosition.z -= 1.0f;
		}
	}

	void detectCollisions() {

		std::cout << "Boat Position: " << glm::to_string(boatPosition) << std::endl;
		std::cout << "Rock1 Position: " << glm::to_string(rock1Position) << std::endl;
		std::cout << "Rock2 Position: " << glm::to_string(rock2Position) << std::endl;

		if(glm::all(glm::equal(boatPosition, rock1Position)) || glm::all(glm::equal(boatPosition, rock2Position))) {
			std::cout << "Collision detected! Restarting game..." << std::endl;
			initGame();
		}
	}

	void initGame() {
		boatPosition = initialBoatPosition;
		rock1Position = initialRock1Position;
		rock2Position = initialRock2Position;

		boatSpeedFactor = speedFactorConstant;
		rockSpeedFactor = fwdSpeedFactorConstant;
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