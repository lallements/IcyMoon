#include "view_frustum.h"

#include <glm/gtx/rotate_vector.hpp>

using namespace im3e;

ViewFrustum::ViewFrustum(const PerspectiveConfig& rConfig)
  : m_nearPlane(rConfig.direction, -glm::dot(rConfig.direction, (rConfig.position + rConfig.near * rConfig.direction)))
  , m_farPlane(rConfig.direction, -glm::dot(rConfig.direction, (rConfig.position + rConfig.far * rConfig.direction)))
{
    // Top / Bottom planes
    {
        const auto topNormalAngle = (rConfig.fovY - std::numbers::pi_v<float>) / 2.0F;
        const auto topNormal = glm::normalize(glm::rotate(rConfig.direction, topNormalAngle, rConfig.right));
        const auto bottomNormal = glm::normalize(glm::rotate(rConfig.direction, -topNormalAngle, rConfig.right));

        m_topPlane = Plane(topNormal, -glm::dot(topNormal, rConfig.position));
        m_bottomPlane = Plane(bottomNormal, -glm::dot(bottomNormal, rConfig.position));
    }

    // Left / Right planes
    {
        const auto fovX = rConfig.fovY * rConfig.aspectRatio;
        const auto leftNormalAngle = (fovX - std::numbers::pi_v<float>) / 2.0F;
        const auto leftNormal = glm::normalize(glm::rotate(rConfig.direction, leftNormalAngle, rConfig.up));
        const auto rightNormal = glm::normalize(glm::rotate(rConfig.direction, -leftNormalAngle, rConfig.up));

        m_leftPlane = Plane(leftNormal, -glm::dot(leftNormal, rConfig.position));
        m_rightPlane = Plane(rightNormal, -glm::dot(rightNormal, rConfig.position));
    }
}