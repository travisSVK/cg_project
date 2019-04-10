#include "FlareRenderer.h"
#include "labhelper.h"

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

void FlareRenderer::render(std::vector<labhelper::Texture> flareTextures, float brightness, int screenWidth, int screenHeight)
{
    prepare(brightness);
    for (const auto &flare : flareTextures) 
    {
        renderFlare(flare, screenWidth, screenHeight);
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void FlareRenderer::renderFlare(const labhelper::Texture& flareTexture, int screenWidth, int screenHeight)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, flareTexture.gl_id);
    float xScale = flareTexture.scale;
    float yScale = xScale * (float)(screenWidth / screenHeight);
    glm::vec2 centerPos = flareTexture.screenPosition;
    labhelper::setUniformSlow(m_flareshader, "transform", glm::vec4(centerPos.x, centerPos.y, xScale, yScale));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void FlareRenderer::prepare(float brightness)
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    glUseProgram(m_flareshader);
    labhelper::setUniformSlow(m_flareshader, "brightness", brightness);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(m_vao);
}