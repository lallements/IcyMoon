#pragma once

#include "device.h"
#include "frame_pipeline.h"

#include <memory>

namespace im3e {

class IAnariWorld
{
public:
    virtual ~IAnariWorld() = default;
};

class IAnariDevice
{
public:
    virtual ~IAnariDevice() = default;

    virtual auto createWorld() const -> std::shared_ptr<IAnariWorld> = 0;
    virtual auto createFramePipeline(std::shared_ptr<IDevice> pDevice) -> std::unique_ptr<IFramePipeline> = 0;
};

}  // namespace im3e