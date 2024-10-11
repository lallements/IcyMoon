#include "vulkan_command_buffers.h"

using namespace im3e;
using namespace std;

namespace {

class VulkanCommandBuffer : public ICommandBuffer
{
public:
    VulkanCommandBuffer(std::string_view name, std::shared_ptr<const ICommandQueue> pQueue,
                        std::shared_ptr<const IDevice> pDevice, VkCommandBuffer vkCommandBuffer)
      : m_name(name)
      , m_pQueue(throwIfArgNull(move(pQueue), "Cannot create Vulkan command buffer without a queue"))
      , m_pDevice(throwIfArgNull(move(pDevice), "Cannot create Vulkan command buffer without a device"))
      , m_pLogger(m_pDevice->createLogger(m_name))
      , m_vkCommandBuffer(
            throwIfArgNull(vkCommandBuffer, "Cannot create Vulkan command buffer without a VkCommandBuffer"))
    {
    }

    auto getVkCommandBuffer() const -> VkCommandBuffer override { return m_vkCommandBuffer; }

private:
    const string m_name;
    shared_ptr<const ICommandQueue> m_pQueue;
    shared_ptr<const IDevice> m_pDevice;
    unique_ptr<ILogger> m_pLogger;

    VkCommandBuffer m_vkCommandBuffer;
};

class VulkanCommandQueue : public ICommandQueue, std::enable_shared_from_this<VulkanCommandQueue>
{
public:
    VulkanCommandQueue(shared_ptr<const IDevice> pDevice, VulkanCommandQueueInfo queueInfo)
      : m_pDevice(throwIfArgNull(move(pDevice), "Vulkan command queue requires a device"))
      , m_queueInfo(move(queueInfo))
    {
    }

    auto startCommandRecording(CommandExecutionType) -> unique_ptr<ICommandBuffer> { return nullptr; }

private:
    shared_ptr<const IDevice> m_pDevice;
    const VulkanCommandQueueInfo m_queueInfo;

    VkUniquePtr<VkCommandPool> m_pVkPool;
    vector<VkUniquePtr<VkCommandBuffer>> m_pVkCommandBuffers;
};

}  // namespace

auto im3e::createVulkanCommandQueue(shared_ptr<const IDevice> pDevice, VulkanCommandQueueInfo queueInfo)
    -> shared_ptr<ICommandQueue>
{
    return make_shared<VulkanCommandQueue>(move(pDevice), move(queueInfo));
}
