#include "QuestManager.h"
#include <sstream>
#include "postfx/PostFxManager.h"

void QuestManager::addQuest(const std::string& description, const glm::vec3& expectedPosition, const glm::vec3& expectedDirection, int expectedEffect)
{
    m_quests.push_back(Quest(description, expectedPosition, expectedDirection, expectedEffect));

}

std::string QuestManager::getCurrentQuestObjective()
{
    if (m_quests.size() == 0)
    {
        return "";
    }
    Quest quest = m_quests.at(m_quests.size() - 1);

    std::ostringstream positionStream;
    positionStream << std::endl << "Expected position: x = " << quest.m_expectedPosition.x << " y = " << quest.m_expectedPosition.y << " z= " << quest.m_expectedPosition.z;
    std::string positionStr(positionStream.str());
    std::ostringstream directionStream;
    directionStream << std::endl << "Expected direction: x = " << quest.m_expectedDirection.x << " y = " << quest.m_expectedDirection.y << " z= " << quest.m_expectedDirection.z << std::endl;
    std::string directionStr(directionStream.str());

    return m_quests.at(m_quests.size() - 1).m_description + positionStr + directionStr + "Expected effect: " +
        engine::PostFxManager::getEffectName(static_cast<engine::PostFxManager::PostFxTypes>(quest.m_expectedEffect));
}

std::vector<Quest> QuestManager::getFinishedQuests()
{
    return m_finishedQuests;
}

void QuestManager::checkQuestCompletion(const glm::vec3& position, const glm::vec3& direction, int effect)
{
    if (m_quests.size() > 0)
    {
        Quest q = m_quests.at(m_quests.size() - 1);
        if ((q.m_expectedPosition.x <= (position.x + 5.0f)) && (q.m_expectedPosition.x >= (position.x - 5.0f)) &&
            (q.m_expectedPosition.y <= (position.y + 5.0f)) && (q.m_expectedPosition.y >= (position.y - 5.0f)) &&
            (q.m_expectedPosition.z <= (position.z + 5.0f)) && (q.m_expectedPosition.z >= (position.z - 5.0f)) &&
            (q.m_expectedDirection.x <= (direction.x + 0.05f)) && (q.m_expectedDirection.x >= (direction.x - 0.05f)) &&
            (q.m_expectedDirection.y <= (direction.y + 0.05f)) && (q.m_expectedDirection.y >= (direction.y - 0.05f)) &&
            (q.m_expectedDirection.z <= (direction.z + 0.05f)) && (q.m_expectedDirection.z >= (direction.z - 0.05f)) &&
            q.m_expectedEffect == effect)
        {
            m_quests.at(m_quests.size() - 1).m_completed = true;
        }
    }
}

void QuestManager::setQuestComplete()
{
    if ((m_quests.size() > 0) && (m_quests.at(m_quests.size() - 1).m_completed))
    {
        m_finishedQuests.push_back(m_quests.back());
        m_quests.pop_back();
    }
}