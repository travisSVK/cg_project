#include "EnvironmentManager.h"
#include "../Helper.h"
#include "hdr.h"

namespace engine
{
    EnvironmentManager::EnvironmentManager(float environmentMultiplier) : m_environmentMultiplier(environmentMultiplier)
    {}

    void EnvironmentManager::createReflectionTexture(const std::vector<std::string>& filenames)
    {
        m_reflectionTexture = engine::loadHdrMipmapTexture(filenames);
    }

    void EnvironmentManager::createEnvironmentProgram(const char* vertPath, const char* fragPath)
    {
         m_environmentProgram = engine::loadShaderProgram(vertPath, fragPath, "", "");
    }

    void EnvironmentManager::createEnvironmentTexture(const char* path, EnvironmentTextureType textureId)
    {
        m_environmentTextures[textureId] = engine::loadHdrTexture(path);
    }

    GLuint EnvironmentManager::getEnvironmentProgram()
    {
        return m_environmentProgram;
    }

    void EnvironmentManager::renderEnvironment(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& cameraPosition)
    {
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, m_environmentTextures[EnvironmentTextureType::EnvironmentMap]);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, m_environmentTextures[EnvironmentTextureType::IrradianceMap]);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, m_reflectionTexture);
        engine::setUniformSlow(m_environmentProgram, "environment_multiplier", m_environmentMultiplier);
        engine::setUniformSlow(m_environmentProgram, "inv_PV", inverse(projectionMatrix * viewMatrix));
        engine::setUniformSlow(m_environmentProgram, "camera_pos", cameraPosition);
        engine::drawFullScreenQuad();
    }

    float EnvironmentManager::getEnvironmentMultiplier()
    {
        return m_environmentMultiplier;
    }

    void EnvironmentManager::destroy()
    {
        for (auto it : m_environmentTextures)
        {
            glDeleteTextures(1, &it.second);
        }
        glDeleteProgram(m_environmentProgram);
    }
}