#pragma once

#include "anari_device.h"

#include <im3e/api/height_map.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>
#include <glm/glm.hpp>

#include <memory>

namespace im3e {

class AnariHeightFieldTile
{
public:
    AnariHeightFieldTile(std::shared_ptr<AnariDevice> pAnDevice, const glm::u32vec2& rSize);

    void load(const IHeightMapTileSampler& rSampler);
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