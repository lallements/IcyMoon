#pragma once

#include "anari.h"
#include "anari_renderer.h"
#include "anari_world.h"

#include <im3e/utils/loggers.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>
#include <anari/frontend/anari_extension_utility.h>

#include <memory>
#include <set>
#include <string>

namespace im3e {

class AnariDevice : public IAnariDevice
{
public:
    AnariDevice(const ILogger& rLogger);

    auto createWorld() -> std::shared_ptr<IAnariWorld> override;
    auto createFramePipeline(std::shared_ptr<IDevice> pDevice, std::shared_ptr<IAnariWorld> pAnWorld)
        -> std::unique_ptr<IAnariFramePipeline> override;
    auto createRendererProperties() -> std::shared_ptr<IPropertyGroup> override;

private:
    std::unique_ptr<ILogger> m_pLogger;

    UniquePtrWithDeleter<anari::api::Library> m_pAnLib;

    const std::string m_anDeviceSubtype;
    const ANARIExtensions m_anExtensions;
    UniquePtrWithDeleter<anari::api::Device> m_pAnDevice;

    std::set<std::shared_ptr<AnariWorld>> m_pAnWorlds;

    std::shared_ptr<AnariRenderer> m_pAnRenderer;
};

}  // namespace im3e