#pragma once

#include <im3e/utils/view_frustum.h>

#include <glm/glm.hpp>

namespace im3e {

class ICamera
{
public:
    virtual ~ICamera() = default;

    virtual auto getViewFrustum() const -> ViewFrustum = 0;
};

}  // namespace im3e