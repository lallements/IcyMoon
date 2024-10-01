#pragma once

#include <im3e/api/device.h>

#include <memory>

namespace im3e {

class MockDevice : public IDevice
{
public:
    MockDevice();
    ~MockDevice() override;

    auto createMockProxy() -> std::unique_ptr<IDevice>;
};

}  // namespace im3e