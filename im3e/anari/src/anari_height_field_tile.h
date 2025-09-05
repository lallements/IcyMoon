#pragma once

#include "anari_device.h"

#include <im3e/api/height_map.h>
#include <im3e/utils/core/types.h>

#include <anari/anari.h>
#include <glm/glm.hpp>

#include <memory>

namespace im3e {

class AnariHeightFieldTile
{
public:
    AnariHeightFieldTile(std::shared_ptr<AnariDevice> pAnDevice, const glm::u32vec2& rSize);

    /// @brief Loads the given height map tile into memory.
    /// @return True if the tile was successfully loaded, False if given tile did not contain any data to load. For
    /// example, a given tile might have its data completely masked out.
    auto load(const IHeightMapTileSampler& rSampler) -> bool;

    void commitChanges();

    auto getInstance() const -> ANARIInstance { return m_pAnInstance.get(); }

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    const glm::u32vec2 m_tileSize;

    bool m_geometryChanged{};

    UniquePtrWithDeleter<anari::api::Geometry> m_pAnGeometry;
    UniquePtrWithDeleter<anari::api::Material> m_pAnMaterial;
    UniquePtrWithDeleter<anari::api::Surface> m_pAnSurface;

    UniquePtrWithDeleter<anari::api::Group> m_pAnGroup;
    UniquePtrWithDeleter<anari::api::Instance> m_pAnInstance;

    std::vector<glm::u32vec3> m_tmpIndices;
};

}  // namespace im3e