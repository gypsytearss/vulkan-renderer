#pragma once

#include <string>
#include "Mesh.hpp"

class GLBLoader
{
public:
    static Model loadGLB(const std::string &filepath);
};
