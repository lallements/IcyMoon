#include "src/imgui_pipeline.h"

#include "src/imgui_workspace.h"

#include <im3e/test_utils/pipeline_integration_test.h>
#include <im3e/utils/imgui_utils.h>

#include <fmt/format.h>

using namespace im3e;
using namespace std;

namespace {

inline void blitImage(const VulkanDeviceFcts& rFcts, VkCommandBuffer vkCommandBuffer, const VkExtent2D& rVkExtent,
                      VkImage vkSrcImage, VkImage vkDstImage)
{
    const VkImageSubresourceLayers vkSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1U};
    const VkOffset3D vkStartOffset{};
    const VkOffset3D vkEndOffset{
        .x = vkStartOffset.x + static_cast<int32_t>(rVkExtent.width),
        .y = vkStartOffset.y + static_cast<int32_t>(rVkExtent.height),
        .z = 1,
    };

    VkImageBlit vkImageBlit{
        .srcSubresource = vkSubresourceLayers,
        .srcOffsets{vkStartOffset, vkEndOffset},
        .dstSubresource = vkSubresourceLayers,
        .dstOffsets{vkStartOffset, vkEndOffset},
    };

    rFcts.vkCmdBlitImage(vkCommandBuffer, vkSrcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkDstImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &vkImageBlit, VK_FILTER_NEAREST);
}

class TestImguiPanel : public IGuiPanel
{
public:
    TestImguiPanel(std::string_view content)
      : m_content(content)
    {
    }

    void draw(const ICommandBuffer&) override { ImGui::Text("%s", m_content.c_str()); }

    auto getName() const -> string override { return m_content; }

private:
    std::string m_content;
};

struct ImguiPipelineIntegration : public PipelineIntegrationTest
{
    ImguiPipelineIntegration()
    {
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Left, make_shared<TestImguiPanel>("Left Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Top, make_shared<TestImguiPanel>("Top Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Bottom, make_shared<TestImguiPanel>("Bottom Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Right, make_shared<TestImguiPanel>("Right Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Center, make_shared<TestImguiPanel>("Center Panel"));
    }

    auto createImguiPipeline() { return make_unique<ImguiPipeline>(getDevice(), nullptr, m_pWorkspace); }

    shared_ptr<ImguiWorkspace> m_pWorkspace = make_shared<ImguiWorkspace>("Test Workspace");
};

}  // namespace

TEST_F(ImguiPipelineIntegration, emptyWorkspace)
{
    constexpr VkExtent2D TestOutputExtent{800U, 600U};
    constexpr auto TestOutputFormat = VK_FORMAT_R8G8B8A8_UNORM;

    shared_ptr<IImage> pGuiImage = getDevice()->getImageFactory()->createImage(ImageConfig{
        .name = "ImguiBackendImage",
        .vkExtent = TestOutputExtent,
        .vkFormat = TestOutputFormat,
        .vkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });

    initialize(
        PipelineIntegrationTest::Config{
            .vkOutputExtent = TestOutputExtent,
            .vkOutputFormat = VK_FORMAT_R8G8B8A8_SRGB,
            .frameInFlightCount = 2U,
        },
        createImguiPipeline());
    runTest(3U);

    auto pImageMapping = mapOutputImage();
    pImageMapping->save(generateFilePath("output", "bmp"));
    auto expectRgbaPixel = [&](uint32_t x, uint32_t y, array<uint8_t, 4U> expected) {
        const auto& rRgbaPixel = *reinterpret_cast<const array<uint8_t, 4U>*>(pImageMapping->getPixel(x, y));
        EXPECT_THAT(rRgbaPixel, ContainerEq(expected)) << fmt::format("Pixel at [{}; {}]", x, y);
    };
    expectRgbaPixel(768U, 160U, {46U, 105U, 175U, 255U});
    expectRgbaPixel(60U, 10U, {28U, 28U, 28U, 255U});
    expectRgbaPixel(379U, 23U, {82U, 82U, 95U, 255U});
    expectRgbaPixel(55U, 238U, {0U, 13U, 28U, 254U});
    expectRgbaPixel(200U, 288U, {79U, 80U, 95U, 255U});
    expectRgbaPixel(290U, 192U, {208U, 209U, 209U, 255U});
    expectRgbaPixel(550U, 167U, {0U, 0U, 0U, 255U});
    expectRgbaPixel(531U, 511U, {28U, 64U, 109U, 255U});
    expectRgbaPixel(272U, 595U, {0U, 13U, 28U, 254U});
}