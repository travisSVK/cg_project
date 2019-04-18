#include "Camera.h"

namespace engine
{
    Camera::Camera(const glm::vec3& position, const glm::vec3& direction) : m_position(position), m_direction(direction), m_worldUp(0.0f, 1.0f, 0.0f), m_cameraSpeed(1.0f)
    {}

    glm::vec3 Camera::getPosition()
    {
        return m_position;
    }

    glm::vec3 Camera::getDirection()
    {
        return m_direction;
    }

    void Camera::setDirection(const glm::vec3& direction)
    {
        m_direction = direction;
    }

    void Camera::setPosition(const glm::vec3& position)
    {
        m_position = position;
    }

    float Camera::getCameraSpeed()
    {
        return m_cameraSpeed;
    }

    glm::vec3 Camera::getWorldUp()
    {
        return m_worldUp;
    }
}