#include "anari_world.h"

#include <im3e/utils/core/throw_utils.h>

#include <algorithm>
#include <ranges>

using namespace im3e;

namespace {

auto createAnariWorld(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anWorld = anariNewWorld(anDevice);
    rLogger.debug("Created new world");
    return UniquePtrWithDeleter<anari::api::World>(anWorld, [anDevice, pLogger = &rLogger](auto* anWorld) {
        anariRelease(anDevice, anWorld);
        pLogger->debug("Destroyed world");
    });
}

auto createAnariLight(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anLight = anariNewLight(anDevice, "directional");
    auto pAnLight = UniquePtrWithDeleter<anari::api::Light>(anLight, [anDevice, pLogger = &rLogger](auto* anLight) {
        anariRelease(anDevice, anLight);
        pLogger->debug("Released light");
    });

    glm::vec3 lightDirection{0.0F, -1.0F, 0.0F};
    anariSetParameter(anDevice, anLight, "direction", ANARI_FLOAT32_VEC3, &lightDirection);
    anariCommitParameters(anDevice, anLight);

    rLogger.debug("Created light");
    return pAnLight;
}

}  // namespace

AnariWorld::AnariWorld(std::shared_ptr<AnariDevice> pAnDevice)
  : m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "Cannot create ANARI World without an ANARI Device"))
  , m_pLogger(m_pAnDevice->createLogger("ANARI World"))
  , m_pAnWorld(createAnariWorld(*m_pLogger, m_pAnDevice->getHandle()))
  , m_pAnLight(createAnariLight(*m_pLogger, m_pAnDevice->getHandle()))
  , m_instanceSet(m_pAnDevice, m_pAnWorld.get())
{
    // Initialize world lights:
    {
        std::vector<ANARILight> lights{m_pAnLight.get()};
        auto pAnLights = m_pAnDevice->createArray1d(lights, ANARI_LIGHT);
        auto anLights = pAnLights.get();
        anariSetParameter(m_pAnDevice->getHandle(), m_pAnWorld.get(), "light", ANARI_ARRAY1D, &anLights);
    }

    anariCommitParameters(m_pAnDevice->getHandle(), m_pAnWorld.get());
}

auto AnariWorld::addPlane(std::string_view name) -> std::shared_ptr<IAnariObject>
{
    auto pPlane = std::make_shared<AnariPlane>(name, m_pAnDevice, m_instanceSet);
    m_pPlanes.emplace_back(pPlane);
    return pPlane;
}

auto AnariWorld::addHeightField(std::unique_ptr<IHeightMap> pHeightMap) -> std::shared_ptr<IAnariObject>
{
    auto pHeightField = std::make_shared<AnariHeightField>(m_pAnDevice, m_instanceSet, std::move(pHeightMap));
    m_pHeightFields.emplace_back(pHeightField);
    return pHeightField;
}

void AnariWorld::updateAsync(const AnariMapCamera& rCamera)
{
    std::ranges::for_each(m_pHeightFields, [&rCamera](auto& pHeightField) { pHeightField->updateAsync(rCamera); });
}

void AnariWorld::commitChanges()
{
    std::ranges::for_each(m_pPlanes, [](auto& pPlane) { pPlane->commitChanges(); });
    std::ranges::for_each(m_pHeightFields, [](auto& pHeightField) { pHeightField->commitChanges(); });

    m_instanceSet.updateWorld();
    anariCommitParameters(m_pAnDevice->getHandle(), m_pAnWorld.get());
}