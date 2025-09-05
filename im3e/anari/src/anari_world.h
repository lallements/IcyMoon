#pragma once

#include "anari_device.h"
#include "anari_height_field.h"
#include "anari_instance_set.h"
#include "anari_map_camera.h"
#include "anari_plane.h"

#include <im3e/utils/core/types.h>
#include <im3e/utils/loggers.h>

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

    /// @brief Initiative asynchronous update of the world with the current camera state.
    /// If called while previous updates were not complete, the latter will be discarded to focus on the new camera
    /// state.
    void updateAsync(const AnariMapCamera& rCamera);

    /// @brief Commit changes to world before rendering a new frame.
    /// Any changes made to the world is not effective until the changes have been committed with this function.
    /// This should not be called while a frame is still rendering so avoid possible race conditions.
    /// This function does NOT wait for any uncomplete async update. Instead, it commits completed changes so far and
    /// allow uncomplete async updates to continue for the next call to commitChanges.
    void commitChanges();

    auto getHandle() const -> ANARIWorld { return m_pAnWorld.get(); }

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    std::unique_ptr<ILogger> m_pLogger;
    UniquePtrWithDeleter<anari::api::World> m_pAnWorld;
    UniquePtrWithDeleter<anari::api::Light> m_pAnLight;

    std::vector<std::shared_ptr<AnariPlane>> m_pPlanes;
    std::vector<std::shared_ptr<AnariHeightField>> m_pHeightFields;

    AnariInstanceSet m_instanceSet;
};

}  // namespace im3e