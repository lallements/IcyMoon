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

    void insert(std::shared_ptr<anari::api::Instance> pInstance);
    void remove(std::shared_ptr<anari::api::Instance> pInstance);

    void updateWorld();

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    ANARIWorld m_anWorld;

    mutable std::mutex m_mutex;
    std::unordered_set<std::shared_ptr<anari::api::Instance>> m_pAnInstances;
    bool m_changed{};
};

}  // namespace im3e