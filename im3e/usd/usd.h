#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>
#include <im3e/api/window.h>

#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/usd/usd/stage.h>

#include <filesystem>

namespace im3e {

/// @brief Open an existing USD file and return a USD stage.
auto openUsdStage(const std::filesystem::path& rFilePath) -> pxr::UsdStageRefPtr;

class IHydraRenderer
{
public:
    virtual ~IHydraRenderer() = default;

    virtual auto getRenderIndex() -> pxr::HdRenderIndex& = 0;
    virtual auto getRenderIndex() const -> const pxr::HdRenderIndex& = 0;
    virtual auto getTasks() const -> pxr::HdTaskSharedPtrVector = 0;
};
auto createHdStormRenderer(std::shared_ptr<const IDevice> pDevice, std::shared_ptr<IGlContext> pGlContext)
    -> std::shared_ptr<IHydraRenderer>;

auto createHydraFramePipeline(std::shared_ptr<const IDevice> pDevice, std::shared_ptr<IHydraRenderer> pRenderer,
                              pxr::UsdStageRefPtr pUsdStage) -> std::unique_ptr<IFramePipeline>;

}  // namespace im3e