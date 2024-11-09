#pragma once

#include <im3e/api/command_buffer.h>
#include <im3e/api/gui.h>

namespace im3e {

class ImguiWorkspace : public IGuiWorkspace
{
public:
    ImguiWorkspace() = default;

    void draw(const ICommandBuffer& rCommandBuffer);

private:
    bool m_imguiDemoVisible = false;
};

}  // namespace im3e