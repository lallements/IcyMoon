#pragma once

#include <glm/glm.hpp>

namespace im3e {

constexpr float operator"" _fdeg(long double deg) noexcept
{
    return static_cast<float>(glm::radians(deg));
}

constexpr auto alignUp(uint32_t value, uint32_t alignment) noexcept
{
    return (value + alignment - 1U) & ~(alignment - 1U);
}

constexpr auto toNormalizedVec(const glm::u8vec4& rVec)
{
    constexpr auto normalizeValue = [](uint8_t value) { return static_cast<float>(value) / 255.0F; };

    return glm::vec4(normalizeValue(rVec.x), normalizeValue(rVec.y), normalizeValue(rVec.z), normalizeValue(rVec.w));
}

}  // namespace im3e