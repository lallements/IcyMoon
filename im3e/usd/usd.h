#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>

namespace im3e {

auto createUsdFramePipeline(std::shared_ptr<const IDevice> pDevice) -> std::unique_ptr<IFramePipeline>;

}  // namespace im3e