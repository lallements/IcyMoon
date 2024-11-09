#pragma once

#include <im3e/api/command_buffer.h>
#include <im3e/api/gui.h>

#include <imgui.h>

#include <map>

namespace im3e {

struct ImguiWorkspacePanelInfo
{
    std::shared_ptr<IGuiPanel> pPanel;
    float fraction = 0.25F;
    ImGuiID dockSpaceId;
};

class ImguiWorkspace : public IGuiWorkspace
{
public:
    ImguiWorkspace(std::string_view name);

    void draw(const ICommandBuffer& rCommandBuffer) override;

    void addPanel(Location location, std::shared_ptr<IGuiPanel> pPanel, float fraction = 0.25F) override;

    auto getName() const -> std::string override { return m_name; }

private:
    const std::string m_name;
    bool m_workspaceInitialized = false;
    bool m_imguiDemoVisible = false;

    ImguiWorkspacePanelInfo m_centerPanel;
    std::map<Location, ImguiWorkspacePanelInfo> m_panelInfos;
};

}  // namespace im3e