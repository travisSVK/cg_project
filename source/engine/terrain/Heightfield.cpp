#include "heightfield.h"

#include <iostream>
#include <stdint.h>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <stb_image.h>
#include "../Helper.h"
#include "../scene/Sun.h"

namespace engine
{
    HeightField::HeightField()
        : m_meshResolution(0)
        , m_vao(UINT32_MAX)
        , m_positionBuffer(UINT32_MAX)
        , m_uvBuffer(UINT32_MAX)
        , m_indexBuffer(UINT32_MAX)
        , m_numIndices(0)
        , m_texid_hf(UINT32_MAX)
        , m_texid_diffuse(UINT32_MAX)
        , m_heightFieldPath("")
        , m_diffuseTexturePath("")
    {
    }

    void HeightField::loadHeightField(const std::string &heigtFieldPath)
    {
        int width, height, components;
        stbi_set_flip_vertically_on_load(true);
        float * data = stbi_loadf(heigtFieldPath.c_str(), &width, &height, &components, 1);
        if (data == nullptr) {
            std::cout << "Failed to load image: " << heigtFieldPath << ".\n";
            return;
        }

        if (m_texid_hf == UINT32_MAX) {
            glGenTextures(1, &m_texid_hf);
        }
        glBindTexture(GL_TEXTURE_2D, m_texid_hf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data); // just one component (float)

        m_heightFieldPath = heigtFieldPath;
        std::cout << "Successfully loaded heigh field texture: " << heigtFieldPath << ".\n";
    }

    GLuint HeightField::useProgram()
    {
        glUseProgram(m_shader);
        return m_shader;
    }

    void HeightField::loadDiffuseTexture(const std::string &diffusePath)
    {
        int width, height, components;
        stbi_set_flip_vertically_on_load(true);
        uint8_t * data = stbi_load(diffusePath.c_str(), &width, &height, &components, 3);
        if (data == nullptr) {
            std::cout << "Failed to load image: " << diffusePath << ".\n";
            return;
        }

        if (m_texid_diffuse == UINT32_MAX) {
            glGenTextures(1, &m_texid_diffuse);
        }

        glBindTexture(GL_TEXTURE_2D, m_texid_diffuse);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); // plain RGB
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cout << "Successfully loaded diffuse texture: " << diffusePath << ".\n";
    }


    void HeightField::generateMesh(int tesselation)
    {
        float offset = 2.0f / (float)tesselation * 2.0f;
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> textureCoordinates;
        std::vector<int> indices;
        int columnCount = 2.0f / offset;
        // reserve number of vertices
        int numVertices = (columnCount + 1) * (columnCount + 1);
        positions.reserve(numVertices);
        textureCoordinates.reserve(numVertices);
        // reserve number of indices
        m_numIndices = columnCount * 2 * columnCount * 3;
        indices.reserve(m_numIndices);

        // verticex and texture positions
        float rowPosition = -1.0f;
        for (float columnPos = -1.0f; columnPos <= 1.0f; columnPos += offset)
        {
            positions.push_back(glm::vec3(columnPos, 10.0f, rowPosition));
            textureCoordinates.push_back(glm::vec2((columnPos + 1.0f) / 2.0f, (rowPosition + 1.0f) / 2.0f));
            if ((columnPos == 1.0f) && (rowPosition != 1.0f))
            {
                columnPos = -1.0f - offset;
                rowPosition += offset;
            }
        }
        glGenBuffers(1, &m_positionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_positionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * positions.size(), &positions[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &m_uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * textureCoordinates.size(), &textureCoordinates[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
        glEnableVertexAttribArray(2);

        // create indices
        for (int i = 0; i < numVertices - columnCount - 1; i++) // skip last row
        {
            // skip last column in each row
            if ((i != 0) && ((i + 1) % (columnCount + 1) == 0))
            {
                continue;
            }
            indices.push_back(i);
            indices.push_back(i + columnCount + 1);
            indices.push_back(i + 1);
            indices.push_back(i + columnCount + 1);
            indices.push_back(i + columnCount + 2);
            indices.push_back(i + 1);
        }
        glGenBuffers(1, &m_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), &indices[0], GL_STATIC_DRAW);

        m_shader = engine::loadShaderProgram("shaders/heightfield.vert", "shaders/simple.frag", "shaders/heightfield.tesc", "shaders/heightfield.tese");
        //m_shader = labhelper::loadShaderProgram("../project/heightfield.vert", "../project/heightfield.frag", "", "");

        glPatchParameteri(GL_PATCH_VERTICES, 3);
    }

    void HeightField::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& cameraWorldPos, float environmentMultiplier, Sun* sun)
    {
        if (m_vao == UINT32_MAX) {
            std::cout << "No vertex array is generated, cannot draw anything.\n";
            return;
        }
        glm::mat4 modelMatrix = glm::scale(glm::vec3(100.f, 1.0f, 100.f));
        // TODO use light source (sun)
        engine::setUniformSlow(m_shader, "directional_light_color", sun->getColor());
        engine::setUniformSlow(m_shader, "directional_light_intensity_multiplier", sun->getIntensityMultiplier());
        glm::mat4 newViewMatrix = viewMatrix;
        newViewMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        engine::setUniformSlow(m_shader, "viewSpaceLightPosition", glm::vec3(newViewMatrix * glm::vec4(sun->getPosition(), 1.0f)));
        engine::setUniformSlow(m_shader, "environment_multiplier", environmentMultiplier);
        engine::setUniformSlow(m_shader, "viewInverse", inverse(viewMatrix));
        
        engine::setUniformSlow(m_shader, "modelMatrix", modelMatrix);
        engine::setUniformSlow(m_shader, "viewMatrix", viewMatrix);
        engine::setUniformSlow(m_shader, "viewProjectionMatrix", projectionMatrix * viewMatrix);
        engine::setUniformSlow(m_shader, "cameraWorldPos", cameraWorldPos);
        engine::setUniformSlow(m_shader, "normalMatrix", inverse(transpose(viewMatrix * modelMatrix)));
        engine::setUniformSlow(m_shader, "has_material_color", 0);
        engine::setUniformSlow(m_shader, "has_texture", 0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(m_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texid_hf);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texid_diffuse);
        glDrawElements(GL_PATCHES, m_numIndices, GL_UNSIGNED_INT, 0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}