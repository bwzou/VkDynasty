#include "runtime/platform/vulkan/VulkanUtil.h"
#include "runtime/code/base/macro.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace DynastyEngine
{
    std::unordered_map<uint32_t, VkSampler> VulkanUtil::mMipmapSamplerMap;
    VkSampler                               VulkanUtil::mNearestSampler = VK_NULL_HANDLE;
    VkSampler                               VulkanUtil::mLinearSampler  = VK_NULL_HANDLE;

    VkSampler VulkanUtil::getOrCreateNearestSampler(VkPhysicalDevice physicalDevice, VkDevice device)
    {
        if (mNearestSampler == VK_NULL_HANDLE)
        {
            VkPhysicalDeviceProperties physicalDeviceProperties {};
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

            VkSamplerCreateInfo samplerInfo {};

            samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter               = VK_FILTER_NEAREST;
            samplerInfo.minFilter               = VK_FILTER_NEAREST;
            samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.mipLodBias              = 0.0f;
            samplerInfo.anisotropyEnable        = VK_FALSE;
            samplerInfo.maxAnisotropy           = physicalDeviceProperties.limits.maxSamplerAnisotropy; // close :1.0f
            samplerInfo.compareEnable           = VK_FALSE;
            samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
            samplerInfo.minLod                  = 0.0f;
            samplerInfo.maxLod                  = 8.0f; // todo: m_irradiance_texture_miplevels
            samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;

            if (vkCreateSampler(device, &samplerInfo, nullptr, &mNearestSampler) != VK_SUCCESS)
            {
                LOG_ERROR("vk create sampler");
            }
        }

        return mNearestSampler;
    }

    VkSampler VulkanUtil::getOrCreateLinearSampler(VkPhysicalDevice physicalDevice, VkDevice device)
    {
        if (mLinearSampler == VK_NULL_HANDLE) 
        {   
            VkPhysicalDeviceProperties physicalDeviceProperties {};
            vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

            VkSamplerCreateInfo samplerInfo {};
            samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter               = VK_FILTER_LINEAR;
            samplerInfo.minFilter               = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.mipLodBias              = 0.0f;
            samplerInfo.anisotropyEnable        = VK_FALSE;
            samplerInfo.maxAnisotropy           = physicalDeviceProperties.limits.maxSamplerAnisotropy; // close :1.0f
            samplerInfo.compareEnable           = VK_FALSE;
            samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
            samplerInfo.minLod                  = 0.0f;
            samplerInfo.maxLod                  = 8.0f; // todo: m_irradiance_texture_miplevels
            samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;

            if (vkCreateSampler(device, &samplerInfo, nullptr, &mLinearSampler) != VK_SUCCESS)
            {
                LOG_ERROR("vk create sampler");
            }
        }

        return mLinearSampler;
    }

    void VulkanUtil::destroyNearestSampler(VkDevice device)
    {
        vkDestroySampler(device, mNearestSampler, nullptr);
        mNearestSampler = VK_NULL_HANDLE;
    }

    void VulkanUtil::destroyLinearSampler(VkDevice device)
    {
        vkDestroySampler(device, mLinearSampler, nullptr);
        mLinearSampler = VK_NULL_HANDLE;
    }
}