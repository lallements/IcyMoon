#pragma once

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
    GlfwWindow(std::shared_ptr<IDevice> pDevice, std::string_view name);

    void draw();

    auto shouldClose() const -> bool { return glfwWindowShouldClose(m_pWindow.get()) != 0; }
    auto isIconified() const -> bool { return m_iconified; }

private:
    void _onWindowResized(int width, int height);
    void _onWindowIconify(bool iconified);

    std::shared_ptr<IDevice> m_pDevice;
    const std::string m_name;
    std::unique_ptr<ILogger> m_pLogger;

    std::unique_ptr<GlfwWindowCallbacks> m_pCallbacks;
    UniquePtrWithDeleter<GLFWwindow> m_pWindow;

    bool m_iconified;
};

}  // namespace im3e