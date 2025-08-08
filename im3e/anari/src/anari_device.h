#pragma once

#include "anari.h"

#include <im3e/utils/loggers.h>
#include <im3e/utils/types.h>

#include <anari/frontend/anari_extension_utility.h>

#include <memory>
#include <string>

namespace im3e {

class AnariDevice
{
public:
    AnariDevice(const ILogger& rLogger, ANARILibrary anLib);

    auto createLogger(std::string_view name) -> std::unique_ptr<ILogger>;

    auto getHandle() const -> ANARIDevice { return m_pAnDevice.get(); }

private:
    std::unique_ptr<ILogger> m_pLogger;

    const std::string m_anDeviceSubtype;
    const ANARIExtensions m_anExtensions;
    UniquePtrWithDeleter<anari::api::Device> m_pAnDevice;
};

}  // namespace im3e