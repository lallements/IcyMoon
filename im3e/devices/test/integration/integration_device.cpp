#include <im3e/devices/devices.h>

#include <gmock/gmock.h>

using namespace im3e;

class DeviceIntegration : public ::testing::Test
{
};

TEST_F(DeviceIntegration, constructor)
{
    EXPECT_THAT(true, ::testing::IsFalse());
}