#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

class ParticleRenderer
{
public:
    struct PushConstants
    {
        glm::mat4 view;
        glm::mat4 proj;
        float particleRadius;
        float padding1;
        float padding2;
        float padding3;
    };

    void init(VkDevice device, VkRenderPass renderPass, VkExtent2D extent);
    void cleanup();

    void render(VkCommandBuffer commandBuffer, VkBuffer particleBuffer,
                uint32_t particleCount, const glm::mat4 &view, const glm::mat4 &proj);

private:
    void createPipeline();

    VkShaderModule createShaderModule(const std::vector<char> &code);
    static std::vector<char> readFile(const std::string &filename);

    VkDevice device = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkExtent2D extent{};

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
};
