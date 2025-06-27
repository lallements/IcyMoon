#include "anari_camera_imgui_listener.h"

#include <im3e/utils/throw_utils.h>

#include <fmt/format.h>

using namespace im3e;
using namespace std;

namespace {

constexpr float ScrollSensitivity = 0.1F;

}  // namespace

AnariCameraImguiListener::AnariCameraImguiListener(const ILogger& rLogger, shared_ptr<ICamera> pCamera)
  : m_pLogger(rLogger.createChild("AnariCameraImguiListener"))
  , m_pCamera(throwIfArgNull(move(pCamera), "AnariCameraImguiListener requires a camera"))
{
}

void AnariCameraImguiListener::onMouseWheel(float scrollSteps)
{
    const auto position = m_pCamera->getPosition();
    const auto direction = m_pCamera->getDirection();
    throwIfFalse<logic_error>(direction.y, "Cannot apply scroll to camera, invalid direction with y = 0");
    const auto t = -position.y / direction.y;

    const auto newT = t * (1.0F - scrollSteps * ScrollSensitivity);
    const auto newPosition = position + (t - newT) * direction;
    m_pCamera->setPosition(newPosition);
}