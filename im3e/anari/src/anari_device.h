#pragma once

#include "anari.h"

#include <im3e/utils/loggers.h>
#include <im3e/utils/types.h>

#include <anari/frontend/anari_extension_utility.h>

#include <memory>
#include <string>
#include <vector>

namespace im3e {

enum class AnariPrimitiveType : uint32_t
{
    Triangle,
};

enum class AnariMaterialType : uint32_t
{
    Matte,
    PhysicallyBased,
};

class AnariDevice
{
public:
    AnariDevice(const ILogger& rLogger, ANARILibrary anLib, std::string_view anLibName, ANARILibrary anDebugLib);

    template <typename T>
    auto createArray1d(const std::vector<T>& rData, ANARIDataType type) -> UniquePtrWithDeleter<anari::api::Array1D>
    {
        return createArray1d(rData.data(), type, rData.size());
    }
    auto createArray1d(const void* pData, ANARIDataType type, size_t count)
        -> UniquePtrWithDeleter<anari::api::Array1D>;

    auto createGroup(const std::vector<ANARISurface>& rAnSurface = {}) -> UniquePtrWithDeleter<anari::api::Group>;
    auto createInstance(ANARIGroup anGroup = nullptr) -> UniquePtrWithDeleter<anari::api::Instance>;
    auto createGeometry(AnariPrimitiveType type) -> UniquePtrWithDeleter<anari::api::Geometry>;
    auto createMaterial(AnariMaterialType type) -> UniquePtrWithDeleter<anari::api::Material>;
    auto createSurface(ANARIGeometry anGeometry, ANARIMaterial anMaterial) -> UniquePtrWithDeleter<anari::api::Surface>;

    auto createLogger(std::string_view name) -> std::unique_ptr<ILogger>;

    auto getHandle() const -> ANARIDevice { return m_pAnDevice.get(); }
    auto getLibraryName() const -> std::string_view { return m_anLibName; }

private:
    std::unique_ptr<ILogger> m_pLogger;
    const std::string m_anLibName;

    const std::string m_anDeviceSubtype;
    const ANARIExtensions m_anExtensions;
    UniquePtrWithDeleter<anari::api::Device> m_pAnDebugWrappedDevice;
    UniquePtrWithDeleter<anari::api::Device> m_pAnDevice;
};

}  // namespace im3e