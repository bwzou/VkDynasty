#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../code/base/Geometry.h"
#include "Vertex.h"
#include "Material.h"

struct Vertex {
    glm::vec3 a_position;
    glm::vec2 a_texCoord;
    glm::vec3 a_normal;
    glm::vec3 a_tangent;
};

struct ModelVertexes : VertexArray {
    PrimitiveType primitiveType;
    size_t primitiveCnt = 0;
    std::vector<Vertex> vertexes;
    std::vector<int32_t> indices;

        
}

