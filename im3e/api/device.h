#pragma once

#include "image.h"

namespace im3e {

class IDevice
{
public:
    virtual ~IDevice() = default;

    virtual auto getImageFactory() const -> std::shared_ptr<const IImageFactory> = 0;
};

}  // namespace im3e