#pragma once

#include "Vertex.hpp"
#include "Mesh.hpp"

#include <vector>

class SceneGeometry
{
public:
    // Generate a cylinder (pipe) mesh
    static Model createPipe(float radius, float height, int segments = 32);

    // Generate a ground plane mesh
    static Model createGroundPlane(float size, float y = 0.0f);

    // Generate a simple box for visualization
    static Model createBox(float width, float height, float depth);
};
