#include "imgui_context.h"

using namespace im3e;
using namespace std;

namespace {

// TODO: bring back ImPlot

class CurrentContextGuard : public ImguiContext::IGuard
{
public:
    CurrentContextGuard(shared_ptr<ImGuiContext> pContext /*, shared_ptr<ImPlotContext> pPlotContext*/)
      : m_pPrevContext(ImGui::GetCurrentContext())
      //, m_pPrevPlotContext(ImPlot::GetCurrentContext())
      , m_pContext(move(pContext))
    //, m_pPlotContext(move(pPlotContext))
    {
        ImGui::SetCurrentContext(m_pContext.get());
        // ImPlot::SetCurrentContext(m_pPlotContext.get());
    }

    ~CurrentContextGuard() override
    {
        ImGui::SetCurrentContext(m_pPrevContext);
        // ImPlot::SetCurrentContext(m_pPrevPlotContext);
    }

private:
    ImGuiContext* m_pPrevContext{};
    // ImPlotContext* m_pPrevPlotContext{};
    shared_ptr<ImGuiContext> m_pContext;
    // shared_ptr<ImPlotContext> m_pPlotContext;
};

}  // namespace

ImguiContext::ImguiContext()
  : m_pContext([&] {
      IMGUI_CHECKVERSION();
      auto pContext = ImGui::CreateContext();
      return shared_ptr<ImGuiContext>(pContext, [](auto* pC) { ImGui::DestroyContext(pC); });
  }())
//, m_pPlotContext(ImPlot::CreateContext(), [](auto* pC) { ImPlot::DestroyContext(pC); })
{
}

auto ImguiContext::makeCurrent() -> unique_ptr<IGuard>
{
    return make_unique<CurrentContextGuard>(m_pContext /*, m_pPlotContext*/);
}
