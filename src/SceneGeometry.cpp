#include "SceneGeometry.hpp"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Model SceneGeometry::createPipe(float radius, float height, int segments)
{
    Model model;

    // Generate cylinder vertices
    for (int i = 0; i <= segments; i++)
    {
        float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(segments);
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);

        float x = radius * cosTheta;
        float z = radius * sinTheta;

        // Normal pointing outward
        glm::vec3 normal = glm::normalize(glm::vec3(cosTheta, 0.0f, sinTheta));

        // Bottom vertex
        Vertex bottomVertex{};
        bottomVertex.pos = glm::vec3(x, 0.0f, z);
        bottomVertex.normal = normal;
        bottomVertex.texCoord = glm::vec2(static_cast<float>(i) / static_cast<float>(segments), 0.0f);
        model.allVertices.push_back(bottomVertex);

        // Top vertex
        Vertex topVertex{};
        topVertex.pos = glm::vec3(x, height, z);
        topVertex.normal = normal;
        topVertex.texCoord = glm::vec2(static_cast<float>(i) / static_cast<float>(segments), 1.0f);
        model.allVertices.push_back(topVertex);
    }

    // Generate indices for the cylinder wall
    for (int i = 0; i < segments; i++)
    {
        uint32_t bottomLeft = i * 2;
        uint32_t topLeft = i * 2 + 1;
        uint32_t bottomRight = (i + 1) * 2;
        uint32_t topRight = (i + 1) * 2 + 1;

        // First triangle
        model.allIndices.push_back(bottomLeft);
        model.allIndices.push_back(bottomRight);
        model.allIndices.push_back(topLeft);

        // Second triangle
        model.allIndices.push_back(topLeft);
        model.allIndices.push_back(bottomRight);
        model.allIndices.push_back(topRight);
    }

    // Add bottom cap
    uint32_t bottomCenterIdx = static_cast<uint32_t>(model.allVertices.size());
    Vertex bottomCenter{};
    bottomCenter.pos = glm::vec3(0.0f, 0.0f, 0.0f);
    bottomCenter.normal = glm::vec3(0.0f, -1.0f, 0.0f);
    bottomCenter.texCoord = glm::vec2(0.5f, 0.5f);
    model.allVertices.push_back(bottomCenter);

    for (int i = 0; i <= segments; i++)
    {
        float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);

        Vertex v{};
        v.pos = glm::vec3(x, 0.0f, z);
        v.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        v.texCoord = glm::vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta));
        model.allVertices.push_back(v);
    }

    for (int i = 0; i < segments; i++)
    {
        model.allIndices.push_back(bottomCenterIdx);
        model.allIndices.push_back(bottomCenterIdx + 1 + i + 1);
        model.allIndices.push_back(bottomCenterIdx + 1 + i);
    }

    // Add top cap
    uint32_t topCenterIdx = static_cast<uint32_t>(model.allVertices.size());
    Vertex topCenter{};
    topCenter.pos = glm::vec3(0.0f, height, 0.0f);
    topCenter.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    topCenter.texCoord = glm::vec2(0.5f, 0.5f);
    model.allVertices.push_back(topCenter);

    for (int i = 0; i <= segments; i++)
    {
        float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);

        Vertex v{};
        v.pos = glm::vec3(x, height, z);
        v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v.texCoord = glm::vec2(0.5f + 0.5f * std::cos(theta), 0.5f + 0.5f * std::sin(theta));
        model.allVertices.push_back(v);
    }

    for (int i = 0; i < segments; i++)
    {
        model.allIndices.push_back(topCenterIdx);
        model.allIndices.push_back(topCenterIdx + 1 + i);
        model.allIndices.push_back(topCenterIdx + 1 + i + 1);
    }

    return model;
}

Model SceneGeometry::createGroundPlane(float size, float y)
{
    Model model;

    float half = size / 2.0f;

    // Four corners of the plane
    Vertex v0{}, v1{}, v2{}, v3{};

    v0.pos = glm::vec3(-half, y, -half);
    v0.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    v0.texCoord = glm::vec2(0.0f, 0.0f);

    v1.pos = glm::vec3(half, y, -half);
    v1.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    v1.texCoord = glm::vec2(1.0f, 0.0f);

    v2.pos = glm::vec3(half, y, half);
    v2.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    v2.texCoord = glm::vec2(1.0f, 1.0f);

    v3.pos = glm::vec3(-half, y, half);
    v3.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    v3.texCoord = glm::vec2(0.0f, 1.0f);

    model.allVertices = {v0, v1, v2, v3};

    // Two triangles
    model.allIndices = {0, 1, 2, 0, 2, 3};

    return model;
}

Model SceneGeometry::createBox(float width, float height, float depth)
{
    Model model;

    float hw = width / 2.0f;
    float hh = height / 2.0f;
    float hd = depth / 2.0f;

    // Front face
    model.allVertices.push_back({{-hw, -hh, hd}, {0, 0, 1}, {0, 0}});
    model.allVertices.push_back({{hw, -hh, hd}, {0, 0, 1}, {1, 0}});
    model.allVertices.push_back({{hw, hh, hd}, {0, 0, 1}, {1, 1}});
    model.allVertices.push_back({{-hw, hh, hd}, {0, 0, 1}, {0, 1}});

    // Back face
    model.allVertices.push_back({{hw, -hh, -hd}, {0, 0, -1}, {0, 0}});
    model.allVertices.push_back({{-hw, -hh, -hd}, {0, 0, -1}, {1, 0}});
    model.allVertices.push_back({{-hw, hh, -hd}, {0, 0, -1}, {1, 1}});
    model.allVertices.push_back({{hw, hh, -hd}, {0, 0, -1}, {0, 1}});

    // Top face
    model.allVertices.push_back({{-hw, hh, hd}, {0, 1, 0}, {0, 0}});
    model.allVertices.push_back({{hw, hh, hd}, {0, 1, 0}, {1, 0}});
    model.allVertices.push_back({{hw, hh, -hd}, {0, 1, 0}, {1, 1}});
    model.allVertices.push_back({{-hw, hh, -hd}, {0, 1, 0}, {0, 1}});

    // Bottom face
    model.allVertices.push_back({{-hw, -hh, -hd}, {0, -1, 0}, {0, 0}});
    model.allVertices.push_back({{hw, -hh, -hd}, {0, -1, 0}, {1, 0}});
    model.allVertices.push_back({{hw, -hh, hd}, {0, -1, 0}, {1, 1}});
    model.allVertices.push_back({{-hw, -hh, hd}, {0, -1, 0}, {0, 1}});

    // Right face
    model.allVertices.push_back({{hw, -hh, hd}, {1, 0, 0}, {0, 0}});
    model.allVertices.push_back({{hw, -hh, -hd}, {1, 0, 0}, {1, 0}});
    model.allVertices.push_back({{hw, hh, -hd}, {1, 0, 0}, {1, 1}});
    model.allVertices.push_back({{hw, hh, hd}, {1, 0, 0}, {0, 1}});

    // Left face
    model.allVertices.push_back({{-hw, -hh, -hd}, {-1, 0, 0}, {0, 0}});
    model.allVertices.push_back({{-hw, -hh, hd}, {-1, 0, 0}, {1, 0}});
    model.allVertices.push_back({{-hw, hh, hd}, {-1, 0, 0}, {1, 1}});
    model.allVertices.push_back({{-hw, hh, -hd}, {-1, 0, 0}, {0, 1}});

    // Indices for all 6 faces (2 triangles each)
    for (uint32_t face = 0; face < 6; face++)
    {
        uint32_t base = face * 4;
        model.allIndices.push_back(base + 0);
        model.allIndices.push_back(base + 1);
        model.allIndices.push_back(base + 2);
        model.allIndices.push_back(base + 0);
        model.allIndices.push_back(base + 2);
        model.allIndices.push_back(base + 3);
    }

    return model;
}
