// Must be included first to guarantee it is not included before OpenGL functions (a requirement from GLEW)
#include <GL/glew.h>

#include "offscreen_opengl_context.h"

#include <fmt/format.h>

using namespace im3e;
using namespace std;

namespace {

auto createInvisibleWindow()
{
    // Important: USD Hydra still uses legacy OpenGL function, setting FORWARD_COMPAT and profile to CORE would cause
    // Hydra to generate error messages:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    constexpr auto Width = 800U;
    constexpr auto Height = 600U;
    auto pGlfwWindow = glfwCreateWindow(Width, Height, "Offscreen GL", nullptr, nullptr);
    throwIfNull<runtime_error>(pGlfwWindow, "Could not create invisible GLFW window for OpenGL offscreen rendering");
    UniquePtrWithDeleter<GLFWwindow> pWindow(pGlfwWindow, [](auto* pGlfwWindow) { glfwDestroyWindow(pGlfwWindow); });

    glfwMakeContextCurrent(pGlfwWindow);

    glewExperimental = GL_TRUE;
    if (auto errorCode = glewInit(); errorCode != GLEW_OK)
    {
        const auto* pErrorMsg = reinterpret_cast<const char*>(glewGetErrorString(errorCode));
        throw runtime_error(fmt::format("Failed to initialize GLEW: {}", pErrorMsg));
    }

    return pWindow;
}

}  // namespace

OffscreenOpenGlContext::OffscreenOpenGlContext()
  : m_pWindow(createInvisibleWindow())
{
}

namespace {

class OffscreenOpenGlContextGuard : public IGlContext::IGuard
{
public:
    OffscreenOpenGlContextGuard(shared_ptr<OffscreenOpenGlContext> pContext, GLFWwindow* pGlfwWindow)
      : m_pContext(throwIfArgNull(move(pContext), "Offscreen OpenGL context guard requires a context"))
      , m_pGlfwWindow(throwIfArgNull(move(pGlfwWindow), "Offscreen OpenGL context guard requires a GLFW window"))
    {
        glfwMakeContextCurrent(m_pGlfwWindow);
    }

    ~OffscreenOpenGlContextGuard() override { glfwMakeContextCurrent(nullptr); }

private:
    shared_ptr<OffscreenOpenGlContext> m_pContext;
    GLFWwindow* m_pGlfwWindow;
};

}  // namespace

auto OffscreenOpenGlContext::makeCurrent() -> unique_ptr<IGuard>
{
    return make_unique<OffscreenOpenGlContextGuard>(shared_from_this(), m_pWindow.get());
}