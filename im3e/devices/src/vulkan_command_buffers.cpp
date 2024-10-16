#include "vulkan_command_buffers.h"

#include <memory>

using namespace im3e;
using namespace std;

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

namespace {

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

auto createVkFence(VkDevice vkDevice, const VulkanDeviceFcts& rFcts)
{
    VkFenceCreateInfo vkCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    VkFence vkFence{};
    throwIfVkFailed(rFcts.vkCreateFence(vkDevice, &vkCreateInfo, nullptr, &vkFence),
                    "Could not create fence for Vulkan command buffer");

    return makeVkUniquePtr<VkFence>(vkDevice, vkFence, rFcts.vkDestroyFence);
}

class VulkanCommandBuffer : public ICommandBuffer
{
public:
    VulkanCommandBuffer(std::shared_ptr<const ICommandQueue> pQueue, std::shared_ptr<const IDevice> pDevice,
                        VkCommandPool vkCommandPool, std::string_view name)
      : m_pQueue(throwIfArgNull(move(pQueue), "Cannot create Vulkan command buffer without a queue"))
      , m_pDevice(throwIfArgNull(move(pDevice), "Cannot create Vulkan command buffer without a device"))
      , m_pLogger(m_pDevice->createLogger(m_name))
      , m_name(name)
      , m_pVkCommandBuffer(createVkCommandBuffer(m_pDevice->getVkDevice(), m_pDevice->getFcts(), vkCommandPool))
      , m_pVkFence(createVkFence(m_pDevice->getVkDevice(), m_pDevice->getFcts()))
    {
    }

    auto startScopedBarrier(string_view name) const -> unique_ptr<ICommandBarrierRecorder> override
    {
        return make_unique<VulkanCommandBarrierRecorder>(name, m_pDevice->getFcts(), *this);
    }

    void reset()
    {
        throwIfVkFailed(m_pDevice->getFcts().vkResetCommandBuffer(m_pVkCommandBuffer.get(), 0U),
                        "Failed to reset command buffer");

        const auto vkFence = m_pVkFence.get();
        throwIfVkFailed(m_pDevice->getFcts().vkResetFences(m_pDevice->getVkDevice(), 1U, &vkFence),
                        "Failed to reset command buffer fence");
    }
    void beginRecording(string_view)
    {
        VkCommandBufferBeginInfo vkBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        throwIfVkFailed(m_pDevice->getFcts().vkBeginCommandBuffer(m_pVkCommandBuffer.get(), &vkBeginInfo),
                        "Could not begin command buffer recording");
    }
    void endRecording()
    {
        throwIfVkFailed(m_pDevice->getFcts().vkEndCommandBuffer(m_pVkCommandBuffer.get()),
                        "Failed to end command buffer recording");
    }

    void submitToQueue(CommandExecutionType executionType)
    {
        const auto vkCommandBuffer = m_pVkCommandBuffer.get();
        VkSubmitInfo vkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1U,
            .pCommandBuffers = &vkCommandBuffer,
        };
        throwIfVkFailed(m_pDevice->getFcts().vkQueueSubmit(m_pQueue->getVkQueue(), 1U, &vkSubmitInfo, m_pVkFence.get()),
                        "Failed to execute command buffer");

        if (executionType == CommandExecutionType::Sync)
        {
            const auto vkFence = m_pVkFence.get();
            throwIfVkFailed(m_pDevice->getFcts().vkWaitForFences(m_pDevice->getVkDevice(), 1U, &vkFence, VK_TRUE,
                                                                 numeric_limits<uint64_t>::max()),
                            "Failed to wait for end of command buffer execution");

            this->reset();
        }
    }

    auto getVkCommandBuffer() const -> VkCommandBuffer override { return m_pVkCommandBuffer.get(); }

    auto isExecutionComplete() const
    {
        const auto vkFence = m_pVkFence.get();
        const auto vkResult = m_pDevice->getFcts().vkWaitForFences(m_pDevice->getVkDevice(), 1U, &vkFence, VK_TRUE, 0U);
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

private:
    shared_ptr<const ICommandQueue> m_pQueue;
    shared_ptr<const IDevice> m_pDevice;
    unique_ptr<ILogger> m_pLogger;
    const string m_name;

    VkUniquePtr<VkCommandBuffer> m_pVkCommandBuffer;
    VkUniquePtr<VkFence> m_pVkFence;
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
    VulkanCommandQueue(shared_ptr<const IDevice> pDevice, VulkanCommandQueueInfo queueInfo, string_view name)
      : m_pDevice(throwIfArgNull(move(pDevice), "Vulkan command queue requires a device"))
      , m_queueInfo(move(queueInfo))
      , m_name(name)
      , m_pVkCommandPool(
            createVkCommandPool(m_pDevice->getVkDevice(), m_pDevice->getFcts(), queueInfo.queueFamilyIndex))
    {
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
            m_pVkCommandBuffers.emplace_back(
                make_unique<VulkanCommandBuffer>(this->shared_from_this(), m_pDevice, m_pVkCommandPool.get(),
                                                 fmt::format("{}_{}", m_name, m_pVkCommandBuffers.size())));
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

    auto getVkQueue() const -> VkQueue override { return m_queueInfo.vkQueue; }

private:
    shared_ptr<const IDevice> m_pDevice;
    const VulkanCommandQueueInfo m_queueInfo;
    const string m_name;

    VkUniquePtr<VkCommandPool> m_pVkCommandPool;
    vector<unique_ptr<VulkanCommandBuffer>> m_pVkCommandBuffers;
    vector<VulkanCommandBuffer*> m_pInFlight;
    vector<VulkanCommandBuffer*> m_pAvailable;
};

}  // namespace

auto im3e::createVulkanCommandQueue(shared_ptr<const IDevice> pDevice, VulkanCommandQueueInfo queueInfo,
                                    string_view name) -> shared_ptr<ICommandQueue>
{
    return make_shared<VulkanCommandQueue>(move(pDevice), move(queueInfo), name);
}
