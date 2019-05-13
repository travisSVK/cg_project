#pragma once
#include "ICollider.h"

namespace engine
{
    class ICollisionDetectionStrategy
    {
    public:
        virtual ~ICollisionDetectionStrategy() {}
        virtual bool checkCollision(ICollider* first, ICollider* second) = 0;
    };
}