#include "anari_map_camera.h"

#include <im3e/utils/core/throw_utils.h>

#include <fmt/format.h>
#include <glm/gtc/quaternion.hpp>

using namespace im3e;
using namespace std;

namespace {

constexpr float ScrollSensitivity = 0.1F;

auto createCamera(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anCamera = anariNewCamera(anDevice, "perspective");
    return unique_ptr<anari::api::Camera, function<void(anari::api::Camera*)>>(
        anCamera, [anDevice, pLogger = &rLogger](auto* anCamera) {
            anariRelease(anDevice, anCamera);
            pLogger->debug("Released camera");
        });
}

}  // namespace

AnariMapCamera::AnariMapCamera(const ILogger& rLogger, ANARIDevice anDevice)
  : m_pLogger(rLogger.createChild("AnariMapCamera"))
  , m_anDevice(throwIfArgNull(anDevice, "Anari Map Camera requires a device"))
  , m_pAnCamera(createCamera(*m_pLogger, anDevice))
{
    m_view.update();  // calculate initial view state
    this->update();
}

void AnariMapCamera::update()
{
    if (!m_needsUpdate)
    {
        return;
    }
    m_needsUpdate = true;

    m_perspective.setCameraParameters(m_anDevice, m_pAnCamera.get());
    m_view.setCameraParameters(m_anDevice, m_pAnCamera.get());
    anariCommitParameters(m_anDevice, m_pAnCamera.get());
}

void AnariMapCamera::onMouseMove(const glm::vec2& rClipOffset, const array<bool, 3U>& rMouseButtonsDown)
{
    if (rMouseButtonsDown[static_cast<size_t>(MouseButton::Left)])
    {
        constexpr auto HalfPi = numbers::pi_v<float> / 2.0F;

        const auto angleXOffset = -rClipOffset.y * HalfPi;
        const auto angleYOffset = -rClipOffset.x * HalfPi / 2.0F;

        m_view.angleX = clamp(m_view.angleX + angleXOffset, 0.0F, HalfPi);
        m_view.angleY += angleYOffset;
        m_view.update();
        m_needsUpdate = true;
    }
    else if (rMouseButtonsDown[static_cast<size_t>(MouseButton::Middle)])
    {
        const auto inverseVpMatrix = glm::inverse(m_perspective.generateMatrix() * m_view.generateMatrix());
        const auto sceneVec = inverseVpMatrix * glm::vec4(rClipOffset, 0.0F, 0.0F);
        const auto sceneOffset = sceneVec.xyz() * m_view.distanceToTarget;
        m_view.targetPoint.x += sceneOffset.x;
        m_view.targetPoint.z += sceneOffset.z;
        m_view.update();
        m_needsUpdate = true;
    }
}

void AnariMapCamera::onMouseWheel(float scrollSteps)
{
    m_view.distanceToTarget = m_view.distanceToTarget * (1.0F - scrollSteps * ScrollSensitivity);
    m_view.update();
    m_needsUpdate = true;
}

void AnariMapCamera::setAspectRatio(float aspectRatio)
{
    if (m_perspective.aspectRatio == aspectRatio)
    {
        return;
    }
    m_perspective.aspectRatio = aspectRatio;
    m_needsUpdate = true;
}

void AnariMapCamera::PerspectiveState::setCameraParameters(ANARIDevice anDevice, ANARICamera anCamera) const
{
    anariSetParameter(anDevice, anCamera, "fovY", ANARI_FLOAT32, &fovY);
    anariSetParameter(anDevice, anCamera, "aspect", ANARI_FLOAT32, &aspectRatio);
    anariSetParameter(anDevice, anCamera, "near", ANARI_FLOAT32, &near);
    anariSetParameter(anDevice, anCamera, "far", ANARI_FLOAT32, &far);
}

auto AnariMapCamera::PerspectiveState::generateMatrix() const -> glm::mat4
{
    return glm::perspective(fovY, aspectRatio, near, far);
}

void AnariMapCamera::ViewState::update()
{
    const auto rotationX = glm::angleAxis(this->angleX, glm::vec3{1.0F, 0.0F, 0.0F});
    const auto rotationY = glm::angleAxis(this->angleY, glm::vec3{0.0F, 1.0F, 0.0F});
    const auto rotation = rotationY * rotationX;

    m_direction = rotation * glm::vec3{0.0F, -1.0F, 0.0F};
    m_position = this->targetPoint + -m_direction * this->distanceToTarget;
    m_up = rotation * glm::vec3{0.0F, 0.0F, -1.0F};
}

void AnariMapCamera::ViewState::setCameraParameters(ANARIDevice anDevice, ANARICamera anCamera) const
{
    anariSetParameter(anDevice, anCamera, "position", ANARI_FLOAT32_VEC3, &m_position);
    anariSetParameter(anDevice, anCamera, "up", ANARI_FLOAT32_VEC3, &m_up);
    anariSetParameter(anDevice, anCamera, "direction", ANARI_FLOAT32_VEC3, &m_direction);
}

auto AnariMapCamera::ViewState::generateMatrix() const -> glm::mat4
{
    return glm::lookAt(m_direction, m_position, m_up);
}