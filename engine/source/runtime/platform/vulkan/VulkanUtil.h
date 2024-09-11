#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <unordered_map>
#include <vector>

namespace DynastyEngine
{
    class VulkanUtil
    {
    public:
        static VkSampler getOrCreateNearestSampler(VkPhysicalDevice physicalDevice, VkDevice device);
        static VkSampler getOrCreateLinearSampler(VkPhysicalDevice physicalDevice, VkDevice device);
        static void      destroyNearestSampler(VkDevice device);
        static void      destroyLinearSampler(VkDevice device);

    private:
        static std::unordered_map<uint32_t, VkSampler> mMipmapSamplerMap;
        static VkSampler                               mNearestSampler;
        static VkSampler                               mLinearSampler;
    };
}