#pragma once
#include <GL/glew.h>
#include <vector>
#include "../model/Model.h"

namespace engine 
{
    class FlareRenderer
    {
    public:
        FlareRenderer(GLuint flareshader);
        void render(std::vector<engine::Texture> flareTextures, float brightness, int screenWidth, int screenHeight);
        void destroy();

    private:
        void renderFlare(const engine::Texture& flareTexture, int screenWidth, int screenHeight);
        void prepare(float brightness);

    private:
        GLuint m_flareshader;
        GLuint m_vao;
    };
}
