#include "view_frustum.h"

#include <glm/gtx/rotate_vector.hpp>

using namespace im3e;

ViewFrustum::ViewFrustum(const PerspectiveConfig& rConfig)
  : m_planes([&rConfig] {
      std::array<Plane, 6U> planes;

      // Near / Far planes
      {
          const auto nearP0 = rConfig.position + rConfig.near * rConfig.direction;
          const auto farP0 = rConfig.position + rConfig.far * rConfig.direction;
          planes[NearPlaneIdx] = Plane(rConfig.direction, -glm::dot(rConfig.direction, nearP0));
          planes[FarPlaneIdx] = Plane(-rConfig.direction, -glm::dot(-rConfig.direction, farP0));
      }

      // Top / Bottom planes
      {
          const auto topNormalAngle = (rConfig.fovY - std::numbers::pi_v<float>) / 2.0F;
          const auto topNormal = glm::normalize(glm::rotate(rConfig.direction, topNormalAngle, rConfig.right));
          const auto bottomNormal = glm::normalize(glm::rotate(rConfig.direction, -topNormalAngle, rConfig.right));

          planes[TopPlaneIdx] = Plane(topNormal, -glm::dot(topNormal, rConfig.position));
          planes[BottomPlaneIdx] = Plane(bottomNormal, -glm::dot(bottomNormal, rConfig.position));
      }

      // Left / Right planes
      {
          const auto fovX = rConfig.fovY * rConfig.aspectRatio;
          const auto leftNormalAngle = (fovX - std::numbers::pi_v<float>) / 2.0F;
          const auto leftNormal = glm::normalize(glm::rotate(rConfig.direction, leftNormalAngle, rConfig.up));
          const auto rightNormal = glm::normalize(glm::rotate(rConfig.direction, -leftNormalAngle, rConfig.up));

          planes[LeftPlaneIdx] = Plane(leftNormal, -glm::dot(leftNormal, rConfig.position));
          planes[RightPlaneIdx] = Plane(rightNormal, -glm::dot(rightNormal, rConfig.position));
      }

      return planes;
  }())

{
}

auto ViewFrustum::isAABBInside(const glm::vec3& rMinPoint, const glm::vec3& rMaxPoint) const -> bool
{
    // An AABB is at least partially inside if the AABB is at least partially inside every plane of the frustum.
    // A box is inside a plane if it is on the same side as the plane normale. This is because the planes of the
    // frustum are defined so that their normal points towards the inside of the frustum.
    for (const auto& rPlane : m_planes)
    {
        // The AABB is at least partially inside the current plane if at least one point is inside. We only need to test
        // the point of the AABB that is farstest from the plane on the side of the normal.
        const glm::vec3 furthestAABBPoint{
            rPlane.x >= 0.0F ? rMaxPoint.x : rMinPoint.x,
            rPlane.y >= 0.0F ? rMaxPoint.y : rMinPoint.y,
            rPlane.z >= 0.0F ? rMaxPoint.z : rMinPoint.z,
        };

        if (glm::dot(rPlane.xyz(), furthestAABBPoint) + rPlane.w < 0.0F)
        {
            return false;
        }
    }
    return true;
}