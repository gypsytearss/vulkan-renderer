#include <Application.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void Application::run()
{
    initVulkan();
    mainLoop();
    cleanup();
}

void Application::initVulkan()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Application::cleanup()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}
