#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Application
{
public:
    void run();

private:
    void initVulkan();
    void mainLoop();
    void cleanup();

    void createVkInstance();

    GLFWwindow *window;
    VkInstance vkInstance;
};