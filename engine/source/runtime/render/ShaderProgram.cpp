#include "ShaderProgram.h"


namespace DynastyEngine
{
    bool ShaderProgram::compileAndLinkGLSLFile(const std::string &vsPath, const std::string &fsPath, const std::string source) 
    {
        std::string jsonVertPath = SHADER_JSON_DIR + source + "_vert.json";
        auto uniformsDescVertex = readFromFile(jsonVertPath);

        std::string jsonFragPath = SHADER_JSON_DIR + source + "_frag.json";
        auto uniformsDescFragment = readFromFile(jsonFragPath);

        return compileAndLinkGLSL(FileUtils::readFile(vsPath), uniformsDescVertex, FileUtils::readFile(fsPath), uniformsDescFragment);
    }


    bool ShaderProgram::compileAndLinkGLSL(const std::vector<char> &vsSource, 
                                        std::unordered_map<std::string, ShaderUniformDesc> uniformsDescVertex, 
                                        const std::vector<char> &fsSource, 
                                        std::unordered_map<std::string, ShaderUniformDesc> uniformsDescFragment) 
    {
        LOG_INFO("compileAndLinkGLSL");
        createShaderModule(vertexShader_, vsSource);
        createShaderModule(fragmentShader_, fsSource);
        LOG_INFO("createShaderStages");
        createShaderStages();

        LOG_INFO("createDescriptorSetLayouts");
        createDescriptorSetLayouts(uniformsDescVertex, uniformsDescFragment);
        createDescriptorPool();
        
        return true;
    }


    std::unordered_map<std::string, ShaderUniformDesc> ShaderProgram::readFromFile(std::string jsonFilePath) 
    {
        std::unordered_map<std::string, ShaderUniformDesc> uniformsDesc;

        LOG_INFO("=== ShaderProgram::readFromFile ===");
        LOG_INFO(jsonFilePath);
        if (!FileUtils::exists(jsonFilePath)) 
        {
            return uniformsDesc;
        }
        std::string err;
        const auto json = json11::Json::parse(FileUtils::readText(jsonFilePath), err);
        const auto &uniformsObj = json["uniformsDesc"].object_items();
        for (const auto &kv : uniformsObj) 
        {
            const auto &name = kv.first;
            const auto &descJson = kv.second;
            ShaderUniformDesc desc;
            desc.name = name;
            desc.type = static_cast<ShaderUniformType>(descJson["type"].int_value());
            desc.location = descJson["location"].int_value();
            LOG_INFO("location：{}", desc.location)
            desc.binding = descJson["binding"].int_value();
            LOG_INFO("binding：{}", desc.binding)
            desc.set = descJson["set"].int_value();
            LOG_INFO("set：{}", desc.set)
            uniformsDesc[name] = desc;
        }
        return uniformsDesc;
    }


    void ShaderProgram::createDescriptorSetLayouts(std::unordered_map<std::string, ShaderUniformDesc> &uniformsDescVertex,
                                                std::unordered_map<std::string, ShaderUniformDesc> &uniformsDescFragment) 
    {
        for (auto &kv : uniformsDescVertex) 
        {
            uniformsInfo_[kv.first].type = kv.second.type;
            uniformsInfo_[kv.first].stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
            uniformsInfo_[kv.first].binding = kv.second.binding;
        }
        for (auto &kv : uniformsDescFragment) 
        {
            uniformsInfo_[kv.first].type = kv.second.type;
            uniformsInfo_[kv.first].stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            uniformsInfo_[kv.first].binding = kv.second.binding;
        }

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (auto &kv : uniformsInfo_) 
        {
            VkDescriptorSetLayoutBinding layoutBinging{};
            layoutBinging.binding = kv.second.binding;
            layoutBinging.descriptorCount = 1;
            layoutBinging.descriptorType = getDescriptorType(kv.second.type);
            layoutBinging.pImmutableSamplers = nullptr;
            layoutBinging.stageFlags = kv.second.stageFlags;
            bindings.push_back(layoutBinging);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        
        descriptorSetLayouts_.resize(1);  // only one set, all uniform bind to set 0
        if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayouts_[0]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }


    void ShaderProgram::createDescriptorPool() 
    {
        uint32_t uniformBlockCnt = 0;
        uint32_t uniformSamplerCnt = 0;
        for (auto &kv : uniformsInfo_) 
        {
            switch (kv.second.type)
            {
                case UniformType_Block:
                    uniformBlockCnt ++;
                    break;
                case UniformType_Sampler:
                    uniformSamplerCnt ++;
                    break;
                default:
                    break;
            }
        }

        std::vector<VkDescriptorPoolSize> poolSizes;
        if (uniformBlockCnt > 0) 
        {
            poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBlockCnt * DESCRIPTOR_SET_POOL_MAX_SIZE});
        }
        if (uniformSamplerCnt > 0) 
        {
            poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uniformSamplerCnt * DESCRIPTOR_SET_POOL_MAX_SIZE});
        }
        // 用于imgui
        poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 * DESCRIPTOR_SET_POOL_MAX_SIZE});
        
        // 我们将为每一帧分配这些描述符之一。此池大小结构由 main 引用VkDescriptorPoolCreateInfo
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = DESCRIPTOR_SET_POOL_MAX_SIZE;

        if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }


    DescriptorSet *ShaderProgram::getNewDescriptorSet(VkDescriptorSetLayout layout) {
        auto it = descriptorSetPool_.find(layout);
        if (it == descriptorSetPool_.end()) 
        {
            std::vector<DescriptorSet> desSetPool;
            desSetPool.reserve(DESCRIPTOR_SET_POOL_MAX_SIZE);
            descriptorSetPool_[layout] = std::move(desSetPool);
        }
        auto &pool = descriptorSetPool_[layout];

        for (auto &desc : pool) 
        {
            if (!desc.inUse) 
            {
                desc.inUse = true;
                return &desc;
            }
        }

        DescriptorSet descSet{};
        descSet.inUse = true;
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool_;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;
        if (vkAllocateDescriptorSets(device_, &allocInfo, &descSet.set) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        pool.push_back(descSet);

        maxDescriptorSetPoolSize_ = std::max(maxDescriptorSetPoolSize_, pool.size());
        if (maxDescriptorSetPoolSize_ >= DESCRIPTOR_SET_POOL_MAX_SIZE) 
        {
            std::cout << "error: descriptor set pool size exceed" << std::endl;
        }
        return &pool.back();
    }


    void ShaderProgram::createShaderStages() 
    {
        shaderStages_.resize(2);
        // 为了实际使用这些着色器，我们需要通过结构将它们分配到一个特定的流水线阶段
        auto &vertShaderStageInfo = shaderStages_[0];
        // 首先填写顶点着色器
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertexShader_;
        vertShaderStageInfo.pName = "main";

        auto &fragShaderStageInfo = shaderStages_[1];
        // 然后填写顶点着色器
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragmentShader_;
        fragShaderStageInfo.pName = "main";
    }


    void ShaderProgram::createShaderModule(VkShaderModule  &shaderModule, const std::vector<char>& code) 
    {
        // 该函数将接收一个带有字节码的缓冲区作为参数，并从中创建一个VkShaderModule。
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        // 只需要指定一个带有字节码的缓冲区的指针和它的长度
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module!");
        }
    }
}