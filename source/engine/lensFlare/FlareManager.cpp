#include "FlareManager.h"
#include "FlareRenderer.h"
#include <stb_image.h>
#include "../Helper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace engine
{
    FlareManager::FlareManager(float spacing, int width, int heigt) : m_spacing(spacing), m_screenWidth(width), m_screenHeight(heigt)
{
    m_flareRenderer = new FlareRenderer(engine::loadShaderProgram("shaders/lens_flare.vert", "shaders/lens_flare.frag"));
    loadTextures();
}

void FlareManager::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& lightPosition)
{
    glm::vec2 lightCoords = convertToScreenSpace(lightPosition, viewMatrix, projectionMatrix);
    // check if light is inside the screen
    if (lightCoords.x > 1.0f || lightCoords.x < 0.0f || lightCoords.y > 1.0f || lightCoords.y < 0.0f)
    {
        return;
    }
    // subtract coords from center of the screen to get the vector
    glm::vec2 lightToCenter = glm::vec2(0.5f, 0.5f) - lightCoords;
    float brightness = 1 - (glm::length(lightToCenter) / 0.7f);
    if (brightness > 0) {
        calcFlarePositions(lightToCenter, lightCoords);
        m_flareRenderer->render(m_textures, brightness, m_screenWidth, m_screenHeight);
    }
}

void FlareManager::calcFlarePositions(const glm::vec2& lightToCenter, const glm::vec2& lightCoords)
{
    for (int i = 0; i < m_textures.size(); i++) {
        glm::vec2 direction = lightToCenter * (float)i * m_spacing;
        m_textures[i].screenPosition = lightCoords + direction;
    }
}

glm::vec2 FlareManager::convertToScreenSpace(const glm::vec3& worldPos, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    glm::vec4 coords = glm::vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f);
    coords = projectionMatrix * viewMatrix * coords;
    float x = (coords.x / coords.w + 1) / 2.0f;
    float y = 1 - ((coords.y / coords.w + 1) / 2.0f);
    return glm::vec2(x, y);
}

void FlareManager::loadTextures()
{
    engine::Texture tex1 = loadTexture("../scenes/tex1.png");
    engine::Texture tex2 = loadTexture("../scenes/tex2.png");
    engine::Texture tex3 = loadTexture("../scenes/tex3.png");
    engine::Texture tex4 = loadTexture("../scenes/tex4.png");
    engine::Texture tex5 = loadTexture("../scenes/tex5.png");
    engine::Texture tex6 = loadTexture("../scenes/tex6.png");
    engine::Texture tex7 = loadTexture("../scenes/tex7.png");
    engine::Texture tex8 = loadTexture("../scenes/tex8.png");
    engine::Texture tex9 = loadTexture("../scenes/tex9.png");

    tex6.scale = 0.5f;
    m_textures.push_back(tex6);
    tex4.scale = 0.23f;
    m_textures.push_back(tex4);
    tex2.scale = 0.1f;
    m_textures.push_back(tex2);
    tex7.scale = 0.05f;
    m_textures.push_back(tex7);
    tex1.scale = 0.02f;
    m_textures.push_back(tex1);
    tex3.scale = 0.06f;
    m_textures.push_back(tex3);
    tex9.scale = 0.12f;
    m_textures.push_back(tex9);
    tex5.scale = 0.07f;
    m_textures.push_back(tex5);
    tex1.scale = 0.012f;
    m_textures.push_back(tex1);
    tex7.scale = 0.2f;
    m_textures.push_back(tex7);
    tex9.scale = 0.1f;
    m_textures.push_back(tex9);
    tex3.scale = 0.07f;
    m_textures.push_back(tex3);
    tex5.scale = 0.3f;
    m_textures.push_back(tex5);
    tex4.scale = 0.4f;
    m_textures.push_back(tex4);
    tex8.scale = 0.6f;
    m_textures.push_back(tex8);
}

engine::Texture FlareManager::loadTexture(const char* path)
{
    int w, h, comp;
    unsigned char* image = stbi_load(path, &w, &h, &comp, STBI_rgb_alpha);

    engine::Texture texture;
    glGenTextures(1, &texture.gl_id);
    glBindTexture(GL_TEXTURE_2D, texture.gl_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    free(image);
    return texture;
}

void FlareManager::destroy()
{
    for (auto texture : m_textures)
    {
        glDeleteTextures(1, &texture.gl_id);
    }
    m_flareRenderer->destroy();
    delete m_flareRenderer;
}
}