#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>

#include <pxr/imaging/hd/renderDelegate.h>

namespace im3e {

auto createUsdRenderDelegate(std::shared_ptr<const IDevice> pDevice) -> std::unique_ptr<pxr::HdRenderDelegate>;
auto createHydraFramePipeline(std::shared_ptr<const IDevice> pDevice,
                              std::unique_ptr<pxr::HdRenderDelegate> pRenderDelegate)
    -> std::unique_ptr<IFramePipeline>;

}  // namespace im3e