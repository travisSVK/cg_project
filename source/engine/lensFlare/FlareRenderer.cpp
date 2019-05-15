#include "FlareRenderer.h"
#include "../Helper.h"

namespace engine 
{
    FlareRenderer::FlareRenderer(GLuint flareShader) : m_flareshader(flareShader)
    {
        const float positions[] = {
            // X Y
            -0.5f, -0.5f,  // v0
            -0.5f, 0.5f,   // v1
            0.5f, -0.5f,   // v2
            0.5f, 0.5f     // v3
        };
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        GLuint positionBuffer;
        glGenBuffers(1, &positionBuffer);													// Create a handle for the vertex position buffer
        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);									// Set the newly created buffer as the current one
        glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);		// Send the vetex position data to the current buffer
        glVertexAttribPointer(0, 2, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
        glEnableVertexAttribArray(0);
    }

    void FlareRenderer::render(std::vector<engine::Texture> flareTextures, float brightness, int screenWidth, int screenHeight, glm::vec2 lightCoords, GLuint depthBuffer)
    {
        prepare(brightness, lightCoords, depthBuffer);
        for (const auto &flare : flareTextures)
        {
            renderFlare(flare, screenWidth, screenHeight, brightness);
        }
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

    void FlareRenderer::renderFlare(const engine::Texture& flareTexture, int screenWidth, int screenHeight, float brightness)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, flareTexture.gl_id);
        float xScale = flareTexture.scale / (brightness + 1.5f);
        float yScale = xScale * (float)(screenWidth / screenHeight);
        glm::vec2 centerPos = flareTexture.screenPosition;
        
        glm::vec2 distanceToCenter = glm::vec2(0.5f, 0.5f) - centerPos;
        brightness = (glm::length(distanceToCenter / 0.7f)) / 4.0f;
        engine::setUniformSlow(m_flareshader, "brightness", brightness);
        engine::setUniformSlow(m_flareshader, "transform", glm::vec4(centerPos.x, centerPos.y, xScale, yScale));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void FlareRenderer::prepare(float brightness, glm::vec2 lightCoords, GLuint depthBuffer)
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(m_flareshader);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthBuffer);
        engine::setUniformSlow(m_flareshader, "sunPos", lightCoords);
        //engine::setUniformSlow(m_flareshader, "brightness", brightness);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(m_vao);
    }

    void FlareRenderer::destroy()
    {
        glDeleteProgram(m_flareshader);
        glDeleteVertexArrays(1, &m_vao);
    }
}