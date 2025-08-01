#include "anari_dem_surface_generator.h"
#include "anari_frame_pipeline.h"

#include <im3e/devices/devices.h>
#include <im3e/guis/guis.h>
#include <im3e/utils/core/throw_utils.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/properties/properties.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>
#define ANARI_EXTENSION_UTILITY_IMPL
#include <anari/frontend/anari_extension_utility.h>
#include <fmt/format.h>

#include <iostream>
#include <memory>
#include <stdexcept>

using namespace im3e;
using namespace std;

namespace {

constexpr array<string_view, 2U> AnariImplementationNames{"visrtx", "helide"};

void statusFct([[maybe_unused]] const void* pUserData, [[maybe_unused]] ANARIDevice anariDevice,
               [[maybe_unused]] ANARIObject anariObject, [[maybe_unused]] ANARIDataType anariSourceType,
               [[maybe_unused]] ANARIStatusSeverity anariStatusSeverity,
               [[maybe_unused]] ANARIStatusCode anariStatusCode, [[maybe_unused]] const char* pMessage)
{
    auto pLogger = static_cast<const ILogger*>(pUserData);
    switch (anariStatusSeverity)
    {
        case ANARI_SEVERITY_FATAL_ERROR: pLogger->error(fmt::format("[ANARI][FATAL] {}", pMessage)); break;
        case ANARI_SEVERITY_ERROR: pLogger->error(fmt::format("[ANARI] {}", pMessage)); break;
        case ANARI_SEVERITY_WARNING: pLogger->warning(fmt::format("[ANARI] {}", pMessage)); break;
        case ANARI_SEVERITY_PERFORMANCE_WARNING:
            pLogger->verbose(fmt::format("[ANARI][PERFORMANCE] {}", pMessage));
            break;
        case ANARI_SEVERITY_INFO: pLogger->info(fmt::format("[ANARI] {}", pMessage)); break;
        case ANARI_SEVERITY_DEBUG: pLogger->debug(fmt::format("[ANARI] {}", pMessage)); break;
        default: pLogger->verbose(fmt::format("[ANARI][UNKNOWN] {}", pMessage)); break;
    }
}

auto loadLibrary(const ILogger& rLogger)
{
    string anLibName{};
    ANARILibrary anLib{};
    for (auto& rLibName : AnariImplementationNames)
    {
        rLogger.info(fmt::format("Loading ANARI implementation \"{}\"", rLibName));
        anLib = anariLoadLibrary(rLibName.data(), statusFct, &rLogger);
        if (anLib)
        {
            anLibName = rLibName;
            rLogger.info(fmt::format("Successfully loaded ANARI implementation \"{}\"", anLibName));
            break;
        }
        rLogger.info(fmt::format("Failed to load ANARI implementation \"{}\"", rLibName));
    }
    throwIfNull<runtime_error>(anLib, "Could not find ANARI implementation to load");

    return shared_ptr<anari::api::Library>(anLib, [anLibName, pLogger = &rLogger](auto* anariLib) {
        anariUnloadLibrary(anariLib);
        pLogger->info(fmt::format("Unloaded Anari lib \"{}\"", anLibName));
    });
}

auto chooseDeviceSubtype(const ILogger& rLogger, ANARILibrary pLib)
{
    auto pSubtypes = anariGetDeviceSubtypes(pLib);
    throwIfNull<runtime_error>(pSubtypes, "Failed to get device subtypes");

    rLogger.debug("Found the following subtypes:");
    for (const char** pSubtype = pSubtypes; *pSubtype != nullptr; pSubtype++)
    {
        rLogger.debug(fmt::format(" - {}", *pSubtype));
    }

    rLogger.info(fmt::format("Selecting the first device subtype found: \"{}\"", *pSubtypes));
    return string{*pSubtypes};
}

auto detectDeviceExtensions(const ILogger& rLogger, ANARILibrary pLib, string_view deviceSubtype)
{
    ANARIExtensions extensions{};
    if (anariGetDeviceExtensionStruct(&extensions, pLib, deviceSubtype.data()))
    {
        throw runtime_error("Failed to get ANARI device extensions");
    }

    auto verifyExt = [&](int supported, string_view name) {
        rLogger.info(fmt::format(R"(Extension "{}" is {})", name, supported ? "supported" : "not supported"));
        // throwIfFalse<runtime_error>(supported, fmt::format("{} not supported", name));
    };
    verifyExt(extensions.ANARI_KHR_CAMERA_PERSPECTIVE, "ANARI_KHR_CAMERA_PERSPECTIVE");
    verifyExt(extensions.ANARI_KHR_GEOMETRY_TRIANGLE, "ANARI_KHR_GEOMETRY_TRIANGLE");
    verifyExt(extensions.ANARI_KHR_LIGHT_DIRECTIONAL, "ANARI_KHR_LIGHT_DIRECTIONAL");
    verifyExt(extensions.ANARI_KHR_LIGHT_POINT, "ANARI_KHR_LIGHT_POINT");
    verifyExt(extensions.ANARI_KHR_MATERIAL_MATTE, "ANARI_KHR_MATERIAL_MATTE");
    verifyExt(extensions.ANARI_KHR_MATERIAL_PHYSICALLY_BASED, "ANARI_KHR_MATERIAL_PHYSICALLY_BASED");

    return extensions;
}

auto createDevice(const ILogger& rLogger, ANARILibrary pLib, string_view deviceSubtype)
{
    auto pDevice = throwIfNull<runtime_error>(
        anariNewDevice(pLib, deviceSubtype.data()),
        fmt::format("Failed to create ANARI device with subtype \"{}\"", deviceSubtype));
    rLogger.info(fmt::format("Created device with subtype \"{}\"", deviceSubtype));

    anariCommitParameters(pDevice, pDevice);

    return shared_ptr<anari::api::Device>(pDevice, [pLogger = &rLogger](auto* pDevice) {
        anariRelease(pDevice, pDevice);
        pLogger->info("Destroyed device");
    });
}

auto chooseRendererSubtype(const ILogger& rLogger, ANARIDevice pDevice)
{
    const auto** pSubtypes = anariGetObjectSubtypes(pDevice, ANARI_RENDERER);
    throwIfNull<runtime_error>(pSubtypes, "Failed to retrieve renderer subtypes");

    rLogger.info("Detected the following renderer subtypes:");
    for (const auto** pSubtype = pSubtypes; *pSubtype != nullptr; pSubtype++)
    {
        rLogger.info(fmt::format(" - {}", *pSubtype));
    }

    rLogger.info(fmt::format("Choosing first renderer subtype found: \"{}\"", *pSubtypes));
    return *pSubtypes;
}

void logRendererParameters(const ILogger& rLogger, ANARIDevice pDevice, string_view rendererSubtype)
{
    const auto* rendererParams = static_cast<const ANARIParameter*>(
        anariGetObjectInfo(pDevice, ANARI_RENDERER, rendererSubtype.data(), "parameter", ANARI_PARAMETER_LIST));
    if (!rendererParams)
    {
        rLogger.info(fmt::format("Renderer \"{}\" does not have any parameter", rendererSubtype));
        return;
    }

    rLogger.info(fmt::format("Renderer \"{}\" parameters:", rendererSubtype));
    for (const auto* pRendererParam = rendererParams; pRendererParam->name != nullptr; pRendererParam++)
    {
        const auto* description = static_cast<const char*>(
            anariGetParameterInfo(pDevice, ANARI_RENDERER, rendererSubtype.data(), pRendererParam->name,
                                  pRendererParam->type, "description", ANARI_STRING));
        const auto* pRequired = static_cast<const int*>(
            anariGetParameterInfo(pDevice, ANARI_RENDERER, rendererSubtype.data(), pRendererParam->name,
                                  pRendererParam->type, "required", ANARI_BOOL));
        rLogger.info(
            fmt::format(" - {}{} - {}", pRequired && *pRequired ? "*" : "", pRendererParam->name, description));
    }
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

auto createPropertyValueFromAnariParameter(ANARIDevice anDevice, ANARIRenderer anRenderer, string_view rendererSubtype,
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

auto createRendererProperties(const ILogger&, ANARIDevice anDevice, ANARIRenderer anRenderer,
                              string_view rendererSubtype)
{
    const auto* pAnParams = static_cast<const ANARIParameter*>(
        anariGetObjectInfo(anDevice, ANARI_RENDERER, rendererSubtype.data(), "parameter", ANARI_PARAMETER_LIST));

    vector<shared_ptr<IProperty>> pProperties;
    for (const auto* pAnParam = pAnParams; pAnParam->name != nullptr; pAnParam++)
    {
        pProperties.emplace_back(
            createPropertyValueFromAnariParameter(anDevice, anRenderer, rendererSubtype, pAnParam));
    }
    return createPropertyGroup(fmt::format("Renderer: \"{}\"", rendererSubtype), pProperties);
}

auto createGroundGeometry(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anGeometry = anariNewGeometry(anDevice, "triangle");
    auto pGeometry = shared_ptr<anari::api::Geometry>(anGeometry, [anDevice, pLogger = &rLogger](auto* anGeometry) {
        anariRelease(anDevice, anGeometry);
        pLogger->debug("Released ground geometry");
    });

    // Vertex positions:
    {
        const float halfWidth = 1000.0F;
        const vector<glm::vec3> vertices{
            glm::vec3{-halfWidth, 0.0F, halfWidth},
            glm::vec3{-halfWidth, 0.0F, -halfWidth},
            glm::vec3{halfWidth, 0.0F, halfWidth},
            glm::vec3{halfWidth, 0.0F, -halfWidth},
        };
        auto anArray = anariNewArray1D(anDevice, vertices.data(), nullptr, nullptr, ANARI_FLOAT32_VEC3,
                                       vertices.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.position", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex normals:
    {
        const vector<glm::vec3> normals{
            glm::vec3(0.0F, 1.0F, 0.0F),
            glm::vec3(0.0F, 1.0F, 0.0F),
            glm::vec3(0.0F, 1.0F, 0.0F),
            glm::vec3(0.0F, 1.0F, 0.0F),
        };
        auto anArray = anariNewArray1D(anDevice, normals.data(), nullptr, nullptr, ANARI_FLOAT32_VEC3, normals.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.normal", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex colors:
    {
        const vector<glm::vec4> colors{
            glm::vec4{1.0F, 0.0F, 0.0F, 1.0F},
            glm::vec4{0.0F, 1.0F, 0.0F, 1.0F},
            glm::vec4{0.0F, 0.0F, 1.0F, 1.0F},
            glm::vec4{1.0F, 1.0F, 1.0F, 1.0F},
        };
        auto anArray = anariNewArray1D(anDevice, colors.data(), nullptr, nullptr, ANARI_FLOAT32_VEC4, colors.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.color", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex indices
    {
        const vector<glm::u32vec3> vertexIndices{
            glm::u32vec3{0U, 1U, 2U},
            glm::u32vec3{1U, 2U, 3U},
        };
        auto anArray = anariNewArray1D(anDevice, vertexIndices.data(), nullptr, nullptr, ANARI_UINT32_VEC3,
                                       vertexIndices.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "primitive.index", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    anariCommitParameters(anDevice, anGeometry);
    rLogger.debug("Created ground geometry");
    return pGeometry;
}

auto createGroundSurface(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto pGeometry = createGroundGeometry(rLogger, anDevice);
    auto anGeometry = pGeometry.get();

    auto anMaterial = anariNewMaterial(anDevice, "matte");
    // glm::vec3 materialColor{0.06F, 0.1F, 0.13F};
    // anariSetParameter(anDevice, anMaterial, "color", ANARI_FLOAT32_VEC3, &materialColor);
    anariSetParameter(anDevice, anMaterial, "color", ANARI_STRING, "color");
    anariCommitParameters(anDevice, anMaterial);

    auto anSurface = anariNewSurface(anDevice);
    anariSetParameter(anDevice, anSurface, "geometry", ANARI_GEOMETRY, &anGeometry);
    anariSetParameter(anDevice, anSurface, "material", ANARI_MATERIAL, &anMaterial);
    anariCommitParameters(anDevice, anSurface);
    anariRelease(anDevice, anMaterial);

    rLogger.debug("Created ground surface");
    return shared_ptr<anari::api::Surface>(anSurface, [anDevice, pLogger = &rLogger](auto* anSurface) {
        anariRelease(anDevice, anSurface);
        pLogger->debug("Released ground surface");
    });
}

auto createLight(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anLight = anariNewLight(anDevice, "directional");
    glm::vec3 lightDirection{0.0F, -1.0F, 0.0F};
    anariSetParameter(anDevice, anLight, "direction", ANARI_FLOAT32_VEC3, &lightDirection);
    anariCommitParameters(anDevice, anLight);

    rLogger.debug("Created light");

    return shared_ptr<anari::api::Light>(anLight, [anDevice, pLogger = &rLogger](auto* anLight) {
        anariRelease(anDevice, anLight);
        pLogger->debug("Released light");
    });
}

auto createWorld(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anWorld = anariNewWorld(anDevice);
    rLogger.debug("Created new world");
    auto pWorld = shared_ptr<anari::api::World>(anWorld, [anDevice, pLogger = &rLogger](auto* anWorld) {
        anariRelease(anDevice, anWorld);
        pLogger->debug("Destroyed world");
    });

    // Surfaces of world:
    {
        auto pDemSurface = AnariDemSurfaceGenerator::generate(rLogger, anDevice);
        auto pGroundSurface = createGroundSurface(rLogger, anDevice);

        vector<ANARISurface> surfaces{
            pDemSurface.get(),
            pGroundSurface.get(),
        };

        auto anArray = anariNewArray1D(anDevice, surfaces.data(), nullptr, nullptr, ANARI_SURFACE, surfaces.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anWorld, "surface", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
        rLogger.debug("Added surface to world");
    }

    {
        auto pLight = createLight(rLogger, anDevice);
        auto anLight = pLight.get();
        auto anArray = anariNewArray1D(anDevice, &anLight, nullptr, nullptr, ANARI_LIGHT, 1U);
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anWorld, "light", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
        rLogger.debug("Added light to world");
    }

    anariCommitParameters(anDevice, anWorld);
    rLogger.debug("Created world");

    // Print out world bounds
    array<float, 6U> worldBounds;
    if (anariGetProperty(anDevice, anWorld, "bounds", ANARI_FLOAT32_BOX3, worldBounds.data(), sizeof(worldBounds),
                         ANARI_WAIT))
    {
        rLogger.info(fmt::format("World bounds: ({}, {}, {}), ({}, {}, {})", worldBounds[0], worldBounds[1],
                                 worldBounds[2], worldBounds[3], worldBounds[4], worldBounds[5]));
    }
    else
    {
        rLogger.warning("world bounds not returned");
    }

    return pWorld;
}

auto createRenderer(const ILogger& rLogger, ANARIDevice anDevice, string_view rendererSubtype)
{
    auto anRenderer = anariNewRenderer(anDevice, rendererSubtype.data());

    array<float, 4U> backgroundColor{0.3F, 0.3F, 0.4F, 1.0F};
    anariSetParameter(anDevice, anRenderer, "background", ANARI_FLOAT32_VEC4, backgroundColor.data());

    anariCommitParameters(anDevice, anRenderer);
    rLogger.debug("Created renderer");

    return shared_ptr<anari::api::Renderer>(anRenderer, [anDevice, pLogger = &rLogger](auto* anRenderer) {
        anariRelease(anDevice, anRenderer);
        pLogger->debug("Released renderer");
    });
}

}  // namespace

int main()
{
    auto pLogger = createTerminalLogger();
    pLogger->setLevelFilter(LogLevel::Verbose);
    pLogger->debug("ANARI App");

    auto pAnLib = loadLibrary(*pLogger);
    const auto anDeviceSubtype = chooseDeviceSubtype(*pLogger, pAnLib.get());
    [[maybe_unused]] const auto anDeviceExtensions = detectDeviceExtensions(*pLogger, pAnLib.get(), anDeviceSubtype);
    auto pAnDevice = createDevice(*pLogger, pAnLib.get(), anDeviceSubtype);

    const auto anRendererSubtype = chooseRendererSubtype(*pLogger, pAnDevice.get());
    logRendererParameters(*pLogger, pAnDevice.get(), anRendererSubtype);

    auto pAnRenderer = createRenderer(*pLogger, pAnDevice.get(), anRendererSubtype);
    auto pAnWorld = createWorld(*pLogger, pAnDevice.get());

    auto pApp = createGlfwWindowApplication(*pLogger, WindowApplicationConfig{
                                                          .name = "ANARI",
                                                          .isDebugEnabled = true,
                                                      });

    auto pDevice = pApp->getDevice();

    auto pGuiWorkspace = createImguiWorkspace("ANARI");

    // Properties
    {
        vector<shared_ptr<IProperty>> pProperties;
        pProperties.emplace_back(
            createRendererProperties(*pLogger, pAnDevice.get(), pAnRenderer.get(), anRendererSubtype));

        static constexpr PropertyValueTConfig<uint32_t> LevelOfDetails{
            .name = "Level of Details",
            .description = "Index of the current level of details of the terrain being rendered",
            .defaultValue = 0U,
        };

        auto pLodProperty = make_shared<PropertyValueT<LevelOfDetails>>();
        auto pTestParameterGroup = createPropertyGroup("Test Parameters", {pLodProperty});
        pProperties.emplace_back(pTestParameterGroup);

        auto pPropertyGroup = createPropertyGroup("Parameters", pProperties);
        auto pParametersPanel = createImguiPropertyPanel(pPropertyGroup);
        pGuiWorkspace->addPanel(IGuiWorkspace::Location::Left, pParametersPanel);
    }

    auto pFramePipeline = make_unique<AnariFramePipeline>(*pLogger, pDevice, pAnDevice, pAnRenderer, pAnWorld);
    auto pRenderPanel = createImguiRenderPanel("Renderer", move(pFramePipeline), pFramePipeline->getCameraListener());
    pGuiWorkspace->addPanel(IGuiWorkspace::Location::Center, pRenderPanel);

    auto pStatsPanel = createImguiStatsPanel("Stats", pDevice->getStatsProvider());
    pGuiWorkspace->addPanel(IGuiWorkspace::Location::Bottom, pStatsPanel);

    pApp->createWindow(WindowConfig{}, pGuiWorkspace);

    pApp->run();
    return 0;
}