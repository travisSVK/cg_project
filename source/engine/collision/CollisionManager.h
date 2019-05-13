#pragma once
#include <vector>
#include "ICollisionDetectionStrategy.h"
#include <GL/glew.h>

namespace engine
{
    class CollisionManager
    {
    public:
        CollisionManager(ICollisionDetectionStrategy* collisionDetectionStrategy);
        void addStaticCollider(ICollider* collider);
        void addDynamicCollider(ICollider* collider);
        void checkCollision();
        void renderColliders(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void destroy();

    private:
        std::vector<ICollider*> m_staticColliders;
        std::vector<ICollider*> m_dynamicColliders;
        ICollisionDetectionStrategy* m_collisionDetectionStrategy;
        GLuint m_program;
    };
}