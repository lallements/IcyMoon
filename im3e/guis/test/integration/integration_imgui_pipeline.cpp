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
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Top, make_shared<TestImguiPanel>("Top Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Bottom, make_shared<TestImguiPanel>("Bottom Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Center, make_shared<TestImguiPanel>("Center Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Left, make_shared<TestImguiPanel>("Left Panel"));
        m_pWorkspace->addPanel(IGuiWorkspace::Location::Right, make_shared<TestImguiPanel>("Right Panel"));
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
            .vkOutputFormat = VK_FORMAT_R8G8B8A8_UNORM,
            .frameInFlightCount = 2U,
        },
        createImguiPipeline());
    runTest(3U);

    auto pImageMapping = mapOutputImage();
    pImageMapping->save(generateFilePath("output", "bmp"));
    /*auto expectRgbaPixel = [&](uint32_t x, uint32_t y, array<uint8_t, 4U> expected) {
        const auto& rRgbaPixel = *reinterpret_cast<const array<uint8_t, 4U>*>(pImageMapping->getPixel(x, y));
        EXPECT_THAT(rRgbaPixel, ContainerEq(expected)) << fmt::format("Pixel at [{}; {}]", x, y);
    };
    expectRgbaPixel(0U, 0U, {76U, 92U, 125U, 255U});
    expectRgbaPixel(1U, 1U, {41U, 74U, 122U, 255U});
    expectRgbaPixel(11U, 8U, {255U, 255U, 255U, 255U});
    expectRgbaPixel(85U, 10U, {41U, 74U, 122U, 255U});
    expectRgbaPixel(82U, 143U, {14U, 14U, 14U, 240U});
    expectRgbaPixel(798U, 535U, {14U, 14U, 14U, 240U});
    expectRgbaPixel(799U, 524U, {62U, 62U, 71U, 248U});
    expectRgbaPixel(798U, 598U, {24U, 41U, 61U, 243U});
    expectRgbaPixel(799U, 599U, {62U, 62U, 71U, 248U});*/
}