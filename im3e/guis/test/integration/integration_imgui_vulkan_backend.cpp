#include "src/imgui_vulkan_backend.h"

#include "src/imgui_context.h"

#include <im3e/devices/devices.h>
#include <im3e/loggers/loggers.h>
#include <im3e/test_utils/test_utils.h>

#include <fmt/format.h>
#include <imgui_impl_vulkan.h>

using namespace im3e;
using namespace std;

struct ImguiVulkanBackendIntegration : public Test
{
    void TearDown() override { EXPECT_THAT(m_pLoggerTracker->getErrors(), IsEmpty()); }

    unique_ptr<ILogger> m_pLogger = createTerminalLogger();
    UniquePtrWithDeleter<ILoggerTracker> m_pLoggerTracker = m_pLogger->createGlobalTracker();

    shared_ptr<IDevice> m_pDevice = createDevice(*m_pLogger, DeviceConfig{.isDebugEnabled = true});
    shared_ptr<const IImageFactory> m_pImageFactory = m_pDevice->getImageFactory();
    shared_ptr<ICommandQueue> m_pCommandQueue = m_pDevice->getCommandQueue();
    ImguiContext m_imguiContext;
};

TEST_F(ImguiVulkanBackendIntegration, drawWindow)
{
    shared_ptr<IHostVisibleImage> pImage = m_pImageFactory->createHostVisibleImage(ImageConfig{
        .name = "TestImage",
        .vkExtent{800U, 600U},
        .vkFormat = VK_FORMAT_R8G8B8A8_UNORM,
        .vkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    });

    // A ImGui context must be active while using the backend:
    auto pContextGuard = m_imguiContext.makeCurrent();
    ImguiVulkanBackend backend(m_pDevice, pImage, 1U, nullptr);
    {
        auto pCommandBuffer = m_pCommandQueue->startScopedCommand("executeBackend", CommandExecutionType::Sync);

        ImGui_ImplVulkan_NewFrame();

        // When doing offscreen rendering, a display size must be manually specified:
        auto& rIo = ImGui::GetIO();
        rIo.DisplaySize.x = pImage->getVkExtent().width;
        rIo.DisplaySize.y = pImage->getVkExtent().height;

        ImGui::NewFrame();

        ImGui::Render();

        backend.scheduleExecution(*pCommandBuffer);
        {
            auto pBarrier = pCommandBuffer->startScopedBarrier("finalizeImage");
            pBarrier->addImageBarrier(*pImage, ImageBarrierConfig{
                                                   .vkLayout = VK_IMAGE_LAYOUT_GENERAL,
                                               });
        }
    }

    auto pImageMapping = pImage->mapReadOnly();
    auto pData = reinterpret_cast<const uint32_t*>(pImageMapping->getConstData());
    const auto rowPitch = pImageMapping->getRowPitch();
    const auto vkExtent = pImage->getVkExtent();
    for (VkDeviceSize row = 0U; row < vkExtent.height; row++)
    {
        for (VkDeviceSize col = 0U; col < vkExtent.width; col++)
        {
            const auto pixel = *(pData + row * rowPitch + col);
            if (pixel != 0U)
            {
                m_pLogger->info(fmt::format("We got a non-zero pixel! {}", pixel));
            }
        }
    }
}