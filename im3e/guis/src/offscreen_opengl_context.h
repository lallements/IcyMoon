#pragma once

#include <im3e/api/window.h>
#include <im3e/utils/types.h>

#include <GLFW/glfw3.h>

#include <memory>

namespace im3e {

class OffscreenOpenGlContext : public IGlContext, public std::enable_shared_from_this<OffscreenOpenGlContext>
{
public:
    OffscreenOpenGlContext();

    [[nodiscard]] auto makeCurrent() -> std::unique_ptr<IGuard> override;

private:
    UniquePtrWithDeleter<GLFWwindow> m_pWindow;
};

}  // namespace im3e