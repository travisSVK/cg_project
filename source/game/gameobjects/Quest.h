#pragma once
#include <string>
#include <glm/glm.hpp>

class Quest
{
public:
    Quest(const std::string& description, const glm::vec3& expectedPosition, const glm::vec3& expectedDirection, int expectedEffect, bool lensFlareOn);
    glm::vec3 m_expectedPosition;
    glm::vec3 m_expectedDirection;
    int m_expectedEffect;
    std::string m_description;
    bool m_lensFlareOn;
    bool m_completed;
};