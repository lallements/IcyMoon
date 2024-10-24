#include "pipeline_integration_test.h"

#include <fmt/format.h>

#include <filesystem>

using namespace im3e;
using namespace std;
using namespace std::filesystem;

void PipelineIntegrationTest::TearDown()
{
    if (Test::HasFailure())
    {
        const auto outputFilePath = generateFilePath(getName(), "png");
        {
            auto pImageMapping = m_pHostVisibleOutputImage->mapReadOnly();
            pImageMapping->save(outputFilePath);
        }
        ADD_FAILURE() << fmt::format(R"(Saved pipeline output to : "{}")", outputFilePath.string());
    }

    DeviceIntegrationTest::TearDown();
}

auto PipelineIntegrationTest::mapOutputImage() const -> unique_ptr<const IHostVisibleImage::IMapping>
{
    return m_pHostVisibleOutputImage->mapReadOnly();
}

void PipelineIntegrationTest::initialize(Config config, unique_ptr<IFramePipeline> pFramePipeline)
{
    throwIfArgNull(pFramePipeline.get(), "Cannot initialize pipeline integration test without a pipeline");

    shared_ptr<IFramePipeline> pSharedPipeline(move(pFramePipeline));
    initialize(move(config), [pSharedPipeline](auto& rCommandBuffer, auto pImage) {
        pSharedPipeline->prepareExecution(rCommandBuffer, move(pImage));
    });
}

void PipelineIntegrationTest::initialize(Config config, PipelineFct pipelineFct)
{
    m_config = move(config);
    m_pipelineFct = throwIfArgNull(move(pipelineFct),
                                   "Cannot initialize pipeline integration test without a pipeline function");

    auto pImageFactory = getDevice()->getImageFactory();
    m_pOutputImage = pImageFactory->createImage(ImageConfig{
        .name = "PipelineTestOutputImage",
        .vkExtent = m_config.vkOutputExtent,
        .vkFormat = m_config.vkOutputFormat,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });
    m_pHostVisibleOutputImage = pImageFactory->createHostVisibleImage(ImageConfig{
        .name = "PipelineTestOutputImage",
        .vkExtent = m_config.vkOutputExtent,
        .vkFormat = m_config.vkOutputFormat,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    });
}

void PipelineIntegrationTest::runTest(uint32_t iterationCount)
{
    auto pCommandQueue = getDevice()->getCommandQueue();

    for (uint32_t iteration = 0U; iteration < iterationCount; iteration++)
    {
        auto pCommandBuffer = pCommandQueue->startScopedCommand(fmt::format("PipelineTestCommand{}", iteration),
                                                                CommandExecutionType::Async);
        m_pipelineFct(*pCommandBuffer, m_pOutputImage);
    }

    {
        auto pCommandBuffer = pCommandQueue->startScopedCommand("PipelineTestBlitToHostVisible",
                                                                CommandExecutionType::Sync);
        {
            auto pBarrierRecorder = pCommandBuffer->startScopedBarrier("prepareHostVisibleOutput");
            pBarrierRecorder->addImageBarrier(*m_pOutputImage, ImageBarrierConfig{
                                                                   .vkDstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                                                                   .vkDstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
                                                                   .vkLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                               });
            pBarrierRecorder->addImageBarrier(*m_pHostVisibleOutputImage,
                                              ImageBarrierConfig{
                                                  .vkDstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                                                  .vkDstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                  .vkLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                              });
        }
        VkImageCopy vkRegion{
            .srcSubresource = m_pOutputImage->getVkSubresourceLayers(),
            .dstSubresource = m_pHostVisibleOutputImage->getVkSubresourceLayers(),
            .extent = toVkExtent3D(m_pOutputImage->getVkExtent()),
        };
        getDevice()->getFcts().vkCmdCopyImage(
            pCommandBuffer->getVkCommandBuffer(), m_pOutputImage->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_pHostVisibleOutputImage->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &vkRegion);
    }
}
