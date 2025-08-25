#include "anari_height_field_tile.h"

#include <im3e/utils/core/throw_utils.h>

using namespace im3e;

namespace {

auto createTileMaterial(AnariDevice& rAnDevice)
{
    auto pAnMaterial = rAnDevice.createMaterial(AnariMaterialType::Matte);

    const glm::vec3 color{0.8F, 0.2F, 0.2F};
    anariSetParameter(rAnDevice.getHandle(), pAnMaterial.get(), "color", ANARI_FLOAT32_VEC3, &color);

    anariCommitParameters(rAnDevice.getHandle(), pAnMaterial.get());
    return pAnMaterial;
}

auto createTileSurface(AnariDevice& rAnDevice, ANARIGeometry anGeometry, ANARIMaterial anMaterial)
{
    auto pAnSurface = rAnDevice.createSurface();

    anariSetParameter(rAnDevice.getHandle(), pAnSurface.get(), "geometry", ANARI_GEOMETRY, &anGeometry);
    anariSetParameter(rAnDevice.getHandle(), pAnSurface.get(), "material", ANARI_MATERIAL, &anMaterial);

    anariCommitParameters(rAnDevice.getHandle(), pAnSurface.get());
    return pAnSurface;
}

}  // namespace

AnariHeightFieldTile::AnariHeightFieldTile(std::shared_ptr<AnariDevice> pAnDevice, const glm::u32vec2& rTileSize)
  : m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "ANARI Height Field Tile requires a device"))
  , m_tileSize(rTileSize)

  , m_pAnGeometry(m_pAnDevice->createGeometry(AnariPrimitiveType::Triangle))
  , m_pAnMaterial(createTileMaterial(*m_pAnDevice))
  , m_pAnSurface(createTileSurface(*m_pAnDevice, m_pAnGeometry.get(), m_pAnMaterial.get()))

  , m_pAnGroup(m_pAnDevice->createGroup({m_pAnSurface.get()}))
  , m_pAnInstance(m_pAnDevice->createInstance(m_pAnGroup.get()))
{
}