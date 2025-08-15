#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace im3e {

class Transform
{
public:
    void translate(const glm::vec3& rTranslation) { m_translation += rTranslation; }
    void scale(const glm::vec3& rScale) { m_scale *= rScale; }
    void rotate(float angle, const glm::vec3& rAxis) { m_orientation = glm::angleAxis(angle, rAxis) * m_orientation; }
    void reset()
    {
        m_translation = glm::vec3{0.0F, 0.0F, 0.0F};
        m_scale = glm::vec3{1.0F, 1.0F, 1.0F};
        m_orientation = glm::quat{1.0F, 0.0F, 0.0F, 0.0F};
    }

    auto toMatrix() const -> glm::mat4
    {
        return glm::translate(m_translation) * glm::scale(m_scale) * glm::mat4_cast(m_orientation);
    }

    void setTranslation(const glm::vec3& rTranslation) { m_translation = rTranslation; }
    void setScale(const glm::vec3& rScale) { m_scale = rScale; }
    void setOrientation(const glm::quat& rOrientation) { m_orientation = rOrientation; }
    void setRotation(float angle, const glm::vec3& rAxis) { m_orientation = glm::angleAxis(angle, rAxis); }

    auto getTranslation() const -> const glm::vec3& { return m_translation; }
    auto getScale() const -> const glm::vec3& { return m_scale; }
    auto getOrientation() const -> const glm::quat& { return m_orientation; }

private:
    glm::vec3 m_translation{0.0F, 0.0F, 0.0F};
    glm::vec3 m_scale{1.0F, 1.0F, 1.0F};
    glm::quat m_orientation{1.0F, 0.0F, 0.0F, 0.0F};
};

}  // namespace im3e