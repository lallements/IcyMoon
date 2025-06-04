#include <im3e/devices/devices.h>
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
constexpr array<uint32_t, 2U> WindowSize{1920U, 1080U};

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

auto createGeometry(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anGeometry = anariNewGeometry(anDevice, "triangle");
    auto pGeometry = shared_ptr<anari::api::Geometry>(anGeometry, [anDevice, pLogger = &rLogger](auto* anGeometry) {
        anariRelease(anDevice, anGeometry);
        pLogger->debug("Released geometry");
    });

    // Vertex positions:
    {
        constexpr array<array<float, 3U>, 4U> Vertices{
            array<float, 3U>{-1.0f, -1.0f, 3.0F},
            array<float, 3U>{-1.0F, 1.0F, 3.0F},
            array<float, 3U>{1.0F, -1.0F, 3.0F},
            array<float, 3U>{0.1F, 0.1F, 0.3F},
        };

        auto anArray = anariNewArray1D(anDevice, Vertices.data(), nullptr, nullptr, ANARI_FLOAT32_VEC3,
                                       Vertices.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.position", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex colors:
    {
        constexpr array<array<float, 4U>, 4U> VertexColors{
            array<float, 4U>{0.9f, 0.5f, 0.5f, 1.0f},  // red
            array<float, 4U>{0.8f, 0.8f, 0.8f, 1.0f},  // 80% gray
            array<float, 4U>{0.8f, 0.8f, 0.8f, 1.0f},  // 80% gray
            array<float, 4U>{0.5f, 0.9f, 0.5f, 1.0f},  // green
        };
        auto anArray = anariNewArray1D(anDevice, VertexColors.data(), nullptr, nullptr, ANARI_FLOAT32_VEC4,
                                       VertexColors.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "vertex.color", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    // Vertex indices:
    {
        constexpr array<array<int32_t, 3U>, 2U> VertexIndices{
            array<int32_t, 3U>{0U, 1U, 2U},
            array<int32_t, 3U>{1U, 2U, 3U},
        };
        auto anArray = anariNewArray1D(anDevice, VertexIndices.data(), nullptr, nullptr, ANARI_UINT32_VEC3,
                                       VertexIndices.size());
        anariCommitParameters(anDevice, anArray);
        anariSetParameter(anDevice, anGeometry, "primitive.index", ANARI_ARRAY1D, &anArray);
        anariRelease(anDevice, anArray);
    }

    anariCommitParameters(anDevice, anGeometry);
    rLogger.debug("Created Geometry");
    return pGeometry;
}

auto createSurface(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto pGeometry = createGeometry(rLogger, anDevice);
    auto anGeometry = pGeometry.get();

    auto anMaterial = anariNewMaterial(anDevice, "matte");
    anariSetParameter(anDevice, anMaterial, "color", ANARI_STRING, "color");
    anariCommitParameters(anDevice, anMaterial);

    auto anSurface = anariNewSurface(anDevice);
    anariSetParameter(anDevice, anSurface, "geometry", ANARI_GEOMETRY, &anGeometry);
    anariSetParameter(anDevice, anSurface, "material", ANARI_MATERIAL, &anMaterial);
    anariCommitParameters(anDevice, anSurface);
    anariRelease(anDevice, anMaterial);

    rLogger.debug("Created surface");

    return shared_ptr<anari::api::Surface>(anSurface, [anDevice, pLogger = &rLogger](auto* anSurface) {
        anariRelease(anDevice, anSurface);
        pLogger->debug("Released surface");
    });
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
        auto pSurface = createSurface(rLogger, anDevice);
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

    array<float, 4U> backgroundColor{1.0F, 1.0F, 1.0F, 1.0F};
    anariSetParameter(anDevice, anRenderer, "background", ANARI_FLOAT32_VEC4, backgroundColor.data());

    anariCommitParameters(anDevice, anRenderer);
    rLogger.debug("Created renderer");

    return shared_ptr<anari::api::Renderer>(anRenderer, [anDevice, pLogger = &rLogger](auto* anRenderer) {
        anariRelease(anDevice, anRenderer);
        pLogger->debug("Released renderer");
    });
}

auto createCamera(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anCamera = anariNewCamera(anDevice, "perspective");

    const auto aspectRatio = static_cast<float>(WindowSize[0]) / static_cast<float>(WindowSize[1]);
    anariSetParameter(anDevice, anCamera, "aspect", ANARI_FLOAT32, &aspectRatio);

    const array<float, 3U> position{};
    anariSetParameter(anDevice, anCamera, "position", ANARI_FLOAT32_VEC3, position.data());

    const array<float, 3U> up{0.0F, 1.0F, 0.0F};
    anariSetParameter(anDevice, anCamera, "up", ANARI_FLOAT32_VEC3, up.data());

    const array<float, 3U> direction{0.1F, 0.0F, 1.0F};
    anariSetParameter(anDevice, anCamera, "direction", ANARI_FLOAT32_VEC3, direction.data());

    anariCommitParameters(anDevice, anCamera);

    return shared_ptr<anari::api::Camera>(anCamera, [anDevice, pLogger = &rLogger](auto* anCamera) {
        anariRelease(anDevice, anCamera);
        pLogger->debug("Released camera");
    });
}

auto createFrame(const ILogger& rLogger, ANARIDevice anDevice, ANARIRenderer anRenderer, ANARICamera anCamera,
                 ANARIWorld anWorld)
{
    auto anFrame = anariNewFrame(anDevice);
    auto pFrame = shared_ptr<anari::api::Frame>(anFrame, [anDevice, pLogger = &rLogger](auto* anFrame) {
        anariRelease(anDevice, anFrame);
        pLogger->debug("Released frame");
    });

    const ANARIDataType format = ANARI_UFIXED8_RGBA_SRGB;
    anariSetParameter(anDevice, anFrame, "size", ANARI_UINT32_VEC2, WindowSize.data());
    anariSetParameter(anDevice, anFrame, "channel.color", ANARI_DATA_TYPE, &format);
    anariSetParameter(anDevice, anFrame, "renderer", ANARI_RENDERER, &anRenderer);
    anariSetParameter(anDevice, anFrame, "camera", ANARI_CAMERA, &anCamera);
    anariSetParameter(anDevice, anFrame, "world", ANARI_WORLD, &anWorld);

    anariCommitParameters(anDevice, anFrame);

    rLogger.debug("Created frame");
    return pFrame;
}

void copyFrame(const ILogger& rLogger, ANARIDevice anDevice, ANARIFrame anFrame, IHostVisibleImage& dstImage)
{
    array<uint32_t, 2U> size{};
    ANARIDataType type = ANARI_UNKNOWN;
    auto* pSrcPixels = reinterpret_cast<const uint32_t*>(
        anariMapFrame(anDevice, anFrame, "channel.color", &size[0], &size[1], &type));
    {
        auto pVkImageMapping = dstImage.map();
        auto* pDstPixels = reinterpret_cast<uint32_t*>(pVkImageMapping->getData());
        for (auto row = 0U; row < size[1]; row++)
        {
            auto* pSrcRow = pSrcPixels + (size[1] - 1 - row) * size[0];
            auto* pDstRow = pDstPixels + row * size[0];
            copy(pSrcRow, pSrcRow + size[0], pDstRow);
        }
        pVkImageMapping->save("anari_output.png");
    }
    anariUnmapFrame(anDevice, anFrame, "channel.color");
    rLogger.info("Copied frame and saved to anari_output.png");
}

}  // namespace

int main()
{
    auto pLogger = createTerminalLogger();
    pLogger->setLevelFilter(LogLevel::Verbose);
    pLogger->debug("ANARI App");

    auto pLib = loadLibrary(*pLogger);
    const auto deviceSubtype = chooseDeviceSubtype(*pLogger, pLib.get());
    [[maybe_unused]] const auto deviceExtensions = detectDeviceExtensions(*pLogger, pLib.get(), deviceSubtype);
    auto pDevice = createDevice(*pLogger, pLib.get(), deviceSubtype);

    const auto rendererSubtype = chooseRendererSubtype(*pLogger, pDevice.get());
    logRendererParameters(*pLogger, pDevice.get(), rendererSubtype);

    auto pRenderer = createRenderer(*pLogger, pDevice.get(), rendererSubtype);
    auto pWorld = createWorld(*pLogger, pDevice.get());
    auto pCamera = createCamera(*pLogger, pDevice.get());
    auto pFrame = createFrame(*pLogger, pDevice.get(), pRenderer.get(), pCamera.get(), pWorld.get());

    anariRenderFrame(pDevice.get(), pFrame.get());
    anariFrameReady(pDevice.get(), pFrame.get(), ANARI_WAIT);
    pLogger->info("rendered first frame");

    auto pVkDevice = im3e::createDevice(*pLogger, DeviceConfig{});
    auto pVkImage = pVkDevice->getImageFactory()->createHostVisibleImage(ImageConfig{
        .name = "anari_output",
        .vkExtent{.width = WindowSize[0], .height = WindowSize[1]},
        .vkFormat = VK_FORMAT_R8G8B8A8_SRGB,
    });

    copyFrame(*pLogger, pDevice.get(), pFrame.get(), *pVkImage);

    return 0;
}