#pragma once

#include <im3e/api/camera.h>
#include <im3e/utils/loggers.h>

#include <anari/anari.h>

#include <memory>

namespace im3e {

class AnariCamera : public ICamera
{
public:
    AnariCamera(const ILogger& rLogger, std::shared_ptr<anari::api::Device> pAnDevice);

    void setPosition(const glm::vec3& rPosition) override;
    void setDirection(const glm::vec3& rDirection) override;
    void setUp(const glm::vec3& rUp) override;
    void setAspectRatio(float ratio) override;

    auto getPosition() const -> glm::vec3 override { return m_position; }
    auto getDirection() const -> glm::vec3 override { return m_direction; }
    auto getUp() const -> glm::vec3 override { return m_up; }
    auto getAspectRatio() const -> float override { return m_aspectRatio; }

    auto getHandle() const { return m_pAnCamera.get(); }

private:
    std::unique_ptr<ILogger> m_pLogger;
    std::shared_ptr<anari::api::Device> m_pAnDevice;
    std::unique_ptr<anari::api::Camera, std::function<void(anari::api::Camera*)>> m_pAnCamera;

    glm::vec3 m_position{0.0F, 0.0F, 0.0F};
    glm::vec3 m_direction{0.0F, 0.0F, -1.0F};
    glm::vec3 m_up{0.0F, 1.0F, 0.0F};

    float m_aspectRatio{1.0F};
};

}  // namespace im3e