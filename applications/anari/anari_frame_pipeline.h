#pragma once

#include "anari_map_camera.h"

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>
#include <im3e/api/image.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class AnariFramePipeline : public IFramePipeline
{
public:
    AnariFramePipeline(const ILogger& rLogger, std::shared_ptr<IDevice> pDevice,
                       std::shared_ptr<anari::api::Device> pAnDevice, std::shared_ptr<anari::api::Renderer> pAnRenderer,
                       std::shared_ptr<anari::api::World> pWorld);

    void prepareExecution(const ICommandBuffer& rCommandBuffer, const VkExtent2D& rVkViewportSize,
                          std::shared_ptr<IImage> pOutputImage) override;

    void resize(const VkExtent2D& rWindowSize, uint32_t frameInFlightCount) override;

    auto getCameraListener() -> std::shared_ptr<IImguiEventListener>;

    virtual auto getDevice() const -> std::shared_ptr<const IDevice> override { return m_pDevice; }

private:
    std::unique_ptr<ILogger> m_pLogger;
    std::shared_ptr<IDevice> m_pDevice;
    std::shared_ptr<anari::api::Device> m_pAnDevice;
    std::shared_ptr<anari::api::Renderer> m_pAnRenderer;
    std::shared_ptr<anari::api::World> m_pAnWorld;

    std::shared_ptr<AnariMapCamera> m_pCamera;
    UniquePtrWithDeleter<anari::api::Frame> m_pAnFrame;
    std::unique_ptr<IHostVisibleImage> m_pImage;

    VkExtent2D m_currentViewportSize{};
    bool m_renderingFrame = false;
};

}  // namespace im3e