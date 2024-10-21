#pragma once

#include <imgui.h>
// #include <implot.h> TODO: bring back implot

#include <memory>

namespace im3e {

class ImguiContext
{
public:
    ImguiContext();

    class IGuard
    {
    public:
        virtual ~IGuard() = default;
    };
    [[nodiscard]] auto makeCurrent() -> std::unique_ptr<IGuard>;

private:
    std::shared_ptr<ImGuiContext> m_pContext;
    // std::shared_ptr<ImPlotContext> m_pPlotContext;
};

}  // namespace im3e