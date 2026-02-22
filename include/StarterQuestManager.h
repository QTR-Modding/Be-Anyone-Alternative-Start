#pragma once

#include <fstream>
#include <unordered_map>
#include <vector>
#include "Quest.h"
#include "RE/Skyrim.h"

class StarterQuestManager {
public:
    struct QuestToSetStage {
        RE::TESQuest* quest = nullptr;
        std::vector<uint32_t> stages;
        void Apply() {
            Quest::SetQuestSage(quest->GetFormID(), stages);
        }
    };
    struct QuestGroup {
        std::vector<QuestToSetStage> questsToSetStage;
        void Apply() {
            for (auto& quest : questsToSetStage) {
                if (quest.quest) {
                    quest.Apply();
                }
            }
        }
    };

    static StarterQuestManager::QuestGroup* GetGroupForCharacter(RE::FormID characterId);
    static void Install();

private:
    static inline std::unordered_map<RE::FormID, QuestGroup> groups;
};