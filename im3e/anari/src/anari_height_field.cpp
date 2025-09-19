#include "anari_height_field.h"

#include <im3e/utils/core/throw_utils.h>

#include <fmt/format.h>

#include <algorithm>
#include <ranges>

using namespace im3e;

namespace {

auto initializeTiles(std::shared_ptr<AnariDevice> pAnDevice, const glm::u32vec2& rTileSize, size_t count)
{
    std::vector<std::shared_ptr<AnariHeightFieldTile>> pTiles(count);
    std::ranges::generate(pTiles, [&] { return std::make_shared<AnariHeightFieldTile>(pAnDevice, rTileSize); });
    return pTiles;
}

}  // namespace

AnariHeightField::AnariHeightField(std::shared_ptr<AnariDevice> pAnDevice, AnariInstanceSet& rInstanceSet,
                                   std::unique_ptr<IHeightMap> pHeightMap)
  : m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "ANARI Height Field requires a device"))
  , m_rInstanceSet(rInstanceSet)
  , m_pHeightMap(throwIfArgNull(std::move(pHeightMap), "ANARI Height Map requires a height map"))
  , m_pLogger(m_pAnDevice->createLogger(fmt::format("ANARI Height Field - {}", m_pHeightMap->getName())))
  , m_pQuadTreeRoot(generateHeightMapQuadTree(*m_pHeightMap))

  , m_pLodProp(std::make_shared<PropertyValue<uint32_t>>(PropertyValueConfig<uint32_t>{
        .name = "Level of Details",
        .description = "Determines level of details of the height field where 0 is highest details.",
        .defaultValue = 0U,
        .minValue = 0U,
        .maxValue = m_pHeightMap->getLodCount() - 1U,
        .onChange = [this](auto) { m_lodChanged = true; },
    }))
  , m_pProperties(createPropertyGroup(m_pHeightMap->getName(), {m_pLodProp}))

  , m_pTiles(initializeTiles(m_pAnDevice, m_pHeightMap->getTileSize(), 10U))
{
}

void AnariHeightField::updateAsync([[maybe_unused]] const AnariMapCamera& rCamera)
{
    if (!m_lodChanged)
    {
        return;
    }
    m_lodChanged = false;

    const auto visibleTileIds = m_pQuadTreeRoot->findVisible(rCamera, m_pLodProp->getValue());

    // Must remove instances first as we may not add back all of the tiles:
    for (auto& rpTile : m_pTiles)
    {
        m_rInstanceSet.remove(rpTile->getInstance());
    }

    uint32_t nextTileIdx{};
    for (auto& rTileId : visibleTileIds)
    {
        auto& rpTile = m_pTiles[nextTileIdx];
        if (rpTile->load(*m_pHeightMap->getTileSampler(rTileId.xy(), rTileId.z)))
        {
            m_rInstanceSet.insert(rpTile->getInstance());
            nextTileIdx++;
        }
    }
}

void AnariHeightField::commitChanges()
{
    std::ranges::for_each(m_pTiles, [](auto& rpTile) { rpTile->commitChanges(); });
}
