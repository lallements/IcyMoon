#pragma once

#include "anari_device.h"

#include <anari/anari.h>

#include <memory>
#include <unordered_set>

namespace im3e {

class AnariInstanceSet
{
public:
    AnariInstanceSet(std::shared_ptr<AnariDevice> pAnDevice, ANARIWorld anWorld);

    void insert(ANARIInstance anInstance);
    void remove(ANARIInstance anInstance);

    void updateWorld();

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    ANARIWorld m_anWorld;

    mutable std::mutex m_mutex;
    std::unordered_set<ANARIInstance> m_anInstances;
    bool m_changed{};
};

}  // namespace im3e