#include "imgui_workspace.h"

#include <im3e/utils/imgui_utils.h>

#include <imgui.h>

using namespace im3e;
using namespace std;

void ImguiWorkspace::draw(const ICommandBuffer&)
{
    if (ImguiScope menuBarScope(ImGui::BeginMainMenuBar(), &ImGui::EndMainMenuBar); menuBarScope.isOpen())
    {
        if (ImguiScope viewScope(ImGui::BeginMenu("View"), &ImGui::EndMenuBar); viewScope.isOpen())
        {
            ImGui::SeparatorText("Demos");
            ImGui::MenuItem("ImGui Demo", "Alt+F", &m_imguiDemoVisible);
        }
    }

    ImGui::DockSpaceOverViewport(0U, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_F))
    {
        m_imguiDemoVisible = !m_imguiDemoVisible;
    }

    if (m_imguiDemoVisible)
    {
        ImGui::ShowDemoWindow();
    }
}