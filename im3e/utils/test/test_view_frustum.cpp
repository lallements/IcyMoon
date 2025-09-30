#include "view_frustum.h"

#include <im3e/test_utils/glm.h>
#include <im3e/test_utils/test_utils.h>

#include <glm/gtx/rotate_vector.hpp>

#include <numbers>

using namespace im3e;

namespace {

void expectNearFarPlanesValid(const ViewFrustum& rFrustum, const ViewFrustum::PerspectiveConfig& rConfig)
{
    const auto nearP0 = rConfig.position + rConfig.direction * rConfig.near;
    EXPECT_THAT(rFrustum.getNearPlane(), FloatEq(glm::vec4(rConfig.direction, -glm::dot(rConfig.direction, nearP0))))
        << "near plane";

    const auto farP0 = rConfig.position + rConfig.direction * rConfig.far;
    EXPECT_THAT(rFrustum.getFarPlane(), FloatEq(glm::vec4(-rConfig.direction, -glm::dot(-rConfig.direction, farP0))))
        << "far plane";
}

void expectTopBottomPlanesValid(const ViewFrustum& rFrustum, const ViewFrustum::PerspectiveConfig& rConfig,
                                const glm::vec3& rExpectedTopNormal, const glm::vec3& rExpectedBottomNormal)
{
    EXPECT_THAT(rFrustum.getTopPlane(),
                FloatEq(glm::vec4(rExpectedTopNormal, -glm::dot(rExpectedTopNormal, rConfig.position))))
        << "top plane";
    EXPECT_THAT(rFrustum.getBottomPlane(),
                FloatEq(glm::vec4(rExpectedBottomNormal, -glm::dot(rExpectedBottomNormal, rConfig.position))))
        << "bottom plane";
}

void expectLeftRightPlanesValid(const ViewFrustum& rFrustum, const ViewFrustum::PerspectiveConfig& rConfig,
                                const glm::vec3& rExpectedLeftNormal, const glm::vec3& rExpectedRightNormal)
{
    EXPECT_THAT(rFrustum.getLeftPlane(),
                FloatEq(glm::vec4(rExpectedLeftNormal, -glm::dot(rExpectedLeftNormal, rConfig.position))))
        << "left plane";
    EXPECT_THAT(rFrustum.getRightPlane(),
                FloatEq(glm::vec4(rExpectedRightNormal, -glm::dot(rExpectedRightNormal, rConfig.position))))
        << "right plane";
}

}  // namespace

TEST(ViewFrustumTest, constructorWithDefaultPerspectiveCamera)
{
    const ViewFrustum::PerspectiveConfig config{
        .fovY = std::numbers::pi_v<float> / 3.0F,
        .aspectRatio = 1.0F,
        .near = 0.1F,
        .far = 100.0F,
        .position = glm::vec3{},
        .direction = glm::vec3{0.0F, 0.0F, -1.0F},
        .up = glm::vec3{0.0F, 1.0F, 0.0F},
        .right = glm::vec3{1.0F, 0.0F, 0.0F},
    };
    ViewFrustum frustum{config};

    expectNearFarPlanesValid(frustum, config);

    // Following plane normals are pre-calculated. They all result in combination of these two numbers and zero:
    const auto v1 = std::sin(config.fovY);
    const auto v2 = -std::cos(config.fovY);
    const glm::vec3 tNormal{0.0F, -v1, v2};
    const glm::vec3 bNormal{0.0F, v1, v2};
    const glm::vec3 lNormal{v1, 0.0F, v2};
    const glm::vec3 rNormal{-v1, 0.0F, v2};
    expectTopBottomPlanesValid(frustum, config, tNormal, bNormal);
    expectLeftRightPlanesValid(frustum, config, lNormal, rNormal);
}

TEST(ViewFrustumTest, constructorWithDefaultPerspectiveCameraTranslated)
{
    const ViewFrustum::PerspectiveConfig config{
        .fovY = std::numbers::pi_v<float> / 3.0F,
        .aspectRatio = 1.0F,
        .near = 0.1F,
        .far = 100.0F,
        .position = glm::vec3{-5.0F, 24.3F, 1932.8F},
        .direction = glm::vec3{0.0F, 0.0F, -1.0F},
        .up = glm::vec3{0.0F, 1.0F, 0.0F},
        .right = glm::vec3{1.0F, 0.0F, 0.0F},
    };
    ViewFrustum frustum{config};

    expectNearFarPlanesValid(frustum, config);

    // Following plane normals are pre-calculated. They all result in combination of these two numbers and zero:
    const auto v1 = std::sin(config.fovY);
    const auto v2 = -std::cos(config.fovY);
    const glm::vec3 tNormal{0.0F, -v1, v2};
    const glm::vec3 bNormal{0.0F, v1, v2};
    const glm::vec3 lNormal{v1, 0.0F, v2};
    const glm::vec3 rNormal{-v1, 0.0F, v2};
    expectTopBottomPlanesValid(frustum, config, tNormal, bNormal);
    expectLeftRightPlanesValid(frustum, config, lNormal, rNormal);
}

TEST(ViewFrustumTest, constructorWithDefaultPerspectiveCameraLookingDown)
{
    const ViewFrustum::PerspectiveConfig config{
        .fovY = std::numbers::pi_v<float> / 3.0F,
        .aspectRatio = 1.0F,
        .near = 0.1F,
        .far = 100.0F,
        .position = glm::vec3{},
        .direction = glm::vec3{0.0F, -1.0F, 0.0F},
        .up = glm::vec3{0.0F, 0.0F, -1.0F},
        .right = glm::vec3{1.0F, 0.0F, 0.0F},
    };
    ViewFrustum frustum{config};

    expectNearFarPlanesValid(frustum, config);

    // Following plane normals are pre-calculated. They all result in combination of these two numbers and zero:
    const auto v1 = std::sin(config.fovY);
    const auto v2 = -std::cos(config.fovY);
    const glm::vec3 tNormal{0.0F, v2, v1};
    const glm::vec3 bNormal{0.0F, v2, -v1};
    const glm::vec3 lNormal{v1, v2, 0.0F};
    const glm::vec3 rNormal{-v1, v2, 0.0F};
    expectTopBottomPlanesValid(frustum, config, tNormal, bNormal);
    expectLeftRightPlanesValid(frustum, config, lNormal, rNormal);
}

TEST(ViewFrustumTest, isAABBInside)
{
    const ViewFrustum::PerspectiveConfig config{
        .fovY = std::numbers::pi_v<float> / 2.0F,
        .aspectRatio = 1920.0F / 1080.0F,
        .near = 1.0F,
        .far = 100.0F,
        .position = glm::vec3{0.0F, 10.0F, 0.0F},
        .direction = glm::vec3{0.0F, 0.0F, -1.0F},
        .right = glm::vec3{1.0F, 0.0F, 0.0F},
    };
    ViewFrustum frustum{config};

    EXPECT_THAT(frustum.isAABBInside(glm::vec3{-1.0F, 9.0F, -5.0F}, glm::vec3{1.0F, 11.0F, -4.0F}), IsTrue());
    EXPECT_THAT(frustum.isAABBInside(glm::vec3{1.0F, 9.0F, 5.0F}, glm::vec3{2.0F, 10.0F, 0.0F}), IsFalse());
}