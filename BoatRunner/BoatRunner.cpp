using namespace std;

#include "BoatRunner.hpp"

#include <fstream>
#include<iostream>
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

#if defined(_WIN64)
#define boatSpeed 0.3f
#define rockSpeed 0.4f
#else
#define boatSpeed 0.5f
#define rockSpeed 1.0f
#endif

#define ESC "\033[;"
#define RED "31m"
#define GREEN "32m"
#define YELLOW "33m"
#define BLUE "34m"
#define PURPLE "35m"
#define RESET "\033[m"

/* --------------------------------- GLOBAL VARIABLES --------------------------------- */

static const string MODEL_PATH = "models";
static const string TEXTURE_PATH = "textures";
static const string FRAGMENT_SHADER = "shaders/frag.spv";
static const string VERTEX_SHADER = "shaders/vert.spv";


static const glm::mat4 I = glm::mat4(1.1f);           // Identity matrix
static const glm::vec3 xAxis = glm::vec3(1, 0, 0);    // x axis
static const glm::vec3 yAxis = glm::vec3(0, 1, 0);    // y axis
static const glm::vec3 zAxis = glm::vec3(0, 0, 1);    // z axis


static const int rocksCount = 10;
static const glm::vec3 boatScalingFactor = glm::vec3(0.3f);
static const glm::vec3 rock1scalingFactor = glm::vec3(0.3f);
static const glm::vec3 rock2scalingFactor = glm::vec3(0.3f);
static const glm::vec3 oceanScalingFactor = glm::vec3(50.0f);
static const float oceanSpeed = 0.25f;
static const float maxAcceleration = 3.0f;


static const float FoV = glm::radians(60.0f);
static const float nearPlane = 0.1f;
static const float farPlane = 100.0f;


static const int horizon = -100.0f;
static const int maxDepth = 10;
static const int leftBound = 10;
static const int rightBound = -10;
static const int forwardBound = -4;
static const int backwardBound = 2;
static const int limitZ = 20;
static const int rockGenDelta = 50;
static const float boatWidth = 3.5f;
static const float boatLength = 5.5f;


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

enum rockType {rock1, rock2};

class Boat {

	protected:
	DescriptorSet DS;
	Model model;
	Texture texture;
	glm::vec3 pos;
	float speedFactor;

	public:
	void init(BaseProject *br, DescriptorSetLayout DSLobj) {
		model.init(br, MODEL_PATH + "/shorterBoat.obj");
		texture.init(br, TEXTURE_PATH + "/Boat.bmp");
		DS.init(br, &DSLobj, {
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			{1, TEXTURE, 0, &texture}
		});

		speedFactor = boatSpeed;
		reset();
		std::cout << ESC << GREEN << "Boat initialized" << RESET << std::endl;
	}

	void reset() {
		pos = initialBoatPosition;
	}

	void cleanup() {
		model.cleanup();
		texture.cleanup();
		DS.cleanup();
	}

	void moveLeft(float deltaT) {
		// z because we need to take into account boat rotation
		pos.z += speedFactor;
	}

	void moveRight(float deltaT) {
		pos.z -= speedFactor;
	}

	void moveForward(float deltaT) {
		pos.x -= speedFactor / 3;
	}

	void moveBackward(float deltaT) {
		pos.x += speedFactor / 3;
	}

	Model getModel() {
		return model;
	}

	DescriptorSet getDS() {
		return DS;
	}

	glm::vec3 getPos() {
		return pos;
	}

	void setPos(glm::vec3 newPos) {
		pos = newPos;
	}

	void printPos() {
		printf("Boat Position: (%.1f, %.1f, %.1f)\n", pos.x, pos.y, pos.z);
	}
};

class Ocean {
	protected:
	DescriptorSet DS;
	Model model;
	Texture texture;
	glm::vec3 pos;
	float speedFactor;
	
	public:
		void init(BaseProject *br, DescriptorSetLayout DSLobj) {
			model.init(br, MODEL_PATH + "/Ocean.obj");
			texture.init(br, TEXTURE_PATH + "/Ocean.png");
			DS.init(br, &DSLobj, {
				{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
				{1, TEXTURE, 0, &texture}
			});

			speedFactor = oceanSpeed;
			pos = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		void cleanup() {
			model.cleanup();
			texture.cleanup();
			DS.cleanup();
		}

		void moveForward(float accelerationFactor) {
			pos.x += speedFactor;
		}


		Model getModel() {
			return model;
		}

		DescriptorSet getDS() {
			return DS;
		}
};

class Rock {
	
	protected:
	DescriptorSet DS;
	rockType type;
	glm::vec3 pos;
	float rot;
	int id;
	float speedFactor;

	public:
	void init(BaseProject *br, DescriptorSetLayout DSLobj, Texture texture, int identifier, rockType type) {
		
		DS.init(br, &DSLobj, {
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			{1, TEXTURE, 0, &texture}
		});
		
		id = identifier;
		speedFactor = rockSpeed;
		reset();
		std::cout << ESC << GREEN << "Rock " << id << " initialized" << RESET << std::endl;
	}

	void reset() {
		// Randomly generated position according to a normal distribution
		// x in [horizon - rockGenDelta], horizon + rockGenDelta]
		// z in [leftBound, rightBound]
		pos = glm::vec3(
			glm::linearRand(horizon - rockGenDelta * 2.0f, horizon + rockGenDelta * 0.5f),
			0, glm::linearRand(rightBound, leftBound)
		);
		// same for rotation
		rot = glm::linearRand(0.0f, 360.0f);
	}

	void cleanup() {
		DS.cleanup();
	}

	void moveForward(float accelerationFactor) {
		pos.x += speedFactor + accelerationFactor;
	}

	DescriptorSet getDS() {
		return DS;
	}

	// getters and setters
	
	glm::vec3 getPos() {
		return pos;
	}

	float getRot() {
		return rot;
	}

	int getId() {
		return id;
	}

	float getSpeedFactor() {
		return speedFactor;
	}

	rockType getType() {
		return type;
	}

	void printPos() {
		printf("Rock %d Position: (%.1f, %.1f, %.1f)\n", id, pos.x, pos.y, pos.z);
	}
};

class BoatRunner : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLglobal;
	DescriptorSetLayout DSLobj;

	// Pipelines [Shader couples]
	Pipeline P1;

	int dsCount = rocksCount + 2;	// boat, rock1, rock2 (plus global set)

	// Models, textures and Descriptors (values assigned to the uniforms)

	Boat boat;
	Ocean ocean;

	Model M_Rock1;
	Texture T_Rock1;

	Model M_Rock2;
	Texture T_Rock2;

	vector<Rock> rocks;
	DescriptorSet DS_global;

	// glm::vec3 boatPosition;
	glm::vec3 oldBoatPos = initialBoatPosition;

	float score = 0.0f;
	float highScore = (float) readScore("highscore.dat");
	float oldScore = 0.0f;
	float accFactor = 0.0f;
	
	// application parameters:
    // Window size, titile and initial background
    void setWindowParameters() {

        // 4:3 aspect ratio
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "Boat Runner";
        initialBackgroundColor = {0.0f, 0.5f, 0.8f, 1.0f};

        // Descriptor pool sizes
        uniformBlocksInPool = dsCount + 1;
        texturesInPool = dsCount + 1;
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
		boat.init(this, DSLobj);
		ocean.init(this, DSLobj);

		M_Rock1.init(this, MODEL_PATH + "/Rock1.obj");
		T_Rock1.init(this, TEXTURE_PATH + "/Rock1.png");

		M_Rock2.init(this, MODEL_PATH + "/Rock2.obj");
		T_Rock2.init(this, TEXTURE_PATH + "/Rock2.jpg");


		for(int i = 0; i < rocksCount; i++) {
			Rock rock;
			if(i % 2 == 0) {
				rock.init(this, DSLobj, T_Rock1, i, rock1);
			} else {
				rock.init(this, DSLobj, T_Rock2, i, rock2);
			}
			rocks.push_back(rock);
		}

		DS_global.init(this, &DSLglobal, {{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}});

		std::cout << "BoatRunner initialized..." << std::endl;
		std::cout << "____________________________________________________" << std::endl;
		std::cout << "GAME OUTPUT [" << "Highest score: " << highScore << "]:" << std::endl;
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		
		// clean stdout
		std::cout << "\n" << std::endl;

		boat.cleanup();
		ocean.cleanup();

		T_Rock1.cleanup();
		M_Rock1.cleanup();
		
		M_Rock2.cleanup();
		T_Rock2.cleanup();

		for(auto & r : rocks) {
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
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
				
		// Pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.graphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
						0, nullptr);

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

		// Rocks
		VkBuffer rock1VertexBuffers[] = {M_Rock1.vertexBuffer};
		VkDeviceSize rock1Offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, rock1VertexBuffers, rock1Offsets);
		vkCmdBindIndexBuffer(commandBuffer, M_Rock1.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);

		VkBuffer rock2VertexBuffers[] = {M_Rock2.vertexBuffer};
		VkDeviceSize rock2Offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, rock2VertexBuffers, rock2Offsets);
		vkCmdBindIndexBuffer(commandBuffer, M_Rock2.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);
		
		for(auto & r : rocks) {
			vkCmdBindDescriptorSets(commandBuffer,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							P1.pipelineLayout, 1, 1, &r.getDS().descriptorSets[currentImage],
							0, nullptr);
			if(r.getType() == 1) {
				vkCmdDrawIndexed(commandBuffer,
							static_cast<uint32_t>(M_Rock1.indices.size()), 1, 0, 0, 0);
			} else {
				vkCmdDrawIndexed(commandBuffer,
							static_cast<uint32_t>(M_Rock2.indices.size()), 1, 0, 0, 0);
			}
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
		score += deltaT;

		// Updating score in console while running
		std::cout << ESC << PURPLE << "score: " << score << RESET << '\r' << std::flush;

		// progressive acceleration
		if(accFactor >= maxAcceleration) {
			accFactor = maxAcceleration;
		} else {
			accFactor += log10(log10(lastTime/8000 + 10));
		}

		updatePosition(accFactor);
		detectCollisions();
					
		globalUniformBufferObject gubo{};
		UniformBufferObject ubo{};
		
		void* data;

		gubo.view = glm::lookAt(cameraPosition, cameraDirection, glm::vec3(0, 0, 1));
		gubo.proj = glm::perspective(FoV, swapChainExtent.width / (float) swapChainExtent.height, nearPlane, farPlane);
		gubo.proj[1][1] *= -1;

		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);
		

		// Boat
		ubo.model = I;
		ubo.model = glm::scale(ubo.model, boatScalingFactor);	// scale the model
		ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), yAxis);	// align the model to the camera
		ubo.model = glm::rotate(ubo.model, glm::radians(sin(2 * time)), xAxis);	// boat oscillation
		ubo.model = glm::rotate(ubo.model, glm::radians(sin(2 * time)), zAxis);	// ocean oscillation
		ubo.model = glm::translate(ubo.model, boat.getPos());	// translating boat according to players input
		ubo.model = glm::translate(ubo.model, glm::vec3(0, -0.8f, 0));	// translating the boat down in the water
		
		vkMapMemory(device, boat.getDS().uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, boat.getDS().uniformBuffersMemory[0][currentImage]);

		// Ocean
		ubo.model = I;
		ubo.model = glm::scale(ubo.model, oceanScalingFactor);	// scale the model
		ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), yAxis);	// align the model to the camera
		ubo.model = glm::rotate(ubo.model, glm::radians(0.5f * sin(time)), zAxis);	// ocean oscillation
		ubo.model = glm::translate(ubo.model, glm::vec3(0, -0.005f, 0));	// translating the ocean down so that it is always under the boat
		ubo.model = glm::scale(ubo.model, glm::vec3(1, 0.5f, 1));	// making it shorter in height so that it doesn't cover the boat

		vkMapMemory(device, ocean.getDS().uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, ocean.getDS().uniformBuffersMemory[0][currentImage]);		

		// Rocks
		for(auto & r : rocks) {
			ubo.model = I;
			ubo.model = glm::scale(ubo.model, rock1scalingFactor);
			ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0, 1, 0));
			ubo.model = glm::translate(ubo.model, r.getPos());
			ubo.model = glm::rotate(ubo.model, r.getRot(), yAxis);

			vkMapMemory(device, r.getDS().uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, r.getDS().uniformBuffersMemory[0][currentImage]);
		}
	}

	void updatePosition(float accelerationFactor) {
		
		ocean.moveForward(accelerationFactor);

		for(auto & r : rocks) {
			r.moveForward(accelerationFactor);
			if(r.getPos().x > maxDepth) {
				r.reset();
			}
		}

		if (glfwGetKey(window, GLFW_KEY_A)) {
			boat.moveLeft(accelerationFactor);
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			boat.moveRight(accelerationFactor);
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			boat.moveForward(accelerationFactor);
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			boat.moveBackward(accelerationFactor);
		}
	}

	void detectCollisions() {
		
		if(!glm::all(glm::equal(boat.getPos(), oldBoatPos))) {
			// boat.printPos();
			oldBoatPos = boat.getPos();
		}
		
		// check for collisions beteween boat and rocks
		for(auto & r : rocks) {
			// r.printPos();
			if(r.getPos().x <= boat.getPos().x + boatLength / 2 && r.getPos().x >= boat.getPos().x - boatLength / 2) {
				if(r.getPos().z <= boat.getPos().z + boatWidth / 2 && r.getPos().z >= boat.getPos().z - boatWidth / 2) {

					// printf("Collided in (%.1f, %.1f, %.1f) with rock %d.\n", r.getPos().x, r.getPos().y, r.getPos().z, r.getId());
					std::cout << ESC << RED << "Final score: " << score << RESET << std::endl;
					
					if(score > highScore) {
						highScore = score;
						std::cout << ESC << GREEN << "New High Score: " << highScore << RESET << std::endl;

						if(writeScore("highscore.dat", highScore)) {
							std::cout << "High Score written to file." << std::endl;
						} else {
							std::cout << "Failed to write High Score to file." << std::endl;
						}
					}
					std::cout << "Restarting the game..." << std::endl;

					initGame();
				}
			}
		}

		//check that the boat stays in the horizontal limits
		if(boat.getPos().z > leftBound) {
			boat.setPos(glm::vec3(boat.getPos().x, boat.getPos().y, leftBound));
		} else if(boat.getPos().z < rightBound) {
			boat.setPos(glm::vec3(boat.getPos().x, boat.getPos().y, rightBound));
		}
		if(boat.getPos().x < forwardBound) {
			boat.setPos(glm::vec3(forwardBound, boat.getPos().y, boat.getPos().z));
		} else if(boat.getPos().x > backwardBound) {
			boat.setPos(glm::vec3(backwardBound, boat.getPos().y, boat.getPos().z));
		}
	}

	void initGame() {
		
		oldScore = score;
		score = 0.0f;
		accFactor = 0.0f;
		boat.reset();
		for(auto & r : rocks) {
			r.reset();
		}
	}

	bool writeScore(string fname, float score) {
		
		ofstream wf(fname, ios::out | ios::binary);
		if(!wf) {
			cout << "Cannot open file!" << endl;
			return false;
		}
		wf.write((char*)&score, sizeof(score));
		wf.close();
   		if(!wf.good()) {
      		cout << "Cannot write high score file..." << endl;
      		return false;
   		}
		return true;
	}

	float readScore(string fname) {
		
		float score;

		ifstream rf(fname, ios::out | ios::binary);
		if(!rf.good()) {
			cout << "Cannot read score file. Creating a new one..." << endl;
			writeScore(fname, 0.0f);
			return -1;
		}
		if(!rf) {
			cout << "Cannot open file!" << endl;
			return 1;
   		}
		rf.read((char*)&score, sizeof(float));
		rf.close();

		return score;
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