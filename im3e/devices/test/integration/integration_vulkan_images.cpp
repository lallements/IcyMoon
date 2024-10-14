#include <im3e/devices/devices.h>
#include <im3e/loggers/loggers.h>
#include <im3e/test_utils/test_utils.h>
#include <im3e/test_utils/vk.h>

using namespace im3e;
using namespace std;

struct VulkanImageIntegration : public Test
{
    void TearDown() override { EXPECT_THAT(m_pLoggerTracker->getErrors(), IsEmpty()); }

    unique_ptr<ILogger> m_pLogger = createTerminalLogger();
    UniquePtrWithDeleter<ILoggerTracker> m_pLoggerTracker = m_pLogger->createGlobalTracker();

    shared_ptr<IDevice> m_pDevice = createDevice(*m_pLogger, DeviceConfig{
                                                                 .isDebugEnabled = true,
                                                             });
    shared_ptr<const IImageFactory> m_pImageFactory = m_pDevice->getImageFactory();
};

TEST_F(VulkanImageIntegration, createImageRgbaSnorm)
{
    constexpr VkExtent2D ExpectedExtent{.width = 256U, .height = 120U};
    constexpr VkFormat ExpectedFormat = VK_FORMAT_R8G8B8A8_SNORM;

    auto pImage = m_pImageFactory->createImage(ImageConfig{
        .name = "testImage",
        .vkExtent = ExpectedExtent,
        .vkFormat = ExpectedFormat,
        .vkUsage = VK_IMAGE_USAGE_STORAGE_BIT,
    });
    ASSERT_THAT(pImage, NotNull());
    EXPECT_THAT(pImage->getVkImage(), Ne(VK_NULL_HANDLE));
    EXPECT_THAT(pImage->getVkExtent(), Eq(ExpectedExtent));
    EXPECT_THAT(pImage->getFormat(), Eq(ExpectedFormat));
}

TEST_F(VulkanImageIntegration, createImageRgbaSrgb)
{
    constexpr VkExtent2D ExpectedExtent{.width = 120U, .height = 64U};
    constexpr VkFormat ExpectedFormat = VK_FORMAT_R8G8B8A8_SRGB;

    auto pImage = m_pImageFactory->createImage(ImageConfig{
        .name = "testImage",
        .vkExtent = ExpectedExtent,
        .vkFormat = ExpectedFormat,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });
    ASSERT_THAT(pImage, NotNull());
    EXPECT_THAT(pImage->getVkImage(), Ne(VK_NULL_HANDLE));
    EXPECT_THAT(pImage->getVkExtent(), Eq(ExpectedExtent));
    EXPECT_THAT(pImage->getFormat(), Eq(ExpectedFormat));
}

TEST_F(VulkanImageIntegration, createHostVisibleImageRgbaUnorm)
{
    constexpr VkExtent2D ExpectedExtent{.width = 24U, .height = 10U};
    constexpr VkFormat ExpectedFormat = VK_FORMAT_R8G8B8A8_UNORM;

    auto pImage = m_pImageFactory->createHostVisibleImage(ImageConfig{
        .name = "testHostVisibleImage",
        .vkExtent = ExpectedExtent,
        .vkFormat = ExpectedFormat,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });
    ASSERT_THAT(pImage, NotNull());
    EXPECT_THAT(pImage->getVkImage(), Ne(VK_NULL_HANDLE));
    EXPECT_THAT(pImage->getVkExtent(), Eq(ExpectedExtent));
    EXPECT_THAT(pImage->getFormat(), Eq(ExpectedFormat));

    auto pImageMapping = pImage->map();
    EXPECT_THAT(pImageMapping->getData(), NotNull());
    EXPECT_THAT(pImageMapping->getConstData(), NotNull());
    EXPECT_THAT(pImageMapping->getRowPitch(), Eq(ExpectedExtent.width * sizeof(uint32_t)));
    EXPECT_THAT(pImageMapping->getSizeInBytes(), Eq(ExpectedExtent.width * ExpectedExtent.height * sizeof(uint32_t)));
}