#include <cstdint>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

namespace
{
    struct GLFWwindow;
}

class Application
{
public:
    void run();

private:
    void initVulkan();
    void mainLoop();
    void cleanup();

    GLFWwindow *window;
};