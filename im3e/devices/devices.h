#pragma once

#include <im3e/api/device.h>
#include <im3e/api/logger.h>

#include <memory>

namespace im3e {

struct DeviceConfig
{
    bool isDebugEnabled = false;
};
auto createDevice(const ILogger& rLogger, DeviceConfig config = {}) -> std::shared_ptr<IDevice>;

}  // namespace im3e