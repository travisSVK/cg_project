#pragma once
#include <string>
#include <glm/glm.hpp>

class Credit
{
public:
    Credit(const std::string& text, const glm::ivec2& position);
    std::string getText();
    void setText(const std::string& text);
    void setPosition(const glm::ivec2& position);

public:
    glm::ivec2 m_position;

private:
    std::string m_text;
};