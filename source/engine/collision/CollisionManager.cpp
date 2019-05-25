#include "CollisionManager.h"
#include "../Helper.h"

namespace engine
{
    CollisionManager::CollisionManager(ICollisionDetectionStrategy* collisionDetectionStrategy) : m_collisionDetectionStrategy(collisionDetectionStrategy)
    {
        m_program = engine::loadShaderProgram("shaders/collider.vert", "shaders/collider.frag", "", "");
    }

    void CollisionManager::addStaticCollider(ICollider* collider)
    {
        collider->setType(engine::ColliderType::Static);
        m_staticColliders.push_back(collider);
    }

    void CollisionManager::addDynamicCollider(ICollider* collider)
    {
        collider->setType(engine::ColliderType::Dynamic);
        m_dynamicColliders.push_back(collider);
    }

    void CollisionManager::renderColliders(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
    {
        glUseProgram(m_program);
        for (auto staticCollider : m_staticColliders)
        {
            staticCollider->render(viewMatrix, projectionMatrix, m_program);
        }
    }

    void CollisionManager::checkCollision()
    {
        for (auto dynamicCollider : m_dynamicColliders)
        {
            for (auto staticCollider : m_staticColliders)
            {
                if (m_collisionDetectionStrategy->checkCollision(dynamicCollider, staticCollider))
                {
                    dynamicCollider->notifyCollision(staticCollider->getType());
                    staticCollider->notifyCollision(dynamicCollider->getType());
                }
            }
        }
    }

    void CollisionManager::destroy()
    {
        // TODO destroy colliders and collision strategy
        for (auto staticCollider : m_staticColliders)
        {
            staticCollider->destroy();
            delete staticCollider;
        }

        for (auto dynamicCollider : m_dynamicColliders)
        {
            dynamicCollider->destroy();
            delete dynamicCollider;
        }
        delete m_collisionDetectionStrategy;
    }
}