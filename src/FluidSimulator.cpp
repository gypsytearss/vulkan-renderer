#include "FluidSimulator.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <random>
#include <stdexcept>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders"
#endif

void FluidSimulator::init(VkDevice device, VkPhysicalDevice physicalDevice,
                          uint32_t computeQueueFamily, VkQueue computeQueue,
                          uint32_t maxParticles)
{
    this->device = device;
    this->physicalDevice = physicalDevice;
    this->computeQueueFamily = computeQueueFamily;
    this->computeQueue = computeQueue;
    this->maxParticles = maxParticles;

    // Initialize default simulation parameters
    params.deltaTime = 0.016f;
    params.smoothingRadius = 6.0f;   // Larger = smoother forces
    params.restDensity = 1000.0f;
    params.gasConstant = 50.0f;      // Lower = less explosive pressure
    params.viscosity = 100.0f;       // Higher = more damping
    params.gravity = -98.0f;
    params.particleCount = 0;
    params.particleRadius = 2.0f;    // Larger = more spacing

    // Boundary (simulation domain)
    params.boundaryMinX = -100.0f;
    params.boundaryMaxX = 100.0f;
    params.boundaryMinY = 0.0f;
    params.boundaryMaxY = 200.0f;
    params.boundaryMinZ = -100.0f;
    params.boundaryMaxZ = 100.0f;
    params.boundaryDamping = 0.3f;

    createDescriptorSetLayout();
    createStorageBuffers();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createComputePipelines();
}

void FluidSimulator::cleanup()
{
    if (device == VK_NULL_HANDLE)
        return;

    vkDestroyPipeline(device, densityPipeline, nullptr);
    vkDestroyPipeline(device, forcesIntegratePipeline, nullptr);
    vkDestroyPipelineLayout(device, computePipelineLayout, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    if (uniformMapped)
    {
        vkUnmapMemory(device, uniformMemory);
    }
    vkDestroyBuffer(device, uniformBuffer, nullptr);
    vkFreeMemory(device, uniformMemory, nullptr);

    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        vkDestroyBuffer(device, particleBuffers[i], nullptr);
        vkFreeMemory(device, particleMemory[i], nullptr);
    }
}

void FluidSimulator::createDescriptorSetLayout()
{
    // Binding 0: Particle buffer (read)
    // Binding 1: Particle buffer (write)
    // Binding 2: Uniform buffer (simulation params)
    VkDescriptorSetLayoutBinding bindings[3] = {};

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute descriptor set layout");
    }
}

void FluidSimulator::createStorageBuffers()
{
    VkDeviceSize bufferSize = sizeof(Particle) * maxParticles;

    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     particleBuffers[i], particleMemory[i]);
    }
}

void FluidSimulator::createUniformBuffer()
{
    VkDeviceSize bufferSize = sizeof(SimulationParams);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniformBuffer, uniformMemory);

    vkMapMemory(device, uniformMemory, 0, bufferSize, 0, &uniformMapped);
}

void FluidSimulator::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = BUFFER_COUNT * 2; // 2 storage buffers per set
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = BUFFER_COUNT;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = BUFFER_COUNT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute descriptor pool");
    }
}

void FluidSimulator::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(BUFFER_COUNT, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = BUFFER_COUNT;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(BUFFER_COUNT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate compute descriptor sets");
    }

    // Update descriptor sets
    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        int readBuffer = i;
        int writeBuffer = (i + 1) % BUFFER_COUNT;

        VkDescriptorBufferInfo readBufferInfo{};
        readBufferInfo.buffer = particleBuffers[readBuffer];
        readBufferInfo.offset = 0;
        readBufferInfo.range = sizeof(Particle) * maxParticles;

        VkDescriptorBufferInfo writeBufferInfo{};
        writeBufferInfo.buffer = particleBuffers[writeBuffer];
        writeBufferInfo.offset = 0;
        writeBufferInfo.range = sizeof(Particle) * maxParticles;

        VkDescriptorBufferInfo uniformBufferInfo{};
        uniformBufferInfo.buffer = uniformBuffer;
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = sizeof(SimulationParams);

        VkWriteDescriptorSet writes[3] = {};

        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = descriptorSets[i];
        writes[0].dstBinding = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[0].descriptorCount = 1;
        writes[0].pBufferInfo = &readBufferInfo;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = descriptorSets[i];
        writes[1].dstBinding = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[1].descriptorCount = 1;
        writes[1].pBufferInfo = &writeBufferInfo;

        writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[2].dstSet = descriptorSets[i];
        writes[2].dstBinding = 2;
        writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[2].descriptorCount = 1;
        writes[2].pBufferInfo = &uniformBufferInfo;

        vkUpdateDescriptorSets(device, 3, writes, 0, nullptr);
    }
}

void FluidSimulator::createComputePipelines()
{
    // Create pipeline layout
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline layout");
    }

    // Load shader modules
    auto densityCode = readFile(std::string(SHADER_DIR) + "/sph_density.comp.spv");
    auto forcesIntegrateCode = readFile(std::string(SHADER_DIR) + "/sph_forces_integrate.comp.spv");

    VkShaderModule densityModule = createShaderModule(densityCode);
    VkShaderModule forcesIntegrateModule = createShaderModule(forcesIntegrateCode);

    // Create pipelines
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = computePipelineLayout;

    // Density pipeline
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = densityModule;
    pipelineInfo.stage.pName = "main";

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &densityPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create density compute pipeline");
    }

    // Forces + Integrate pipeline
    pipelineInfo.stage.module = forcesIntegrateModule;
    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &forcesIntegratePipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create forces+integrate compute pipeline");
    }

    // Clean up shader modules
    vkDestroyShaderModule(device, densityModule, nullptr);
    vkDestroyShaderModule(device, forcesIntegrateModule, nullptr);
}

void FluidSimulator::reset()
{
    particleCount = 0;
    emissionAccumulator = 0.0f;
    currentBuffer = 0;
}

void FluidSimulator::setEmitterPosition(const glm::vec3 &position)
{
    emitterPosition = position;
}

void FluidSimulator::setEmitterDirection(const glm::vec3 &direction)
{
    emitterDirection = glm::normalize(direction);
}

void FluidSimulator::updateParams(float viscosity, float pressure, float newFlowRate, float gravity)
{
    params.viscosity = viscosity;
    params.gasConstant = pressure;
    params.gravity = gravity;
    flowRate = newFlowRate;
}

void FluidSimulator::emitParticles(float deltaTime, VkCommandPool commandPool)
{
    // Calculate how many particles to emit
    emissionAccumulator += flowRate * deltaTime;
    int particlesToEmit = static_cast<int>(emissionAccumulator);
    emissionAccumulator -= particlesToEmit;

    // Clamp to available space
    int availableSpace = static_cast<int>(maxParticles - particleCount);
    particlesToEmit = std::min(particlesToEmit, availableSpace);

    if (particlesToEmit <= 0)
        return;

    // Generate new particles
    std::vector<Particle> newParticles(particlesToEmit);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float initialSpeed = 20.0f;

    for (int i = 0; i < particlesToEmit; i++)
    {
        // Random position within emitter radius
        float angle = dist(gen) * 3.14159f;
        float radius = std::sqrt(std::abs(dist(gen))) * emitterRadius;
        float offsetX = radius * std::cos(angle);
        float offsetZ = radius * std::sin(angle);

        glm::vec3 pos = emitterPosition + glm::vec3(offsetX, 0.0f, offsetZ);
        glm::vec3 vel = emitterDirection * initialSpeed;

        // Add some randomness to velocity
        vel += glm::vec3(dist(gen), dist(gen), dist(gen)) * 2.0f;

        newParticles[i].position = glm::vec4(pos, 0.0f);
        newParticles[i].velocity = glm::vec4(vel, 0.0f);
    }

    // Create staging buffer and upload
    VkDeviceSize uploadSize = sizeof(Particle) * particlesToEmit;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    createBuffer(uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingMemory);

    void *data;
    vkMapMemory(device, stagingMemory, 0, uploadSize, 0, &data);
    memcpy(data, newParticles.data(), uploadSize);
    vkUnmapMemory(device, stagingMemory);

    // Copy to both particle buffers at the current particle count offset
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = sizeof(Particle) * particleCount;
    copyRegion.size = uploadSize;

    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        vkCmdCopyBuffer(cmdBuffer, stagingBuffer, particleBuffers[i], 1, &copyRegion);
    }

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(computeQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &cmdBuffer);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    particleCount += particlesToEmit;
}

void FluidSimulator::simulate(VkCommandBuffer cmd, float deltaTime)
{
    if (particleCount == 0)
        return;

    // Update simulation parameters
    params.deltaTime = deltaTime;
    params.particleCount = particleCount;
    memcpy(uniformMapped, &params, sizeof(SimulationParams));

    uint32_t groupCount = (particleCount + 255) / 256;

    // Bind descriptor set for current frame
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                            computePipelineLayout, 0, 1, &descriptorSets[currentBuffer], 0, nullptr);

    // Memory barrier helper
    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // Pass 1: Compute density and pressure (in-place update)
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, densityPipeline);
    vkCmdDispatch(cmd, groupCount, 1, 1);

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

    // Pass 2: Forces + Integration (reads density from pass 1, writes to output buffer)
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, forcesIntegratePipeline);
    vkCmdDispatch(cmd, groupCount, 1, 1);

    // Final barrier for graphics read
    VkBufferMemoryBarrier bufferBarrier{};
    bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.buffer = particleBuffers[(currentBuffer + 1) % BUFFER_COUNT];
    bufferBarrier.offset = 0;
    bufferBarrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);

    // Swap buffers
    currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;
}

VkShaderModule FluidSimulator::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }

    return shaderModule;
}

std::vector<char> FluidSimulator::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void FluidSimulator::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                                  VkDeviceMemory &memory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory");
    }

    vkBindBufferMemory(device, buffer, memory, 0);
}

uint32_t FluidSimulator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}
