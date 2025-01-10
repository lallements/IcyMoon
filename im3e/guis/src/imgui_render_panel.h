#pragma once

#include <im3e/api/frame_pipeline.h>
#include <im3e/api/gui.h>
#include <im3e/api/image.h>

namespace im3e {

class ImguiRenderPanel : public IGuiPanel
{
public:
    ImguiRenderPanel(std::string_view name, std::unique_ptr<IFramePipeline> pFramePipeline);

    void draw(const ICommandBuffer& rCommandBuffer) override;

    void onWindowResized(const VkExtent2D& rVkWindowSize, VkFormat vkFormat, uint32_t frameInFlightCount) override;

    auto getName() const -> std::string override { return m_name; }

private:
    std::string m_name;
    std::unique_ptr<IFramePipeline> m_pFramePipeline;
    std::shared_ptr<const IDevice> m_pDevice;

    VkExtent2D m_viewportSize{};
    VkFormat m_vkWindowFormat{};
    uint32_t m_frameInFlightCount{};
    bool m_needsResize{};  // set to true if the frame pipeline should be resized in the next draw call

    std::shared_ptr<IImage> m_pRenderOutput;
    std::shared_ptr<IImageView> m_pRenderOutputView;
    VkUniquePtr<VkSampler> m_pRenderOutputSampler;
    VkDescriptorSet m_vkRenderOutputSet;
};

}  // namespace im3e