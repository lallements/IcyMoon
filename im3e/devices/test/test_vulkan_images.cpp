#include "src/vulkan_images.h"

#include <im3e/mock/mock_device.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

struct ImageFactoryTest : public Test
{
    NiceMock<MockDevice> m_mockDevice;
};

TEST_F(ImageFactoryTest, createVulkanImageFactory)
{
    auto pFactory = createVulkanImageFactory(m_mockDevice.createMockProxy());
    ASSERT_THAT(pFactory, NotNull());
}

TEST_F(ImageFactoryTest, createVulkanImageFactoryThrowsIfDeviceNull)
{
    EXPECT_THROW(auto pFactory = createVulkanImageFactory(nullptr), invalid_argument);
}