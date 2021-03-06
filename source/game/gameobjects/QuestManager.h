#pragma once
#include <vector>
#include "Quest.h"

class QuestManager
{
public:
    bool firstQuestComplete;
public:
    void addQuest(const std::string& description, const glm::vec3& expectedPosition, const glm::vec3& expectedDirection, int expectedEffect, bool lensFlareOn);
    bool checkQuestCompletion(const glm::vec3& position, const glm::vec3& direction, int effect, bool lensFlareOn);
    std::string getCurrentQuestObjective();
    bool isCurrentPositionWithinRange(const glm::vec3& position);
    bool isCurrentDirectionWithinRange(const glm::vec3& direction);
    std::vector<Quest> getFinishedQuests();
    int setQuestComplete();

    void setFirstQuestComplete(bool isComplete);
    bool getFirstQuestComplete();

private:
    std::vector<Quest> m_quests;
    std::vector<Quest> m_finishedQuests;
};