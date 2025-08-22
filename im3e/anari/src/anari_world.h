#pragma once

#include "anari_device.h"
#include "anari_height_field.h"
#include "anari_plane.h"

#include <im3e/utils/loggers.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>

#include <memory>
#include <vector>

namespace im3e {

class AnariWorld : public IAnariWorld
{
public:
    AnariWorld(std::shared_ptr<AnariDevice> pAnDevice);

    auto addPlane(std::string_view name) -> std::shared_ptr<IAnariObject> override;
    auto addHeightField(std::unique_ptr<IHeightMap> pHeightMap) -> std::shared_ptr<IAnariObject> override;
    void commitChanges();

    auto getHandle() const -> ANARIWorld { return m_pAnWorld.get(); }

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    std::unique_ptr<ILogger> m_pLogger;
    UniquePtrWithDeleter<anari::api::World> m_pAnWorld;
    UniquePtrWithDeleter<anari::api::Light> m_pAnLight;

    std::vector<std::shared_ptr<AnariPlane>> m_pPlanes;
    std::vector<std::shared_ptr<AnariHeightField>> m_pHeightFields;

    std::vector<ANARISurface> m_anSurfaces;
    bool m_surfacesChanged{};

    std::vector<ANARIInstance> m_anInstances;
    bool m_instancesChanged{};
};

}  // namespace im3e