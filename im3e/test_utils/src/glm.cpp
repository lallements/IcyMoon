#include "glm.h"

#include <fmt/format.h>

void glm::PrintTo(const glm::vec2& rVec, std::ostream* pStream)
{
    *pStream << fmt::format("({}, {})", rVec.x, rVec.y);
}

void glm::PrintTo(const glm::vec3& rVec, std::ostream* pStream)
{
    *pStream << fmt::format("({}, {}, {})", rVec.x, rVec.y, rVec.z);
}

void glm::PrintTo(const glm::vec4& rVec, std::ostream* pStream)
{
    *pStream << fmt::format("({}, {}, {}, {})", rVec.x, rVec.y, rVec.z, rVec.w);
}

void glm::PrintTo(const glm::quat& rQuat, std::ostream* pStream)
{
    *pStream << fmt::format("({}, {}, {}, {})", rQuat.w, rQuat.x, rQuat.y, rQuat.z);
}

void glm::PrintTo(const glm::u8vec4& rVec, std::ostream* pS)
{
    *pS << fmt::format("({}, {}, {}, {})", rVec.x, rVec.y, rVec.z, rVec.w);
}
