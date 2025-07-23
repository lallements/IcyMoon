#pragma once

#include <im3e/utils/loggers.h>

#include <memory>

namespace im3e {

class IGdalInstance
{
public:
    virtual ~IGdalInstance() = default;
};

auto getGdalInstance(const ILogger& rLogger) -> std::shared_ptr<IGdalInstance>;

}  // namespace im3e