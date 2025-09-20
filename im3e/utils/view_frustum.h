#pragma once

#include <glm/glm.hpp>

#include <numbers>

namespace im3e {

/// @brief Class representing the view frustum of a camera.
class ViewFrustum
{
public:
    /// @brief Plane represented as (a, b, c, d) so that a point (x, y, z) is on the plane if ax + by + cz + d = 0.
    /// This means that:
    ///      - (a, b, c) is the normal to the plane
    ///      - d is formed using a point P_0 on the plane with coordinates (x_0, y_0, z_0):
    ///          d = -(ax_0 + bx_0 + cx_0)
    using Plane = glm::vec4;

    struct PerspectiveConfig
    {
        float fovY = std::numbers::pi_v<float> / 3.0F;
        float aspectRatio = 1.0F;

        float near = 0.1F;
        float far = 1000.0F;

        glm::vec3 position{0.0F, 0.0F, 0.0F};
        glm::vec3 direction{0.0F, 0.0F, -1.0F};
        glm::vec3 up{0.0F, 1.0F, 0.0F};
        glm::vec3 right{1.0F, 0.0F, 0.0F};
    };
    ViewFrustum(const PerspectiveConfig& rConfig);

    auto getNearPlane() const { return m_nearPlane; }
    auto getFarPlane() const { return m_farPlane; }
    auto getTopPlane() const { return m_topPlane; }
    auto getBottomPlane() const { return m_bottomPlane; }
    auto getLeftPlane() const { return m_leftPlane; }
    auto getRightPlane() const { return m_rightPlane; }

private:
    Plane m_nearPlane{};
    Plane m_farPlane{};

    Plane m_topPlane{};
    Plane m_bottomPlane{};

    Plane m_leftPlane{};
    Plane m_rightPlane{};
};

}  // namespace im3e