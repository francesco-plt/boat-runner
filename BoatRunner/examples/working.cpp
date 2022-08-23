// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"
//#include "test.hpp"

const std::string MODEL_PATH = "models";
const std::string TEXTURE_PATH = "textures";

struct globalUniformBufferObject
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};


// MAIN ! 
class MyProject : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL_obj;
	DescriptorSetLayout DSL_global;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model Boat_M;
	Texture Boat_T;
	DescriptorSet DS_Boat;
	Model Rock1_M;
	Texture Rock1_T;
	DescriptorSet DS_Rock1;
	Model Rock2_M;
	Texture Rock2_T;
	DescriptorSet DS_Rock2;

	DescriptorSet DS_global;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Boat runner";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 4;
		texturesInPool = 3;
		setsInPool = 4;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSL_obj.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });
		
		DSL_global.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
		});

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL_global, &DSL_obj});

		// Models, textures and Descriptors (values assigned to the uniforms)
		Boat_M.init(this, MODEL_PATH + "/Boat.obj");
		Boat_T.init(this, TEXTURE_PATH + "/Boat.bmp");
		DS_Boat.init(this, &DSL_obj, {
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			{1, TEXTURE, 0, &Boat_T}
		});
		Rock1_M.init(this, MODEL_PATH + "/Rock1.obj");
		Rock1_T.init(this, TEXTURE_PATH + "/Rock1.png");
		DS_Rock1.init(this, &DSL_obj, {
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			{1, TEXTURE, 0, &Rock1_T}
		});
		Rock2_M.init(this, MODEL_PATH + "/Rock2.obj");
		Rock2_T.init(this, TEXTURE_PATH + "/Rock2.jpg");
		DS_Rock2.init(this, &DSL_obj, {
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			{1, TEXTURE, 0, &Rock2_T}
		});

		DS_global.init(this, &DSL_global, {
			{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr},
		});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS_Boat.cleanup();
		Boat_M.cleanup();
		Boat_T.cleanup();
		DS_Rock1.cleanup();
		Rock1_M.cleanup();
		Rock1_T.cleanup();
		DS_Rock2.cleanup();
		Rock2_M.cleanup();
		Rock2_T.cleanup();

		P1.cleanup();
		DSL_global.cleanup();
		DSL_obj.cleanup();
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
				
		VkBuffer vertexBuffersBoat[] = {Boat_M.vertexBuffer};
		VkDeviceSize offsetsBoat[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersBoat, offsetsBoat);
		vkCmdBindIndexBuffer(commandBuffer, Boat_M.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &DS_Boat.descriptorSets[currentImage],
						0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Boat_M.indices.size()), 1, 0, 0, 0);

		VkBuffer vertexBuffersRock1[] = {Rock1_M.vertexBuffer};
		VkDeviceSize offsetsRock1[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock1, offsetsRock1);
		vkCmdBindIndexBuffer(commandBuffer, Rock1_M.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &DS_Rock1.descriptorSets[currentImage],
						0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Rock1_M.indices.size()), 1, 0, 0, 0);

		VkBuffer vertexBuffersRock2[] = {Rock2_M.vertexBuffer};
		VkDeviceSize offsetsRock2[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersRock2, offsetsRock2);
		vkCmdBindIndexBuffer(commandBuffer, Rock2_M.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &DS_Rock2.descriptorSets[currentImage],
						0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Rock2_M.indices.size()), 1, 0, 0, 0);
	}

	glm::mat4 updatePosition(float time) {
		glm::mat4 translY = glm::translate(glm::mat4(1), glm::vec3(0.0f, -2.0f, 0.0f));
		//glm::mat4 rotX = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 rotY = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 rotZ = glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.001f, 0.001f, 0.001f));
		glm::mat4 oscillX = glm::rotate(glm::mat4(1), sin(time) * glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::mat4 oscillZ = glm::rotate(glm::mat4(1), sin(time / 2) * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 steer = glm::mat4(1);
		glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.01, 0.01, 0.01));
		if (glfwGetKey(window, GLFW_KEY_A)) {
			steer = glm::rotate(glm::mat4(1), glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		} else if (glfwGetKey(window, GLFW_KEY_D)) {
			steer = glm::rotate(glm::mat4(1), glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			steer *= glm::rotate(glm::mat4(1), glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}

		return translY * rotY * oscillX * steer * scale;
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
					(currentTime - startTime).count();
					
		globalUniformBufferObject gubo{};
		UniformBufferObject ubo{};// = updatePosition(time);
		
		void* data;

		gubo.view = glm::mat4(1);
		/*
        gubo.proj = glm::perspective(glm::radians(0.0f),
                                     swapChainExtent.width / (float)swapChainExtent.height,
                                     0.1f, 10.0f);
        gubo.proj[1][1] *= -1;
		*/

		gubo.proj = glm::mat4(1);

        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

		//ubo.model = glm::mat4(1);

		// Boat
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.2f, 1.9f, -0.86f));
		ubo.model = updatePosition(time);
		vkMapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage], 0,
							sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_Boat.uniformBuffersMemory[0][currentImage]);

		// Rock1
		//ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.04422f, 0.0f));
		ubo.model = glm::mat4(1);
        vkMapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Rock1.uniformBuffersMemory[0][currentImage]);

		// Rock2
		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.2f, 1.9f, -0.86f));
		//ubo.model = glm::mat4(1);
        vkMapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Rock2.uniformBuffersMemory[0][currentImage]);
	}
};

// This is the main: probably you do not need to touch this!
int main() {
    MyProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}