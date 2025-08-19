#include "anari_device.h"

#include "anari_frame_pipeline.h"
#include "anari_world.h"

#include <im3e/utils/core/throw_utils.h>
#include <im3e/utils/loggers.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <array>
#include <string_view>

using namespace im3e;

namespace {

auto chooseAnDeviceSubtype(const ILogger& rLogger, ANARILibrary anLib)
{
    auto pSubtypes = anariGetDeviceSubtypes(anLib);
    throwIfNull<std::runtime_error>(pSubtypes, "Failed to get device subtypes");

    std::vector<std::string> deviceSubtypes;
    for (const char** pSubtype = pSubtypes; *pSubtype != nullptr; pSubtype++)
    {
        deviceSubtypes.emplace_back(*pSubtype);
    }
    throwIfFalse<std::runtime_error>(!deviceSubtypes.empty(), "Failed to find ANARI device subtypes");
    rLogger.debug(fmt::format("Found the following device subtypes: {}", deviceSubtypes));
    rLogger.info(fmt::format("Selecting the first device subtype found: \"{}\"", deviceSubtypes.front()));
    return deviceSubtypes.front();
}

auto detectAnDeviceExtensions(const ILogger& rLogger, ANARILibrary pLib, std::string_view deviceSubtype)
{
    ANARIExtensions extensions{};
    if (anariGetDeviceExtensionStruct(&extensions, pLib, deviceSubtype.data()))
    {
        throw std::runtime_error("Failed to get ANARI device extensions");
    }

    auto checkExt = [&](int supported, std::string_view name) {
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

auto createAnDevice(const ILogger& rLogger, ANARILibrary anLib, std::string_view deviceSubtype)
{
    auto anDevice = anariNewDevice(anLib, deviceSubtype.data());
    throwIfNull<std::runtime_error>(anDevice,
                                    fmt::format("Failed to create device with subtype \"{}\"", deviceSubtype));
    rLogger.info(fmt::format("Created device with subtype \"{}\"", deviceSubtype));

    anariCommitParameters(anDevice, anDevice);

    return UniquePtrWithDeleter<anari::api::Device>(anDevice, [pLogger = &rLogger, deviceSubtype](auto* anDevice) {
        anariRelease(anDevice, anDevice);
        pLogger->info(fmt::format("Destroyed device with subtype \"{}\"", deviceSubtype));
    });
}

}  // namespace

AnariDevice::AnariDevice(const ILogger& rLogger, ANARILibrary anLib)
  : m_pLogger(rLogger.createChild("ANARI Device"))
  , m_anDeviceSubtype(chooseAnDeviceSubtype(*m_pLogger, anLib))
  , m_anExtensions(detectAnDeviceExtensions(*m_pLogger, anLib, m_anDeviceSubtype))
  , m_pAnDevice(createAnDevice(*m_pLogger, anLib, m_anDeviceSubtype))
{
}

auto AnariDevice::createArray1d(const void* pData, ANARIDataType type, size_t count)
    -> UniquePtrWithDeleter<anari::api::Array1D>
{
    auto anArray = anariNewArray1D(m_pAnDevice.get(), pData, nullptr, nullptr, type, count);
    auto pAnArray = UniquePtrWithDeleter<anari::api::Array1D>(
        anArray, [anDevice = m_pAnDevice.get()](auto anArray) { anariRelease(anDevice, anArray); });

    anariCommitParameters(m_pAnDevice.get(), anArray);
    return pAnArray;
}

auto AnariDevice::createLogger(std::string_view name) -> std::unique_ptr<ILogger>
{
    return m_pLogger->createChild(name);
}