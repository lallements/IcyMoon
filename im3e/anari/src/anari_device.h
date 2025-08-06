#pragma once

#include <im3e/api/anari.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>
#include <anari/frontend/anari_extension_utility.h>

#include <memory>
#include <string>

namespace im3e {

class AnariDevice : public IAnariDevice
{
public:
    AnariDevice(const ILogger& rLogger);

    auto createWorld() const -> std::shared_ptr<IAnariWorld> override;
    auto createFramePipeline(std::shared_ptr<IDevice> pDevice) -> std::unique_ptr<IFramePipeline> override;

private:
    std::unique_ptr<ILogger> m_pLogger;

    UniquePtrWithDeleter<anari::api::Library> m_pAnLib;

    const std::string m_anDeviceSubtype;
    const ANARIExtensions m_anExtensions;
    UniquePtrWithDeleter<anari::api::Device> m_pAnDevice;

    const std::string m_anRendererSubtype;
    std::shared_ptr<anari::api::Renderer> m_pAnRenderer;
};

}  // namespace im3e