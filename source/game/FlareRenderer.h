#pragma once
#include <GL/glew.h>
#include <vector>
#include "Model.h"

class FlareRenderer
{
public:
    FlareRenderer(GLuint flareshader);
    void render(std::vector<labhelper::Texture> flareTextures, float brightness, int screenWidth, int screenHeight);

private:
    void renderFlare(const labhelper::Texture& flareTexture, int screenWidth, int screenHeight);
    void prepare(float brightness);

private:
    GLuint m_flareshader;
    GLuint m_vao;
};