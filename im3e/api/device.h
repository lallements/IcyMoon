#pragma once

#include "command_buffer.h"
#include "image.h"
#include "logger.h"
#include "vulkan_loader.h"

namespace im3e {

class IDevice
{
public:
    virtual ~IDevice() = default;

    virtual auto createLogger(std::string_view name) const -> std::unique_ptr<ILogger> = 0;

    virtual auto getVkInstance() const -> VkInstance = 0;
    virtual auto getVkPhysicalDevice() const -> VkPhysicalDevice = 0;
    virtual auto getVkDevice() const -> VkDevice = 0;
    virtual auto getFcts() const -> const VulkanDeviceFcts& = 0;
    virtual auto getInstanceFcts() const -> const VulkanInstanceFcts& = 0;
    virtual auto getImageFactory() const -> std::shared_ptr<const IImageFactory> = 0;
    virtual auto getCommandQueue() const -> std::shared_ptr<const ICommandQueue> = 0;
    virtual auto getCommandQueue() -> std::shared_ptr<ICommandQueue> = 0;
};

}  // namespace im3e