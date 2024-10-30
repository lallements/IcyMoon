#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>
#include <im3e/api/gui.h>

namespace im3e {

auto createImguiPipeline(std::shared_ptr<const IDevice> pDevice, std::shared_ptr<IGuiWorkspace> pGuiWorkspace)
    -> std::unique_ptr<IFramePipeline>;

}  // namespace im3e