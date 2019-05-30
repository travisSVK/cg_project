#include "Credit.h"

Credit::Credit(const std::string& text, const glm::ivec2& position) : m_text(text), m_position(position) {}

std::string Credit::getText()
{
    return m_text;
}

void Credit::setText(const std::string& text)
{
    m_text = text;
}

void Credit::setPosition(const glm::ivec2& position)
{
    m_position = position;
}