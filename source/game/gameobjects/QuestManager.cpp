#include "QuestManager.h"

void QuestManager::addQuest(const std::string& description, const glm::vec3& expectedPosition, const glm::vec3& expectedDirection, int expectedEffect)
{
    m_quests.push_back(Quest(description, expectedPosition, expectedDirection, expectedEffect));

}

std::string QuestManager::getCurrentQuestObjective()
{
    return m_quests.at(m_quests.size() - 1).m_description;
}

std::vector<Quest> QuestManager::getFinishedQuests()
{
    return m_finishedQuests;
}

void QuestManager::checkQuestCompletion(const glm::vec3& position, const glm::vec3& direction, int effect)
{
    Quest q = m_quests.at(m_quests.size() - 1);
    if (q.m_expectedPosition == position && q.m_expectedDirection == direction && q.m_expectedEffect == effect)
    {
        m_quests.at(m_quests.size() - 1).m_completed = true;
    }
}

void QuestManager::setQuestComplete()
{
    if (m_quests.at(m_quests.size() - 1).m_completed)
    {
        m_finishedQuests.push_back(m_quests.back());
        m_quests.pop_back();
    }
}