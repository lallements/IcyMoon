#include "vulkan_command_buffers.h"

#include <memory>

using namespace im3e;
using namespace std;

namespace {

class VulkanCommandBarrierRecorder : public ICommandBarrierRecorder
{
public:
    VulkanCommandBarrierRecorder(string_view name, const VulkanDeviceFcts& rFcts, const ICommandBuffer& rCommandBuffer)
      : m_name(name)
      , m_rFcts(rFcts)
      , m_rCommandBuffer(rCommandBuffer)
    {
    }

    ~VulkanCommandBarrierRecorder() override
    {
        if (m_vkImageBarriers.empty())
        {
            return;
        }

        VkDependencyInfo vkInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .imageMemoryBarrierCount = static_cast<uint32_t>(m_vkImageBarriers.size()),
            .pImageMemoryBarriers = m_vkImageBarriers.data(),
        };
        m_rFcts.vkCmdPipelineBarrier2(m_rCommandBuffer.getVkCommandBuffer(), &vkInfo);
    }

    void addImageBarrier(IImage& rImage, ImageBarrierConfig config) override
    {
        auto pMetadata = rImage.getMetadata();

        m_vkImageBarriers.emplace_back(VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = pMetadata->getLastStageMask(),
            .srcAccessMask = pMetadata->getLastAccessMask(),
            .dstStageMask = config.vkDstStageMask,
            .dstAccessMask = config.vkDstAccessMask,
            .oldLayout = pMetadata->getLayout(),
            .newLayout = config.vkLayout.has_value() ? config.vkLayout.value() : pMetadata->getLayout(),
            .srcQueueFamilyIndex = pMetadata->getQueueFamilyIndex(),
            .dstQueueFamilyIndex = pMetadata->getQueueFamilyIndex(),
            .image = rImage.getVkImage(),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1U,
                .layerCount = 1U,
            },
        });

        if (config.vkLayout.has_value())
        {
            pMetadata->setLayout(config.vkLayout.value());
        }
        pMetadata->setLastStageMask(config.vkDstStageMask);
        pMetadata->setLastAccessMask(config.vkDstAccessMask);
    }

private:
    const string m_name;
    const VulkanDeviceFcts& m_rFcts;
    const ICommandBuffer& m_rCommandBuffer;
    vector<VkImageMemoryBarrier2> m_vkImageBarriers;
};

auto createVkCommandBuffer(VkDevice vkDevice, const VulkanDeviceFcts& rFcts, VkCommandPool vkCommandPool)
{
    VkCommandBufferAllocateInfo vkAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vkCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U,
    };

    VkCommandBuffer vkCommandBuffer{};
    throwIfVkFailed(rFcts.vkAllocateCommandBuffers(vkDevice, &vkAllocateInfo, &vkCommandBuffer),
                    "Failed to allocate command for command queue pool");

    return VkUniquePtr<VkCommandBuffer>(vkCommandBuffer, [vkDevice, vkCommandPool, pFcts = &rFcts](auto* vkCmdBuffer) {
        pFcts->vkFreeCommandBuffers(vkDevice, vkCommandPool, 1U, &vkCmdBuffer);
    });
}

class VulkanCommandBuffer : public ICommandBuffer
{
public:
    VulkanCommandBuffer(const ICommandQueue& rQueue, const IDevice& rDevice, VkCommandPool vkCommandPool,
                        string_view name)
      : m_rQueue(rQueue)
      , m_rDevice(rDevice)
      , m_pLogger(m_rDevice.createLogger(name))
      , m_name(name)
      , m_pVkCommandBuffer(createVkCommandBuffer(m_rDevice.getVkDevice(), m_rDevice.getFcts(), vkCommandPool))
      , m_pVkFence(m_rDevice.createVkFence())
    {
    }

    auto startScopedBarrier(string_view name) const -> unique_ptr<ICommandBarrierRecorder> override
    {
        return make_unique<VulkanCommandBarrierRecorder>(name, m_rDevice.getFcts(), *this);
    }

    void setVkSignalSemaphore(VkSemaphore vkSemaphore) override { m_vkSignalSemaphore = vkSemaphore; }
    void setVkWaitSemaphore(VkSemaphore vkSemaphore) override { m_vkWaitSemaphore = vkSemaphore; }

    void reset()
    {
        throwIfVkFailed(m_rDevice.getFcts().vkResetCommandBuffer(m_pVkCommandBuffer.get(), 0U),
                        "Failed to reset command buffer");

        const auto vkFence = m_pVkFence.get();
        throwIfVkFailed(m_rDevice.getFcts().vkResetFences(m_rDevice.getVkDevice(), 1U, &vkFence),
                        "Failed to reset command buffer fence");

        m_vkWaitSemaphore = {};
    }
    void beginRecording(string_view)
    {
        VkCommandBufferBeginInfo vkBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        throwIfVkFailed(m_rDevice.getFcts().vkBeginCommandBuffer(m_pVkCommandBuffer.get(), &vkBeginInfo),
                        "Could not begin command buffer recording");
    }
    void endRecording()
    {
        throwIfVkFailed(m_rDevice.getFcts().vkEndCommandBuffer(m_pVkCommandBuffer.get()),
                        "Failed to end command buffer recording");
    }

    void submitToQueue(CommandExecutionType executionType)
    {
        const auto vkCommandBuffer = m_pVkCommandBuffer.get();
        VkSubmitInfo vkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1U,
            .pCommandBuffers = &vkCommandBuffer,
            .signalSemaphoreCount = m_vkSignalSemaphore ? 1U : 0U,
            .pSignalSemaphores = m_vkSignalSemaphore ? &m_vkSignalSemaphore : nullptr,
        };
        const VkPipelineStageFlags vkWaitDstMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        if (m_vkWaitSemaphore)
        {
            vkSubmitInfo.waitSemaphoreCount = 1U;
            vkSubmitInfo.pWaitSemaphores = &m_vkWaitSemaphore;
            vkSubmitInfo.pWaitDstStageMask = &vkWaitDstMask;
        }
        throwIfVkFailed(m_rDevice.getFcts().vkQueueSubmit(m_rQueue.getVkQueue(), 1U, &vkSubmitInfo, m_pVkFence.get()),
                        "Failed to execute command buffer");

        if (executionType == CommandExecutionType::Sync)
        {
            this->waitForCompletion();
            this->reset();
        }
    }

    void waitForCompletion() const
    {
        const auto vkFence = m_pVkFence.get();
        logIfVkFailed(m_rDevice.getFcts().vkWaitForFences(m_rDevice.getVkDevice(), 1U, &vkFence, VK_TRUE,
                                                          numeric_limits<uint64_t>::max()),
                      *m_pLogger, "Failed to wait for fence while destroying command buffer");
    }

    auto isExecutionComplete() const
    {
        const auto vkFence = m_pVkFence.get();
        const auto vkResult = m_rDevice.getFcts().vkWaitForFences(m_rDevice.getVkDevice(), 1U, &vkFence, VK_TRUE, 0U);
        if (vkResult == VK_SUCCESS)
        {
            return true;
        }
        else if (vkResult == VK_TIMEOUT)
        {
            return false;
        }
        throwIfVkFailed(vkResult, "Failed to query state of command buffer fence");
        return false;
    }

    auto getVkCommandBuffer() const -> VkCommandBuffer override { return m_pVkCommandBuffer.get(); }
    auto getVkFence() const -> VkSharedPtr<VkFence> override { return m_pVkFence; }

private:
    const ICommandQueue& m_rQueue;
    const IDevice& m_rDevice;
    unique_ptr<ILogger> m_pLogger;
    const string m_name;

    VkUniquePtr<VkCommandBuffer> m_pVkCommandBuffer;
    VkUniquePtr<VkSemaphore> m_pVkSignalSemaphore;
    VkSharedPtr<VkFence> m_pVkFence;

    VkSemaphore m_vkSignalSemaphore{};
    VkSemaphore m_vkWaitSemaphore{};
};

auto createVkCommandPool(VkDevice vkDevice, const VulkanDeviceFcts& rFcts, uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo vkCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex,
    };

    VkCommandPool vkCommandPool{};
    throwIfVkFailed(rFcts.vkCreateCommandPool(vkDevice, &vkCreateInfo, nullptr, &vkCommandPool),
                    "Failed to create pool for command queue");

    return makeVkUniquePtr<VkCommandPool>(vkDevice, vkCommandPool, rFcts.vkDestroyCommandPool);
}

class VulkanCommandQueue : public ICommandQueue, public enable_shared_from_this<VulkanCommandQueue>
{
public:
    VulkanCommandQueue(const IDevice& rDevice, VulkanCommandQueueInfo queueInfo, string_view name)
      : m_rDevice(rDevice)
      , m_queueInfo(move(queueInfo))
      , m_name(name)
      , m_pVkCommandPool(createVkCommandPool(m_rDevice.getVkDevice(), m_rDevice.getFcts(), queueInfo.queueFamilyIndex))
    {
    }

    ~VulkanCommandQueue()
    {
        for (auto* pInFlight : m_pInFlight)
        {
            pInFlight->waitForCompletion();
        }
    }

    auto startScopedCommand(string_view name, CommandExecutionType executionType)
        -> UniquePtrWithDeleter<ICommandBuffer> override
    {
        // Release any in-flight buffer whose execution is complete:
        auto itInFlight = m_pInFlight.begin();
        while (itInFlight != m_pInFlight.end())
        {
            if (!(*itInFlight)->isExecutionComplete())
            {
                itInFlight++;
                continue;
            }
            (*itInFlight)->reset();
            m_pAvailable.emplace_back((*itInFlight));
            itInFlight = m_pInFlight.erase(itInFlight);
        }

        // If no buffer is available, create a new one:
        VulkanCommandBuffer* pCommandBuffer{};
        if (m_pAvailable.empty())
        {
            m_pVkCommandBuffers.emplace_back(make_unique<VulkanCommandBuffer>(
                *this, m_rDevice, m_pVkCommandPool.get(), fmt::format("{}_{}", m_name, m_pVkCommandBuffers.size())));
            pCommandBuffer = m_pVkCommandBuffers.back().get();
        }
        else
        {
            pCommandBuffer = m_pAvailable.back();
            m_pAvailable.pop_back();
        }

        pCommandBuffer->beginRecording(name);
        return UniquePtrWithDeleter<ICommandBuffer>(
            pCommandBuffer, [pThis = this->shared_from_this(), executionType, pVulkanCommand = pCommandBuffer](auto*) {
                pVulkanCommand->endRecording();
                pVulkanCommand->submitToQueue(executionType);
                if (executionType == CommandExecutionType::Sync)
                {
                    pThis->m_pAvailable.emplace_back(pVulkanCommand);
                }
                else
                {
                    pThis->m_pInFlight.emplace_back(pVulkanCommand);
                }
            });
    }

    auto getQueueFamilyIndex() const -> uint32_t override { return m_queueInfo.queueFamilyIndex; }
    auto getVkQueue() const -> VkQueue override { return m_queueInfo.vkQueue; }

private:
    const IDevice& m_rDevice;
    const VulkanCommandQueueInfo m_queueInfo;
    const string m_name;

    VkUniquePtr<VkCommandPool> m_pVkCommandPool;
    vector<unique_ptr<VulkanCommandBuffer>> m_pVkCommandBuffers;
    vector<VulkanCommandBuffer*> m_pInFlight;
    vector<VulkanCommandBuffer*> m_pAvailable;
};

}  // namespace

auto im3e::createVulkanCommandQueue(const IDevice& rDevice, VulkanCommandQueueInfo queueInfo, string_view name)
    -> shared_ptr<ICommandQueue>
{
    return make_shared<VulkanCommandQueue>(rDevice, move(queueInfo), name);
}
