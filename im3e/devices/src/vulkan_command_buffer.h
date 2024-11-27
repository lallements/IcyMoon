#pragma once

#include <im3e/api/command_buffer.h>
#include <im3e/api/device.h>
#include <im3e/api/vulkan_loader.h>

#include <memory>
#include <string>
#include <vector>

namespace im3e {

class VulkanCommandBufferFuture;

class VulkanCommandBuffer : public ICommandBuffer, public std::enable_shared_from_this<VulkanCommandBuffer>
{
public:
    VulkanCommandBuffer(const ICommandQueue& rQueue, const IDevice& rDevice, VkCommandPool vkCommandPool,
                        std::string_view name);
    ~VulkanCommandBuffer() override;

    auto startScopedBarrier(std::string_view name) const -> std::unique_ptr<ICommandBarrierRecorder> override;
    auto createFuture() -> std::shared_ptr<ICommandBufferFuture> override;

    void setVkSignalSemaphore(VkSemaphore vkSemaphore) override { m_vkSignalSemaphore = vkSemaphore; }
    void setVkWaitSemaphore(VkSemaphore vkSemaphore) override { m_vkWaitSemaphore = vkSemaphore; }

    void reset();
    void beginRecording(std::string_view);
    void endRecording();

    void submitToQueue(CommandExecutionType executionType);
    void waitForCompletion() const;
    auto isExecutionComplete() const -> bool;

    auto getVkCommandBuffer() const -> VkCommandBuffer override { return m_pVkCommandBuffer.get(); }

private:
    const ICommandQueue& m_rQueue;
    const IDevice& m_rDevice;
    std::unique_ptr<ILogger> m_pLogger;
    const std::string m_name;

    VkUniquePtr<VkCommandBuffer> m_pVkCommandBuffer;
    VkUniquePtr<VkSemaphore> m_pVkSignalSemaphore;
    VkSharedPtr<VkFence> m_pVkFence;

    VkSemaphore m_vkSignalSemaphore{};
    VkSemaphore m_vkWaitSemaphore{};
    std::vector<std::shared_ptr<VulkanCommandBufferFuture>> m_pFutures;
};

}  // namespace im3e