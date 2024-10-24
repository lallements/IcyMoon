#include "src/vulkan_command_buffers.h"

#include <im3e/mock/mock_device.h>
#include <im3e/mock/mock_image.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

struct VulkanCommandBuffersTest : public Test
{
    auto createCommandQueue()
    {
        EXPECT_CALL(m_rMockFcts, vkCreateCommandPool(m_mockVkDevice, NotNull(), IsNull(), NotNull()))
            .WillOnce(Invoke([this](Unused, auto* pVkCreateInfo, Unused, auto* pVkPool) {
                EXPECT_THAT(pVkCreateInfo->sType, Eq(VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO));
                EXPECT_THAT(pVkCreateInfo->flags, Eq(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
                EXPECT_THAT(pVkCreateInfo->queueFamilyIndex, Eq(m_queueFamilyIndex));
                *pVkPool = m_mockVkCommandPool;
                return VK_SUCCESS;
            }));

        return createVulkanCommandQueue(m_pMockDevice,
                                        VulkanCommandQueueInfo{
                                            .vkQueue = m_mockVkQueue,
                                            .queueFamilyIndex = m_queueFamilyIndex,
                                        },
                                        "testQueue");
    }

    void expectCommandBufferAllocated(VkCommandBuffer vkCommandBuffer)
    {
        EXPECT_CALL(m_rMockFcts, vkAllocateCommandBuffers(m_mockVkDevice, NotNull(), NotNull()))
            .WillOnce(Invoke([this, vkCommandBuffer](Unused, auto* pVkAllocInfo, auto* pVkCommandBuffer) {
                EXPECT_THAT(pVkAllocInfo->sType, Eq(VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO));
                EXPECT_THAT(pVkAllocInfo->commandPool, Eq(m_mockVkCommandPool));
                EXPECT_THAT(pVkAllocInfo->level, Eq(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
                EXPECT_THAT(pVkAllocInfo->commandBufferCount, Eq(1U));
                *pVkCommandBuffer = vkCommandBuffer;
                return VK_SUCCESS;
            }));
    }

    void expectBeginCommand(VkCommandBuffer vkCommandBuffer)
    {
        EXPECT_CALL(m_rMockFcts, vkBeginCommandBuffer(vkCommandBuffer, NotNull()))
            .WillOnce(Invoke([](Unused, auto* pVkBeginInfo) {
                EXPECT_THAT(pVkBeginInfo->sType, Eq(VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO));
                return VK_SUCCESS;
            }));
    }

    void expectQueueSubmit(VkCommandBuffer vkCommandBuffer, VkFence vkFence)
    {
        EXPECT_CALL(m_rMockFcts, vkQueueSubmit(m_mockVkQueue, 1U, NotNull(), vkFence))
            .WillOnce(Invoke([vkCommandBuffer](Unused, Unused, auto* pVkSubmitInfo, Unused) {
                EXPECT_THAT(pVkSubmitInfo->sType, Eq(VK_STRUCTURE_TYPE_SUBMIT_INFO));
                EXPECT_THAT(pVkSubmitInfo->commandBufferCount, Eq(1U));
                EXPECT_THAT(*pVkSubmitInfo->pCommandBuffers, Eq(vkCommandBuffer));
                return VK_SUCCESS;
            }));
    }

    void expectEndCommand(VkCommandBuffer vkCommandBuffer)
    {
        EXPECT_CALL(m_rMockFcts, vkEndCommandBuffer(vkCommandBuffer));
    }

    void expectFenceCreated(VkFence vkFence)
    {
        EXPECT_CALL(m_rMockFcts, vkCreateFence(m_mockVkDevice, NotNull(), IsNull(), NotNull()))
            .WillOnce(Invoke([this, vkFence](Unused, auto* pVkCreateInfo, Unused, auto* pVkFence) {
                EXPECT_THAT(pVkCreateInfo->sType, Eq(VK_STRUCTURE_TYPE_FENCE_CREATE_INFO));
                *pVkFence = vkFence;
                return VK_SUCCESS;
            }));
    }

    void expectCommandAndFenceReset(VkCommandBuffer vkCommandBuffer, VkFence vkFence)
    {
        EXPECT_CALL(m_rMockFcts, vkResetCommandBuffer(vkCommandBuffer, 0U));
        EXPECT_CALL(m_rMockFcts, vkResetFences(m_mockVkDevice, 1U, Pointee(vkFence)));
    }

    enum class WaitForFenceType
    {
        InfiniteWait,
        NoWait
    };
    void expectWaitForFence(VkFence vkFence, WaitForFenceType waitType, VkResult vkResult = VK_SUCCESS)
    {
        const auto waitTime = (waitType == WaitForFenceType::InfiniteWait) ? numeric_limits<uint64_t>::max() : 0U;
        EXPECT_CALL(m_rMockFcts, vkWaitForFences(m_mockVkDevice, 1U, Pointee(vkFence), VK_TRUE, waitTime))
            .WillOnce(Return(vkResult));
    }

    shared_ptr<MockDevice> m_pMockDevice = make_shared<NiceMock<MockDevice>>();
    MockVulkanDeviceFcts& m_rMockFcts = m_pMockDevice->getMockDeviceFcts();
    const VkDevice m_mockVkDevice = m_pMockDevice->getMockVkDevice();
    const VkQueue m_mockVkQueue = reinterpret_cast<VkQueue>(0xaf31e5f);
    const uint32_t m_queueFamilyIndex = 42U;
    const VkCommandPool m_mockVkCommandPool = reinterpret_cast<VkCommandPool>(0x493ead23);
};

TEST_F(VulkanCommandBuffersTest, createCommandQueueThrowsIfDeviceNull)
{
    EXPECT_THROW(auto pCommandQueue = createVulkanCommandQueue(nullptr, VulkanCommandQueueInfo{}, "test"),
                 invalid_argument);
}

TEST_F(VulkanCommandBuffersTest, createCommandQueue)
{
    auto pCommandQueue = createCommandQueue();
    ASSERT_THAT(pCommandQueue, NotNull());
    EXPECT_THAT(pCommandQueue->getVkQueue(), Eq(m_mockVkQueue));

    EXPECT_CALL(m_rMockFcts, vkDestroyCommandPool(m_mockVkDevice, m_mockVkCommandPool, IsNull()));
    pCommandQueue.reset();
}

TEST_F(VulkanCommandBuffersTest, startScopedCommandSync)
{
    const auto mockVkCommandBuffer = reinterpret_cast<VkCommandBuffer>(0xaf3e56);
    const auto mockVkFence = reinterpret_cast<VkFence>(0x418e4a);
    auto pCommandQueue = createCommandQueue();

    expectCommandBufferAllocated(mockVkCommandBuffer);
    expectFenceCreated(mockVkFence);
    expectBeginCommand(mockVkCommandBuffer);
    auto pCommandBuffer = pCommandQueue->startScopedCommand("testCommand", CommandExecutionType::Sync);
    EXPECT_THAT(pCommandBuffer->getVkCommandBuffer(), Eq(mockVkCommandBuffer));

    {
        InSequence s;
        expectEndCommand(mockVkCommandBuffer);
        expectQueueSubmit(mockVkCommandBuffer, mockVkFence);
        expectWaitForFence(mockVkFence, WaitForFenceType::InfiniteWait);
    }
    pCommandBuffer.reset();

    EXPECT_CALL(m_rMockFcts,
                vkFreeCommandBuffers(m_mockVkDevice, m_mockVkCommandPool, 1U, Pointee(mockVkCommandBuffer)));
    EXPECT_CALL(m_rMockFcts, vkDestroyFence(m_mockVkDevice, mockVkFence, IsNull()));
    pCommandQueue.reset();
}

TEST_F(VulkanCommandBuffersTest, startScopedCommandSyncReusesBuffers)
{
    const auto mockVkCommandBuffer = reinterpret_cast<VkCommandBuffer>(0xa3bec);
    const auto mockVkFence = reinterpret_cast<VkFence>(0x53e6);
    auto pCommandQueue = createCommandQueue();

    expectCommandBufferAllocated(mockVkCommandBuffer);
    expectFenceCreated(mockVkFence);
    {
        auto pCommandBuffer = pCommandQueue->startScopedCommand("command", CommandExecutionType::Sync);
        EXPECT_THAT(pCommandBuffer->getVkCommandBuffer(), Eq(mockVkCommandBuffer));

        EXPECT_CALL(m_rMockFcts, vkQueueSubmit(_, _, _, _))
            .WillOnce(Invoke([&](Unused, Unused, auto* pVkSubmitInfo, Unused) {
                EXPECT_THAT(*pVkSubmitInfo->pCommandBuffers, Eq(mockVkCommandBuffer));
                return VK_SUCCESS;
            }));
    }

    EXPECT_CALL(m_rMockFcts, vkAllocateCommandBuffers(_, _, _)).Times(0);
    expectCommandAndFenceReset(mockVkCommandBuffer, mockVkFence);
    {
        auto pCommandBuffer = pCommandQueue->startScopedCommand("command 2", CommandExecutionType::Sync);
        EXPECT_THAT(pCommandBuffer->getVkCommandBuffer(), Eq(mockVkCommandBuffer));

        EXPECT_CALL(m_rMockFcts, vkQueueSubmit(_, _, _, _))
            .WillOnce(Invoke([&](Unused, Unused, auto* pVkSubmitInfo, Unused) {
                EXPECT_THAT(*pVkSubmitInfo->pCommandBuffers, Eq(mockVkCommandBuffer));
                return VK_SUCCESS;
            }));
    }
}

TEST_F(VulkanCommandBuffersTest, startScopedCommandAsync)
{
    const auto mockVkCommandBuffer = reinterpret_cast<VkCommandBuffer>(0xfe43a);
    const auto mockVkFence = reinterpret_cast<VkFence>(0xa32e1f);
    auto pCommandQueue = createCommandQueue();

    expectCommandBufferAllocated(mockVkCommandBuffer);
    expectFenceCreated(mockVkFence);
    expectBeginCommand(mockVkCommandBuffer);
    auto pCommandBuffer = pCommandQueue->startScopedCommand("asyncCommand", CommandExecutionType::Async);

    expectEndCommand(mockVkCommandBuffer);
    expectQueueSubmit(mockVkCommandBuffer, mockVkFence);
    EXPECT_CALL(m_rMockFcts, vkWaitForFences(_, _, _, _, _)).Times(0);
    pCommandBuffer.reset();

    // Our command is still executing, expect a wait for the completion before deletion of the queue:
    expectWaitForFence(mockVkFence, WaitForFenceType::InfiniteWait);
    pCommandQueue.reset();
}

TEST_F(VulkanCommandBuffersTest, startScopedCommandRecyclesAsyncCommandIfComplete)
{
    const auto mockVkCommandBuffer = reinterpret_cast<VkCommandBuffer>(0x2a5e);
    const auto mockVkFence = reinterpret_cast<VkFence>(0x8f3a);
    auto pCommandQueue = createCommandQueue();

    expectCommandBufferAllocated(mockVkCommandBuffer);
    expectFenceCreated(mockVkFence);
    auto pCommandBuffer = pCommandQueue->startScopedCommand("command1", CommandExecutionType::Async);
    pCommandBuffer.reset();

    // The previous command is still running, the queue should then check if it is now complete and reuse if so:
    expectWaitForFence(mockVkFence, WaitForFenceType::NoWait, VK_SUCCESS);
    expectCommandAndFenceReset(mockVkCommandBuffer, mockVkFence);
    EXPECT_CALL(m_rMockFcts, vkAllocateCommandBuffers(_, _, _)).Times(0);
    pCommandBuffer = pCommandQueue->startScopedCommand("command2", CommandExecutionType::Sync);
    EXPECT_THAT(pCommandBuffer->getVkCommandBuffer(), Eq(mockVkCommandBuffer));

    expectWaitForFence(mockVkFence, WaitForFenceType::InfiniteWait);
    expectCommandAndFenceReset(mockVkCommandBuffer, mockVkFence);
    pCommandBuffer.reset();
}

TEST_F(VulkanCommandBuffersTest, startScopedCommandCreatesNewCommandIfAllNotComplete)
{
    auto pCommandQueue = createCommandQueue();

    const auto mockVkCommandBuffer1 = reinterpret_cast<VkCommandBuffer>(0xdc34f);
    const auto mockVkFence1 = reinterpret_cast<VkFence>(0x9f3c);
    expectCommandBufferAllocated(mockVkCommandBuffer1);
    expectFenceCreated(mockVkFence1);
    auto pCommandBuffer = pCommandQueue->startScopedCommand("cmd1", CommandExecutionType::Async);
    pCommandBuffer.reset();

    // When the queue checks our first command's state, we return timeout to let it know it is still in progress:
    expectWaitForFence(mockVkFence1, WaitForFenceType::NoWait, VK_TIMEOUT);

    // As a result, the command should be left alone:
    EXPECT_CALL(m_rMockFcts, vkResetCommandBuffer(_, _)).Times(0);
    EXPECT_CALL(m_rMockFcts, vkResetFences(_, _, _)).Times(0);

    // And a new command should be allocated instead:
    const auto mockVkCommandBuffer2 = reinterpret_cast<VkCommandBuffer>(0xc46e2a8);
    const auto mockVkFence2 = reinterpret_cast<VkFence>(0xac34bd);
    expectCommandBufferAllocated(mockVkCommandBuffer2);
    expectFenceCreated(mockVkFence2);
    pCommandBuffer = pCommandQueue->startScopedCommand("cmd2", CommandExecutionType::Async);
    EXPECT_THAT(pCommandBuffer->getVkCommandBuffer(), Eq(mockVkCommandBuffer2));

    // The two commands are then waited for completion when the queue is deleted:
    expectWaitForFence(mockVkFence1, WaitForFenceType::InfiniteWait);
    expectWaitForFence(mockVkFence2, WaitForFenceType::InfiniteWait);
}

TEST_F(VulkanCommandBuffersTest, startScopedBarrierWithNoBarrier)
{
    auto pCommandQueue = createCommandQueue();
    auto pCommandBuffer = pCommandQueue->startScopedCommand("test", CommandExecutionType::Sync);
    auto pBarrierRecorder = pCommandBuffer->startScopedBarrier("barrier");

    // We did not add any barrier so there should be no barrier call:
    EXPECT_CALL(m_rMockFcts, vkCmdPipelineBarrier2(_, _)).Times(0);
    pBarrierRecorder.reset();
}

TEST_F(VulkanCommandBuffersTest, startScopedBarrierWithImageBarrier)
{
    MockImage mockImage;
    const auto mockVkImage = reinterpret_cast<VkImage>(0xb43ea);
    EXPECT_CALL(mockImage, getVkImage()).WillRepeatedly(Return(mockVkImage));

    auto pCommandQueue = createCommandQueue();
    const auto mockVkCommandBuffer = reinterpret_cast<VkCommandBuffer>(0x92e6fa);
    expectCommandBufferAllocated(mockVkCommandBuffer);
    auto pCommandBuffer = pCommandQueue->startScopedCommand("test", CommandExecutionType::Sync);
    auto pBarrierRecorder = pCommandBuffer->startScopedBarrier("barrier");

    const auto vkLastLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    const auto vkNextLayout = VK_IMAGE_LAYOUT_GENERAL;
    const auto vkLastStageMask = VK_PIPELINE_STAGE_2_NONE;
    const auto vkNextStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
    const auto vkLastAccessMask = VK_ACCESS_2_NONE;
    const auto vkNextAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
    const auto queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    EXPECT_CALL(mockImage, getMetadata());
    auto& rMockMetadata = mockImage.getMockMetadata();

    EXPECT_CALL(rMockMetadata, getLayout()).WillOnce(Return(vkLastLayout));
    EXPECT_CALL(rMockMetadata, getLastStageMask()).WillOnce(Return(vkLastStageMask));
    EXPECT_CALL(rMockMetadata, getLastAccessMask()).WillOnce(Return(vkLastAccessMask));
    EXPECT_CALL(rMockMetadata, getQueueFamilyIndex()).WillRepeatedly(Return(queueFamilyIndex));

    EXPECT_CALL(rMockMetadata, setLayout(vkNextLayout));
    EXPECT_CALL(rMockMetadata, setLastStageMask(vkNextStageMask));
    EXPECT_CALL(rMockMetadata, setLastAccessMask(vkNextAccessMask));

    pBarrierRecorder->addImageBarrier(mockImage, ImageBarrierConfig{.vkDstStageMask = vkNextStageMask,
                                                                    .vkDstAccessMask = vkNextAccessMask,
                                                                    .vkLayout = vkNextLayout});

    EXPECT_CALL(m_rMockFcts, vkCmdPipelineBarrier2(mockVkCommandBuffer, NotNull()))
        .WillOnce(Invoke([&](Unused, auto* pVkInfo) {
            EXPECT_THAT(pVkInfo->sType, Eq(VK_STRUCTURE_TYPE_DEPENDENCY_INFO));
            EXPECT_THAT(pVkInfo->dependencyFlags, Eq(VK_DEPENDENCY_BY_REGION_BIT));
            EXPECT_THAT(pVkInfo->imageMemoryBarrierCount, Eq(1U));
            ASSERT_THAT(pVkInfo->pImageMemoryBarriers, NotNull());

            auto* pImageBarrier = pVkInfo->pImageMemoryBarriers;
            EXPECT_THAT(pImageBarrier->sType, Eq(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2));
            EXPECT_THAT(pImageBarrier->srcStageMask, Eq(vkLastStageMask));
            EXPECT_THAT(pImageBarrier->srcAccessMask, Eq(vkLastAccessMask));
            EXPECT_THAT(pImageBarrier->dstStageMask, Eq(vkNextStageMask));
            EXPECT_THAT(pImageBarrier->dstAccessMask, Eq(vkNextAccessMask));
            EXPECT_THAT(pImageBarrier->oldLayout, Eq(vkLastLayout));
            EXPECT_THAT(pImageBarrier->newLayout, Eq(vkNextLayout));
            EXPECT_THAT(pImageBarrier->image, Eq(mockVkImage));
            EXPECT_THAT(pImageBarrier->subresourceRange.aspectMask, Eq(VK_IMAGE_ASPECT_COLOR_BIT));
            EXPECT_THAT(pImageBarrier->subresourceRange.baseMipLevel, Eq(0U));
            EXPECT_THAT(pImageBarrier->subresourceRange.levelCount, Eq(1U));
            EXPECT_THAT(pImageBarrier->subresourceRange.baseArrayLayer, Eq(0U));
            EXPECT_THAT(pImageBarrier->subresourceRange.layerCount, Eq(1U));
        }));
    pBarrierRecorder.reset();
}