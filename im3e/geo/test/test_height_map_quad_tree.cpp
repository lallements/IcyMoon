#include "geo.h"

#include <im3e/mock/mock_height_map.h>
#include <im3e/test_utils/glm.h>
#include <im3e/test_utils/test_utils.h>

#include <fmt/format.h>

using namespace im3e;

namespace {

void expectTreesEqual(HeightMapQuadTreeNode& rParent, HeightMapQuadTreeNode& rExpectedParent)
{
    const auto errorMessage = fmt::format("Tree Node at LOD {} and Tile Pos ({}, {})", rExpectedParent.lod,
                                          rExpectedParent.tilePos.x, rExpectedParent.tilePos.y);

    EXPECT_THAT(rParent.lod, Eq(rExpectedParent.lod)) << errorMessage;
    EXPECT_THAT(rParent.tilePos, Eq(rExpectedParent.tilePos)) << errorMessage;
    EXPECT_THAT(rParent.worldPos, FloatEq(rExpectedParent.worldPos)) << errorMessage;
    EXPECT_THAT(rParent.worldSize, FloatEq(rExpectedParent.worldSize)) << errorMessage;
    for (auto i = 0U; i < 4U; i++)
    {
        const auto childErrorMessage = fmt::format("Child {} of {}", i, errorMessage);
        if (!rExpectedParent.pChildren[i])
        {
            EXPECT_THAT(rParent.pChildren[i], IsNull()) << childErrorMessage;
        }
        else
        {
            ASSERT_THAT(rParent.pChildren[i], NotNull()) << childErrorMessage;
            expectTreesEqual(*rParent.pChildren[i], *rExpectedParent.pChildren[i]);
        }
    }
}

}  // namespace

TEST(HeightMapQuadTreeTest, generateHeightMapQuadTree)
{
    constexpr glm::u32vec2 TestSize{520U, 1050U};
    constexpr glm::u32vec2 TestTileSize{512U, 512U};
    constexpr uint32_t TestLodCount{3U};

    // Expected Level 2: a single tile
    auto pExpectedRoot = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
        .lod = 2U,
        .worldSize = glm::vec2{520.0F, 1050.0F},
    });

    // Expected Level 1: 2x1 tiles
    {
        pExpectedRoot->pChildren[0] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 1U,
            .worldSize = glm::vec2{520.0F, 1024.0F},
        });
        pExpectedRoot->pChildren[1] = nullptr;
        pExpectedRoot->pChildren[2] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 1U,
            .tilePos = glm::u32vec2{0U, 1U},
            .worldPos = glm::vec2{0.0F, 1024.0F},
            .worldSize = glm::vec2{520.0F, 26.0F},
        });
        pExpectedRoot->pChildren[3] = nullptr;
    }

    // Expected Level 0: 2x3 tiles
    {
        auto pChild0L1 = pExpectedRoot->pChildren[0];
        pChild0L1->pChildren[0] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 0U,
            .worldSize = glm::vec2{512.0F, 512.0F},
        });
        pChild0L1->pChildren[1] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 0U,
            .tilePos = glm::u32vec2{1U, 0U},
            .worldPos = glm::vec2{512.0F, 0.0F},
            .worldSize = glm::vec2{8.0F, 512.0F},
        });
        pChild0L1->pChildren[2] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 0U,
            .tilePos = glm::u32vec2{0U, 1U},
            .worldPos = glm::vec2{0.0F, 512.0F},
            .worldSize = glm::vec2{512.0F, 512.0F},
        });
        pChild0L1->pChildren[3] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 0U,
            .tilePos = glm::u32vec2{1U, 1U},
            .worldPos = glm::vec2{512.0F, 512.0F},
            .worldSize = glm::vec2{8.0F, 512.0F},
        });
        auto pChild1L1 = pExpectedRoot->pChildren[2];
        pChild1L1->pChildren[0] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 0U,
            .tilePos = glm::u32vec2{0U, 2U},
            .worldPos = glm::vec2{0.0F, 1024.0F},
            .worldSize = glm::vec2{512.0F, 26.0F},
        });
        pChild1L1->pChildren[1] = std::make_shared<HeightMapQuadTreeNode>(HeightMapQuadTreeNode{
            .lod = 0U,
            .tilePos = glm::u32vec2{1U, 2U},
            .worldPos = glm::vec2{512.0F, 1024.0F},
            .worldSize = glm::vec2{8.0F, 26.0F},
        });
        pChild1L1->pChildren[2] = nullptr;
        pChild1L1->pChildren[3] = nullptr;
    }

    StrictMock<MockHeightMap> heightMap;
    EXPECT_CALL(heightMap, getSize()).WillRepeatedly(Return(TestSize));
    EXPECT_CALL(heightMap, getTileSize()).WillRepeatedly(Return(TestTileSize));
    EXPECT_CALL(heightMap, getLodCount()).WillRepeatedly(Return(TestLodCount));

    const auto pTreeRoot = generateHeightMapQuadTree(heightMap);
    ASSERT_THAT(pTreeRoot, NotNull());
    expectTreesEqual(*pTreeRoot, *pExpectedRoot);
}