#pragma once
#include <glm/glm.hpp>

namespace engine
{
    class Camera
    {
    public:
        Camera(const glm::vec3& position, const glm::vec3& direction);
        glm::vec3 getPosition();
        glm::vec3 getDirection();
        glm::vec3 getWorldUp();
        float getCameraSpeed();
        void setDirection(const glm::vec3& direction);
        void setPosition(const glm::vec3& position);

    private:
        glm::vec3 m_position;
        glm::vec3 m_direction;
        glm::vec3 m_worldUp;
        float m_cameraSpeed;
    };
}