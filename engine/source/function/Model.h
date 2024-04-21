#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <array>
#include <unordered_map>

#include "../code/base/Geometry.h"
#include "Vertex.h"
#include "Material.h"


static VkFormat vertexAttributeFormat(size_t size) {
    switch (size) {
      case 1: return VK_FORMAT_R32_SFLOAT;
      case 2: return VK_FORMAT_R32G32_SFLOAT;
      case 3: return VK_FORMAT_R32G32B32_SFLOAT;
      case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
      default:
        break;
    }
    return VK_FORMAT_UNDEFINED;
}

struct Vertex {
    glm::vec3 a_position;
    glm::vec2 a_texCoord;
    glm::vec3 a_normal;
    glm::vec3 a_tangent;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, a_position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, a_texCoord);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, a_normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, a_tangent);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return a_position == other.a_position && a_texCoord == other.a_texCoord && a_normal == other.a_normal && a_tangent == other.a_tangent;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.a_position) ^
             (hash<glm::vec2>()(vertex.a_texCoord) << 1)) >> 1) ^
             (hash<glm::vec3>()(vertex.a_normal) << 1) ^ 
             (hash<glm::vec3>()(vertex.a_tangent) << 1);
        }
    };
}

struct ModelVertexes : VertexArray {
    PrimitiveType primitiveType;
    size_t primitiveCnt = 0;
    std::vector<Vertex> vertexes;
    std::vector<int32_t> indices;

    std::shared_ptr<VertexArrayObject> vao = nullptr;

    void UpdateVertexes() const {
        if (vao) {
            vao->updateVertexData(vertexesBuffer, vertexesBufferLength);
        }
    };

    void InitVertexes() {
        vertexSize = sizeof(Vertex);

        vertexesDesc.resize(4);
        vertexesDesc[0] = {3, sizeof(Vertex), offsetof(Vertex, a_position)};
        vertexesDesc[1] = {2, sizeof(Vertex), offsetof(Vertex, a_texCoord)};
        vertexesDesc[2] = {3, sizeof(Vertex), offsetof(Vertex, a_normal)};
        vertexesDesc[3] = {3, sizeof(Vertex), offsetof(Vertex, a_tangent)};

        vertexesBuffer = vertexes.empty() ? nullptr : (uint8_t *) &vertexes[0];
        vertexesBufferLength = vertexes.size() * sizeof(Vertex);

        indexBuffer = indices.empty() ? nullptr : &indices[0];
        indexBufferLength = indices.size() * sizeof(int32_t);
    }
};

struct ModelBase : ModelVertexes {
    BoundingBox aabb{};
    std::shared_ptr<Material> material = nullptr;

    virtual void resetStates() {
        vao = nullptr;
        if (material) {
            material->resetStates();
        }
    }
};

struct ModelPoints : ModelBase {
};

struct ModelLines : ModelBase {
};

struct ModelMesh : ModelBase {
};

struct ModelNode {
    glm::mat4 transform = glm::mat4(1.f);
    std::vector<ModelMesh> meshes;
    std::vector<ModelNode> children;
};

struct Model {
    std::string resourcePath;

    ModelNode rootNode;
    BoundingBox rootAABB;

    size_t meshCnt = 0;
    size_t primitiveCnt = 0;
    size_t vertexCnt = 0;

    glm::mat4 centeredTransform;

    void resetStates() {
        resetNodeStates(rootNode);
    }

    void resetNodeStates(ModelNode &node) {
        for (auto &mesh : node.meshes) {
        mesh.resetStates();
        }
        for (auto &childNode : node.children) {
        resetNodeStates(childNode);
        }
    }
};

struct DemoScene {
    std::shared_ptr<Model> model;
    ModelLines worldAxis;
    ModelPoints pointLight;
    ModelMesh floor;
    ModelMesh skybox;

    void resetStates() {
        if (model) { model->resetStates(); }
        worldAxis.resetStates();
        pointLight.resetStates();
        floor.resetStates();
        skybox.resetStates();
    }
};


