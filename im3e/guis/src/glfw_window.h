#pragma once

#include "imgui_workspace.h"
#include "presenter.h"

#include <im3e/api/gui.h>
#include <im3e/api/window.h>

#include <GLFW/glfw3.h>

namespace im3e {

struct GlfwWindowCallbacks
{
    std::function<void(int width, int height)> onWindowResized;
    std::function<void(bool iconified)> onWindowIconify;
};

class GlfwWindow : public IWindow
{
public:
    struct Config
    {
        std::string name;
        std::optional<std::string> iniFilename{"imgui.ini"};
    };
    GlfwWindow(std::shared_ptr<IDevice> pDevice, Config config, std::shared_ptr<ImguiWorkspace> pWorkspace);

    void draw();

    auto shouldClose() const -> bool { return glfwWindowShouldClose(m_pWindow.get()) != 0; }
    auto isIconified() const -> bool { return m_iconified; }

private:
    void _onWindowResized(int width, int height);
    void _onWindowIconify(bool iconified);

    std::shared_ptr<IDevice> m_pDevice;
    const Config m_config;
    std::shared_ptr<ImguiWorkspace> m_pWorkspace;
    std::unique_ptr<ILogger> m_pLogger;

    std::unique_ptr<GlfwWindowCallbacks> m_pCallbacks;
    UniquePtrWithDeleter<GLFWwindow> m_pWindow;
    VkUniquePtr<VkSurfaceKHR> m_pVkSurface;
    bool m_iconified;

    std::unique_ptr<Presenter> m_pPresenter;
};

}  // namespace im3e