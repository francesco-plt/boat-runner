// Following the Vulkan Tutorial as shown in the enclose Assignment14.pdf, complete
// this Vulkan initialization sample. You can copy and past code from Example E08,
// or from other assginments such as Assignment0.cpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <cstring>
#include <set>
#include <map>
#include <optional>

const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;

struct QueueFamilyIndices
{
    /*
        std::optional is a wrapper that contains no
        value until you assign something to it. At
        any point you can query if it contains a value
        or not by calling its has_value() member function.
    */
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Assignment14
{
private:
    static constexpr char pName[] = "Assignment 13";
    GLFWwindow *window;
    VkInstance instance;
    VkResult result;
    VkSurfaceKHR surface;
    // physical and logical devices
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    // queue family indices
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    // command pool, command buffer
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    // required extensions
    const std::vector<const char *> deviceExtensions =
        {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    // swap chain
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    // images view
    std::vector<VkImageView> swapChainImageViews;

public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    // function to generate a GLFWwindow object
    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, pName, nullptr, nullptr);
    }

    void createInstance()
    {
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo createInfo{};
        uint32_t glfwExtensionCount;
        const char **glfwExtensions;

        // appinfo
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = pName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // createinfo
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        // for now we do not care about layers
        // createInfo.enabledLayerCount = 0;
        // ... all following layer related code is missing

        result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        // false if no suitable device extensions are found
        return requiredExtensions.empty();
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {

        // ensure that a device can present images to the surface we created
        QueueFamilyIndices indices = findQueueFamilies(device);

        // ensure that a device supports some extensions
        // deviceExtensions containts the extensions we need to be supported
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        // verifying swap chain support
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    void pickPhysicalDevice()
    {

        // Enumerate devices
        uint32_t deviceCount = 0;
        result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (result != VK_SUCCESS || deviceCount <= 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan Support!");
        }
        std::cout << deviceCount << " Physical Device(s) found \n\n";

        // now we'll put them in a vector and choose the best one
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto &device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                std::cout << "Suitable Physical Device Found ("
                          << device << ")\n\n";
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void getDeviceInfo()
    {
        // check device properties and features
        // Show device properties
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        std::cout << "\tAPI version: 0x" << std::hex << deviceProperties.apiVersion << "\n";
        std::cout << "\tDriver version: 0x" << std::hex << deviceProperties.driverVersion << "\n";
        std::cout << "\tVendor ID: 0x" << std::hex << deviceProperties.vendorID << "\n";
        std::cout << "\tDevice ID: 0x" << std::hex << deviceProperties.deviceID << "\n";
        std::cout << "\tPhysical Device Type: " << deviceProperties.deviceType << "\t";
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            std::cout << " (Discrete GPU)\n";
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            std::cout << " (Integrated GPU)\n";
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            std::cout << " (Virtual GPU)\n";
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
            std::cout << " (CPU)\n";
        std::cout << "\tDevice Name: " << deviceProperties.deviceName << "\n";

        // Show device features
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        std::cout << "\n\tPhysical Device Features:\n";
        std::cout << "\t\tgeometryShader = " << deviceFeatures.geometryShader << "\n";
        std::cout << "\t\ttessellationShader = " << deviceFeatures.tessellationShader << "\n";
        std::cout << "\t\tmultiDrawIndirect = " << deviceFeatures.multiDrawIndirect << "\n";
        std::cout << "\t\twideLines = " << deviceFeatures.wideLines << "\n";
        std::cout << "\t\tlargePoints = " << deviceFeatures.largePoints << "\n";
        std::cout << "\t\tmultiViewport = " << deviceFeatures.multiViewport << "\n";
        std::cout << "\t\tocclusionQueryPrecise = " << deviceFeatures.occlusionQueryPrecise << "\n";
        std::cout << "\t\tpipelineStatisticsQuery = " << deviceFeatures.pipelineStatisticsQuery << "\n";
        std::cout << "\t\tshaderFloat64 = " << deviceFeatures.shaderFloat64 << "\n";
        std::cout << "\t\tshaderInt64 = " << deviceFeatures.shaderInt64 << "\n";
        std::cout << "\t\tshaderInt16 = " << deviceFeatures.shaderInt16 << "\n";

        // Show device memory properties
        VkPhysicalDeviceMemoryProperties vpdmp;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &vpdmp);

        std::cout << "\n\tMemory Types: " << vpdmp.memoryTypeCount << "\n";
        for (unsigned int i = 0; i < vpdmp.memoryTypeCount; i++)
        {
            VkMemoryType vmt = vpdmp.memoryTypes[i];
            std::cout << "\t\tMemory: " << i << ":";
            if ((vmt.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
                std::cout << " DeviceLocal";
            if ((vmt.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
                std::cout << " HostVisible";
            if ((vmt.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
                std::cout << " HostCoherent";
            if ((vmt.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0)
                std::cout << " HostCached";
            if ((vmt.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0)
                std::cout << " LazilyAllocated";
            std::cout << "\n";
        }

        std::cout << "\n";
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;
        VkBool32 presentSupport;

        // what follows is the logic to find queue
        // familiy indices to populate struct with

        // we get those queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // we filter out the ones which supports the graphics bit
        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    void createLogicalDevice()
    {
        // Specifying the queues to be created
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        /*
            Vulkan lets you assign priorities to queues to influence
            the scheduling of command buffer execution using floating
            point numbers between 0.0 and 1.0. This is required even
            if there is only a single queue:
        */
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Specifying used device features
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Creating the logical device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        // enabling swapchains
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // skipping validation layers

        // That's it, we're now ready to instantiate the logical device with a call to
        // the appropriately named vkCreateDevice function.
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }
        std::cout << "Logical Device Created (" << device << ")\n\n";

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
        std::cout << "Command Pool Created (" << commandPool << ")\n\n";
    }

    void createCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        std::cout << "Command Buffer Created (" << commandBuffer << ")\n\n";
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        // the next line takes as input the physical device and surface
        // and returns the supported capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // querying supported surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // same shit but we're querying the supported presentation modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        // now the struct (details) holding all infos is ready

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        // Each VkSurfaceFormatKHR entry contains a format and a colorSpace member.

        // For the color space we'll use SRGB if it is available, because it
        // results in more accurate perceived colors (VK_FORMAT_B8G8R8A8_SRGB).
        // checking if it's available:
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        // if not available, let's settle with he first format that is specified
        return availableFormats[0];
    }

    /*
        The presentation mode is arguably the most important setting for the swap chain,
        because it represents the actual conditions for showing images to the screen.
        There are four possible modes available in Vulkan:
            1. VK_PRESENT_MODE_IMMEDIATE_KHR: images submitted by your application are transferred
            to the screen right away, which may result in tearing.
            2. VK_PRESENT_MODE_FIFO_KHR:  The swap chain is a queue where the display takes an
            image from the front of the queue when the display is refreshed and the program inserts
            rendered images at the back of the queue. If the queue is full then the program has to wait.
            This is most similar to vertical sync as found in modern games. The moment that the display
            is refreshed is known as "vertical blank".
            3. VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the
            application is late and the queue was empty at the last vertical blank. Instead of waiting
            for the next vertical blank, the image is transferred right away when it finally arrives.
            This may result in visible tearing.
            4. VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of
            blocking the application when the queue is full, the images that are already queued are
            simply replaced with the newer ones. This mode can be used to render frames as fast as
            possible while still avoiding tearing, resulting in fewer latency issues than standard
            vertical sync. This is commonly known as "triple buffering", although the existence of
            three buffers alone does not necessarily mean that the framerate is unlocked.
    */

    // only VK_PRESENT_MODE_FIFO_KHR is guaranteed to be present,
    // not let's prioritize VK_PRESENT_MODE_MAILBOX_KHR if present
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    /*
        Swap extent = resolution of the swap chain images = resolution of the window that we're drawing to.
        The range of the possible resolutions is defined in the VkSurfaceCapabilitiesKHR structure.

        GLFW uses two units when measuring sizes: pixels and screen coordinates.
        For example, the resolution {WIDTH, HEIGHT} that we specified earlier when creating the window is measured
        in screen coordinates. But Vulkan works with pixels, so the swap chain extent must be specified in pixels as
        well. Unfortunately, if you are using a high DPI display (like Apple's Retina display), screen coordinates
        don't correspond to pixels. Instead, due to the higher pixel density, the resolution of the window in pixel
        will be larger than the resolution in screen coordinates. So if Vulkan doesn't fix the swap extent for us,
        we can't just use the original {WIDTH, HEIGHT}. Instead, we must use glfwGetFramebufferSize to query the
        resolution of the window in pixel before matching it against the minimum and maximum image extent.
    */
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

            // The clamp function is used here to bound the values of width and height between
            // the allowed minimum and maximum extents that are supported by the implementation.
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        // how many images we would like to have in the swap chain.
        // it is recommended to request at least one more image than the minimum (+ 1):
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        // We should also make sure to not exceed the maximum number of images while doing this,
        // where 0 is a special value that means that there is no maximum:
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // filling information about the swap chain
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;                             // amount of layers each image consists of
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // what kind of operations we'll use the images in the swap chain for
                                                                     // since that we're going to render directly to them,
                                                                     // which means that they're used as color attachment
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE; // if we want to re-create the swap chain, we need to pass the old one here
                                                  // so that the driver knows which images to keep and which to free

        // finally creating this mf
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }
        std::cout << "Swap chain created "
                  << "(" << swapChain << ")" << std::endl;

        // code for getting the swap chain images
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        // storing the swap chain image format and extent
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews()
    {
        // fitting all the to be created views
        swapChainImageViews.resize(swapChainImages.size());

        // iterating over swap chain images
        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
            std::cout << "Image view created "
                      << "(" << swapChainImageViews[i] << ")\n\n"
                      << std::endl;
        }
    }

    /*
    the main part of this assignment is the initVulkan function
    It's articulated in the following steps:
    1. create the instance
    2. create the surface
    3. enumerate and pick the physical device
    4. create the logical device
    5. create the graphic and presentation queues
    6. create a command pool for the graphics queue
    7. create a command buffer for the graphics queue
*/
    void initVulkan()
    {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        getDeviceInfo();
        createLogicalDevice();
        createCommandPool();
        createCommandBuffer();
        createSwapChain();
        createImageViews();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }

    void cleanup()
    {
        // lots of cleanup code here

        // Unlike images, the image views were explicitly created by us,
        // so we need to add a similar loop to destroy them again at
        // the end of the program:
        for (auto imageView : swapChainImageViews)
        {
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
        std::cout << "Cleanup done. Exiting..." << std::endl;
    }
};

int main()
{
    Assignment14 app;
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