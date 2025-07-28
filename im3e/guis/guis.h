#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>
#include <im3e/api/gui.h>
#include <im3e/api/window.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/properties/api/property.h>
#include <im3e/utils/stats.h>

#include <glm/glm.hpp>

namespace im3e {

auto createImguiPipeline(std::shared_ptr<const IDevice> pDevice, std::shared_ptr<IGuiWorkspace> pGuiWorkspace)
    -> std::unique_ptr<IFramePipeline>;

class IImguiEventListener
{
public:
    virtual ~IImguiEventListener() = default;

    enum class MouseButton : uint32_t
    {
        Left = 0U,
        Right = 1U,
        Middle = 2U,
    };
    virtual void onMouseMove(const glm::vec2& rClipOffset, const std::array<bool, 3U>& rMouseButtonsDown) = 0;
    virtual void onMouseWheel(float scrollSteps) = 0;
};

auto createImguiRenderPanel(std::string_view name, std::unique_ptr<IFramePipeline> pFramePipeline,
                            std::shared_ptr<IImguiEventListener> pEventListener = nullptr)
    -> std::shared_ptr<IGuiPanel>;

auto createImguiStatsPanel(std::string_view name, std::shared_ptr<IStatsProvider> pStatsProvider)
    -> std::shared_ptr<IGuiPanel>;

auto createImguiPropertyPanel(std::shared_ptr<IPropertyGroup> pPropertyGroup) -> std::shared_ptr<IGuiPanel>;

auto createImguiWorkspace(std::string_view name) -> std::shared_ptr<IGuiWorkspace>;

struct WindowApplicationConfig
{
    std::string name;
    bool isDebugEnabled = false;
};
auto createGlfwWindowApplication(const ILogger& rLogger, WindowApplicationConfig config)
    -> std::shared_ptr<IWindowApplication>;

}  // namespace im3e