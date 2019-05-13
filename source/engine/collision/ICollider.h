#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

namespace engine
{
    enum class ColliderType : unsigned int
    {
        Static = 0,
        Dynamic = 1
    };

    class ICollider
    {
    public:

        virtual ~ICollider() {}
        virtual void notifyCollision(ColliderType other) = 0;
        virtual void getColliderInfo(std::vector<glm::vec3>& edges, std::vector<glm::vec3>& vertices) = 0;
        virtual void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, GLuint program) = 0;
        
        void setType(ColliderType type);
        ColliderType getType();

    protected:
        ColliderType m_type;
    };
}