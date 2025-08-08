#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>
#include <im3e/api/gui.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/properties/properties.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class IAnariWorld
{
public:
    virtual ~IAnariWorld() = default;
};

class IAnariFramePipeline : public IFramePipeline
{
public:
    virtual ~IAnariFramePipeline() = default;

    virtual auto getCameraListener() -> std::shared_ptr<IGuiEventListener> = 0;
};

class IAnariDevice
{
public:
    virtual ~IAnariDevice() = default;

    virtual auto createWorld() -> std::shared_ptr<IAnariWorld> = 0;
    virtual auto createFramePipeline(std::shared_ptr<IDevice> pDevice, std::shared_ptr<IAnariWorld> pAnWorld)
        -> std::unique_ptr<IAnariFramePipeline> = 0;
    virtual auto createRendererProperties() -> std::shared_ptr<IPropertyGroup> = 0;
};

auto createAnariDevice(const ILogger& rLogger) -> std::shared_ptr<IAnariDevice>;

}  // namespace im3e