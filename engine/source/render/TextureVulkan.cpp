#pragma once

#include "TextureVulkan.h"


TextureVulkan::TextureVulkan(VulkanContext &ctx, const TextureDesc &desc) : vkCtx_(ctx) {
    device_ =  ctx.device();

    width = desc.width;
    height = desc.height;
    type = desc.type;
    format = desc.format;
    usage = desc.usage;
    useMipmaps = desc.useMipmaps;
    multiSample = desc.multiSample;

    // image format
    vkFormat_ = cvtImageFormat(format, usage);

    // only multi sampe color attacthment need to resolved
    needResolve_ = multiSample && (usage & TextureUsage_AttachmentColor);
    needMipmaps_ = useMipmaps && vkCtx_.linearBlitAvailable(vkFormat_);
    layerCount_ = getLayerCount();
    if (needMipmaps_) {
        levelCount_ = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    } else {
        levelCount_ = 1;
    }
    imageAspect_ = getImageAspect();

    if (needResolve_) {
        createImage();
        // try lazy allocation
        bool memoryReady = vkCtx_.createImageMemory(image_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
        if (!memoryReady) {
            vkCtx_.createImageMemory(image_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }

        // resolve
        // createImageResolve(imageCreatePNext);
        // vkCtx_.createImageMemory(imageResolve_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryCreatePNext);

        // if (needGLInterop_) {
        //     glInterop_.createSharedMemory(imageResolve_.memory, imageResolve_.allocationSize);
        // }
    } else {
        // createImage(imageCreatePNext);
        // vkCtx_.createImageMemory(image_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryCreatePNext);

        // if (needGLInterop_) {
        //     glInterop_.createSharedMemory(image_.memory, image_.allocationSize);
        // }
    }
}


TextureVulkan::~TextureVulkan() {
    vkDestroySampler(device_, sampler_, nullptr);
    vkDestroyImageView(device_, sampleView_, nullptr);

    image_.destroy(device_);
    imageResolve_.destroy(device_);

    if (hostImage_.memory != VK_NULL_HANDLE) {
        vkUnmapMemory(device_, hostImage_.memory);
    }
    hostImage_.destroy(device_);

    uploadStagingBuffer_.destroy(vkCtx_.allocator());
}


void TextureVulkan::initImageData() {
    if (usage & TextureUsage_Sampler) {
        auto *cmd = vkCtx_.beginCommands();

        VkImageSubresourceRange subRange{};
        subRange.aspectMask = imageAspect_;
        subRange.baseMipLevel = 0;
        subRange.baseArrayLayer = 0;
        subRange.levelCount = levelCount_;
        subRange.layerCount = layerCount_;

        transitionImageLayout(cmd->cmdBuffer, image_.image, subRange,
                            0,
                            VK_ACCESS_SHADER_READ_BIT,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        vkCtx_.endCommands(cmd);
    }
}
    

void TextureVulkan::dumpImage(const char *path, uint32_t w, uint32_t h) {

}


void TextureVulkan::setImageData(const std::vector<std::shared_ptr<Buffer<RGBA>>> &buffers) {

}


void TextureVulkan::setImageData(const std::vector<std::shared_ptr<Buffer<float>>> &buffers) {

}


void TextureVulkan::readPixels(uint32_t layer, uint32_t level, const std::function<void(uint8_t *buffer, uint32_t width, uint32_t height, uint32_t rowStride)> &func) {

}


VkSampler &TextureVulkan::getSampler() {

}


void TextureVulkan::createImage(const void *pNext) {
    if (image_.image != VK_NULL_HANDLE) {
        return;
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = pNext;
    imageInfo.flags = 0;
    imageInfo.imageType = cvtImageType(type);
    imageInfo.format = vkFormat_;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = levelCount_;
    imageInfo.arrayLayers = layerCount_;
    imageInfo.samples = getSampleCount();
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = 0;

    if (type == TextureType_CUBE) {
        imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    if (usage & TextureUsage_Sampler) {
        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (usage & TextureUsage_UploadData) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (usage & TextureUsage_AttachmentColor) {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (usage & TextureUsage_AttachmentDepth) {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (needResolve_) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    } else {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (VK_SUCCESS != vkCreateImage(device_, &imageInfo, nullptr, &image_.image)) {
        throw std::runtime_error("vkcreate image error!");
    }
}


void TextureVulkan::createImageView(VkImageView &view, VkImage &image) {
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = cvtImageViewType(type);
    imageViewCreateInfo.format = vkFormat_;
    imageViewCreateInfo.subresourceRange = {};
    imageViewCreateInfo.subresourceRange.aspectMask = imageAspect_;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = levelCount_;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = layerCount_;
    imageViewCreateInfo.image = image;
    
    if (VK_SUCCESS != vkCreateImageView(device_, &imageViewCreateInfo, nullptr, &view)) {
        std::runtime_error("vkcreate image view error!");
    }
}


VkImageView TextureVulkan::createResolveView() {
    VkImageView view{};
    createImageView(view, imageResolve_.image);
    return view;
}


VkImageView TextureVulkan::createAttachmentView(VkImageAspectFlags aspect, uint32_t layer, uint32_t level) {

}


void TextureVulkan::createImageResolve(const void *pNext) {

}


bool TextureVulkan::createImageHost(uint32_t level) {

}


void TextureVulkan::generateMipmaps() {

}
    
    
void TextureVulkan::setImageDataInternal(const std::vector<const void *> &buffers, VkDeviceSize imageSize) {
    
}


void TextureVulkan::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                                          VkImageSubresourceRange subresourceRange,
                                          VkAccessFlags srcMask,
                                          VkAccessFlags dstMask,
                                          VkImageLayout oldLayout,
                                          VkImageLayout newLayout,
                                          VkPipelineStageFlags srcStage,
                                          VkPipelineStageFlags dstStage) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcAccessMask = srcMask;
    barrier.dstAccessMask = dstMask;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}