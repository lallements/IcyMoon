#pragma once

#include <im3e/utils/types.h>

namespace im3e {

class ICommandBuffer
{
public:
    virtual ~ICommandBuffer() = default;
};

class ICommandBuffers
{
public:
    virtual ~ICommandBuffers() = default;

    virtual auto recordCommand() -> UniquePtrWithDeleter<ICommandBuffer> = 0;
};

}  // namespace im3e