#include "Credit.h"

Credit::Credit(const std::string& text, const glm::ivec2& position) : m_text(text), m_position(position) {}

std::string Credit::getText()
{
    return m_text;
}