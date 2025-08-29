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

  , m_pProperties(createPropertyGroup(m_pHeightMap->getName(), {}))

  , m_pTiles(initializeTiles(m_pAnDevice, m_pHeightMap->getTileSize(), 4U))
{
    const auto tileCounts = m_pHeightMap->getTileCounts();
    uint32_t iteration{};
    for (auto& rpTile : m_pTiles)
    {
        rpTile->load(*m_pHeightMap->getTileSampler({iteration % tileCounts.x, iteration / tileCounts.y}, 0U));
        m_rInstanceSet.insert(rpTile->getInstance());
        iteration++;
    }
}

void AnariHeightField::updateAsync([[maybe_unused]] const AnariMapCamera& rCamera) {}

void AnariHeightField::commitChanges() {}
