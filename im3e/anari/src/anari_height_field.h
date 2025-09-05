#pragma once

#include "anari.h"
#include "anari_device.h"
#include "anari_height_field_tile.h"
#include "anari_instance_set.h"
#include "anari_map_camera.h"

#include <im3e/api/height_map.h>
#include <im3e/utils/core/types.h>
#include <im3e/utils/properties/properties.h>

namespace im3e {

class AnariHeightField : public IAnariObject
{
public:
    AnariHeightField(std::shared_ptr<AnariDevice> pAnDevice, AnariInstanceSet& rInstanceSet,
                     std::unique_ptr<IHeightMap> pHeightMap);

    void updateAsync(const AnariMapCamera& rCamera);
    void commitChanges();

    auto getProperties() -> std::shared_ptr<IPropertyGroup> override { return m_pProperties; }

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    AnariInstanceSet& m_rInstanceSet;
    std::unique_ptr<IHeightMap> m_pHeightMap;

    std::unique_ptr<ILogger> m_pLogger;

    bool m_lodChanged{};
    std::shared_ptr<PropertyValue<uint32_t>> m_pLodProp;
    std::shared_ptr<IPropertyGroup> m_pProperties;

    std::vector<std::shared_ptr<AnariHeightFieldTile>> m_pTiles;
};

}  // namespace im3e