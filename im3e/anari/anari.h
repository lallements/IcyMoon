#pragma once

#include <im3e/api/anari.h>
#include <im3e/utils/loggers.h>

#include <memory>

namespace im3e {

auto createAnariDevice(const ILogger& rLogger) -> std::shared_ptr<IAnariDevice>;

}  // namespace im3e