#include "src/imgui_render_panel.h"

#include "src/imgui_pipeline.h"
#include "src/imgui_workspace.h"

#include <im3e/test_utils/pipeline_integration_test.h>

using namespace im3e;
using namespace std;

namespace {

class TestPipeline : public IFramePipeline
{
public:
    TestPipeline(shared_ptr<const IDevice> pDevice, const VkClearColorValue& rVkClearColor)
      : m_pDevice(move(pDevice))
      , m_vkClearColor(rVkClearColor)
    {
    }

    void prepareExecution(const ICommandBuffer& rCommandBuffer, shared_ptr<IImage> pOutputImage) override
    {
        {
            auto pBarrierRecorder = rCommandBuffer.startScopedBarrier("beforeClearColor");
            pBarrierRecorder->addImageBarrier(*pOutputImage, ImageBarrierConfig{
                                                                 .vkDstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT,
                                                                 .vkDstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                                 .vkLayout = VK_IMAGE_LAYOUT_GENERAL,
                                                             });
        }

        const VkImageSubresourceRange vkRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1U,
            .layerCount = 1U,
        };

        const auto& rFcts = m_pDevice->getFcts();
        rFcts.vkCmdClearColorImage(rCommandBuffer.getVkCommandBuffer(), pOutputImage->getVkImage(),
                                   VK_IMAGE_LAYOUT_GENERAL, &m_vkClearColor, 1U, &vkRange);
    }

    void resize(const VkExtent2D&, uint32_t) override {}

    auto getDevice() const -> shared_ptr<const IDevice> override { return m_pDevice; }

private:
    shared_ptr<const IDevice> m_pDevice;
    const VkClearColorValue m_vkClearColor{};
};

}  // namespace

struct ImguiRenderPanelIntegration : public PipelineIntegrationTest
{
    auto createImguiPipeline()
    {
        return make_unique<ImguiPipeline>(getDevice(), nullptr, m_pWorkspace,
                                          fmt::format("{}.{}.ini", getSuiteName(), getName()));
    }

    auto addRenderPanel(const VkClearColorValue& rVkClearColor)
    {
        auto pRenderPanel = make_shared<ImguiRenderPanel>("Render",
                                                          make_unique<TestPipeline>(getDevice(), rVkClearColor));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Center, pRenderPanel, 0.5F);
    }

    auto runPanelTest(VkFormat vkOutputFormat)
    {
        initialize(
            PipelineIntegrationTest::Config{
                .vkOutputExtent = VkExtent2D{800U, 600U},
                .vkOutputFormat = vkOutputFormat,
                .frameInFlightCount = 1U,
            },
            createImguiPipeline());

        runTest(2U);
    }

    shared_ptr<ImguiWorkspace> m_pWorkspace = make_shared<ImguiWorkspace>("Test Workspace");
};

TEST_F(ImguiRenderPanelIntegration, toSRgb)
{
    constexpr VkClearColorValue ClearColor{0.0177445058F, 0.0656561702F, 0.198943928F, 1.0F};
    constexpr array<uint8_t, 4U> ExpectedColor{36U, 72U, 123U, 255U};

    addRenderPanel(ClearColor);
    runPanelTest(VK_FORMAT_R8G8B8A8_SRGB);

    auto pImageMapping = mapOutputImage();
    expectRgbaPixelRegion(*pImageMapping, {8U, 56U}, {792U, 592U}, ExpectedColor);
}

TEST_F(ImguiRenderPanelIntegration, toRgb)
{
    constexpr VkClearColorValue ClearColor{0.5F, 0.5F, 0.5F, 1.0F};
    constexpr array<uint8_t, 4U> ExpectedColor{127U, 127U, 127U, 255U};

    addRenderPanel(ClearColor);
    runPanelTest(VK_FORMAT_R8G8B8A8_UNORM);

    auto pImageMapping = mapOutputImage();
    expectRgbaPixelRegion(*pImageMapping, {8U, 56U}, {792U, 592U}, ExpectedColor);
}