#pragma once

#include "anari_device.h"

#include <im3e/utils/loggers.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class AnariWorld : public IAnariWorld
{
public:
    AnariWorld(std::shared_ptr<AnariDevice> pAnDevice);

    void commitChanges();

    auto getHandle() const -> ANARIWorld { return m_pAnWorld.get(); }

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    std::unique_ptr<ILogger> m_pLogger;
    std::shared_ptr<anari::api::World> m_pAnWorld;
};

}  // namespace im3e