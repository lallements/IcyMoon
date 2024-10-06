#include "src/vulkan_images.h"

#include <im3e/mock/mock_device.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

struct ImageFactoryTest : public Test
{
    auto createFactory() { return createVulkanImageFactory(m_mockDevice.createMockProxy()); }

    NiceMock<MockDevice> m_mockDevice;
    MockVulkanDeviceFcts& m_rMockFcts = m_mockDevice.getMockDeviceFcts();
};

TEST_F(ImageFactoryTest, createVulkanImageFactoryThrowsIfDeviceNull)
{
    EXPECT_THROW(auto pFactory = createVulkanImageFactory(nullptr), invalid_argument);
}

TEST_F(ImageFactoryTest, createVulkanImageFactory)
{
    auto pFactory = createFactory();
    ASSERT_THAT(pFactory, NotNull());
}

TEST_F(ImageFactoryTest, createImage)
{
    auto pFactory = createFactory();

    auto pImage = pFactory->createImage(ImageConfig{
        .name = "testImage",
        .vkExtent{.width = 1920U, .height = 1080U},
        .vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT,
        .vkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    });
}