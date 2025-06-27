#include "anari_camera.h"

#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

namespace {

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

AnariCamera::AnariCamera(const ILogger& rLogger, shared_ptr<anari::api::Device> pAnDevice)
  : m_pLogger(rLogger.createChild("AnariCamera"))
  , m_pAnDevice(throwIfArgNull(move(pAnDevice), "Anari Camera requires a device"))
  , m_pAnCamera(createCamera(*m_pLogger, m_pAnDevice.get()))
{
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "position", ANARI_FLOAT32_VEC3, &m_position);
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "up", ANARI_FLOAT32_VEC3, &m_up);
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "direction", ANARI_FLOAT32_VEC3, &m_direction);
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "aspect", ANARI_FLOAT32, &m_aspectRatio);

    const float near = 0.1F;
    const float far = 1000.0F;
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "near", ANARI_FLOAT32, &near);
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "far", ANARI_FLOAT32, &far);

    anariCommitParameters(m_pAnDevice.get(), m_pAnCamera.get());
}

void AnariCamera::setPosition(const glm::vec3& rPosition)
{
    m_position = rPosition;
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "position", ANARI_FLOAT32_VEC3, &m_position);
    anariCommitParameters(m_pAnDevice.get(), m_pAnCamera.get());
}

void AnariCamera::setDirection(const glm::vec3& rDirection)
{
    m_direction = glm::normalize(rDirection);
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "direction", ANARI_FLOAT32_VEC3, &m_direction);
    anariCommitParameters(m_pAnDevice.get(), m_pAnCamera.get());
}

void AnariCamera::setUp(const glm::vec3& rUp)
{
    m_up = rUp;
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "up", ANARI_FLOAT32_VEC3, &m_up);
    anariCommitParameters(m_pAnDevice.get(), m_pAnCamera.get());
}

void AnariCamera::setAspectRatio(float ratio)
{
    m_aspectRatio = ratio;
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "aspect", ANARI_FLOAT32, &m_aspectRatio);
    anariCommitParameters(m_pAnDevice.get(), m_pAnCamera.get());
}