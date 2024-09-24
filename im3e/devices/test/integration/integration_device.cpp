#include <im3e/devices/devices.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;

class DeviceIntegration : public Test
{
};

TEST_F(DeviceIntegration, constructor)
{
    auto pDevice = createDevice();
    ASSERT_THAT(pDevice, NotNull());
}