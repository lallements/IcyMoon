#pragma once

#include <im3e/api/camera.h>
#include <im3e/guis/guis.h>
#include <im3e/utils/loggers.h>

namespace im3e {

class AnariCameraImguiListener : public IImguiEventListener
{
public:
    AnariCameraImguiListener(const ILogger& rLogger, std::shared_ptr<ICamera> pCamera);

    void onMouseWheel(float scrollSteps) override;

private:
    std::unique_ptr<ILogger> m_pLogger;
    std::shared_ptr<ICamera> m_pCamera;
};

}  // namespace im3e