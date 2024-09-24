#pragma once

#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>

#include <vector>

namespace im3e {

class VulkanExtensions
{
public:
    VulkanExtensions(const ILogger& logger, const VulkanGlobalFcts& rFcts, bool isDebugEnabled,
                     const std::vector<std::string_view>& rRequiredInstanceExtensions = {});

    const auto& getInstanceExtensions() const { return m_instanceExtensions; }
    const auto& getDeviceExtensions() const { return m_deviceExtensions; }
    const auto& getLayers() const { return m_layers; }

    auto areDebugUtilsEnabled() const { return m_debugUtilsEnabled; }

private:
    const bool m_debugUtilsEnabled = false;
    std::vector<std::string_view> m_instanceExtensions;
    std::vector<std::string_view> m_deviceExtensions;
    std::vector<std::string_view> m_layers;
};

}  // namespace im3e