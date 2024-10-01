#pragma once

#include <im3e/api/device.h>
#include <im3e/api/image.h>

namespace im3e {

auto createVulkanImageFactory(std::shared_ptr<const IDevice> pDevice) -> std::unique_ptr<IImageFactory>;

}  // namespace im3e