#include "ICollider.h"

namespace engine
{
    void ICollider::setType(ColliderType type)
    {
        m_type = type;
    }

    ColliderType ICollider::getType()
    {
        return m_type;
    }
}