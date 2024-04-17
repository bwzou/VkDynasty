#pragma once

#include "RenderStates.h"

class PipelineStates {
public:
    explicit PipelineStates(const RenderStates &states) 
        : renderStates(states) {}

    virtual ~PipelineStates() = default;

public:
    RenderStates renderStates;
};