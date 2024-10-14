#pragma once

#include <im3e/utils/vk_utils.h>

namespace im3e {

class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() = default;

    virtual auto getVkCommandBuffer() const -> VkCommandBuffer = 0;
};

enum class CommandExecutionType : uint8_t
{
    Sync = 0U,
    Async = 1U,
};

class ICommandQueue
{
public:
    virtual ~ICommandQueue() = default;

    /// @brief Start recording commands to a command buffer associated to this queue.
    /// The commands are submitted once the returned object goes out of scope.
    ///
    /// @param[in] name Name of the command used as a debug label
    /// @param[in] executionType Determines whether the execution of the commands is blocking or non-blocking when
    /// submitted i.e. when the returned object is destroyed.
    ///
    /// @return Pointer to the command buffer assigned for recording. The buffer is submitted for execution as soon
    /// as the returned object is destroyed.
    virtual auto startScopedCommand(std::string_view name, CommandExecutionType executionType)
        -> UniquePtrWithDeleter<ICommandBuffer> = 0;

    virtual auto getVkQueue() const -> VkQueue = 0;
};

}  // namespace im3e