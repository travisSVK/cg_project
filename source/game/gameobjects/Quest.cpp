#include "Quest.h"

Quest::Quest(const std::string& description, const glm::vec3& expectedPosition, const glm::vec3& expectedDirection, int expectedEffect) 
    : m_description(description), m_expectedPosition(expectedPosition), m_expectedDirection(expectedDirection), m_expectedEffect(expectedEffect)
{
    m_completed = false;
}