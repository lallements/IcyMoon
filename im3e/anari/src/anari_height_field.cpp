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

  , m_pLodProp(std::make_shared<PropertyValue<uint32_t>>(PropertyValueConfig<uint32_t>{
        .name = "Level of Details",
        .description = "Determines level of details of the height field where 0 is highest details.",
        .defaultValue = 0U,
        .onChange = [this](auto) { m_lodChanged = true; },
    }))
  , m_pProperties(createPropertyGroup(m_pHeightMap->getName(), {m_pLodProp}))

  , m_pTiles(initializeTiles(m_pAnDevice, m_pHeightMap->getTileSize(), 2U))
{
    const auto lodLevel = m_pLodProp->getValue();
    const auto tileCount = m_pHeightMap->getTileCount(lodLevel);
    const auto totalTileCount = tileCount.x * tileCount.y;
    uint32_t iteration{};
    for (auto& rpTile : m_pTiles)
    {
        rpTile->load(*m_pHeightMap->getTileSampler({iteration % tileCount.x, iteration / tileCount.x}, lodLevel));
        rpTile->commitChanges();
        m_rInstanceSet.insert(rpTile->getInstance());

        if (++iteration >= totalTileCount)
        {
            break;
        }
    }
}

void AnariHeightField::updateAsync([[maybe_unused]] const AnariMapCamera& rCamera)
{
    if (m_lodChanged)
    {
        // Must remove instances first as we may not add back all of the tiles:
        for (auto& rpTile : m_pTiles)
        {
            m_rInstanceSet.remove(rpTile->getInstance());
        }

        const auto lodLevel = m_pLodProp->getValue();
        const auto tileCount = m_pHeightMap->getTileCount(lodLevel);
        const auto totalTileCount = tileCount.x * tileCount.y;
        uint32_t iteration{};
        for (auto& rpTile : m_pTiles)
        {
            rpTile->load(*m_pHeightMap->getTileSampler({iteration % tileCount.x, iteration / tileCount.y}, lodLevel));
            m_rInstanceSet.insert(rpTile->getInstance());
            if (++iteration >= totalTileCount)
            {
                break;
            }
        }

        m_lodChanged = false;
    }
}

void AnariHeightField::commitChanges()
{
    std::ranges::for_each(m_pTiles, [](auto& rpTile) { rpTile->commitChanges(); });
}
