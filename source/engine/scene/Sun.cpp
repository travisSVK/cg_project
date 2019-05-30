#include "Sun.h"
#include <stb_image.h>
#include "../Helper.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace engine
{
    Sun::Sun(const glm::vec3& color, float intensityMultiplier) : m_color(color), m_intensityMultiplier(intensityMultiplier)
    {
        glGenVertexArrays(1, &m_sunVao);
        glBindVertexArray(m_sunVao);
        m_position = glm::vec3(-65.0f, 115.0f, -500.0f);
        const float positions[] = {
            // X Y Z
            -80.0f, -80.0f, 0.0f,    // v0
            -80.0f, 80.0f, 0.0f,   // v1
            80.0f, 80.0f, 0.0f,   // v2
            80.0f, -80.0f, 0.0f     // v3
        };

        glGenBuffers(1, &m_positionsVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_positionsVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
        glEnableVertexAttribArray(0);

        float texcoords[] = {
        0.0f, 0.0f,	// (u,v) for v0
        0.0f, 1.0f,	// (u,v) for v1
        1.0f, 1.0f,	// (u,v) for v2
        1.0f, 0.0f	// (u,v) for v3
        };
        glGenBuffers(1, &m_textureCoordsVbo);														// Create a handle for the vertex color buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_textureCoordsVbo);										// Set the newly created buffer as the current one
        glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);			// Send the color data to the current buffer
        glVertexAttribPointer(2, 2, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
        glEnableVertexAttribArray(2);

        const int indices[] = {
        0, 1, 3, // Triangle 1
        1, 2, 3  // Triangle 2
        };
        glGenBuffers(1, &m_indicesVbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesVbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        int w, h, comp;
        unsigned char* image = stbi_load("../scenes/tex6.png", &w, &h, &comp, STBI_rgb_alpha);
        if (image == nullptr) {
            std::cout << "Failed to load image: sun.\n";
            return;
        }

        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        free(image);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        m_program = engine::loadShaderProgram("shaders/sun.vert", "shaders/sun.frag", "", "");
    }

    glm::vec3 Sun::getPosition()
    {
        return m_position;
    }

    glm::vec3 Sun::getColor()
    {
        return m_color;
    }

    float Sun::getIntensityMultiplier()
    {
        return m_intensityMultiplier;
    }

    void Sun::render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& rotationMatrix)
    {
        glUseProgram(m_program);
        glm::mat4 newViewMatrix = viewMatrix;
        newViewMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // lets keep the sun at the same spot and without bilboard rotation
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_position);
        engine::setUniformSlow(m_program, "viewProjectionMatrix", projectionMatrix * newViewMatrix * translationMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glBindVertexArray(m_sunVao);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    void Sun::destroy()
    {
        glDeleteTextures(1, &m_texture);
        glDeleteBuffers(1, &m_positionsVbo);
        glDeleteBuffers(1, &m_textureCoordsVbo);
        glDeleteBuffers(1, &m_indicesVbo);
        glDeleteVertexArrays(1, &m_sunVao);
        glDeleteProgram(m_program);
    }
}