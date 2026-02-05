#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

#include <Application.hpp>

int main(int argc, char* argv[])
{
    // Build check flag: verify the executable builds and links correctly
    if (argc > 1 && std::strcmp(argv[1], "--build-check") == 0)
    {
        std::cout << "Build check passed" << std::endl;
        return EXIT_SUCCESS;
    }

    Application app;
    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}