#include "BoundingBox.h"
#include "../model/Model.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "../Helper.h"

namespace engine
{
    BoundingBox::BoundingBox(glm::vec3 dimensions, float rotationAngle, Model* model) : m_dimensions(dimensions), m_model(model)
    {
        glm::mat4 rotMatrix = glm::rotate(rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        m_vertices.push_back(rotMatrix * glm::vec4(-1.0f * dimensions.x, -1.0f * dimensions.y, -1.0f * dimensions.z, 1.0f));
        m_vertices.push_back(rotMatrix * glm::vec4(1.0f * dimensions.x, -1.0f * dimensions.y, -1.0f * dimensions.z, 1.0f));
        m_vertices.push_back(rotMatrix * glm::vec4(1.0f * dimensions.x, -1.0f * dimensions.y, 1.0f * dimensions.z, 1.0f));
        m_vertices.push_back(rotMatrix * glm::vec4(-1.0f * dimensions.x, -1.0f * dimensions.y, 1.0f * dimensions.z, 1.0f));

        m_vertices.push_back(rotMatrix * glm::vec4(-1.0f * dimensions.x, 1.0f * dimensions.y, -1.0f * dimensions.z, 1.0f));
        m_vertices.push_back(rotMatrix * glm::vec4(1.0f * dimensions.x, 1.0f * dimensions.y, -1.0f * dimensions.z, 1.0f));
        m_vertices.push_back(rotMatrix * glm::vec4(1.0f * dimensions.x, 1.0f * dimensions.y, 1.0f * dimensions.z, 1.0f));
        m_vertices.push_back(rotMatrix * glm::vec4(-1.0f * dimensions.x, 1.0f * dimensions.y, 1.0f * dimensions.z, 1.0f));

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_positionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_positionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
        glEnableVertexAttribArray(0);

        /*const int indices[] = {
            0, 3, 2,
            0, 2, 1,
            
            4, 7, 6,
            4, 6, 5,
            
            0, 4, 1,
            4, 5, 1,

            1, 5, 2,
            5, 6, 2,

            2, 6, 3,
            6, 7, 3,

            3, 7, 0,
            7, 4, 0
        };*/

        const int indices[] = {
            0, 1,
            1, 2,
            2, 3,
            3, 0,
            4, 5,
            5, 6,
            6, 7,
            7, 4,
            0, 4,
            1, 5,
            2, 6,
            3, 7
        };

        glGenBuffers(1, &m_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    void BoundingBox::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, GLuint program)
    {
        glBindVertexArray(m_vao);
        engine::setUniformSlow(program, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * m_model->m_modelMatrix);
        glDrawElements(GL_LINES, 25, GL_UNSIGNED_INT, 0);
    }
    
    void BoundingBox::notifyCollision(ColliderType other)
    {
        if (m_type == ColliderType::Static)
        {
            return;
        }
        if ((m_type == ColliderType::Dynamic) && (other == ColliderType::Static))
        {
            m_model->m_modelMatrix = m_model->m_previousModelMatrix;
        }
    }

    void BoundingBox::getColliderInfo(std::vector<glm::vec3>& edges, std::vector<glm::vec3>& vertices)
    {
        // first, compute world coordinates of vertices
        glm::mat4 modelMatrix = m_model->m_modelMatrix;
        vertices = m_vertices;
        for (auto& vertex : vertices)
        {
            vertex = glm::vec3(modelMatrix * glm::vec4(vertex, 1.0f));
        }

        // get edges - basically get obb normals
        edges.push_back(vertices[1] - vertices[0]);
        edges.push_back(vertices[2] - vertices[1]);
        edges.push_back(vertices[4] - vertices[0]);
    }
}