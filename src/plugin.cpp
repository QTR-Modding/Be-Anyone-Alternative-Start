#include "Logger.h"
#include "UI.h"
#include "Hooks.h"
#include "Manager.h"
#include "SpeachManager.h"
#include "IntroVoiceManager.h"
#include "StarterQuestManager.h"
#include "StartObjectManager.h"
#include "IgnoreManager.h"
#include "Translation.h"
#include "Configuration.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        IntroVoiceManager::Install();
        StartObjectManager::Install();
        StarterQuestManager::Install();
        IgnoreManager::Install(); 
        Manager::Install();
    }
    if (message->type == SKSE::MessagingInterface::kNewGame)
    {
        //auto quest = reinterpret_cast<RE::TESQuest*>(message->data);
        //logger::info("{:x} {} {} {}", quest->GetFormID(), message->dataLen, message->sender, message->type);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    SetupLog();
    logger::info("Plugin loaded");
    Configuration::Load();
    Translation::Install();
    Hooks::Install();
    UI::Register();
    return true;
}
