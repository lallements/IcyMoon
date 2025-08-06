#pragma once

#include <im3e/api/anari.h>
#include <im3e/utils/loggers.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class AnariWorld : public IAnariWorld
{
public:
    AnariWorld(const ILogger& rLogger, ANARIDevice anDevice);

private:
    std::unique_ptr<ILogger> m_pLogger;
    ANARIDevice m_anDevice;
};

}  // namespace im3e