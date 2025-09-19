#pragma once

#include <im3e/api/camera.h>
#include <im3e/api/height_map.h>
#include <im3e/utils/loggers.h>

#include <glm/glm.hpp>

#include <array>
#include <filesystem>
#include <memory>
#include <optional>

namespace im3e {

struct HeightMapFileConfig
{
    std::filesystem::path path;
    bool readOnly = true;
};
auto loadHeightMapFromFile(const ILogger& rLogger, HeightMapFileConfig config) -> std::unique_ptr<IHeightMap>;

struct HeightMapQuadTreeNode
{
    uint32_t lod;

    glm::u32vec2 tilePos;

    glm::vec2 worldPos;
    glm::vec2 worldSize;

    std::array<std::shared_ptr<HeightMapQuadTreeNode>, 4U> pChildren;

    /// @brief Find tiles that visible from the given camera.
    /// @return List of vec3 defined as (x, y, l) with (x, y) the tile position at the level of detail l.
    auto findVisible(const ICamera& rCamera, uint32_t lod) const -> std::vector<glm::u32vec3>;
};
auto generateHeightMapQuadTree(const IHeightMap& rHeightMap) -> std::shared_ptr<HeightMapQuadTreeNode>;

}  // namespace im3e