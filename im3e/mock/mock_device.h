#pragma once

#include "mock_image.h"

#include <im3e/api/device.h>

#include <gmock/gmock.h>

#include <memory>

namespace im3e {

class MockDevice : public IDevice
{
public:
    MockDevice();
    ~MockDevice() override;

    MOCK_METHOD(std::shared_ptr<const IImageFactory>, getImageFactory, (), (const, override));

    auto createMockProxy() -> std::unique_ptr<IDevice>;

    auto getMockImageFactory() -> MockImageFactory& { return m_mockImageFactory; }

private:
    ::testing::NiceMock<MockImageFactory> m_mockImageFactory;
};

}  // namespace im3e