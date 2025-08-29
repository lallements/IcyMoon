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

void AnariInstanceSet::insert(ANARIInstance anInstance)
{
    throwIfArgNull(anInstance, "Cannot insert null to ANARI Instance Set");

    std::lock_guard lock(m_mutex);
    m_changed |= m_anInstances.emplace(anInstance).second;
}

void AnariInstanceSet::remove(ANARIInstance anInstance)
{
    std::lock_guard lock(m_mutex);
    m_changed |= !!m_anInstances.erase(anInstance);
}

void AnariInstanceSet::updateWorld()
{
    std::lock_guard lock(m_mutex);
    if (!m_changed)
    {
        return;
    }
    m_changed = false;

    if (m_anInstances.empty())
    {
        anariUnsetParameter(m_pAnDevice->getHandle(), m_anWorld, InstanceParam);
    }
    else
    {
        std::vector<ANARIInstance> anInstances;
        anInstances.reserve(m_anInstances.size());
        for (auto& rAnInstance : m_anInstances)
        {
            anInstances.emplace_back(rAnInstance);
        }

        auto pAnArray = m_pAnDevice->createArray1d(anInstances, ANARI_INSTANCE);
        auto anArray = pAnArray.get();
        anariSetParameter(m_pAnDevice->getHandle(), m_anWorld, InstanceParam, ANARI_ARRAY1D, &anArray);
    }
}