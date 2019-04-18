#pragma once
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace engine
{
    class EnvironmentManager
    {
    public:
        enum class EnvironmentTextureType : unsigned int
        {
            EnvironmentMap = 0,
            IrradianceMap = 1
        };

        EnvironmentManager(float environmentMultiplier);
        void createReflectionTexture(const std::vector<std::string>& filenames);
        void createEnvironmentTexture(const char* path, EnvironmentTextureType textureId);
        void createEnvironmentProgram(const char* vertPath, const char* fragPath);
        GLuint getEnvironmentProgram();
        void renderEnvironment(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& cameraPosition);
        float getEnvironmentMultiplier();
        void destroy();

    private:
        std::unordered_map<EnvironmentTextureType, GLuint> m_environmentTextures;
        GLuint m_reflectionTexture;
        GLuint m_environmentProgram;
        float m_environmentMultiplier;
    };
}