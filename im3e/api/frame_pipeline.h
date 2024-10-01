#pragma once

#include "command_buffer.h"
#include "image.h"

#include <im3e/utils/vk_utils.h>

namespace im3e {

/// @brief Pipeline of commands to execute to generate a frame.
class IFramePipeline
{
public:
    virtual ~IFramePipeline() = default;

    /// @brief Schedule pipeline execution
    ///
    /// @param rCommandBuffer Object used to record commands that will be executed.
    /// @param pOutputImage Image that the pipeline should write the final results to.
    virtual void prepareExecution(ICommandBuffer& rCommandBuffer, std::shared_ptr<IImage> pOutputImage) = 0;

    /// @brief Resize the frame
    virtual void resize(const VkExtent2D& rVkExtent) = 0;
};

}  // namespace im3e