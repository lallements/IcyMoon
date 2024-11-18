#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>
#include <im3e/api/gui.h>
#include <im3e/api/logger.h>
#include <im3e/api/window.h>

namespace im3e {

auto createImguiPipeline(std::shared_ptr<const IDevice> pDevice, std::shared_ptr<IGuiWorkspace> pGuiWorkspace)
    -> std::unique_ptr<IFramePipeline>;

auto createImguiWorkspace(std::string_view name) -> std::shared_ptr<IGuiWorkspace>;

struct WindowApplicationConfig
{
    std::string name;
    bool isDebugEnabled = false;
};
auto createGlfwWindowApplication(const ILogger& rLogger, WindowApplicationConfig config)
    -> std::shared_ptr<IWindowApplication>;

}  // namespace im3e