#include "anari_world.h"

#include <im3e/utils/core/throw_utils.h>

using namespace im3e;

namespace {

auto createAnariWorld(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anWorld = anariNewWorld(anDevice);
    rLogger.debug("Created new world");
    return std::shared_ptr<anari::api::World>(anWorld, [anDevice, pLogger = &rLogger](auto* anWorld) {
        anariRelease(anDevice, anWorld);
        pLogger->debug("Destroyed world");
    });
}

}  // namespace

AnariWorld::AnariWorld(const ILogger& rLogger, ANARIDevice anDevice)
  : m_pLogger(rLogger.createChild("ANARI World"))
  , m_anDevice(throwIfArgNull(anDevice, "Cannot create ANARI World without a device"))
  , m_pAnWorld(createAnariWorld(*m_pLogger, m_anDevice))
{
}