#include "anari_instance_set.h"

#include <im3e/utils/core/throw_utils.h>

#include <utility>

using namespace im3e;

namespace {

constexpr auto InstanceParam = "instance";

}  // namespace

AnariInstanceSet::AnariInstanceSet(std::shared_ptr<AnariDevice> pAnDevice, ANARIWorld anWorld)
  : m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "ANARI Instance Set requires a device"))
  , m_anWorld(throwIfArgNull(anWorld, "ANARI Instance Set requires a world"))
{
    anariUnsetParameter(m_pAnDevice->getHandle(), m_anWorld, InstanceParam);
}

void AnariInstanceSet::insert(std::shared_ptr<anari::api::Instance> pAnInstance)
{
    throwIfArgNull(pAnInstance.get(), "Cannot insert null to ANARI Instance Set");

    std::lock_guard lock(m_mutex);
    m_changed |= m_pAnInstances.emplace(std::move(pAnInstance)).second;
}

void AnariInstanceSet::remove(std::shared_ptr<anari::api::Instance> pAnInstance)
{
    std::lock_guard lock(m_mutex);
    m_changed |= !!m_pAnInstances.erase(pAnInstance);
}

void AnariInstanceSet::updateWorld()
{
    std::lock_guard lock(m_mutex);
    if (!m_changed)
    {
        return;
    }
    m_changed = false;

    if (m_pAnInstances.empty())
    {
        anariUnsetParameter(m_pAnDevice->getHandle(), m_anWorld, InstanceParam);
    }
    else
    {
        std::vector<ANARIInstance> anInstances;
        anInstances.reserve(m_pAnInstances.size());
        for (auto& rpAnInstance : m_pAnInstances)
        {
            anInstances.emplace_back(rpAnInstance.get());
        }

        auto pAnArray = m_pAnDevice->createArray1d(anInstances, ANARI_INSTANCE);
        auto anArray = pAnArray.get();
        anariSetParameter(m_pAnDevice->getHandle(), m_anWorld, InstanceParam, ANARI_ARRAY1D, &anArray);
    }
}