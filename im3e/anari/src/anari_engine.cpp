#include "anari.h"

#include "anari_device.h"
#include "anari_frame_pipeline.h"

#include <im3e/utils/core/throw_utils.h>

#include <memory>

using namespace im3e;

namespace {

constexpr std::array<std::string_view, 2U> AnImplementationNames{"visrtx", "helide"};

void anStatusFct(const void* pUserData, [[maybe_unused]] ANARIDevice anDevice, [[maybe_unused]] ANARIObject anObject,
                 [[maybe_unused]] ANARIDataType anSourceType, ANARIStatusSeverity anStatusSeverity,
                 ANARIStatusCode anStatusCode, const char* pMessage)
{
    auto pLogger = static_cast<const ILogger*>(pUserData);
    const auto message = fmt::format("{}: {}", anStatusCode, pMessage);
    switch (anStatusSeverity)
    {
        case ANARI_SEVERITY_FATAL_ERROR: pLogger->error(fmt::format("[FATAL] {}", message)); break;
        case ANARI_SEVERITY_ERROR: pLogger->error(message); break;
        case ANARI_SEVERITY_WARNING: pLogger->warning(message); break;
        case ANARI_SEVERITY_PERFORMANCE_WARNING: pLogger->verbose(fmt::format("[PERFORMANCE] {}", message)); break;
        case ANARI_SEVERITY_INFO: pLogger->info(message); break;
        case ANARI_SEVERITY_DEBUG:  // Debug messages are too verbose and not that useful for development
            break;
        default: pLogger->verbose(fmt::format("[UNKNOWN] {}", message)); break;
    }
}

auto createAnLibrary(const ILogger& rLogger, std::string_view anLibName) -> UniquePtrWithDeleter<anari::api::Library>
{
    rLogger.debug(fmt::format("Loading ANARI implementation \"{}\"", anLibName));
    auto anLib = anariLoadLibrary(anLibName.data(), anStatusFct, &rLogger);
    if (!anLib)
    {
        rLogger.debug(fmt::format("Failed to load ANARI implementation \"{}\"", anLibName));
        return nullptr;
    }
    rLogger.info(fmt::format("Successfully loaded ANARI implementation \"{}\"", anLibName));
    return UniquePtrWithDeleter<anari::api::Library>(
        anLib, [anLibNameStr = std::string(anLibName), &rLogger](auto* anariLib) {
            anariUnloadLibrary(anariLib);
            rLogger.info(fmt::format("Unloaded ANARI implementation \"{}\"", anLibNameStr));
        });
}

auto loadAnLibrary(const ILogger& rLogger, std::string& rAnLibName)
{
    for (auto& rLibName : AnImplementationNames)
    {
        if (auto pAnLib = createAnLibrary(rLogger, rLibName))
        {
            rAnLibName = rLibName;
            return pAnLib;
        }
    }
    throw std::runtime_error{"Could not find ANARI implementation to load"};
}

class AnariEngine : public IAnariEngine
{
public:
    AnariEngine(const ILogger& rLogger, std::shared_ptr<IDevice> pDevice, bool debugEnabled)
      : m_pLogger(rLogger.createChild("ANARI"))
      , m_pDevice(throwIfArgNull(std::move(pDevice), "ANARI Engine requires a Device"))
      , m_pAnLib(loadAnLibrary(*m_pLogger, m_anLibName))
      , m_pAnDebugLib(debugEnabled ? createAnLibrary(*m_pLogger, "debug") : nullptr)
      , m_pAnDevice(std::make_shared<AnariDevice>(*m_pLogger, m_pAnLib.get(), m_anLibName, m_pAnDebugLib.get()))
    {
    }

    auto createFramePipeline() -> std::unique_ptr<IAnariFramePipeline> override
    {
        return std::make_unique<AnariFramePipeline>(m_pDevice, m_pAnDevice);
    }

private:
    std::unique_ptr<ILogger> m_pLogger;
    std::shared_ptr<IDevice> m_pDevice;

    std::string m_anLibName;
    UniquePtrWithDeleter<anari::api::Library> m_pAnLib;
    UniquePtrWithDeleter<anari::api::Library> m_pAnDebugLib;
    std::shared_ptr<AnariDevice> m_pAnDevice;
};

}  // namespace

auto im3e::createAnariEngine(const ILogger& rLogger, std::shared_ptr<IDevice> pDevice, bool debugEnabled)
    -> std::unique_ptr<IAnariEngine>
{
    return std::make_unique<AnariEngine>(rLogger, std::move(pDevice), debugEnabled);
}