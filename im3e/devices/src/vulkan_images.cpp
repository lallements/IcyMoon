#include "vulkan_images.h"

#include <im3e/utils/throw_utils.h>

#include <fmt/format.h>

using namespace im3e;
using namespace std;

namespace {

struct VulkanImageBuffer
{
    VulkanImageBuffer(shared_ptr<const IDevice> pDevice, ImageConfig config, VkImageTiling vkTiling,
                      VmaMemoryUsage vmaMemoryUsage)
      : m_pDevice(throwIfArgNull(move(pDevice), "Cannot create Vulkan image without a device"))
      , m_config(move(config))
    {
        VkImageCreateInfo vkCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = m_config.vkCreateFlags,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_config.vkFormat,
            .extent{
                .width = m_config.vkExtent.width,
                .height = m_config.vkExtent.height,
                .depth = 1U,
            },
            .mipLevels = 1U,
            .arrayLayers = 1U,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = vkTiling,
            .usage = m_config.vkUsage,
        };
        VmaAllocationCreateInfo vmaCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
            .usage = vmaMemoryUsage,
            .pUserData = const_cast<char*>(m_config.name.c_str()),
        };
        throwIfVkFailed(vmaCreateImage(m_pDevice->getVmaAllocator(), &vkCreateInfo, &vmaCreateInfo, &m_vkImage,
                                       &m_vmaAllocation, &m_vmaAllocationInfo),
                        fmt::format("Failed to create image \"{}\" with VMA", m_config.name));
    }

    ~VulkanImageBuffer() { vmaDestroyImage(m_pDevice->getVmaAllocator(), m_vkImage, m_vmaAllocation); }

    shared_ptr<const IDevice> m_pDevice;
    const ImageConfig m_config;

    VkImage m_vkImage{};
    VmaAllocation m_vmaAllocation{};
    VmaAllocationInfo m_vmaAllocationInfo{};
};

class VulkanImage : public IImage
{
public:
    VulkanImage(shared_ptr<const IDevice> pDevice, ImageConfig config)
      : m_pImageBuffer(make_unique<VulkanImageBuffer>(move(pDevice), move(config), VK_IMAGE_TILING_OPTIMAL,
                                                      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE))
    {
    }

    auto getVkImage() const -> VkImage override { return m_pImageBuffer->m_vkImage; }
    auto getVkExtent() const -> VkExtent2D override { return m_pImageBuffer->m_config.vkExtent; }
    auto getFormat() const -> VkFormat override { return m_pImageBuffer->m_config.vkFormat; }

private:
    unique_ptr<VulkanImageBuffer> m_pImageBuffer;
};

auto mapHostVisibleMemory(const IDevice& rDevice, const VmaAllocationInfo& rVmaAllocInfo)
{
    const auto vkDevice = rDevice.getVkDevice();
    const auto vkDeviceMemory = rVmaAllocInfo.deviceMemory;
    const auto& rFcts = rDevice.getFcts();

    void* pData{};
    throwIfVkFailed(rFcts.vkMapMemory(vkDevice, vkDeviceMemory, rVmaAllocInfo.offset, rVmaAllocInfo.size, 0U, &pData),
                    "Failed to map host-visible image memory");

    return UniquePtrWithDeleter<uint8_t>(
        reinterpret_cast<uint8_t*>(pData),
        [vkDevice, vkDeviceMemory, &rFcts](uint8_t*) { rFcts.vkUnmapMemory(vkDevice, vkDeviceMemory); });
}

auto queryImageRowPitch(const IDevice& rDevice, VkImage vkImage)
{
    const auto vkDevice = rDevice.getVkDevice();
    const auto& rFcts = rDevice.getFcts();

    VkImageSubresource vkSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT};

    VkSubresourceLayout vkResult{};
    rFcts.vkGetImageSubresourceLayout(vkDevice, vkImage, &vkSubresource, &vkResult);

    return vkResult.rowPitch;
}

class VulkanHostVisibleImageMapping : public IHostVisibleImage::IMapping
{
public:
    VulkanHostVisibleImageMapping(shared_ptr<VulkanImageBuffer> pImageBuffer)
      : m_pImageBuffer(throwIfArgNull(move(pImageBuffer), "Host-visible image mapping requires a buffer"))
      , m_pData(mapHostVisibleMemory(*m_pImageBuffer->m_pDevice, m_pImageBuffer->m_vmaAllocationInfo))
      , m_rowPitch(queryImageRowPitch(*m_pImageBuffer->m_pDevice, m_pImageBuffer->m_vkImage))
    {
    }

    auto getData() -> uint8_t* override { return m_pData.get(); }
    auto getConstData() const -> const uint8_t* override { return m_pData.get(); }
    auto getSizeInBytes() const -> VkDeviceSize override { return m_pImageBuffer->m_vmaAllocationInfo.size; }
    auto getRowPitch() const -> VkDeviceSize override { return m_rowPitch; }

private:
    shared_ptr<VulkanImageBuffer> m_pImageBuffer;
    UniquePtrWithDeleter<uint8_t> m_pData;
    const VkDeviceSize m_rowPitch{};
};

class VulkanHostVisibleImage : public IHostVisibleImage
{
public:
    VulkanHostVisibleImage(shared_ptr<const IDevice> pDevice, ImageConfig config)
      : m_pImageBuffer(make_shared<VulkanImageBuffer>(move(pDevice), move(config), VK_IMAGE_TILING_LINEAR,
                                                      VMA_MEMORY_USAGE_AUTO_PREFER_HOST))
    {
    }

    auto map() -> unique_ptr<IMapping> override { return make_unique<VulkanHostVisibleImageMapping>(m_pImageBuffer); }
    auto mapReadOnly() const -> unique_ptr<const IMapping> override
    {
        return make_unique<VulkanHostVisibleImageMapping>(m_pImageBuffer);
    }

    auto getVkImage() const -> VkImage override { return m_pImageBuffer->m_vkImage; }
    auto getVkExtent() const -> VkExtent2D override { return m_pImageBuffer->m_config.vkExtent; }
    auto getFormat() const -> VkFormat override { return m_pImageBuffer->m_config.vkFormat; }

private:
    shared_ptr<VulkanImageBuffer> m_pImageBuffer;
};

class VulkanImageFactory : public IImageFactory
{
public:
    VulkanImageFactory(shared_ptr<const IDevice> pDevice)
      : m_pDevice(throwIfArgNull(move(pDevice), "Image Factory requires a device"))
    {
    }

    auto createImage(ImageConfig config) const -> unique_ptr<IImage> override
    {
        return make_unique<VulkanImage>(m_pDevice, move(config));
    }

    auto createHostVisibleImage(ImageConfig) const -> unique_ptr<IHostVisibleImage> override { return nullptr; }

private:
    shared_ptr<const IDevice> m_pDevice;
};

}  // namespace

auto im3e::createVulkanImageFactory(shared_ptr<const IDevice> pDevice) -> unique_ptr<IImageFactory>
{
    return make_unique<VulkanImageFactory>(move(pDevice));
}