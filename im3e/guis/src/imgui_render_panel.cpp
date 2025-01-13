#include "imgui_render_panel.h"

#include "guis.h"

#include <im3e/utils/throw_utils.h>

#include <imgui.h>
#include <imgui_impl_vulkan.h>

using namespace im3e;
using namespace std;

namespace {

auto getWindowContentRegionSize()
{
    const auto imguiContentRegionMin = ImGui::GetWindowContentRegionMin();
    const auto imguiContentRegionMax = ImGui::GetWindowContentRegionMax();
    return VkExtent2D{
        .width = static_cast<uint32_t>(ceilf(imguiContentRegionMax.x - imguiContentRegionMin.x)),
        .height = static_cast<uint32_t>(ceilf(imguiContentRegionMax.y - imguiContentRegionMin.y)),
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
    constexpr auto RenderOutputFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    const auto newViewportSize = getWindowContentRegionSize();
    if (newViewportSize.width != m_viewportSize.width ||  //
        newViewportSize.height != m_viewportSize.height || m_needsResize)
    {
        m_viewportSize = newViewportSize;
        if (m_viewportSize.width != 0U && m_viewportSize.height != 0U)
        {
            m_needsResize = false;
            m_pFramePipeline->resize(m_viewportSize, m_frameInFlightCount);
            m_pRenderOutput = m_pDevice->getImageFactory()->createImage(ImageConfig{
                .name = fmt::format("{}RenderOutput", m_name),
                .vkExtent = m_viewportSize,
                .vkFormat = m_vkWindowFormat,
                .vkUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            });
            m_pRenderOutputView = m_pRenderOutput->createView();
            m_vkRenderOutputSet = ImGui_ImplVulkan_AddTexture(
                m_pRenderOutputSampler.get(), m_pRenderOutputView->getVkImageView(), RenderOutputFinalLayout);
        }
    }

    if (!m_pRenderOutput)
    {
        return;
    }

    m_pFramePipeline->prepareExecution(rCommandBuffer, m_pRenderOutput);
    {
        auto pBarrierRecorder = rCommandBuffer.startScopedBarrier(fmt::format("barrierToDrawIn{}", m_name));
        pBarrierRecorder->addImageBarrier(*m_pRenderOutput,
                                          ImageBarrierConfig{
                                              .vkDstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                              .vkDstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                                              .vkLayout = RenderOutputFinalLayout,
                                          });
    }

    const auto imViewportSize = ImVec2(m_viewportSize.width, m_viewportSize.height);
    const auto imagePos = ImGui::GetCursorScreenPos();
    ImGui::SetNextItemAllowOverlap();
    ImGui::Image(reinterpret_cast<ImTextureID>(m_vkRenderOutputSet), imViewportSize);
    ImGui::SetCursorScreenPos(imagePos);
    ImGui::InvisibleButton(fmt::format("{}_mouseArea", m_name).c_str(), imViewportSize,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
}

void ImguiRenderPanel::onWindowResized(const VkExtent2D&, VkFormat vkFormat, uint32_t frameInFlightCount)
{
    m_frameInFlightCount = frameInFlightCount;
    m_vkWindowFormat = vkFormat;
    m_needsResize = true;
}

auto im3e::createImguiRenderPanel(string_view name, unique_ptr<IFramePipeline> pFramePipeline) -> shared_ptr<IGuiPanel>
{
    return make_shared<ImguiRenderPanel>(name, move(pFramePipeline));
}