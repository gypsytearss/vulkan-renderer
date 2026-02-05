#include "GLBLoader.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include <stdexcept>

namespace
{
    glm::vec3 getVec3(const float *data, size_t index, size_t stride)
    {
        const float *ptr = data + index * (stride / sizeof(float));
        return glm::vec3(ptr[0], ptr[1], ptr[2]);
    }

    glm::vec2 getVec2(const float *data, size_t index, size_t stride)
    {
        const float *ptr = data + index * (stride / sizeof(float));
        return glm::vec2(ptr[0], ptr[1]);
    }

    const float *getAccessorData(const cgltf_accessor *accessor)
    {
        const cgltf_buffer_view *view = accessor->buffer_view;
        const uint8_t *data = static_cast<const uint8_t *>(view->buffer->data);
        return reinterpret_cast<const float *>(data + view->offset + accessor->offset);
    }

    size_t getAccessorStride(const cgltf_accessor *accessor)
    {
        if (accessor->buffer_view->stride != 0)
        {
            return accessor->buffer_view->stride;
        }
        return cgltf_calc_size(accessor->type, accessor->component_type);
    }

    uint32_t getIndex(const cgltf_accessor *accessor, size_t index)
    {
        const cgltf_buffer_view *view = accessor->buffer_view;
        const uint8_t *data = static_cast<const uint8_t *>(view->buffer->data) + view->offset + accessor->offset;

        switch (accessor->component_type)
        {
        case cgltf_component_type_r_8u:
            return data[index];
        case cgltf_component_type_r_16u:
            return reinterpret_cast<const uint16_t *>(data)[index];
        case cgltf_component_type_r_32u:
            return reinterpret_cast<const uint32_t *>(data)[index];
        default:
            return 0;
        }
    }
} // namespace

Model GLBLoader::loadGLB(const std::string &filepath)
{
    cgltf_options options = {};
    cgltf_data *data = nullptr;

    cgltf_result result = cgltf_parse_file(&options, filepath.c_str(), &data);
    if (result != cgltf_result_success)
    {
        throw std::runtime_error("Failed to parse GLB file: " + filepath);
    }

    result = cgltf_load_buffers(&options, data, filepath.c_str());
    if (result != cgltf_result_success)
    {
        cgltf_free(data);
        throw std::runtime_error("Failed to load GLB buffers: " + filepath);
    }

    Model model;

    for (size_t i = 0; i < data->meshes_count; i++)
    {
        const cgltf_mesh &gltfMesh = data->meshes[i];

        for (size_t j = 0; j < gltfMesh.primitives_count; j++)
        {
            const cgltf_primitive &primitive = gltfMesh.primitives[j];

            if (primitive.type != cgltf_primitive_type_triangles)
            {
                continue;
            }

            Mesh mesh;
            if (gltfMesh.name)
            {
                mesh.name = gltfMesh.name;
            }

            const cgltf_accessor *posAccessor = nullptr;
            const cgltf_accessor *normalAccessor = nullptr;
            const cgltf_accessor *texCoordAccessor = nullptr;

            for (size_t k = 0; k < primitive.attributes_count; k++)
            {
                const cgltf_attribute &attr = primitive.attributes[k];
                if (attr.type == cgltf_attribute_type_position)
                {
                    posAccessor = attr.data;
                }
                else if (attr.type == cgltf_attribute_type_normal)
                {
                    normalAccessor = attr.data;
                }
                else if (attr.type == cgltf_attribute_type_texcoord && attr.index == 0)
                {
                    texCoordAccessor = attr.data;
                }
            }

            if (!posAccessor)
            {
                continue;
            }

            size_t vertexCount = posAccessor->count;
            mesh.vertices.resize(vertexCount);

            const float *posData = getAccessorData(posAccessor);
            size_t posStride = getAccessorStride(posAccessor);

            const float *normalData = normalAccessor ? getAccessorData(normalAccessor) : nullptr;
            size_t normalStride = normalAccessor ? getAccessorStride(normalAccessor) : 0;

            const float *texCoordData = texCoordAccessor ? getAccessorData(texCoordAccessor) : nullptr;
            size_t texCoordStride = texCoordAccessor ? getAccessorStride(texCoordAccessor) : 0;

            for (size_t v = 0; v < vertexCount; v++)
            {
                mesh.vertices[v].pos = getVec3(posData, v, posStride);
                mesh.vertices[v].normal = normalData ? getVec3(normalData, v, normalStride) : glm::vec3(0.0f, 1.0f, 0.0f);
                mesh.vertices[v].texCoord = texCoordData ? getVec2(texCoordData, v, texCoordStride) : glm::vec2(0.0f, 0.0f);
            }

            if (primitive.indices)
            {
                size_t indexCount = primitive.indices->count;
                mesh.indices.resize(indexCount);
                for (size_t idx = 0; idx < indexCount; idx++)
                {
                    mesh.indices[idx] = getIndex(primitive.indices, idx);
                }
            }
            else
            {
                mesh.indices.resize(vertexCount);
                for (size_t idx = 0; idx < vertexCount; idx++)
                {
                    mesh.indices[idx] = static_cast<uint32_t>(idx);
                }
            }

            model.meshes.push_back(std::move(mesh));
        }
    }

    cgltf_free(data);

    model.flatten();

    return model;
}
