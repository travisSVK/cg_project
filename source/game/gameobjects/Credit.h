#pragma once
#include <string>
#include <glm/glm.hpp>

class Credit
{
public:
    Credit(const std::string& text, const glm::ivec2& position);
    std::string getText();

public:
    glm::ivec2 m_position;

private:
    std::string m_text;
};