#include "transform.h"

#include "math_utils.h"

#include <im3e/test_utils/glm.h>
#include <im3e/test_utils/test_utils.h>

#include <fmt/format.h>
#include <gmock/gmock.h>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

using namespace im3e;

TEST(TransformTest, constructor)
{
    Transform transform;
    EXPECT_THAT(transform.getTranslation(), Eq(glm::vec3(0.0F, 0.0F, 0.0F)));
    EXPECT_THAT(transform.getScale(), Eq(glm::vec3(1.0F, 1.0F, 1.0F)));
    EXPECT_THAT(transform.getOrientation(), Eq(glm::quat(1.0F, 0.0F, 0.0F, 0.0F)));
}

TEST(TransformTest, translate)
{
    Transform transform;

    transform.translate(glm::vec3{1.0F, 0.5F, 2.0F});
    EXPECT_THAT(transform.getTranslation(), Eq(glm::vec3{1.0F, 0.5F, 2.0F}));

    transform.translate(glm::vec3{-0.4F, 0.25F, -1.0F});
    EXPECT_THAT(transform.getTranslation(), Eq(glm::vec3{0.6F, 0.75F, 1.0F}));
}

TEST(TransformTest, scale)
{
    Transform transform;

    transform.scale(glm::vec3{2.0F, 3.0F, 4.0F});
    EXPECT_THAT(transform.getScale(), Eq(glm::vec3{2.0F, 3.0F, 4.0F}));

    transform.scale(glm::vec3{0.5F, -1.0F, 2.0F});
    EXPECT_THAT(transform.getScale(), Eq(glm::vec3{1.0F, -3.0F, 8.0F}));
}

TEST(TransformTest, rotate)
{
    const auto rotation1 = glm::rotation(glm::vec3(0.0F, 0.0F, -1.0F), glm::vec3(0.0F, 1.0F, 0.0F));
    const auto rotation2 = glm::rotation(glm::vec3(0.0F, 1.0F, 0.0F), glm::vec3(1.0F, 0.0F, 0.0F));

    Transform transform;

    transform.rotate(90.0_fdeg, glm::vec3(1.0F, 0.0F, 0.0F));
    EXPECT_THAT(transform.getOrientation(), rotation1);

    transform.rotate(-90.0_fdeg, glm::vec3(0.0F, 0.0F, 1.0F));
    EXPECT_THAT(transform.getOrientation(), Eq(rotation2 * rotation1));
}

TEST(TransformTest, reset)
{
    Transform transform;
    transform.setTranslation(glm::vec3(1.0F, 2.0F, 3.0F));
    transform.setScale(glm::vec3(-12.0F, 5.0F, 4.0F));
    transform.setOrientation(glm::quat(2.0F, 3.0F, 4.0F, 5.0F));

    transform.reset();
    EXPECT_THAT(transform.getTranslation(), Eq(glm::vec3(0.0F, 0.0F, 0.0F)));
    EXPECT_THAT(transform.getScale(), Eq(glm::vec3(1.0F, 1.0F, 1.0F)));
    EXPECT_THAT(transform.getOrientation(), Eq(glm::quat(1.0F, 0.0F, 0.0F, 0.0F)));
}

TEST(TransformTest, setRotation)
{
    Transform transform;

    transform.setRotation(90.0_fdeg, glm::vec3(0.0F, 0.0F, 1.0F));
    EXPECT_THAT(transform.getOrientation(),
                Eq(glm::rotation(glm::vec3(1.0F, 0.0F, 0.0F), glm::vec3(0.0F, 1.0F, 0.0F))));
}
