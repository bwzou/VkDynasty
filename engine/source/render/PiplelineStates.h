#pragma once

#include "RenderStates.h"

class PiplelineStates {
public:
    explicit PipelineStates(const RenderStates &states) 
        : renderStates(states) {}

    virtual ~PipelineStates() = default;

public:
    RenderStates renderStates;
};