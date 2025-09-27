#include "geo.h"

#include <im3e/utils/core/throw_utils.h>

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
    if (rParentNode.tileID.z == 0U)
    {
        return;
    }
    const auto childLod = rParentNode.tileID.z - 1U;
    const auto childTileWorldSize = glm::vec3{glm::ceil(rParentTileWorldSize / 2.0F),
                                              rParentNode.maxWorldPos.z - rParentNode.minWorldPos.z};

    for (uint16_t y = 0U; y < 2U; y++)
    {
        for (uint16_t x = 0U; x < 2U; x++)
        {
            const auto childMinWorldPos = rParentNode.minWorldPos + glm::vec3{x, y, 0.0F} * childTileWorldSize;
            const auto childMaxWorldPos = glm::min(childMinWorldPos + childTileWorldSize, rParentNode.maxWorldPos);
            if (childMaxWorldPos.x <= childMinWorldPos.x || childMaxWorldPos.y <= childMinWorldPos.y)
            {
                // If the child world size is negative, this means that this child tile is outside of the boundaries
                // of the parent. It should therefore be skipped:
                continue;
            }

            auto pChildNode = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
                .tileID = TileID{uint16_t{2U} * rParentNode.tileID.xy() + glm::u16vec2{x, y}, childLod},
                .minWorldPos = childMinWorldPos,
                .maxWorldPos = childMaxWorldPos,
            });
            rParentNode.pChildren[y * 2U + x] = pChildNode;
            generateTreeNodeChildren(*pChildNode, childTileWorldSize);
        }
    }
}

void findVisibleInQuadTree(const HeightMapQuadTreeNode& rNode, const ViewFrustum& rViewFrustum, uint32_t lod,
                           std::vector<TileID>& rVisibleTileIDs)
{
    if (!rViewFrustum.isAABBInside(rNode.minWorldPos, rNode.maxWorldPos))
    {
        return;
    }

    if (rNode.tileID.z == lod)
    {
        rVisibleTileIDs.emplace_back(rNode.tileID);
        return;
    }

    for (auto& rpChild : rNode.pChildren)
    {
        if (rpChild)
        {
            findVisibleInQuadTree(*rpChild, rViewFrustum, lod, rVisibleTileIDs);
        }
    }
}

}  // namespace

auto HeightMapQuadTreeNode::findVisible(const ViewFrustum& rViewFrustum, uint32_t lod) const -> std::vector<TileID>
{
    throwIfFalse<std::invalid_argument>(
        lod <= this->tileID.z, fmt::format("Invalid lod {} passed to quad tree of max level {}", lod, this->tileID.z));

    std::vector<TileID> visibleTileIDs;
    findVisibleInQuadTree(*this, rViewFrustum, lod, visibleTileIDs);
    return visibleTileIDs;
}

auto im3e::generateHeightMapQuadTree(const IHeightMap& rHeightMap) -> std::shared_ptr<HeightMapQuadTreeNode>
{
    const auto size = rHeightMap.getSize();
    const auto tileSize = rHeightMap.getTileSize();
    const auto lodCount = calculateLodCount(size, tileSize);

    auto pRoot = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
        .tileID = TileID{0U, 0U, lodCount - 1U},
        .minWorldPos = glm::vec3{0.0F, 0.0F, rHeightMap.getMinHeight()},
        .maxWorldPos = glm::vec3{size, rHeightMap.getMaxHeight()},
    });

    const auto tileWorldSize = glm::vec2{tileSize} * static_cast<float>(std::pow(2U, pRoot->tileID.z));
    generateTreeNodeChildren(*pRoot, tileWorldSize);
    return pRoot;
}