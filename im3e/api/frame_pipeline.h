#pragma once

#include <im3e/api/commands.h>
#include <im3e/utils/vk_utils.h>

#include <functional>

namespace im3e {

struct FramePipelineSinkConfig
{
    /// @brief Optional. When specified, the pipeline copies the output image to this image.
    std::shared_ptr<IImage> pImage;

    /// Optional function to be executed immediately after the pipeline result is blit into the sink image.
    std::function<void(const ICommandBuffer& rCommandBuffer, IImage& rSinkImage)> postSinkTask;
};

/// @brief Pipeline of commands to execute to generate a frame.
class IFramePipeline
{
public:
    virtual ~IFramePipeline() = default;

    /// @brief Schedule pipeline execution
    ///
    /// @param rCommands Object used to record commands that will be executed.
    virtual void prepareExecution(ICommandBuffers& rCommands) = 0;

    /// @brief Resize the frame
    virtual void resize(const VkExtent2D& rVkExtent, uint32_t frameInFlightCount) = 0;
};

/// @brief Object that manages the execution of a IFramePipeline.
class IFramePipelineRunner
{
public:
    virtual ~IFramePipelineRunner() = default;

    /// @brief Schedule pipeline execution.
    ///
    /// Frame pipeline stages are prepared for execution and submitted to the GPU queue. The actual execution will be
    /// started when vkWaitSemaphore is set or immediately if no semaphore is provided in parameter.
    /// The function is non-blocking, a semaphore is returned to notify when the pipeline execution is complete.
    ///
    /// @param[in] vkWaitSemaphore Semaphore the pipeline should wait for before GPU command execution. If not provided,
    /// the pipeline is executed as soon as possible.
    /// @return Semaphore to notify when the pipeline execution is complete.
    virtual auto executeAsync(VkSemaphore vkWaitSemaphore = nullptr) -> VkSemaphore = 0;

    /// @brief Schedule pipeline execution and wait for its completion.
    virtual void execute(VkSemaphore vkWaitSemaphore = nullptr) = 0;

    /// @brief Resize the frame
    virtual void resize(const VkExtent2D& rVkExtent, uint32_t frameInFlightCount) = 0;
};

}  // namespace im3e