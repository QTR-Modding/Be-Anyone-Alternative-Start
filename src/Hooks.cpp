#include "Hooks.h"
#include "Persistence.h"
#include "Manager.h"
#include "stl.h"
#include "UI.h"
#include "Quest.h"
#include "SpeachManager.h"

class NewGameHook {
public:
    static void Install() { originalFunction = stl::write_prologue_hook(REL::RelocationID(51246, 52118).address(), thunk); }

private:
    static void thunk() { 
        auto success = reinterpret_cast<const SKSE::detail::SKSEMessagingInterface*>(SKSE::GetMessagingInterface())->Dispatch(0, SKSE::MessagingInterface::kNewGame, (void*)RE::TESForm::LookupByID<RE::TESQuest>(0x3372b), sizeof(void*), nullptr);
        if (!success) {
            logger::error("Failed to patch the SKSE event");
        }
        SpeechManager::NewGame();
        Manager::OnNewGame();
        UI::NewGame::Next();
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
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

void Hooks::Install() {
    SaveGameHook::Install();
    LoadGameHook::Install();
    NewGameHook::Install();
    GameInitHook::Install();
    AddMessageHook::Install();
}