#include "Hooks.h"
#include "Persistence.h"
#include "Manager.h"
#include "stl.h"
#include "UI.h"
#include "Quest.h"
#include "SpeachManager.h"

class NewGameHook {
public:
    static void Install() { UI::NewGame::startNewGame = stl::write_prologue_hook(REL::RelocationID(51246, 52118).address(), thunk); }

private:
    static void thunk() { 
 
        UI::NewGame::Next();
    }
};

struct SaveGameHook {
    static char* thunk(RE::BGSSaveLoadManager* manager, void* a2, char* fileName, void* a4, int32_t a5) {
        auto util = RE::BSWin32SaveDataSystemUtility::GetSingleton();
        char fullPath[242];
        util->PrepareFileSavePath(fileName, fullPath, 0, 0);
        Persistence::Save(fullPath);
        return originalFunction(manager, a2, fileName, a4, a5);
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static void Install() {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        originalFunction = trampoline.write_call<5>(REL::RelocationID(34818, 35727).address() + REL::Relocate(0x112, 0x1ce), thunk);
    }
};

struct LoadGameHook {
    static int32_t thunk(RE::BSWin32SaveDataSystemUtility* util, char* fileName, void* unknown) {
        char fullPath[242];
        util->PrepareFileSavePath(fileName, fullPath, 0, 0);
        Persistence::Load(fullPath);
        return originalFunction(util, fileName, unknown);
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static void Install() {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        originalFunction = trampoline.write_call<5>(REL::RelocationID(34677, 35600).address() + REL::Relocate(0xab, 0xab), thunk);
    }
};

struct GameInitHook {
    static void thunk(int64_t a1) {
        originalFunction(a1);
        Manager::ShowRaceMenu();
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static void Install() {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        originalFunction = trampoline.write_call<5>(REL::RelocationID(39366, 40438).address() + REL::Relocate(0x9bc, 0xa4b), thunk);
    }
};

struct AddMessageHook {
    static void thunk(RE::UIMessageQueue* a_queue, const RE::BSFixedString& a_menuName, RE::UI_MESSAGE_TYPE a_type, RE::IUIMessageData* a_data) { 
        if (a_menuName == RE::RaceSexMenu::MENU_NAME && a_type == RE::UI_MESSAGE_TYPE::kHide) {
            Manager::PlayerHideRaceMenu();
        }
        originalFunction(a_queue, a_menuName, a_type, a_data);
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static void Install() {
        originalFunction = stl::write_prologue_hook(REL::RelocationID(13530, 13631).address(), thunk); 
    }
};

 struct QuestStartHook {
    static bool thunk(RE::TESQuest* a_quest, bool& a_result, bool a_startNow) {
        if (a_quest) {
            logger::info("Quest started: {} {:x} {}", a_quest->GetName(), a_quest->GetFormID(), a_startNow);
        }
        return originalFunction(a_quest, a_result, a_startNow);
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static void Install() { originalFunction = stl::write_prologue_hook(REL::RelocationID(24481, 25003).address(), thunk); }
};

 struct QuestStageHook {
    static bool thunk(RE::TESQuest* a_quest, uint16_t a_stage) {
        auto result = originalFunction(a_quest, a_stage);
        if (a_quest) {
            logger::info("Quest Sage: {} {:x} {}", a_quest->GetName(), a_quest->GetFormID(), a_stage);

            #define DLC1NPCMentalModel 0x2002b6e
            if (a_quest->GetFormID() == DLC1NPCMentalModel) {

            }
        }
        return result;
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static void Install() {
    SKSE::AllocTrampoline(14);
        originalFunction = stl::write_prologue_hook(REL::RelocationID(24482, 25004).address(), thunk);
    }
};


void Hooks::Install() {
    SaveGameHook::Install();
    LoadGameHook::Install();
    NewGameHook::Install();
    GameInitHook::Install();
    AddMessageHook::Install();
    //QuestStartHook::Install();
    //QuestStageHook::Install();
}