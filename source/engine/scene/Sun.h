#pragma once
#include <glm/glm.hpp>
#include <gl/glew.h>

namespace engine
{
    class Sun
    {
    public:
        Sun(const glm::vec3& color, float intensityMultiplier);
        void render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& rotationMatrix);
        glm::vec3 getPosition();
        glm::vec3 getColor();
        float getIntensityMultiplier();
        void destroy();

    private:
        GLuint m_sunVao;
        GLuint m_positionsVbo;
        GLuint m_indicesVbo;
        GLuint m_textureCoordsVbo;
        GLuint m_program;
        GLuint m_texture;
        float m_intensityMultiplier;
        glm::vec3 m_position;
        glm::vec3 m_color;
    };
}