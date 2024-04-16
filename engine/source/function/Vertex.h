#pragma once

#include <vector>
#include <memory>
#include "code/base/GLMInc.h"


class VertexArrayObject {
public:
    virtual int getId() const = 0;
    virtual void updateVertexData(void *data, size_t length) = 0;
};


struct VertexAttributeDesc {
    size_t size;
    size_t stride;
    size_t offset;
};


struct VertexArray {
    size_t vertexSize = 0;
    std::vector<VertexAttributeDesc> vertexesDesc;

    uint8_t *vertexesBuffer = nullptr;
    size_t vertexesBufferLength = 0;

    int32_t *indexBuffer = nullptr;
    size_t indexBufferLength = 0;
};