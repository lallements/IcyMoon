#pragma once

#include <im3e/api/vulkan_loader.h>

#include <memory>

namespace im3e {

auto createVulkanLoader() -> std::unique_ptr<IVulkanLoader>;

}  // namespace im3e