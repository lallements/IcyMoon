#include <im3e/devices/devices.h>
#include <im3e/loggers/loggers.h>
#include <im3e/test_utils/test_utils.h>
#include <im3e/test_utils/vk.h>

using namespace im3e;
using namespace std;

struct VulkanCommandBuffersIntegration : public Test
{
    void TearDown() override { EXPECT_THAT(m_pLoggerTracker->getErrors(), IsEmpty()); }

    unique_ptr<ILogger> m_pLogger = createTerminalLogger();
    UniquePtrWithDeleter<ILoggerTracker> m_pLoggerTracker = m_pLogger->createGlobalTracker();

    shared_ptr<IDevice> m_pDevice = createDevice(*m_pLogger, DeviceConfig{
                                                                 .isDebugEnabled = true,
                                                             });
    shared_ptr<const IImageFactory> m_pImageFactory = m_pDevice->getImageFactory();
    shared_ptr<ICommandQueue> m_pCommandQueue = m_pDevice->getCommandQueue();
};

TEST_F(VulkanCommandBuffersIntegration, clearColorImage)
{
    auto pImage = m_pImageFactory->createHostVisibleImage(ImageConfig{
        .vkExtent{.width = 64U, .height = 64U},
        .vkFormat = VK_FORMAT_R8G8B8A8_SNORM,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    });
    auto pImageMapping = pImage->map();
    auto* const pImageBegin = reinterpret_cast<uint32_t*>(pImageMapping->getData());
    auto* const pImageEnd = pImageBegin + pImageMapping->getSizeInBytes();
    fill(pImageBegin, pImageEnd, 0U);

    {
        auto pCommandBuffer = m_pCommandQueue->startScopedCommand("clearColorImage", CommandExecutionType::Sync);

        const VkClearColorValue vkClearColor{255U, 255U, 255U, 255U};
        const VkImageSubresourceRange vkRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1U,
            .layerCount = 1U,
        };
        m_pDevice->getFcts().vkCmdClearColorImage(pCommandBuffer->getVkCommandBuffer(), pImage->getVkImage(),
                                                  VK_IMAGE_LAYOUT_GENERAL, &vkClearColor, 1U, &vkRange);
    }

    for (VkDeviceSize p = 0U; p < pImageMapping->getSizeInBytes(); p++)
    {
        const auto& rPixel = *reinterpret_cast<const array<uint8_t, 4U>*>(pImageBegin + p);
        EXPECT_THAT(*(pImageBegin + p), Eq(0xFFFFFFFF))
            << fmt::format("pixel #{} = ({}; {}; {}; {})", p, rPixel[0], rPixel[1], rPixel[2], rPixel[3]);
    }
}
