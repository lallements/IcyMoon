#pragma once

#include "imgui_context.h"
#include "imgui_vulkan_backend.h"

#include <im3e/api/device.h>
#include <im3e/api/frame_pipeline.h>
#include <im3e/api/gui.h>
#include <im3e/api/image.h>

#include <GLFW/glfw3.h>

#include <memory>

namespace im3e {

class ImguiPipeline : public IFramePipeline
{
public:
    ImguiPipeline(std::shared_ptr<const IDevice> pDevice, GLFWwindow* pGlfwWindow,
                  std::shared_ptr<IGuiWorkspace> pWorkspace);
    ~ImguiPipeline() override;

    void prepareExecution(ICommandBuffer& rCommandBuffer, std::shared_ptr<IImage> pOutputImage) override;

    void resize(const VkExtent2D& rVkExtent, uint32_t frameInFlightCount) override;

private:
    std::shared_ptr<const IDevice> m_pDevice;
    std::unique_ptr<ILogger> m_pLogger;
    GLFWwindow* m_pGlfwWindow;
    std::shared_ptr<IGuiWorkspace> m_pWorkspace;

    std::unique_ptr<ImguiContext> m_pContext;
    std::shared_ptr<IImage> m_pFrame;
    std::unique_ptr<ImguiVulkanBackend> m_pBackend;
};

}  // namespace im3e