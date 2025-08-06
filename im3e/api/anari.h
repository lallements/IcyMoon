#pragma once

#include "device.h"
#include "frame_pipeline.h"
#include "gui.h"

#include <anari/anari.h>

#include <memory>

namespace im3e {

class IAnariWorld
{
public:
    virtual ~IAnariWorld() = default;

    virtual auto getHandle() const -> ANARIWorld = 0;
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

    virtual auto createWorld() const -> std::shared_ptr<IAnariWorld> = 0;
    virtual auto createFramePipeline(std::shared_ptr<IDevice> pDevice, std::shared_ptr<IAnariWorld> pAnWorld)
        -> std::unique_ptr<IAnariFramePipeline> = 0;
};

}  // namespace im3e