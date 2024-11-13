#pragma once

#include <im3e/api/device.h>

#include <memory>

namespace im3e {

class IWindow
{
public:
    virtual ~IWindow() = default;
};

class IWindowApplication
{
public:
    virtual ~IWindowApplication() = default;

    virtual void createWindow() = 0;

    virtual void run() = 0;

    virtual auto getDevice() -> std::shared_ptr<IDevice> = 0;
    virtual auto getDevice() const -> std::shared_ptr<const IDevice> = 0;
};

}  // namespace im3e