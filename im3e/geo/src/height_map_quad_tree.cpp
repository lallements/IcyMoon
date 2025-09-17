#include "geo.h"

using namespace im3e;

namespace {

auto calculateLodCount(const glm::u32vec2& rSize, const glm::u32vec2& rTileSize)
{
    const auto tileCount = glm::vec2{rSize} / glm::vec2{rTileSize};
    const auto maxTileCount = std::ceil(std::max(tileCount.x, tileCount.y));
    const auto downsampleCount = std::ceil(std::log2(maxTileCount));
    return static_cast<uint32_t>(downsampleCount) + 1U;  // add 1 to account for the base level
}

auto generateTreeNodeChildren(HeightMapQuadTreeNode& rParentNode, const glm::vec2& rParentTileWorldSize)
{
    if (rParentNode.lod == 0U)
    {
        return;
    }
    const auto childLod = rParentNode.lod - 1U;
    const auto childTileWorldSize = glm::ceil(rParentTileWorldSize / 2.0F);

    for (auto y = 0U; y < 2U; y++)
    {
        for (auto x = 0U; x < 2U; x++)
        {
            const auto localChildWorldPos = glm::vec2{x, y} * childTileWorldSize;
            const auto childWorldSize = glm::min(localChildWorldPos + childTileWorldSize, rParentNode.worldSize) -
                                        localChildWorldPos;
            if (childWorldSize.x < 0.0F || childWorldSize.y < 0.0F)
            {
                // If the child world size is negative, this means that this child tile is outside of the boundaries
                // of the parent. It should be therefore not be skipped:
                continue;
            }

            auto pChildNode = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
                .lod = childLod,
                .tilePos = 2U * rParentNode.tilePos + glm::u32vec2{x, y},
                .worldPos = rParentNode.worldPos + localChildWorldPos,
                .worldSize = childWorldSize,
            });
            rParentNode.pChildren[y * 2U + x] = pChildNode;
            generateTreeNodeChildren(*pChildNode, childTileWorldSize);
        }
    }
}

}  // namespace

auto im3e::generateHeightMapQuadTree(const IHeightMap& rHeightMap) -> std::shared_ptr<HeightMapQuadTreeNode>
{
    // TODO:
    // We should generate the tree with cells present regardless of whether tiles actually exist.
    // However, we do need to add a bool to determine whether the current cell contains an actual tile

    const auto size = rHeightMap.getSize();
    const auto tileSize = rHeightMap.getTileSize();
    const auto lodCount = calculateLodCount(size, tileSize);

    auto pRoot = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
        .lod = lodCount - 1U,
        .worldSize = glm::vec2{size},
    });

    const auto tileWorldSize = glm::vec2{tileSize} * static_cast<float>(std::pow(2U, pRoot->lod));
    generateTreeNodeChildren(*pRoot, tileWorldSize);
    return pRoot;
}