#pragma once

#include "image.h"
#include "vulkan_loader.h"

namespace im3e {

class IMemoryAllocator
{
public:
    virtual ~IMemoryAllocator() = default;

    virtual auto createImage(const VkImageCreateInfo* pVkCreateInfo, const VmaAllocationCreateInfo* pVmaCreateInfo,
                             VkImage* pVkImage, VmaAllocation* pVmaAllocation, VmaAllocationInfo* pVmaAllocationInfo)
        -> VkResult = 0;
    virtual void destroyImage(VkImage vkImage, VmaAllocation vmaAllocation) = 0;
};

class IDevice
{
public:
    virtual ~IDevice() = default;

    virtual auto getVkInstance() const -> VkInstance = 0;
    virtual auto getVkPhysicalDevice() const -> VkPhysicalDevice = 0;
    virtual auto getVkDevice() const -> VkDevice = 0;
    virtual auto getFcts() const -> const VulkanDeviceFcts& = 0;
    virtual auto getMemoryAllocator() const -> std::shared_ptr<IMemoryAllocator> = 0;
    virtual auto getImageFactory() const -> std::shared_ptr<const IImageFactory> = 0;
};

}  // namespace im3e