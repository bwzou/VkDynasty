#include "PipelineStates.h"


void PipelineStates::create(VkPipelineVertexInputStateCreateInfo &vertexInputInfo,
                            ShaderProgram *program,
                            VkRenderPass &renderPass,
                            VkSampleCountFlagBits sampleCount) {
    size_t cacheKey = getPipelineCacheKey(program, renderPass, sampleCount);
    auto it = pipelineCache_.find(cacheKey);
    if (it != pipelineCache_.end()) {
        currPipeline_ = it->second;
    } else {
        currPipeline_ = createGraphicsPipeline(vertexInputInfo, program, renderPass, sampleCount);
        pipelineCache_[cacheKey] = currPipeline_;
    }
}

PipelineContainer PipelineStates::createGraphicsPipeline(VkPipelineVertexInputStateCreateInfo &vertexInputInfo,
                                             ShaderProgram *program,
                                             VkRenderPass &renderPass,
                                             VkSampleCountFlagBits sampleCount) {
    PipelineContainer ret{};

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // 将这个视口和剪切形矩形组合成一个视口状态
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // 光栅化从顶点着色器中获取由顶点塑造的几何体，并将其转化为片段，由片段着色器进行着色。
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // depthClampEnable被设置为VK_TRUE，那么超出近平面和远平面的片段会被夹住，而不是丢弃
    // 使用这个需要启用一个GPU功能。
    rasterizer.depthClampEnable = VK_FALSE;
    // 如果rasterizerDiscardEnable被设置为VK_TRUE，那么几何体就不会通过光栅化器阶段。
    // 这基本上是禁用任何输出到帧缓冲区的功能。
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // polygonMode 决定了如何生成几何体的片段。有以下模式可供选择。
    // VK_POLYGON_MODE_FILL：用片段填充多边形的区域。
    // VK_POLYGON_MODE_LINE：多边形的边缘被绘制成线。
    // VK_POLYGON_MODE_POINT：多边形顶点被画成点。
    rasterizer.polygonMode = cvtPolygonMode(renderStates.polygonMode);
    // 它以片段的数量来描述线条的厚度。
    // 任何比1.0f厚的线都需要你启用wideLinesGPU功能。
    rasterizer.lineWidth = renderStates.lineWidth;
    // cullMode 变量确定要使用的面剔除类型。您可以禁用剔除、剔除正面、剔除背面或两者。
    rasterizer.cullMode = renderStates.cullFace ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    // frontFace 变量指定被视为正面的面的顶点顺序，可以是顺时针或逆时针。
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // 通过添加一个常量值或根据片段的斜率对其进行偏置来改变深度值
    rasterizer.depthBiasEnable = VK_FALSE;

    // VkPipelineMultisampleStateCreateInfo结构配置了多重采样，这是执行抗锯齿的方法之一
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    multisampling.rasterizationSamples = sampleCount;

    // 在片段着色器返回颜色后，需要将其与已经存在于帧缓冲器中的颜色结合起来。这种转换被称为颜色混合，有两种方法可以做到这一点。
    // 将新旧值混合，产生最终的颜色
    // 使用位操作将新旧值结合起来
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = renderStates.depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = renderStates.depthMask ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = cvtDepthFunc(renderStates.depthFunc);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    // 每个附加帧缓冲区的配置   
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = renderStates.blend  ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.colorBlendOp = cvtBlendFunction(renderStates.blendParams.blendFuncRgb);
    colorBlendAttachment.srcColorBlendFactor = cvtBlendFactor(renderStates.blendParams.blendSrcRgb);
    colorBlendAttachment.dstColorBlendFactor = cvtBlendFactor(renderStates.blendParams.blendDstRgb);
    colorBlendAttachment.alphaBlendOp = cvtBlendFunction(renderStates.blendParams.blendFuncAlpha);
    colorBlendAttachment.srcAlphaBlendFactor = cvtBlendFactor(renderStates.blendParams.blendSrcAlpha);
    colorBlendAttachment.dstAlphaBlendFactor = cvtBlendFactor(renderStates.blendParams.blendDstAlpha);

    // 第二个结构引用了所有帧缓冲器的结构数组，并允许你设置混合常数，你可以在上述计算中作为混合系数使用。
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // 我们在前面的结构中指定的有限数量的状态可以在不重新创建管道的情况下实际改变。例如，视口的大小、线宽和混合常数。如果你想这样做，那么你必须填写一个VkPipelineDynamicStateCreateInfo结构，
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto &descriptorSetLayouts = program->getDescriptorSetLayouts();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // 我们需要在管道创建期间指定描述符集布局，以告知 Vulkan 着色器将使用哪些描述符。描述符集布局在管道布局对象中指定。修改VkPipelineLayoutCreateInfo 以引用布局对象
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &ret.pipelineLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    auto &shaderStages = program->getShaderStages();

    // 你可以在着色器中使用uniform值，它是类似于动态状态变量的球状物，可以在绘制时改变，以改变着色器的行为，而不必重新创建它们。它们通常被用来向顶点着色器传递变换矩阵，或者在片段着色器中创建纹理采样器。
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    // 引用VkPipelineShaderStageCreateInfo结构的阵列。
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    // 引用所有描述固定功能阶段的结构。
    // 之后是管道布局，它是一个Vulkan句柄而不是一个结构指针。
    pipelineInfo.layout = ret.pipelineLayout_;
    // 渲染通道的引用和子通道的索引。
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    // Vulkan允许你通过衍生现有的管道来创建一个新的图形管道
    // 可以用basePipelineHandle指定一个现有管道的句柄
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    // 第二个参数，我们为其传递了VK_NULL_HANDLE参数，引用了一个可选的VkPipelineCache对象。管线缓存可以用来存储和重用与管线创建相关的数据，
    // 跨越对vkCreateGraphicsPipelines的多次调用，如果缓存被存储到文件中，甚至跨越程序执行。
    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ret.graphicsPipeline_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    std::cout << "------- createGraphicsPipeline: end! -----" << std::endl;

    return ret;
}