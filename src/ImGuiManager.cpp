#include "ImGuiManager.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <stdexcept>

void ImGuiManager::init(GLFWwindow *window, VkInstance instance, VkPhysicalDevice physicalDevice,
                        VkDevice device, uint32_t graphicsFamily, VkQueue graphicsQueue,
                        VkRenderPass renderPass, uint32_t imageCount)
{
    this->device = device;

    createDescriptorPool();

    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Initialize ImGui for Vulkan
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.QueueFamily = graphicsFamily;
    initInfo.Queue = graphicsQueue;
    initInfo.DescriptorPool = descriptorPool;
    initInfo.MinImageCount = imageCount;
    initInfo.ImageCount = imageCount;

    ImGui_ImplVulkan_Init(&initInfo, renderPass);

    // Upload fonts
    ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGuiManager::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(sizeof(poolSizes) / sizeof(poolSizes[0]));
    poolInfo.pPoolSizes = poolSizes;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create ImGui descriptor pool");
    }
}

void ImGuiManager::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
}

void ImGuiManager::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderUI();
}

void ImGuiManager::render(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiManager::renderUI()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Fluid Simulation");

    ImGui::Text("Simulation");
    ImGui::Separator();

    ImGui::SliderFloat("Viscosity", &params.viscosity, 0.0f, 100.0f);
    ImGui::SliderFloat("Pressure", &params.pressure, 0.0f, 500.0f);
    ImGui::SliderFloat("Flow Rate", &params.flowRate, 0.0f, 1000.0f);

    ImGui::Spacing();

    ImGui::Checkbox("Gravity", &params.gravityEnabled);
    if (params.gravityEnabled)
    {
        ImGui::SliderFloat("Gravity Strength", &params.gravity, -200.0f, 0.0f);
    }

    ImGui::Spacing();

    if (ImGui::Checkbox("Paused", &params.paused))
    {
        // Toggle pause state
    }

    ImGui::Spacing();
    ImGui::Text("Rendering");
    ImGui::Separator();

    const char *modes[] = {"Sprites", "Screen-Space Fluid"};
    ImGui::Combo("Mode", &params.renderMode, modes, 2);

    ImGui::Spacing();
    ImGui::Text("Stats");
    ImGui::Separator();

    ImGui::Text("Particles: %u", particleCount);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    ImGui::Spacing();

    if (ImGui::Button("Reset Simulation"))
    {
        params.resetRequested = true;
    }

    ImGui::End();
}
