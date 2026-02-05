#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "Vertex.hpp"

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::string name;
};

struct Model
{
    std::vector<Mesh> meshes;
    std::vector<Vertex> allVertices;
    std::vector<uint32_t> allIndices;

    void flatten()
    {
        allVertices.clear();
        allIndices.clear();

        uint32_t vertexOffset = 0;
        for (const auto &mesh : meshes)
        {
            allVertices.insert(allVertices.end(), mesh.vertices.begin(), mesh.vertices.end());

            for (uint32_t index : mesh.indices)
            {
                allIndices.push_back(index + vertexOffset);
            }
            vertexOffset += static_cast<uint32_t>(mesh.vertices.size());
        }
    }
};
