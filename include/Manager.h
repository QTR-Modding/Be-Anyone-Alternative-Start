#pragma once

#include <unordered_set>
#include <unordered_map>

class Manager {
public:
    static void Install();
    static void ShowRaceMenu();
    static void PlayerHideRaceMenu();
    static void SetBaseCharacter(RE::Character* character);
    static RE::Character* GetBaseCharacter();
    static RE::FormID GetPatternFormID();
    static void StartGame();
    static void CopyData();
    static std::vector<RE::Character*>& GetAllCharacters();
    static void OnNewGame();
    static inline bool defaultStart = false;
    static inline bool startAtNPCLocation = false;
    static inline bool startMainQuestLine = true;
    static inline bool enabled = false;

    static inline bool doesGameStartedNow = false;
    static inline bool firstRaceMenuHide = false;
private:
    static inline std::vector<RE::Character*> allNpcs;
    static inline std::vector<RE::FACTION_RANK> addedFactions;
    static inline std::unordered_map<std::string, RE::SpellItem*> spellTranslations;
    static inline RE::Character* oldCopyTarget = nullptr;
    static inline RE::Character* playerCopyTarget = nullptr;
    static inline std::string name;

};