#include <im3e/devices/devices.h>

#include <gmock/gmock.h>

using namespace im3e;

class DeviceIntegration : public ::testing::Test
{
};

TEST_F(DeviceIntegration, constructor)
{
    auto pDevice = createDevice();
    ASSERT_THAT(pDevice, ::testing::NotNull());
}