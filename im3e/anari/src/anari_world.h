#pragma once

#include "anari.h"

#include <im3e/utils/loggers.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class AnariWorld : public IAnariWorld
{
public:
    AnariWorld(const ILogger& rLogger, ANARIDevice anDevice);

    void commitChanges();

    auto getHandle() const -> ANARIWorld { return m_pAnWorld.get(); }

private:
    std::unique_ptr<ILogger> m_pLogger;
    ANARIDevice m_anDevice;
    std::shared_ptr<anari::api::World> m_pAnWorld;
};

}  // namespace im3e