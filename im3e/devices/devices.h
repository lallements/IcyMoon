#pragma once

#include <im3e/api/device.h>

#include <memory>

namespace im3e {

auto createDevice() -> std::shared_ptr<IDevice>;

}  // namespace im3e