#pragma once


#include "VulKanContext.h"


// class VulKanGLInterop {
// public:
//     explicit VulKanGLInterop(VulkanContext &ctx) : vkCtx_(ctx) {
//         device_ = ctx.device();
//     }
//     ~VulKanGLInterop();

//     static inline const std::vector<const char *> &getRequiredInstanceExtensions() { return requiredInstanceExtensions; }
//     static inline const std::vector<const char *> &getRequiredDeviceExtensions() { return requiredDeviceExtensions; }

// private:
//     VulkanContext &vkCtx_;  
//     VkDevice device_ = VK_NULL_HANDLE;  
//     static const std::vector<const char *> requiredInstanceExtensions;
//     static const std::vector<const char *> requiredDeviceExtensions;
// };