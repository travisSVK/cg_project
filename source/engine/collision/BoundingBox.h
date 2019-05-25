#pragma once
#include "ICollider.h"

namespace engine
{
    class Model;
    class BoundingBox : public ICollider
    {
    public:
        BoundingBox(glm::vec3 dimensions, float rotationAngle, Model* model);
        virtual ~BoundingBox() {}
        virtual void notifyCollision(ColliderType other);
        virtual void getColliderInfo(std::vector<glm::vec3>& edges, std::vector<glm::vec3>& vertices);
        virtual void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, GLuint program);
        virtual void destroy();
    
    protected:
        std::vector<glm::vec3> m_vertices;

    private:
        glm::vec3 m_dimensions;
        Model* m_model;
        GLuint m_vao;
        GLuint m_positionBuffer;
        GLuint m_indexBuffer;
    };
}