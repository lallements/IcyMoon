#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace im3e {

class IHeightMapTileSampler
{
public:
    virtual ~IHeightMapTileSampler() = default;

    virtual auto at(uint32_t x, uint32_t y) const -> float = 0;
    virtual auto at(const glm::u32vec2& rPos) const -> float = 0;

    virtual auto getPos() const -> const glm::u32vec2& = 0;
    virtual auto getSize() const -> const glm::u32vec2& = 0;
    virtual auto getActualSize() const -> const glm::u32vec2& = 0;
    virtual auto getScale() const -> float = 0;
};

class IHeightMap
{
public:
    virtual ~IHeightMap() = default;

    virtual auto getTileSampler(const glm::u32vec2& rTilePos, uint32_t lod)
        -> std::unique_ptr<IHeightMapTileSampler> = 0;

    virtual auto getName() const -> std::string = 0;
    virtual auto getTileSize() const -> glm::u32vec2 = 0;
    virtual auto getTileCount(uint32_t lod) const -> glm::u32vec2 = 0;
    virtual auto getLodCount() const -> uint32_t = 0;
};

}  // namespace im3e