#pragma once

#include <gmock/gmock.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#include <limits>

namespace glm {

void PrintTo(const glm::u16vec3& rVec, std::ostream* pStream);
void PrintTo(const glm::u32vec2& rVec, std::ostream* pStream);
void PrintTo(const glm::u32vec3& rVec, std::ostream* pStream);
void PrintTo(const glm::vec2& rVec, std::ostream* pStream);
void PrintTo(const glm::vec3& rVec, std::ostream* pStream);
void PrintTo(const glm::vec4& rVec, std::ostream* pStream);
void PrintTo(const glm::quat& rQuat, std::ostream* pStream);
void PrintTo(const glm::u8vec4& rVec, std::ostream* pStream);

}  // namespace glm

namespace im3e {

inline auto FloatEq(const glm::vec2& rVec)
{
    return testing::AllOf(testing::Field(&glm::vec2::x, testing::FloatEq(rVec.x)),
                          testing::Field(&glm::vec2::y, testing::FloatEq(rVec.y)));
}

inline auto FloatEq(const glm::vec3& rVec)
{
    return testing::AllOf(testing::Field(&glm::vec3::x, testing::FloatEq(rVec.x)),
                          testing::Field(&glm::vec3::y, testing::FloatEq(rVec.y)),
                          testing::Field(&glm::vec3::z, testing::FloatEq(rVec.z)));
}

inline auto FloatEq(const glm::vec4& rVec)
{
    return testing::AllOf(testing::Field(&glm::vec4::x, testing::FloatEq(rVec.x)),
                          testing::Field(&glm::vec4::y, testing::FloatEq(rVec.y)),
                          testing::Field(&glm::vec4::z, testing::FloatEq(rVec.z)),
                          testing::Field(&glm::vec4::w, testing::FloatEq(rVec.w)));
}

inline auto FloatEq(const glm::quat& rQuat)
{
    return testing::AllOf(testing::Field(&glm::quat::w, testing::FloatEq(rQuat.w)),
                          testing::Field(&glm::quat::x, testing::FloatEq(rQuat.x)),
                          testing::Field(&glm::quat::y, testing::FloatEq(rQuat.y)),
                          testing::Field(&glm::quat::z, testing::FloatEq(rQuat.z)));
}

inline auto FloatNear(const glm::vec2& rVec, float maxAbsError = std::numeric_limits<float>::epsilon())
{
    return testing::AllOf(testing::Field(&glm::vec2::x, testing::FloatNear(rVec.x, maxAbsError)),
                          testing::Field(&glm::vec2::y, testing::FloatNear(rVec.y, maxAbsError)));
}

inline auto FloatNear(const glm::vec3& rVec, float maxAbsError = std::numeric_limits<float>::epsilon())
{
    return testing::AllOf(testing::Field(&glm::vec3::x, testing::FloatNear(rVec.x, maxAbsError)),
                          testing::Field(&glm::vec3::y, testing::FloatNear(rVec.y, maxAbsError)),
                          testing::Field(&glm::vec3::z, testing::FloatNear(rVec.z, maxAbsError)));
}

inline auto FloatNear(const glm::quat& rQuat, float maxAbsError = std::numeric_limits<float>::epsilon())
{
    return testing::AllOf(testing::Field(&glm::quat::w, testing::FloatNear(rQuat.w, maxAbsError)),
                          testing::Field(&glm::quat::x, testing::FloatNear(rQuat.x, maxAbsError)),
                          testing::Field(&glm::quat::y, testing::FloatNear(rQuat.y, maxAbsError)),
                          testing::Field(&glm::quat::z, testing::FloatNear(rQuat.z, maxAbsError)));
}

}  // namespace im3e