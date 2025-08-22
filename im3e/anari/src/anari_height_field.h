#pragma once

#include "anari.h"
#include "anari_device.h"

#include <im3e/api/height_map.h>
#include <im3e/utils/types.h>

namespace im3e {

class AnariHeightField : public IAnariObject
{
public:
    AnariHeightField(std::shared_ptr<AnariDevice> pAnDevice, std::unique_ptr<IHeightMap> pHeightMap);

    void commitChanges();

    auto getProperties() -> std::shared_ptr<IPropertyGroup> override { return m_pProperties; }
    auto getInstance() const -> ANARIInstance { return m_pAnInstance.get(); }

private:
    std::shared_ptr<AnariDevice> m_pAnDevice;
    std::unique_ptr<IHeightMap> m_pHeightMap;

    std::unique_ptr<ILogger> m_pLogger;

    UniquePtrWithDeleter<anari::api::Group> m_pAnGroup;
    UniquePtrWithDeleter<anari::api::Instance> m_pAnInstance;

    std::shared_ptr<IPropertyGroup> m_pProperties;
};

}  // namespace im3e