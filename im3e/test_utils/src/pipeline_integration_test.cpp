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
            auto pImageMapping = m_pOutputImage->mapReadOnly();
            pImageMapping->save(outputFilePath);
        }
        ADD_FAILURE() << fmt::format(R"(Saved pipeline output to : "{}")", outputFilePath.string());
    }

    DeviceIntegrationTest::TearDown();
}

auto PipelineIntegrationTest::mapOutputImage() const -> unique_ptr<const IHostVisibleImage::IMapping>
{
    return m_pOutputImage->mapReadOnly();
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
    // TODO: create sink image (device only) here and find a way to get usage from pipeline somehow
    m_pOutputImage = pImageFactory->createHostVisibleImage(ImageConfig{
        .name = "PipelineTestOutputImage",
        .vkExtent = m_config.vkOutputExtent,
        .vkFormat = m_config.vkOutputFormat,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    });
}

void PipelineIntegrationTest::runTest(uint32_t iterationCount)
{
    for (uint32_t iteration = 0U; iteration < iterationCount; iteration++)
    {
        const auto executionType = iteration == (iterationCount - 1U) ? CommandExecutionType::Sync
                                                                      : CommandExecutionType::Async;
        auto pCommandBuffer = getDevice()->getCommandQueue()->startScopedCommand(
            fmt::format("PipelineTestCommand{}", iteration), executionType);
        m_pipelineFct(*pCommandBuffer, m_pOutputImage);
    }
}
