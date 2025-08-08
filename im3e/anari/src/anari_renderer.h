#pragma once

#include "anari.h"

#include <im3e/utils/loggers.h>
#include <im3e/utils/properties/properties.h>
#include <im3e/utils/types.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class AnariRenderer
{
public:
    AnariRenderer(const ILogger& rLogger, ANARIDevice anDevice);

    void commitChanges();
    auto createRendererProperties() -> std::shared_ptr<IPropertyGroup>;

    auto getHandle() const -> ANARIRenderer { return m_pAnRenderer.get(); }

private:
    std::unique_ptr<ILogger> m_pLogger;
    const ANARIDevice m_anDevice;

    const std::string m_anRendererSubtype;
    std::shared_ptr<anari::api::Renderer> m_pAnRenderer;
};

}  // namespace im3e