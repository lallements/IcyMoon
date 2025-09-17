#pragma once

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
};
auto generateHeightMapQuadTree(const IHeightMap& rHeightMap) -> std::shared_ptr<HeightMapQuadTreeNode>;

}  // namespace im3e