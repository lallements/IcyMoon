#include "anari_dem_surface_generator.h"
#include "anari_frame_pipeline.h"

#include <im3e/devices/devices.h>
#include <im3e/guis/guis.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/throw_utils.h>

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

constexpr auto AnariImplementationName = "helide";

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
    const auto libName = AnariImplementationName;

    auto anariLib = anariLoadLibrary(libName, statusFct, &rLogger);
    throwIfNull<runtime_error>(anariLib, fmt::format("Failed to load Anari lib: \"{}\"", libName));
    rLogger.info(fmt::format("Loaded Anari lib \"{}\"", libName));
    return shared_ptr<anari::api::Library>(anariLib, [libName, pLogger = &rLogger](auto* anariLib) {
        anariUnloadLibrary(anariLib);
        pLogger->info(fmt::format("Unloaded Anari lib \"{}\"", libName));
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

auto createLight(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anLight = anariNewLight(anDevice, "directional");
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
        auto pSurface = AnariDemSurfaceGenerator::generate(rLogger, anDevice);
        auto anSurface = pSurface.get();

        auto anArray = anariNewArray1D(anDevice, &anSurface, nullptr, nullptr, ANARI_SURFACE, 1U);
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

    auto pFramePipeline = make_unique<AnariFramePipeline>(*pLogger, pDevice, pAnDevice, pAnRenderer, pAnWorld);
    auto pRenderPanel = createImguiRenderPanel("Renderer", std::move(pFramePipeline));

    auto pGuiWorkspace = createImguiWorkspace("ANARI");
    pGuiWorkspace->addPanel(IGuiWorkspace::Location::Center, pRenderPanel);
    pApp->createWindow(WindowConfig{}, pGuiWorkspace);

    pApp->run();
    return 0;
}