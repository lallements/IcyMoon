#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>

#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/usd/usd/stage.h>

#include <filesystem>

namespace im3e {

/// @brief Open an existing USD file and return a USD stage.
auto openUsdStage(const std::filesystem::path& rFilePath) -> pxr::UsdStageRefPtr;

auto createUsdRenderDelegate(std::shared_ptr<const IDevice> pDevice) -> std::unique_ptr<pxr::HdRenderDelegate>;
auto createHydraFramePipeline(std::shared_ptr<const IDevice> pDevice,
                              std::unique_ptr<pxr::HdRenderDelegate> pRenderDelegate, pxr::UsdStageRefPtr pUsdStage)
    -> std::unique_ptr<IFramePipeline>;

}  // namespace im3e