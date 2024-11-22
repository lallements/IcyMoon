#pragma once

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>

namespace im3e {

class Presenter
{
public:
    Presenter(std::shared_ptr<IDevice> pDevice, VkSurfaceKHR vkSurface, std::unique_ptr<IFramePipeline> pFramePipeline);
    ~Presenter();

    void present();
    void reset();

private:
    std::shared_ptr<IDevice> m_pDevice;
    VkSurfaceKHR m_vkSurface;
    std::shared_ptr<IFramePipeline> m_pFramePipeline;

    std::unique_ptr<ILogger> m_pLogger;

    VkUniquePtr<VkSwapchainKHR> m_pVkSwapchain;
    std::vector<std::shared_ptr<IImage>> m_pImages;
    std::vector<VkSharedPtr<VkFence>> m_pImageVkFences;
    VkExtent2D m_vkExtent{};
    bool m_isOutOfDate = true;

    std::vector<VkUniquePtr<VkSemaphore>> m_pReadyToWriteSemaphores;
    std::vector<VkUniquePtr<VkSemaphore>> m_pReadyToPresentSemaphores;
    size_t m_semaphoreIndex{};
};

}  // namespace im3e