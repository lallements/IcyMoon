#pragma once

#include "anari_device.h"

#include <string>

namespace im3e {

class AnariPlane
{
public:
    AnariPlane(std::string_view name, std::shared_ptr<AnariDevice> pAnDevice);

    void commitChanges();

    auto getSurface() const -> ANARISurface { return m_pAnSurface.get(); }

private:
    const std::string m_name;
    std::shared_ptr<AnariDevice> m_pAnDevice;
    std::unique_ptr<ILogger> m_pLogger;

    std::shared_ptr<anari::api::Geometry> m_pAnGeometry;
    std::shared_ptr<anari::api::Material> m_pAnMaterial;
    std::shared_ptr<anari::api::Surface> m_pAnSurface;
};

}  // namespace im3e