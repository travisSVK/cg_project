#pragma once
#include "ICollisionDetectionStrategy.h"

namespace engine
{
    class SATCollisionStrategy : public ICollisionDetectionStrategy
    {
    public:
        virtual bool checkCollision(ICollider* first, ICollider* second);

    private:
        bool detectCollision(const std::vector<glm::vec3>& edges,
            const std::vector<glm::vec3>& verticesFirst, const std::vector<glm::vec3>& verticesSecond);
    };
}