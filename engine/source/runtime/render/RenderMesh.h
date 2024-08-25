
#pragma once

#include <vulkan/vulkan.h>
#include "runtime/code/base/GLMInc.h"

// 测试专用
struct Vertex 
{
        glm::vec2 pos;
        glm::vec3 color;
        // glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription() 
        {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;  // float: VK_FORMAT_R32_SFLOAT  vec2: VK_FORMAT_R32G32_SFLOAT  vec3: VK_FORMAT_R32G32B32_SFLOAT  vec4: VK_FORMAT_R32G32B32A32_SFLOAT
            attributeDescriptions[0].offset = offsetof(Vertex, pos);  

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color); 

            // attributeDescriptions[2].binding = 0;
            // attributeDescriptions[2].location = 2;
            // attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            // attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const 
        {
            // return pos == other.pos && color == other.color && texCoord == other.texCoord;
            return pos == other.pos && color == other.color;
        }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
        return ((hash<glm::vec2>()(vertex.pos) ^
            (hash<glm::vec3>()(vertex.color) << 1)) >> 1);
        }
    };
}

namespace DynastyEngine 
{
    struct VertexAttributeDescription
    {
        size_t size;
        size_t stride;
        size_t offset;
    };

    struct VulkanMeshVertexPostition
    {
        glm::vec3 position;
    };

    struct VulkanMeshVertexTexture
    {
        glm::vec3 position;
    };

    struct MeshVertex
	{
		glm::vec3 position;
        glm::vec2 texcoord;
		glm::vec3 normal;
		glm::vec3 tangent;

		// int entityID;

        static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions()
        {
            std::array<VkVertexInputBindingDescription, 1> bindingDescriptions {};

            bindingDescriptions[0].binding   = 0;
            bindingDescriptions[0].stride    = sizeof(MeshVertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  
            return bindingDescriptions;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions {};

            attributeDescriptions[0].binding  = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset   = offsetof(MeshVertex, position);

            attributeDescriptions[1].binding  = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[1].offset   = offsetof(MeshVertex, texcoord);

            attributeDescriptions[2].binding  = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[2].offset   = offsetof(MeshVertex, normal);

            attributeDescriptions[3].binding  = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset   = offsetof(MeshVertex, tangent);

            return attributeDescriptions;
        }
	};

    struct NormalVertex : MeshVertex
    {

    };

	struct SkinnedVertex : MeshVertex
	{
        glm::vec4 weights;
        glm::ivec4 boneIDs;

        static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions()
        {
            std::array<VkVertexInputBindingDescription, 1> bindingDescriptions {};

            bindingDescriptions[0].binding   = 0;
            bindingDescriptions[0].stride    = sizeof(SkinnedVertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  
            return bindingDescriptions;
        }

        static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions {};

            attributeDescriptions[0].binding  = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset   = offsetof(SkinnedVertex, position);

            attributeDescriptions[1].binding  = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[1].offset   = offsetof(SkinnedVertex, texcoord);

            attributeDescriptions[2].binding  = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[2].offset   = offsetof(SkinnedVertex, normal);

            attributeDescriptions[3].binding  = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset   = offsetof(SkinnedVertex, tangent);

            attributeDescriptions[3].binding  = 0;
            attributeDescriptions[3].location = 4;
            attributeDescriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset   = offsetof(SkinnedVertex, weights);

            attributeDescriptions[3].binding  = 0;
            attributeDescriptions[3].location = 5;
            attributeDescriptions[3].format   = VK_FORMAT_R32G32B32A32_SINT;
            attributeDescriptions[3].offset   = offsetof(SkinnedVertex, boneIDs);

            return attributeDescriptions;
        }
	};

    // struct Vertex 
    // {
    //     glm::vec2 pos;
    //     glm::vec3 color;
    //     // glm::vec2 texCoord;

    //     static VkVertexInputBindingDescription getBindingDescription() 
    //     {
    //         VkVertexInputBindingDescription bindingDescription{};
    //         bindingDescription.binding = 0;
    //         bindingDescription.stride = sizeof(Vertex);
    //         bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    //         return bindingDescription;
    //     }

    //     static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
    //     {
    //         std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

    //         attributeDescriptions[0].binding = 0;
    //         attributeDescriptions[0].location = 0;
    //         attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;  // float: VK_FORMAT_R32_SFLOAT  vec2: VK_FORMAT_R32G32_SFLOAT  vec3: VK_FORMAT_R32G32B32_SFLOAT  vec4: VK_FORMAT_R32G32B32A32_SFLOAT
    //         attributeDescriptions[0].offset = offsetof(Vertex, pos);  

    //         attributeDescriptions[1].binding = 0;
    //         attributeDescriptions[1].location = 1;
    //         attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    //         attributeDescriptions[1].offset = offsetof(Vertex, color); 

    //         // attributeDescriptions[2].binding = 0;
    //         // attributeDescriptions[2].location = 2;
    //         // attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    //         // attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    //         return attributeDescriptions;
    //     }

    //     bool operator==(const Vertex& other) const 
    //     {
    //         // return pos == other.pos && color == other.color && texCoord == other.texCoord;
    //         return pos == other.pos && color == other.color;
    //     }
    // };

    struct UniformBufferObject 
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    // const std::vector<Vertex> vertices = 
    // {
    //     {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    //     {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    //     {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

    //     {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    //     {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    //     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    //     {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    // };

    // const std::vector<uint16_t> indices = 
    // {
    //     0, 1, 2, 2, 3, 0,
    //     4, 5, 6, 6, 7, 4
    // };

    const std::vector<Vertex> vertices = {
        // {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        // {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        // {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        {{-50.5f, -50.5f}, {1.0f, 0.0f, 0.0f}},
        {{50.5f, -50.5f}, {0.0f, 1.0f, 0.0f}},
        {{50.5f, 50.5f}, {0.0f, 0.0f, 1.0f}},
        {{-50.5f, 50.5f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0
    };
}