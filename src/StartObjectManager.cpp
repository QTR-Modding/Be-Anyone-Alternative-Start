#include "StartObjectManager.h"
#include "Folder.h"
#include "Form.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

void StartObjectManager::Install() {
    groups.clear();
    auto files = Folder::GetAllFiles(".\\Data\\", "_BA_STARTER_OBJECT.json");
    for (const auto& path : files) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) continue;
            logger::info("Found config file: {}", path.string());
            json data;
            file >> data;

            ObjectGroup group;

            if (data.contains("ObjectsToAdd") && data["ObjectsToAdd"].is_array()) {
                for (const auto& entry : data["ObjectsToAdd"]) {
                    if (!entry.is_object()) continue;
                    ObjectToAdd itemEntry{};
                    itemEntry.count = entry.value("Count", 1);
                    if (entry.contains("Object") && entry["Object"].is_string()) {
                        std::string itemStr = entry["Object"];
                        if (!itemStr.empty()) {
                            auto formId = Form::GetIdFromString(itemStr);
                            if (formId) {
                                itemEntry.baseObject = RE::TESForm::LookupByID(formId);
                            } else {
                                logger::error("Form not found {}", itemStr);
                            }
                        } else {
                            logger::error("Item string is empty");
                        }
                    }
                    if (itemEntry.baseObject) {
                        group.objectsToAdd.push_back(itemEntry);
                    }
                }
            }


            logger::info("---- Dumping ObjectGroup from file: {}", path.string());
            for (std::size_t i = 0; i < group.objectsToAdd.size(); ++i) {
                const auto& itm = group.objectsToAdd[i];
                logger::info("  ObjectToAdd [{}]", i);
                logger::info("    Item: {:08X}", itm.baseObject ? itm.baseObject->GetFormID() : 0);
                logger::info("    Count: {}", itm.count);
            }

            if (data.contains("RequiredActor")) {
                auto requiredArray = data["RequiredActor"];
                if (data["RequiredActor"].empty() || data["RequiredActor"].is_null()) {
                    if (groups.contains(0)) {
                        for (auto item : group.objectsToAdd) {
                            groups[0].objectsToAdd.push_back(item);
                        }
                    } else {
                        groups[0] = group;
                    }
                } 
                else 
                {
                    for (std::string requiredStr : requiredArray) {
                        if (requiredStr.empty()) {
                            logger::error("string is empty");
                            continue;
                        }
                        auto formId = Form::GetIdFromString(requiredStr);
                        if (formId) {
                            if (groups.contains(formId)) {
                                for (auto item : group.objectsToAdd) {
                                    groups[formId].objectsToAdd.push_back(item);
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
        } 
        catch (const json::parse_error& e) {
            logger::error("On file: {}; {}", path.string(), e.what());
        } catch (const std::exception& e) {
            logger::error("On file: {}; Exception: {}", path.string(), e.what());
        }
    }
}

StartObjectManager::ObjectGroup* StartObjectManager::GetGroupForCharacter(RE::FormID characterId) {
    auto it = groups.find(characterId);
    if (it == groups.end()) {
        return nullptr;
    }
    auto& vec = it->second;
    return &vec;
}
