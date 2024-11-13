#include "glfw_window.h"

#include <fmt/format.h>

using namespace im3e;
using namespace std;

namespace {

GlfwWindowCallbacks& getWindowCallbacks(GLFWwindow* pGlfwWindow)
{
    return *reinterpret_cast<GlfwWindowCallbacks*>(glfwGetWindowUserPointer(pGlfwWindow));
}

void framebufferResizeCallback(GLFWwindow* pGlfwWindow, int width, int height)
{
    getWindowCallbacks(pGlfwWindow).onWindowResized(width, height);
}

void windowIconifyCallback(GLFWwindow* pGlfwWindow, int iconified)
{
    getWindowCallbacks(pGlfwWindow).onWindowIconify(iconified == GLFW_TRUE);
}

auto createWindow(const ILogger& rLogger, std::string_view name, GlfwWindowCallbacks* pCallbacks)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // for Vulkan
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    constexpr int Width = 1200;
    constexpr int Height = 800;

    auto* pGlfwWindow = glfwCreateWindow(Width, Height, name.data(), nullptr, nullptr);
    throwIfNull<runtime_error>(pGlfwWindow, "Could not create GLFW window");

    glfwSetWindowUserPointer(pGlfwWindow, pCallbacks);
    glfwSetFramebufferSizeCallback(pGlfwWindow, framebufferResizeCallback);
    glfwSetWindowIconifyCallback(pGlfwWindow, windowIconifyCallback);

    int width{}, height{};
    glfwGetWindowSize(pGlfwWindow, &width, &height);
    rLogger.info(fmt::format("Created window of size {}x{}", width, height));

    return UniquePtrWithDeleter<GLFWwindow>(pGlfwWindow, [](auto* pW) { glfwDestroyWindow(pW); });
}

}  // namespace

GlfwWindow::GlfwWindow(shared_ptr<IDevice> pDevice, string_view name)
  : m_pDevice(throwIfArgNull(move(pDevice), "Glfw window requires a device"))
  , m_name(name)
  , m_pLogger(m_pDevice->createLogger(m_name))
  , m_pCallbacks([&] {
      auto pCallbacks = make_unique<GlfwWindowCallbacks>();
      pCallbacks->onWindowResized = [this](auto w, auto h) { this->_onWindowResized(w, h); };
      pCallbacks->onWindowIconify = [this](bool i) { this->_onWindowIconify(i); };
      return pCallbacks;
  }())
  , m_pWindow(createWindow(*m_pLogger, m_name, m_pCallbacks.get()))
{
}

void GlfwWindow::draw() {}

void GlfwWindow::_onWindowResized(int width, int height)
{
    m_pLogger->info(fmt::format("Resized to {}x{}", width, height));
}

void GlfwWindow::_onWindowIconify(bool iconify)
{
    m_pLogger->info(fmt::format("Window iconify set to {}", iconify));
    m_iconified = iconify;
}