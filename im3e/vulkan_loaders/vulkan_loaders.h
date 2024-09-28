#pragma once

#include <im3e/api/vulkan_loader.h>

#include <memory>

namespace im3e {

struct VulkanLoaderConfig
{
    bool isDebugEnabled = false;
};
auto createVulkanLoader(VulkanLoaderConfig config) -> std::unique_ptr<IVulkanLoader>;

}  // namespace im3e