#pragma once

#include "anari.h"
#include "anari_device.h"

#include <im3e/utils/properties/properties.h>
#include <im3e/utils/types.h>

#include <string>

namespace im3e {

class AnariPlane : public IAnariObject
{
public:
    AnariPlane(std::string_view name, std::shared_ptr<AnariDevice> pAnDevice);

    void commitChanges();

    auto getProperties() -> std::shared_ptr<IPropertyGroup> override { return m_pProperties; }
    auto getInstance() const -> ANARIInstance { return m_pAnInstance.get(); }

private:
    const std::string m_name;
    std::shared_ptr<AnariDevice> m_pAnDevice;
    std::unique_ptr<ILogger> m_pLogger;

    UniquePtrWithDeleter<anari::api::Geometry> m_pAnGeometry;
    UniquePtrWithDeleter<anari::api::Material> m_pAnMaterial;
    UniquePtrWithDeleter<anari::api::Surface> m_pAnSurface;
    UniquePtrWithDeleter<anari::api::Group> m_pAnGroup;
    UniquePtrWithDeleter<anari::api::Instance> m_pAnInstance;

    std::shared_ptr<IPropertyGroup> m_pProperties;
};

}  // namespace im3e