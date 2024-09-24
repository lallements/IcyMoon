#include "device.h"

#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

Device::Device(const ILogger& rLogger, DeviceConfig config)
  : m_pLogger(rLogger.createChild("Device"))
  , m_config(move(config))
  , m_instance(*m_pLogger, m_config.isDebugEnabled)
{
}

auto im3e::createDevice(const ILogger& rLogger, DeviceConfig config) -> shared_ptr<IDevice>
{
    return make_shared<Device>(rLogger, move(config));
}