#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Particle.hpp"

#include <string>
#include <vector>

class FluidSimulator
{
public:
    void init(VkDevice device, VkPhysicalDevice physicalDevice,
              uint32_t computeQueueFamily, VkQueue computeQueue,
              uint32_t maxParticles);
    void cleanup();

    void reset();
    void setEmitterPosition(const glm::vec3 &position);
    void setEmitterDirection(const glm::vec3 &direction);

    void updateParams(float viscosity, float pressure, float flowRate, float gravity);
    void emitParticles(float deltaTime, VkCommandPool commandPool);
    void simulate(VkCommandBuffer cmd, float deltaTime);

    VkBuffer getParticleBuffer() const { return particleBuffers[currentBuffer]; }
    uint32_t getParticleCount() const { return particleCount; }
    uint32_t getMaxParticles() const { return maxParticles; }

private:
    void createDescriptorSetLayout();
    void createComputePipelines();
    void createStorageBuffers();
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSets();

    VkShaderModule createShaderModule(const std::vector<char> &code);
    static std::vector<char> readFile(const std::string &filename);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &memory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    uint32_t computeQueueFamily = 0;

    // Descriptor resources
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets; // One per buffer swap

    // Compute pipelines (2-pass SPH)
    VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;
    VkPipeline densityPipeline = VK_NULL_HANDLE;
    VkPipeline forcesIntegratePipeline = VK_NULL_HANDLE;

    // Particle storage buffers (double-buffered)
    static const int BUFFER_COUNT = 2;
    VkBuffer particleBuffers[BUFFER_COUNT] = {};
    VkDeviceMemory particleMemory[BUFFER_COUNT] = {};

    // Uniform buffer for simulation parameters
    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformMemory = VK_NULL_HANDLE;
    void *uniformMapped = nullptr;

    // Simulation state
    uint32_t maxParticles = 0;
    uint32_t particleCount = 0;
    int currentBuffer = 0;

    // Emitter state
    glm::vec3 emitterPosition = glm::vec3(0.0f, 100.0f, 0.0f);
    glm::vec3 emitterDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    float emitterRadius = 5.0f;
    float flowRate = 500.0f;
    float emissionAccumulator = 0.0f;

    // Simulation parameters
    SimulationParams params{};
};
