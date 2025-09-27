#pragma once

#include <glm/glm.hpp>

#include <unordered_set>

namespace im3e {

using TileID = glm::u16vec3;

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

template <>
struct std::hash<im3e::TileID>
{
    std::size_t operator()(const im3e::TileID& rTileID) const noexcept
    {
        return static_cast<std::size_t>(rTileID.x) << 32 |  //
               static_cast<std::size_t>(rTileID.y) << 16 |  //
               static_cast<std::size_t>(rTileID.z);
    }
};
