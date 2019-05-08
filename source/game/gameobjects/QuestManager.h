#pragma once
#include <vector>
#include "Quest.h"

class QuestManager
{
public:
    void addQuest(const std::string& description, const glm::vec3& expectedPosition, const glm::vec3& expectedDirection, int expectedEffect);
    void checkQuestCompletion(const glm::vec3& position, const glm::vec3& direction, int effect);
    std::string getCurrentQuestObjective();
    std::vector<Quest> getFinishedQuests();
    void setQuestComplete();

private:
    std::vector<Quest> m_quests;
    std::vector<Quest> m_finishedQuests;
};