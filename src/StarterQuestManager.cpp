#include "StarterQuestManager.h"
#include "Folder.h"
#include "Form.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void StarterQuestManager::Install() {
    groups.clear();

    auto files = Folder::GetAllFiles(".\\Data\\", "_BA_STARTER_QUEST.json");

    for (const auto& path : files) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) continue;

            logger::info("Found config file: {}", path.string());

            json data;
            file >> data;

            QuestGroup group;

            if (data.contains("QuestsToSetStage") && data["QuestsToSetStage"].is_array()) {
                for (const auto& entry : data["QuestsToSetStage"]) {
                    if (!entry.is_object()) continue;

                    QuestToSetStage questEntry{};

                    if (entry.contains("Stage") && entry["Stage"].is_array()) {
                        for (const auto& stageValue : entry["Stage"]) {
                            if (stageValue.is_number_unsigned() || stageValue.is_number_integer()) {
                                questEntry.stages.push_back(stageValue.get<std::uint32_t>());
                            }
                        }
                    }

                    if (entry.contains("Quest") && entry["Quest"].is_string()) {
                        std::string questStr = entry["Quest"];
                        if (!questStr.empty()) {
                            auto formId = Form::GetIdFromString(questStr);
                            if (formId) {
                                questEntry.quest = RE::TESForm::LookupByID<RE::TESQuest>(formId);
                            }
                        }
                    }

                    if (questEntry.quest && !questEntry.stages.empty()) {
                        group.questsToSetStage.push_back(questEntry);
                    }
                }
            }

            logger::info("---- Dumping QuestGroup from file: {}", path.string());

            for (std::size_t i = 0; i < group.questsToSetStage.size(); ++i) {
                const auto& q = group.questsToSetStage[i];

                logger::info("  QuestToSetStage [{}]", i);
                logger::info("    Quest: {:08X}", q.quest ? q.quest->GetFormID() : 0);

                for (std::size_t s = 0; s < q.stages.size(); ++s) {
                    logger::info("    Stage [{}]: {}", s, q.stages[s]);
                }
            }


            if (data.contains("RequiredActor")) {
                auto requiredArray = data["RequiredActor"];
                if (data["RequiredActor"].empty() || data["RequiredActor"].is_null()) {
                    if (groups.contains(0)) {
                        for (auto item : group.questsToSetStage) {
                            groups[0].questsToSetStage.push_back(item);
                        }
                    } else {
                        groups[0] = group;
                    }
                } else {
                    for (std::string requiredStr : requiredArray) {
                        if (requiredStr.empty()) {
                            logger::error("string is empty");
                            continue;
                        }
                        auto formId = Form::GetIdFromString(requiredStr);
                        if (formId) {
                            if (groups.contains(formId)) {
                                for (auto item : group.questsToSetStage) {
                                    groups[formId].questsToSetStage.push_back(item);
                                }
                            } else {
                                groups[formId] = group;
                            }
                        } else {
                            logger::error("form not found");
                        }
                    }
                }
            }

        } catch (const json::parse_error& e) {
            logger::error("On file: {}; {}", path.string(), e.what());
        } catch (const std::exception& e) {
            logger::error("On file: {}; Exception: {}", path.string(), e.what());
        }
    }
}

StarterQuestManager::QuestGroup* StarterQuestManager::GetGroupForCharacter(RE::FormID characterId) {
    auto it = groups.find(characterId);
    if (it == groups.end()) {
        return nullptr;
    }
    return &it->second;
}