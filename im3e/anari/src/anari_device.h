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

private:
    std::unique_ptr<ILogger> m_pLogger;
    UniquePtrWithDeleter<anari::api::Library> m_pAnLib;
    const std::string m_anDeviceSubtype;
    const ANARIExtensions m_anExtensions;
    UniquePtrWithDeleter<anari::api::Device> m_pAnDevice;
};

}  // namespace im3e