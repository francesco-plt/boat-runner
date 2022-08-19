// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

const std::string MODEL_PATH = "models";
const std::string TEXTURE_PATH = "textures";

// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};


// MAIN ! 
class MyProject : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL1;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model Boat_M;
	Model Rock1_M;
	Model Rock2_M;
	Texture Boat_T;
	Texture Rock1_T;
	Texture Rock2_T;
	DescriptorSet DS1;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Boat runner";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 1;
		texturesInPool = 1;
		setsInPool = 1;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSL1.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL1});

		// Models, textures and Descriptors (values assigned to the uniforms)
		Boat_M.init(this, MODEL_PATH + "/Boat.obj");
		Rock1_M.init(this, MODEL_PATH + "/Rock1.obj");
		Boat_T.init(this, TEXTURE_PATH + "/Boat.bmp");
		Rock1_T.init(this, TEXTURE_PATH + "/Rock1.png");
		DS1.init(this, &DSL1, {
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			{1, TEXTURE, 0, &Boat_T}
		});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS1.cleanup();
		Boat_T.cleanup();
		Boat_M.cleanup();
		P1.cleanup();
		DSL1.cleanup();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
				
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.graphicsPipeline);
				
		VkBuffer vertexBuffers[] = {Boat_M.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, Boat_M.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);

		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 0, 1, &DS1.descriptorSets[currentImage],
						0, nullptr);
						
		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Boat_M.indices.size()), 1, 0, 0, 0);
	}

	UniformBufferObject updatePosition(float time) {
		UniformBufferObject ubo{};
		glm::mat4 rotX = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.001f, 0.001f, 0.001f));
		glm::mat4 oscillX = glm::rotate(glm::mat4(1), sin(time) * glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//glm::mat4 oscillZ = glm::rotate(glm::mat4(1), sin(time / 2) * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 steer = glm::mat4(1);
		if (glfwGetKey(window, GLFW_KEY_A)) {
			steer = glm::rotate(glm::mat4(1), glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		} else if (glfwGetKey(window, GLFW_KEY_D)) {
			steer = glm::rotate(glm::mat4(1), glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			steer *= glm::rotate(glm::mat4(1), glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		ubo.model = rotX * scale * oscillX * steer;
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
							   glm::vec3(0.0f, 0.0f, 0.0f),
							   glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f),
						swapChainExtent.width / (float) swapChainExtent.height,
						0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		return ubo;
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
					(currentTime - startTime).count();
					
		UniformBufferObject ubo = updatePosition(time);
		
		void* data;

		// Here is where you actually update your uniforms
		vkMapMemory(device, DS1.uniformBuffersMemory[0][currentImage], 0,
							sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS1.uniformBuffersMemory[0][currentImage]);
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