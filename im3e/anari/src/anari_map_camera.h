#pragma once

#include <im3e/api/gui.h>
#include <im3e/utils/loggers.h>

#include <anari/anari.h>

#include <numbers>

namespace im3e {

class AnariMapCamera : public IGuiEventListener
{
public:
    AnariMapCamera(const ILogger& rLogger, ANARIDevice anDevice);

    void commitChanges();

    void onMouseMove(const glm::vec2& rClipOffset, const std::array<bool, 3U>& rMouseButtonsDown) override;
    void onMouseWheel(float scrollSteps) override;

    void setAspectRatio(float ratio);

    auto getHandle() const -> ANARICamera { return m_pAnCamera.get(); }

private:
    std::unique_ptr<ILogger> m_pLogger;
    ANARIDevice m_anDevice;
    std::shared_ptr<anari::api::Camera> m_pAnCamera;

    bool m_needsUpdate{true};

    struct PerspectiveState
    {
        float fovY{std::numbers::pi_v<float> / 3.0F};
        float aspectRatio{1.0F};
        float near{0.1F};
        float far{10'000.0F};

        void setCameraParameters(ANARIDevice anDevice, ANARICamera anCamera) const;

        auto generateMatrix() const -> glm::mat4;
    };
    PerspectiveState m_perspective;

    struct ViewState
    {
        glm::vec3 targetPoint{};
        float distanceToTarget{1400.0F};
        float angleX{};
        float angleY{};

        void update();
        void setCameraParameters(ANARIDevice anDevice, ANARICamera anCamera) const;

        auto generateMatrix() const -> glm::mat4;

    private:
        glm::vec3 m_position;
        glm::vec3 m_direction;
        glm::vec3 m_up;
    };
    ViewState m_view;
};

}  // namespace im3e