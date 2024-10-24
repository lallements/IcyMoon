#include "vulkan_images.h"

#include <im3e/utils/throw_utils.h>

#include <CImg.h>
#include <fmt/format.h>

using namespace im3e;
using namespace std;
using namespace std::filesystem;

namespace {

struct VulkanImageBuffer
{
    VulkanImageBuffer(shared_ptr<const IDevice> pDevice, ImageConfig config, VkImageTiling vkTiling,
                      VmaMemoryUsage vmaMemoryUsage)
      : m_pDevice(throwIfArgNull(move(pDevice), "Cannot create Vulkan image without a device"))
      , m_pMemoryAllocator(m_pDevice->getMemoryAllocator())
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

        VmaAllocationCreateFlags vmaFlags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        VkMemoryPropertyFlags vkRequiredFlags{};
        if (vmaMemoryUsage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
        {
            vmaFlags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        }

        VmaAllocationCreateInfo vmaCreateInfo{
            .flags = vmaFlags,
            .usage = vmaMemoryUsage,
            .requiredFlags = vkRequiredFlags,
            .pUserData = const_cast<char*>(m_config.name.c_str()),
        };
        throwIfVkFailed(m_pMemoryAllocator->createImage(&vkCreateInfo, &vmaCreateInfo, &m_vkImage, &m_vmaAllocation,
                                                        &m_vmaAllocationInfo),
                        fmt::format("Failed to create image \"{}\" with VMA", m_config.name));
    }

    ~VulkanImageBuffer() { m_pMemoryAllocator->destroyImage(m_vkImage, m_vmaAllocation); }

    auto getVkSubresourceLayers() const
    {
        return VkImageSubresourceLayers{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1U,
        };
    }

    shared_ptr<const IDevice> m_pDevice;
    shared_ptr<IMemoryAllocator> m_pMemoryAllocator;
    const ImageConfig m_config;

    VkImage m_vkImage{};
    VmaAllocation m_vmaAllocation{};
    VmaAllocationInfo m_vmaAllocationInfo{};
};

auto getAspectMaskFromImageUsage(VkImageUsageFlags vkImageUsage) -> VkImageAspectFlags
{
    if (vkFlagsContain(vkImageUsage, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

class VulkanImageView : public IImageView
{
public:
    VulkanImageView(shared_ptr<const VulkanImageBuffer> pImageBuffer)
      : m_pImageBuffer(throwIfArgNull(move(pImageBuffer), "Cannot create a Vulkan image view without an image"))
    {
        VkImageViewCreateInfo vkCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_pImageBuffer->m_vkImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_pImageBuffer->m_config.vkFormat,
            .subresourceRange{
                .aspectMask = getAspectMaskFromImageUsage(m_pImageBuffer->m_config.vkUsage),
                .levelCount = 1U,
                .layerCount = 1U,
            },
        };

        const auto vkDevice = m_pImageBuffer->m_pDevice->getVkDevice();
        const auto& rFcts = m_pImageBuffer->m_pDevice->getFcts();

        VkImageView vkImageView{};
        throwIfVkFailed(rFcts.vkCreateImageView(vkDevice, &vkCreateInfo, nullptr, &vkImageView),
                        "Could not create image buffer view");
        m_pVkImageView = makeVkUniquePtr<VkImageView>(vkDevice, vkImageView, rFcts.vkDestroyImageView);
    }

    auto getVkImageView() const -> VkImageView override { return m_pVkImageView.get(); }
    auto getVkImage() const -> VkImage override { return m_pImageBuffer->m_vkImage; }

private:
    shared_ptr<const VulkanImageBuffer> m_pImageBuffer;
    VkUniquePtr<VkImageView> m_pVkImageView;
};

class VulkanImageMetadata : public IImageMetadata
{
public:
    void setLayout(VkImageLayout vkLayout) override { m_vkLayout = vkLayout; }
    void setLastStageMask(VkPipelineStageFlags2 vkLastStageMask) override { m_vkLastStageMask = vkLastStageMask; }
    void setLastAccessMask(VkAccessFlags2 vkLastAccessMask) override { m_vkLastAccessMask = vkLastAccessMask; }

    auto getLayout() const -> VkImageLayout override { return m_vkLayout; }
    auto getQueueFamilyIndex() const -> uint32_t override { return VK_QUEUE_FAMILY_IGNORED; }
    auto getLastStageMask() const -> VkPipelineStageFlags2 override { return m_vkLastStageMask; }
    auto getLastAccessMask() const -> VkAccessFlags2 override { return m_vkLastAccessMask; }

private:
    VkImageLayout m_vkLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPipelineStageFlags2 m_vkLastStageMask = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 m_vkLastAccessMask = VK_ACCESS_2_NONE;
};

class VulkanImage : public IImage
{
public:
    VulkanImage(shared_ptr<const IDevice> pDevice, ImageConfig config)
      : m_pImageBuffer(make_shared<VulkanImageBuffer>(move(pDevice), move(config), VK_IMAGE_TILING_OPTIMAL,
                                                      VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE))
      , m_pMetadata(make_shared<VulkanImageMetadata>())
    {
    }

    auto createView() const -> unique_ptr<IImageView> override { return make_unique<VulkanImageView>(m_pImageBuffer); }

    auto getVkImage() const -> VkImage override { return m_pImageBuffer->m_vkImage; }
    auto getVkExtent() const -> VkExtent2D override { return m_pImageBuffer->m_config.vkExtent; }
    auto getVkFormat() const -> VkFormat override { return m_pImageBuffer->m_config.vkFormat; }
    auto getVkSubresourceLayers() const -> VkImageSubresourceLayers override
    {
        return m_pImageBuffer->getVkSubresourceLayers();
    }
    auto getMetadata() -> shared_ptr<IImageMetadata> override { return m_pMetadata; }
    auto getMetadata() const -> shared_ptr<const IImageMetadata> override { return m_pMetadata; }

private:
    shared_ptr<VulkanImageBuffer> m_pImageBuffer;
    shared_ptr<IImageMetadata> m_pMetadata;
};

auto mapHostVisibleMemory(shared_ptr<IMemoryAllocator> pAllocator, VmaAllocation vmaAllocation)
{
    void* pData{};
    throwIfVkFailed(pAllocator->mapMemory(vmaAllocation, &pData), "Failed to map host-visible image memory");

    return UniquePtrWithDeleter<uint8_t>(reinterpret_cast<uint8_t*>(pData), [pAllocator, vmaAllocation](uint8_t*) {
        pAllocator->unmapMemory(vmaAllocation);
    });
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
      , m_pData(mapHostVisibleMemory(m_pImageBuffer->m_pMemoryAllocator, m_pImageBuffer->m_vmaAllocation))
      , m_formatProperties(getFormatProperties(m_pImageBuffer->m_config.vkFormat))
      , m_vkExtent(m_pImageBuffer->m_config.vkExtent)
      , m_rowPitch(queryImageRowPitch(*m_pImageBuffer->m_pDevice, m_pImageBuffer->m_vkImage))
    {
    }

    void save(const filesystem::path& rFileName) const override
    {
        cimg_library::cimg::exception_mode(0U);  // disable log messages from cimg
        cimg_library::CImg<uint8_t> image(m_pData.get(), m_formatProperties.componentCount, m_vkExtent.width,
                                          m_vkExtent.height, 1U, false);
        image.permute_axes("yzcx");
        image.save(rFileName.string().c_str());
    }

    auto getData() -> uint8_t* override { return m_pData.get(); }
    auto getConstData() const -> const uint8_t* override { return m_pData.get(); }
    auto getSizeInBytes() const -> VkDeviceSize override { return m_pImageBuffer->m_vmaAllocationInfo.size; }
    auto getRowPitch() const -> VkDeviceSize override { return m_rowPitch; }
    auto getPixel(uint32_t x, uint32_t y) const -> const uint8_t* override
    {
        return m_pData.get() + y * m_rowPitch + x * m_formatProperties.sizeInBytes;
    }

private:
    shared_ptr<VulkanImageBuffer> m_pImageBuffer;
    UniquePtrWithDeleter<uint8_t> m_pData;
    const FormatProperties m_formatProperties{};
    const VkExtent2D m_vkExtent{};
    const VkDeviceSize m_rowPitch{};
};

class VulkanHostVisibleImage : public IHostVisibleImage
{
public:
    VulkanHostVisibleImage(shared_ptr<const IDevice> pDevice, ImageConfig config)
      : m_pImageBuffer(make_shared<VulkanImageBuffer>(move(pDevice), move(config), VK_IMAGE_TILING_LINEAR,
                                                      VMA_MEMORY_USAGE_AUTO_PREFER_HOST))
      , m_pMetadata(make_shared<VulkanImageMetadata>())
    {
    }

    auto createView() const -> unique_ptr<IImageView> override { return make_unique<VulkanImageView>(m_pImageBuffer); }

    auto map() -> unique_ptr<IMapping> override { return make_unique<VulkanHostVisibleImageMapping>(m_pImageBuffer); }
    auto mapReadOnly() const -> unique_ptr<const IMapping> override
    {
        return make_unique<VulkanHostVisibleImageMapping>(m_pImageBuffer);
    }

    auto getVkImage() const -> VkImage override { return m_pImageBuffer->m_vkImage; }
    auto getVkExtent() const -> VkExtent2D override { return m_pImageBuffer->m_config.vkExtent; }
    auto getVkFormat() const -> VkFormat override { return m_pImageBuffer->m_config.vkFormat; }
    auto getVkSubresourceLayers() const -> VkImageSubresourceLayers override
    {
        return m_pImageBuffer->getVkSubresourceLayers();
    }
    auto getMetadata() -> shared_ptr<IImageMetadata> override { return m_pMetadata; }
    auto getMetadata() const -> shared_ptr<const IImageMetadata> override { return m_pMetadata; }

private:
    shared_ptr<VulkanImageBuffer> m_pImageBuffer;
    shared_ptr<VulkanImageMetadata> m_pMetadata;
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

    auto createHostVisibleImage(ImageConfig config) const -> unique_ptr<IHostVisibleImage> override
    {
        return make_unique<VulkanHostVisibleImage>(m_pDevice, move(config));
    }

private:
    shared_ptr<const IDevice> m_pDevice;
};

}  // namespace

auto im3e::createVulkanImageFactory(shared_ptr<const IDevice> pDevice) -> unique_ptr<IImageFactory>
{
    return make_unique<VulkanImageFactory>(move(pDevice));
}