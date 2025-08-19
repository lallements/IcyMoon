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
        case ANARI_SEVERITY_DEBUG: pLogger->debug(message); break;
        default: pLogger->verbose(fmt::format("[UNKNOWN] {}", message)); break;
    }
}

auto loadAnLibrary(const ILogger& rLogger)
{
    std::string anLibName{};
    ANARILibrary anLib{};
    for (auto& rLibName : AnImplementationNames)
    {
        rLogger.debug(fmt::format("Loading ANARI implementation \"{}\"", rLibName));
        anLib = anariLoadLibrary(rLibName.data(), anStatusFct, &rLogger);
        if (anLib)
        {
            anLibName = rLibName;
            rLogger.info(fmt::format("Successfully loaded ANARI implementation \"{}\"", anLibName));
            break;
        }
        rLogger.debug(fmt::format("Failed to load ANARI implementation \"{}\"", rLibName));
    }
    throwIfNull<std::runtime_error>(anLib, "Could not find ANARI implementation to load");

    return UniquePtrWithDeleter<anari::api::Library>(anLib, [anLibName, pLogger = &rLogger](auto* anariLib) {
        anariUnloadLibrary(anariLib);
        pLogger->info(fmt::format("Unloaded ANARI implementation \"{}\"", anLibName));
    });
}

class AnariEngine : public IAnariEngine
{
public:
    AnariEngine(const ILogger& rLogger, std::shared_ptr<IDevice> pDevice)
      : m_pLogger(rLogger.createChild("ANARI"))
      , m_pDevice(throwIfArgNull(std::move(pDevice), "ANARI Engine requires a Device"))
      , m_pAnLib(loadAnLibrary(*m_pLogger))
      , m_pAnDevice(std::make_shared<AnariDevice>(*m_pLogger, m_pAnLib.get()))
    {
    }

    auto createFramePipeline() -> std::unique_ptr<IAnariFramePipeline> override
    {
        return std::make_unique<AnariFramePipeline>(m_pDevice, m_pAnDevice);
    }

private:
    std::unique_ptr<ILogger> m_pLogger;
    std::shared_ptr<IDevice> m_pDevice;

    UniquePtrWithDeleter<anari::api::Library> m_pAnLib;
    std::shared_ptr<AnariDevice> m_pAnDevice;
};

}  // namespace

auto im3e::createAnariEngine(const ILogger& rLogger, std::shared_ptr<IDevice> pDevice) -> std::unique_ptr<IAnariEngine>
{
    return std::make_unique<AnariEngine>(rLogger, std::move(pDevice));
}