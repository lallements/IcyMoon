#pragma once

#include <glm/glm.hpp>

namespace im3e {

class ICamera
{
public:
    virtual ~ICamera() = default;

    virtual void setPosition(const glm::vec3& rPosition) = 0;
    virtual void setDirection(const glm::vec3& rDirection) = 0;
    virtual void setUp(const glm::vec3& rUp) = 0;
    virtual void setAspectRatio(float ratio) = 0;

    virtual auto getPosition() const -> glm::vec3 = 0;
    virtual auto getDirection() const -> glm::vec3 = 0;
    virtual auto getUp() const -> glm::vec3 = 0;
    virtual auto getAspectRatio() const -> float = 0;
};

}  // namespace im3e