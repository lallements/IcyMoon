#include "view_frustum.h"

#include <im3e/test_utils/glm.h>
#include <im3e/test_utils/test_utils.h>

#include <glm/gtx/rotate_vector.hpp>

using namespace im3e;

TEST(ViewFrustumTest, constructorWithDefaultPerspectiveCamera)
{
    const ViewFrustum::PerspectiveConfig config{
        .near = 0.1F,
        .far = 100.0F,
        .position = glm::vec3{},
        .direction = glm::vec3{0.0F, 0.0F, -1.0F},
        .up = glm::vec3{0.0F, 1.0F, 0.0F},
        .right = glm::vec3{1.0F, 0.0F, 0.0F},
    };
    ViewFrustum frustum{config};

    EXPECT_THAT(frustum.getNearPlane(), FloatEq(glm::vec4(config.direction, -config.near))) << "near plane";
    EXPECT_THAT(frustum.getFarPlane(), FloatEq(glm::vec4(config.direction, -config.far))) << "far plane";

    // Following plane normals are pre-calculated. They all result in combination of these two numbers and zero:
    const auto v1 = 0.86602545F;
    const auto normalZ = -0.49999997F;  // all normals have the same z component
    const glm::vec3 tNormal{0.0F, -v1, normalZ};
    const glm::vec3 bNormal{0.0F, v1, normalZ};
    const glm::vec3 lNormal{v1, 0.0F, normalZ};
    const glm::vec3 rNormal{-v1, 0.0F, normalZ};

    EXPECT_THAT(frustum.getTopPlane(), FloatEq(glm::vec4(tNormal, 0.0F))) << "top plane";
    EXPECT_THAT(frustum.getBottomPlane(), FloatEq(glm::vec4(bNormal, 0.0F))) << "bottom plane";

    EXPECT_THAT(frustum.getLeftPlane(), FloatEq(glm::vec4(lNormal, 0.0F))) << "left plane";
    EXPECT_THAT(frustum.getRightPlane(), FloatEq(glm::vec4(rNormal, 0.0F))) << "right plane";
}