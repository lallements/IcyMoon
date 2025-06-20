#include "imgui_render_panel.h"

#include "guis.h"

#include <im3e/utils/throw_utils.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>

using namespace im3e;
using namespace std;

namespace {

constexpr auto RenderOutputFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

auto getWindowContentRegionSize()
{
    const auto imguiContentRegionMin = ImGui::GetWindowContentRegionMin();
    const auto imguiContentRegionMax = ImGui::GetWindowContentRegionMax();
    return ImVec2{
        ceilf(imguiContentRegionMax.x - imguiContentRegionMin.x),
        ceilf(imguiContentRegionMax.y - imguiContentRegionMin.y),
    };
}

auto toVkExtent2D(const ImVec2& rImVec2)
{
    return VkExtent2D{
        .width = static_cast<uint32_t>(rImVec2.x),
        .height = static_cast<uint32_t>(rImVec2.y),
    };
}

auto createRenderOutputSampler(const IDevice& rDevice)
{
    const auto vkDevice = rDevice.getVkDevice();
    const auto& rVkFcts = rDevice.getFcts();

    VkSamplerCreateInfo vkSamplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
    };

    VkSampler vkSampler{};
    throwIfVkFailed(rVkFcts.vkCreateSampler(vkDevice, &vkSamplerInfo, nullptr, &vkSampler),
                    "Failed to create sampler for render panel output");

    return makeVkUniquePtr<VkSampler>(vkDevice, vkSampler, rVkFcts.vkDestroySampler);
}

}  // namespace

ImguiRenderPanel::ImguiRenderPanel(string_view name, unique_ptr<IFramePipeline> pFramePipeline)
  : m_name(name)
  , m_pFramePipeline(throwIfArgNull(move(pFramePipeline), "ImGui render panel requires a frame pipeline"))
  , m_pDevice(m_pFramePipeline->getDevice())
  , m_pRenderOutputSampler(createRenderOutputSampler(*m_pDevice))
{
}

void ImguiRenderPanel::draw(const ICommandBuffer& rCommandBuffer)
{
    if (!m_pRenderOutput)
    {
        return;
    }

    const auto imViewportSize = getWindowContentRegionSize();
    m_pFramePipeline->prepareExecution(rCommandBuffer, toVkExtent2D(imViewportSize), m_pRenderOutput);
    {
        auto pBarrierRecorder = rCommandBuffer.startScopedBarrier(fmt::format("barrierToDrawIn{}", m_name));
        pBarrierRecorder->addImageBarrier(*m_pRenderOutput,
                                          ImageBarrierConfig{
                                              .vkDstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                              .vkDstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                                              .vkLayout = RenderOutputFinalLayout,
                                          });
    }

    const auto imagePos = ImGui::GetCursorScreenPos();
    const auto renderOutputSize = m_pRenderOutput->getVkExtent();
    const ImVec2 maxUV{
        imViewportSize.x / static_cast<float>(renderOutputSize.width),
        imViewportSize.y / static_cast<float>(renderOutputSize.height),
    };

    ImGui::SetNextItemAllowOverlap();
    ImGui::Image(reinterpret_cast<ImTextureID>(m_vkRenderOutputSet), imViewportSize, ImVec2{}, maxUV);
    ImGui::SetCursorScreenPos(imagePos);
    ImGui::InvisibleButton(fmt::format("{}_mouseArea", m_name).c_str(), imViewportSize,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
}

void ImguiRenderPanel::onWindowResized(const VkExtent2D& rVkWindowSize, VkFormat vkFormat, uint32_t frameInFlightCount)
{
    m_pFramePipeline->resize(rVkWindowSize, frameInFlightCount);

    m_pRenderOutput = m_pDevice->getImageFactory()->createImage(ImageConfig{
        .name = fmt::format("{}.RenderOutput", m_name),
        .vkExtent = rVkWindowSize,
        .vkFormat = vkFormat,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    });
    m_pRenderOutputView = m_pRenderOutput->createView();
    m_vkRenderOutputSet = ImGui_ImplVulkan_AddTexture(m_pRenderOutputSampler.get(),
                                                      m_pRenderOutputView->getVkImageView(), RenderOutputFinalLayout);
}

auto im3e::createImguiRenderPanel(string_view name, unique_ptr<IFramePipeline> pFramePipeline) -> shared_ptr<IGuiPanel>
{
    return make_shared<ImguiRenderPanel>(name, move(pFramePipeline));
}