#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vertex.hpp"
#include "Mesh.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

struct PushConstants
{
    glm::mat4 mvp;
};

struct QueueFamilyIndices
{
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

class Application
{
public:
    void run();

private:
    void initVulkan();
    void mainLoop();
    void cleanup();

    void createVkInstance();
    void pickPhysicalDevice();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice &device);
    bool checkRequiredExtensionSupport(VkPhysicalDevice &device);
    void createLogicalDevice();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void createCommandBuffers();
    void createSyncObjects();
    void createDepthResources();
    void loadModel();

    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage &image, VkDeviceMemory &imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    void drawFrame();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    VkShaderModule createShaderModule(const std::vector<char> &code);
    static std::vector<char> readFile(const std::string &filename);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createSurface();

    void initIndices();

    GLFWwindow *window;

    VkInstance vkInstance;
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;

    VkSwapchainKHR vkSwapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat vkSwapChainImageFormat;
    VkExtent2D vkSwapChainExtent;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSurfaceKHR vkSurface;

    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t currentFrame = 0;

    Model model;
    uint32_t indexCount = 0;
};