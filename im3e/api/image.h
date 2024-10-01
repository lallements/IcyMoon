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
};

class IImageFactory
{
public:
    virtual ~IImageFactory() = default;

    virtual auto createImage(ImageConfig config) const -> std::unique_ptr<IImage> = 0;
    virtual auto createHostVisibleImage(ImageConfig config) const -> std::unique_ptr<IHostVisibleImage> = 0;
};

}  // namespace im3e