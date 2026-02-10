#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class ImGuiManager
{
public:
    void init(GLFWwindow *window, VkInstance instance, VkPhysicalDevice physicalDevice,
              VkDevice device, uint32_t graphicsFamily, VkQueue graphicsQueue,
              VkRenderPass renderPass, uint32_t imageCount);
    void cleanup();

    void newFrame();
    void render(VkCommandBuffer commandBuffer);

    // Simulation parameters (exposed for UI)
    struct SimulationParams
    {
        float viscosity = 50.0f;
        float pressure = 200.0f;
        float flowRate = 500.0f;
        int renderMode = 0; // 0 = Sprites, 1 = Screen-Space Fluid
        bool paused = false;
    };

    SimulationParams &getParams() { return params; }

private:
    void createDescriptorPool();
    void renderUI();

    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    SimulationParams params;

    // Stats
    uint32_t particleCount = 0;

public:
    void setParticleCount(uint32_t count) { particleCount = count; }
};
