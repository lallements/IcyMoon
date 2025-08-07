#include "anari_device.h"

#include "anari_frame_pipeline.h"
#include "anari_world.h"

#include <im3e/utils/core/throw_utils.h>
#include <im3e/utils/loggers.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <array>
#include <string_view>

using namespace im3e;
using namespace std;

namespace {

constexpr array<string_view, 2U> AnImplementationNames{"visrtx", "helide"};

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
    string anLibName{};
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
    throwIfNull<runtime_error>(anLib, "Could not find ANARI implementation to load");

    return UniquePtrWithDeleter<anari::api::Library>(anLib, [anLibName, pLogger = &rLogger](auto* anariLib) {
        anariUnloadLibrary(anariLib);
        pLogger->info(fmt::format("Unloaded ANARI implementation \"{}\"", anLibName));
    });
}

auto chooseAnDeviceSubtype(const ILogger& rLogger, ANARILibrary anLib)
{
    auto pSubtypes = anariGetDeviceSubtypes(anLib);
    throwIfNull<runtime_error>(pSubtypes, "Failed to get device subtypes");

    vector<string> deviceSubtypes;
    for (const char** pSubtype = pSubtypes; *pSubtype != nullptr; pSubtype++)
    {
        deviceSubtypes.emplace_back(*pSubtype);
    }
    throwIfFalse<runtime_error>(!deviceSubtypes.empty(), "Failed to find ANARI device subtypes");
    rLogger.debug(fmt::format("Found the following device subtypes: {}", deviceSubtypes));
    rLogger.info(fmt::format("Selecting the first device subtype found: \"{}\"", deviceSubtypes.front()));
    return deviceSubtypes.front();
}

auto detectAnDeviceExtensions(const ILogger& rLogger, ANARILibrary pLib, string_view deviceSubtype)
{
    ANARIExtensions extensions{};
    if (anariGetDeviceExtensionStruct(&extensions, pLib, deviceSubtype.data()))
    {
        throw runtime_error("Failed to get ANARI device extensions");
    }

    auto checkExt = [&](int supported, string_view name) {
        rLogger.verbose(fmt::format("\t- {}: {}", name, supported ? "supported" : "not supported"));
    };
    checkExt(extensions.ANARI_KHR_CAMERA_PERSPECTIVE, "ANARI_KHR_CAMERA_PERSPECTIVE");
    checkExt(extensions.ANARI_KHR_GEOMETRY_TRIANGLE, "ANARI_KHR_GEOMETRY_TRIANGLE");
    checkExt(extensions.ANARI_KHR_LIGHT_DIRECTIONAL, "ANARI_KHR_LIGHT_DIRECTIONAL");
    checkExt(extensions.ANARI_KHR_LIGHT_POINT, "ANARI_KHR_LIGHT_POINT");
    checkExt(extensions.ANARI_KHR_MATERIAL_MATTE, "ANARI_KHR_MATERIAL_MATTE");
    checkExt(extensions.ANARI_KHR_MATERIAL_PHYSICALLY_BASED, "ANARI_KHR_MATERIAL_PHYSICALLY_BASED");

    return extensions;
}

auto createAnDevice(const ILogger& rLogger, ANARILibrary anLib, string_view deviceSubtype)
{
    auto anDevice = anariNewDevice(anLib, deviceSubtype.data());
    throwIfNull<runtime_error>(anDevice, fmt::format("Failed to create device with subtype \"{}\"", deviceSubtype));
    rLogger.info(fmt::format("Created device with subtype \"{}\"", deviceSubtype));

    anariCommitParameters(anDevice, anDevice);

    return UniquePtrWithDeleter<anari::api::Device>(anDevice, [pLogger = &rLogger, deviceSubtype](auto* anDevice) {
        anariRelease(anDevice, anDevice);
        pLogger->info(fmt::format("Destroyed device with subtype \"{}\"", deviceSubtype));
    });
}

auto chooseAnRendererSubtype(const ILogger& rLogger, ANARIDevice pDevice)
{
    const auto** pSubtypes = anariGetObjectSubtypes(pDevice, ANARI_RENDERER);
    throwIfNull<runtime_error>(pSubtypes, "Failed to retrieve renderer subtypes");

    std::string subtypesMsg = "[";
    for (const auto** pSubtype = pSubtypes; *pSubtype != nullptr; pSubtype++)
    {
        if (pSubtype != pSubtypes)
        {
            subtypesMsg += ", ";
        }
        subtypesMsg += fmt::format("{}", *pSubtype);
    }
    subtypesMsg += "]";

    const std::string chosenSubtype = *pSubtypes;
    rLogger.info(fmt::format("Available renderer subtypes: {}. Choosing: {}", subtypesMsg, chosenSubtype));
    return chosenSubtype;
}

auto createAnRenderer(const ILogger& rLogger, ANARIDevice anDevice, std::string_view anSubtype)
{
    auto anRenderer = anariNewRenderer(anDevice, anSubtype.data());
    throwIfNull<std::runtime_error>(anRenderer, fmt::format("Failed to create renderer with subtype {}", anSubtype));

    constexpr std::array<float, 4U> BackgroundColor{0.3F, 0.3F, 0.4F, 1.0F};
    anariSetParameter(anDevice, anRenderer, "background", ANARI_FLOAT32_VEC4, BackgroundColor.data());

    anariCommitParameters(anDevice, anRenderer);
    rLogger.debug(fmt::format("Created renderer with subtype {}", anSubtype));

    return std::shared_ptr<anari::api::Renderer>(
        anRenderer, [anDevice, pLogger = &rLogger, anSubtype](auto* anRenderer) {
            anariRelease(anDevice, anRenderer);
            pLogger->debug(fmt::format("Destroyed renderer with subtype {}", anSubtype));
        });
}

template <typename T>
auto getRenderParamInfo(ANARIDevice anDevice, string_view rendererSubtype, const ANARIParameter* pAnParam,
                        string_view attribute, ANARIDataType attributeType)
{
    return static_cast<const T*>(anariGetParameterInfo(anDevice, ANARI_RENDERER, rendererSubtype.data(), pAnParam->name,
                                                       pAnParam->type, attribute.data(), attributeType));
}

template <typename T>
auto createPropertyValue(ANARIDevice anDevice, ANARIRenderer anRenderer, string_view rendererSubtype,
                         const ANARIParameter* pAnParam)
{
    const auto* description = getRenderParamInfo<char>(anDevice, rendererSubtype, pAnParam, "description",
                                                       ANARI_STRING);
    const auto* defaultValue = getRenderParamInfo<T>(anDevice, rendererSubtype, pAnParam, "default", pAnParam->type);

    PropertyValueConfig<T> config{
        .name = pAnParam->name,
        .description = description,
        .onChange =
            [anDevice, anRenderer, pAnParam](T newValue) {
                anariSetParameter(anDevice, anRenderer, pAnParam->name, pAnParam->type, &newValue);
                anariCommitParameters(anDevice, anRenderer);
            },
    };
    if (defaultValue)
    {
        config.defaultValue = *defaultValue;
    }
    return make_shared<PropertyValue<T>>(move(config));
}

auto createPropertyValueFromAnParameter(ANARIDevice anDevice, ANARIRenderer anRenderer, string_view rendererSubtype,
                                        const ANARIParameter* pAnParam) -> shared_ptr<IPropertyValue>
{
    switch (static_cast<int>(pAnParam->type))
    {
        case ANARI_BOOL: return createPropertyValue<bool>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_INT32: return createPropertyValue<int32_t>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_FLOAT32: return createPropertyValue<float>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_STRING: return createPropertyValue<string>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_FLOAT32_VEC3: return createPropertyValue<glm::vec3>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_FLOAT32_VEC4: return createPropertyValue<glm::vec4>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_ARRAY2D: return createPropertyValue<void*>(anDevice, anRenderer, rendererSubtype, pAnParam);
        case ANARI_UNKNOWN: throw runtime_error(fmt::format("Unsupported UNKNOWN ANARI data type"));
        default: break;
    }
    throw runtime_error(fmt::format("Unsupported ANARI data type: {}", static_cast<int>(pAnParam->type)));
}

}  // namespace

AnariDevice::AnariDevice(const ILogger& rLogger)
  : m_pLogger(rLogger.createChild("ANARI Device"))

  , m_pAnLib(loadAnLibrary(*m_pLogger))

  , m_anDeviceSubtype(chooseAnDeviceSubtype(*m_pLogger, m_pAnLib.get()))
  , m_anExtensions(detectAnDeviceExtensions(*m_pLogger, m_pAnLib.get(), m_anDeviceSubtype))
  , m_pAnDevice(createAnDevice(*m_pLogger, m_pAnLib.get(), m_anDeviceSubtype))

  , m_anRendererSubtype(chooseAnRendererSubtype(*m_pLogger, m_pAnDevice.get()))
  , m_pAnRenderer(createAnRenderer(*m_pLogger, m_pAnDevice.get(), m_anRendererSubtype))
{
}

auto AnariDevice::createWorld() const -> std::shared_ptr<IAnariWorld>
{
    return make_shared<AnariWorld>(*m_pLogger, m_pAnDevice.get());
}

auto AnariDevice::createFramePipeline(std::shared_ptr<IDevice> pDevice, std::shared_ptr<IAnariWorld> pAnWorld)
    -> std::unique_ptr<IAnariFramePipeline>
{
    return make_unique<AnariFramePipeline>(*m_pLogger, std::move(pDevice), m_pAnDevice.get(), m_pAnRenderer.get(),
                                           std::move(pAnWorld));
}

auto AnariDevice::createRendererProperties() -> std::shared_ptr<IPropertyGroup>
{
    const auto* pAnParams = static_cast<const ANARIParameter*>(anariGetObjectInfo(
        m_pAnDevice.get(), ANARI_RENDERER, m_anRendererSubtype.data(), "parameter", ANARI_PARAMETER_LIST));

    vector<shared_ptr<IProperty>> pProperties;
    for (const auto* pAnParam = pAnParams; pAnParam->name != nullptr; pAnParam++)
    {
        pProperties.emplace_back(
            createPropertyValueFromAnParameter(m_pAnDevice.get(), m_pAnRenderer.get(), m_anRendererSubtype, pAnParam));
    }
    return createPropertyGroup(fmt::format("Renderer: \"{}\"", m_anRendererSubtype), pProperties);
}

auto im3e::createAnariDevice(const ILogger& rLogger) -> shared_ptr<IAnariDevice>
{
    return make_shared<AnariDevice>(rLogger);
}