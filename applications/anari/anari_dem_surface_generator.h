#pragma once

#include <im3e/utils/loggers.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class AnariDemSurfaceGenerator
{
public:
    static auto generate(const ILogger& rLogger, ANARIDevice anDevice) -> std::shared_ptr<anari::api::Surface>;
};

}  // namespace im3e