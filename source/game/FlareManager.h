#pragma once
#include "Model.h"

class FlareRenderer;
class FlareManager
{
public:
    FlareManager(float spacing, int w, int h);
    void destroy();
    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& lightPosition);

private:
    void loadTextures();
    labhelper::Texture loadTexture(const char* path);
    void calcFlarePositions(const glm::vec2& lightToCenter, const glm::vec2& lightCoords);
    glm::vec2 convertToScreenSpace(const glm::vec3& worldPos, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

private:
    std::vector<labhelper::Texture> m_textures;
    float m_spacing;
    FlareRenderer* m_flareRenderer;
    int m_screenWidth;
    int m_screenHeight;
};