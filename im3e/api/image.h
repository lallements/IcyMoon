#pragma once

#include <im3e/utils/vk_utils.h>

namespace im3e {

struct ImageConfig
{
    std::string name;
    VkExtent2D vkExtent{};
    VkFormat vkFormat = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags vkUsage{};
    VkImageCreateFlags vkCreateFlags{};
};

class IImage
{
public:
    virtual ~IImage() = default;

    virtual auto getVkImage() const -> VkImage = 0;
    virtual auto getVkExtent() const -> VkExtent2D = 0;
    virtual auto getFormat() const -> VkFormat = 0;
};

class IHostVisibleImage : public IImage
{
public:
    virtual ~IHostVisibleImage() = default;

    class IMapping
    {
    public:
        virtual ~IMapping() = default;

        virtual auto getData() -> uint8_t* = 0;
        virtual auto getConstData() const -> const uint8_t* = 0;
        virtual auto getSizeInBytes() const -> VkDeviceSize = 0;
        virtual auto getRowPitch() const -> VkDeviceSize = 0;
    };

    /// @brief Map image data to access it from the CPU.
    /// @details Mapped data is guaranteed to remain valid until it is no longer referenced by any mapping, even if the
    /// original image is destroyed.
    virtual auto map() -> std::unique_ptr<IMapping> = 0;
    virtual auto mapReadOnly() const -> std::unique_ptr<const IMapping> = 0;
};

class IImageFactory
{
public:
    virtual ~IImageFactory() = default;

    virtual auto createImage(ImageConfig config) const -> std::unique_ptr<IImage> = 0;
    virtual auto createHostVisibleImage(ImageConfig config) const -> std::unique_ptr<IHostVisibleImage> = 0;
};

}  // namespace im3e