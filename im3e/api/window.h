#pragma once

#include <im3e/api/device.h>
#include <im3e/api/gui.h>

#include <functional>
#include <memory>

namespace im3e {

class IGlContext
{
public:
    virtual ~IGlContext() = default;

    class IGuard
    {
    public:
        virtual ~IGuard() = default;
    };
    [[nodiscard]] virtual auto makeCurrent() -> std::unique_ptr<IGuard> = 0;
};

class IWindowApplication
{
public:
    virtual ~IWindowApplication() = default;

    virtual void createWindow(std::shared_ptr<IGuiWorkspace> pWorkspace) = 0;

    /// @brief Create an invisible window to perform offscreen rendering with OpenGL.
    virtual auto createOffscreenOpenGlContext() -> std::shared_ptr<IGlContext> = 0;

    /// @brief Starts execution of the application.
    /// This includes running the execution loop that listens to input events (keyboard, mouse) and refreshes windows
    /// that have been created prior to calling this function.
    /// The function is blocking and will only release once all windows have been closed or IWindowApplication::stop()
    /// is called.
    ///
    /// @param loopIterationFct Function to be called by the application at the start of every iteration within the
    /// execution loop.
    virtual void run(std::function<void()> loopIterationFct = {}) = 0;

    /// @brief Stops the execution of the application.
    virtual void stop() = 0;

    virtual auto getDevice() -> std::shared_ptr<IDevice> = 0;
    virtual auto getDevice() const -> std::shared_ptr<const IDevice> = 0;
};

}  // namespace im3e