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

AnariWorld::AnariWorld(std::shared_ptr<AnariDevice> pAnDevice)
  : m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "Cannot create ANARI World without an ANARI Device"))
  , m_pLogger(m_pAnDevice->createLogger("ANARI World"))
  , m_pAnWorld(createAnariWorld(*m_pLogger, m_pAnDevice->getHandle()))
{
}

void AnariWorld::commitChanges() {}