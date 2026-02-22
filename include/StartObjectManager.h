#pragma once

class StartObjectManager {

public:
    struct ObjectToAdd {
        RE::TESForm* baseObject = nullptr;
        int count = 1;
        void Add() {
            if (!baseObject) {
                return;
            }
            auto player = RE::PlayerCharacter::GetSingleton();
            auto playerBase = player->GetActorBase();
            if (auto inventoryItem = baseObject->As<RE::TESBoundObject>()) {
                if (inventoryItem->IsInventoryObject()) {
                    player->AddObjectToContainer(inventoryItem, nullptr, count, nullptr);
                }
            }
            if (auto spell = baseObject->As<RE::SpellItem>()) {
                player->AddSpell(spell);
            }
        }
    };

    struct ObjectGroup {
        std::vector<ObjectToAdd> objectsToAdd;
        void AddToPlayer() {
            for (auto item : objectsToAdd) {
                item.Add();
            }
        }
    };

    static StartObjectManager::ObjectGroup* GetGroupForCharacter(RE::FormID characterId);
    static void Install();

private:
    static inline std::unordered_map<RE::FormID, ObjectGroup> groups;

};