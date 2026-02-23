#include "Manager.h"
#include "Quest.h"
#include "SpeachManager.h"
#include "IntroVoiceManager.h"
#include "StartObjectManager.h"
#include "StarterQuestManager.h"
#include "IgnoreManager.h"
#include "Configuration.h"
#include "Form.h"

bool IsNullOrWhitespace(const char* str) {
    if (!str) return true;
    while (*str) {
        if (!isspace(static_cast<unsigned char>(*str))) {
            return false;
        }
        ++str;
    }
    return true;
}

#define MQ101 0x3372b
#define MQ102 0x4E50D
#define MQ102A 0x2BF9C
#define MQ102B 0x2610A
#define GHOST_SPELL 0x5030B
#define RIVERWOOD 0x9732
#define CW 0x19E53
#define MQ00 0x1C5D9

inline bool IsLiveAnotherLifeInstalled() {
    constexpr auto dllPath = "Data/alternate start - live another life.esp";
    return std::filesystem::exists(dllPath);
}

void OnSpreachEnd() {

    auto quests = StarterQuestManager::GetGroupForCharacter(Manager::GetPatternFormID());
    if (quests) {
        quests->Apply();
    }
    auto defaultQuests = StarterQuestManager::GetGroupForCharacter(0);
    if (defaultQuests) {
        defaultQuests->Apply();
    }
    if (Manager::startMainQuestLine) {
        if (IsLiveAnotherLifeInstalled()) {
            auto ARTHLALRumorsOfWarQuest = Form::GetIdFromString("alternate start - live another life.esp~0x7a334");
            auto ARTHLALChargenQuest = Form::GetIdFromString("alternate start - live another life.esp~0xDAF");
            Quest::CallQuestVoidFunction(ARTHLALChargenQuest, "arth_lal_startquest", "CleanupHelgen");
            Quest::SetQuestSage(ARTHLALRumorsOfWarQuest, {20, 25, 28, 29, 30, 31, 32, 40, 41, 50});
        }
        Quest::SetQuestSage(CW, {0});
        Quest::SetQuestSage(MQ102, {30});
    }
}

void Manager::Install() {
    
    SpeechManager::AddSpeachEndCallback(OnSpreachEnd);
    const auto& [map, lock] = RE::TESForm::GetAllForms();
    const RE::BSReadWriteLock l{lock};


    for (auto& [id, form] : *map) {
        if (!form) {
            continue;
        }

        if (auto book = form->As<RE::TESObjectBOOK>()) {
            if (auto spell = book->GetSpell()) {
                spellTranslations[spell->GetName()] = spell;
            }
        }

        auto actor = form->As<RE::Character>();

        if (!actor) {
            continue;
        }


        if (actor->IsChild()) {
            continue;
        }

        if (!actor->IsHumanoid()) {
            continue;
        }

        if (IsNullOrWhitespace(actor->GetName())) {
            continue;
        }

        if (actor->GetParentCell() && IsNullOrWhitespace(actor->GetParentCell()->GetName())) {
            continue;
        }

        if (IgnoreManager::DoesIgnore(actor)) {
            continue;
        }

        auto base = actor->GetActorBase();
        if (!base) {
            continue;
        }

        auto rawFormID = base->GetFormID();
        auto rawIndex = (rawFormID & 0xFF000000) >> 24;

        if (rawIndex == 0xFF){
            continue;
        }

        allNpcs.push_back(actor);
    }

    std::sort(allNpcs.begin(), allNpcs.end(), [](RE::Character* a, RE::Character* b) {
        if (a->GetParentCell() && b->GetParentCell()) {
            if (a->GetParentCell()->GetName() != b->GetParentCell()->GetName()) return a->GetParentCell()->GetName() < b->GetParentCell()->GetName();
        }
        return std::strcmp(a->GetName(), b->GetName()) < 0;
    });
}
inline bool IsRaceMenuInstalled() {
    constexpr auto dllPath = "Data/RaceMenu.esp";
    return std::filesystem::exists(dllPath);
}
void ModifyRaceMenu(const char* name) {
    auto ui = RE::UI::GetSingleton();
    auto faderMenu = ui->GetMenu<RE::RaceSexMenu>();
    if (!faderMenu) {
        logger::error("no fader menu");
        return;
    }

    RE::GFxValue _root;
    RE::GFxValue RaceSexMenuBaseInstance;
    RE::GFxValue RaceSexPanelsInstance;

    if (!faderMenu->uiMovie->GetVariable(&_root, "_root")) {
        logger::error("no root");
        return;
    }

    if (!_root.GetMember("RaceSexMenuBaseInstance", &RaceSexMenuBaseInstance)) {
        logger::error("no RaceSexMenuBaseInstance");
        return;
    }
    if (!RaceSexMenuBaseInstance.GetMember("RaceSexPanelsInstance", &RaceSexPanelsInstance)) {
        logger::error("no RaceSexPanelsInstance");
        return;
    }

    if (IsRaceMenuInstalled()) {
        RE::GFxValue textEntryField;
        if (RaceSexPanelsInstance.GetMember("textEntry", &textEntryField)) {
            RE::GFxValue textInput;
            if (textEntryField.GetMember("TextInputInstance", &textInput)) {
                textInput.SetText(name);
                textInput.SetMember("focused", true);  // or textInput.SetBoolean("focused", true);
            }

            // FadeTextEntry(true) is on the panel, not the field
            RE::GFxValue fadeArgs[1];
            fadeArgs[0].SetBoolean(true);
            RaceSexPanelsInstance.Invoke("ShowTextEntry", nullptr, fadeArgs, 1);
        }
    } else {
        RE::GFxValue textEntryField;
        if (RaceSexPanelsInstance.GetMember("_TextEntryField", &textEntryField)) {
            RE::GFxValue textInput;
            if (textEntryField.GetMember("TextInputInstance", &textInput)) {
                textInput.SetText(name);
                textInput.SetMember("focused", true);  // or textInput.SetBoolean("focused", true);
            }

            // FadeTextEntry(true) is on the panel, not the field
            RE::GFxValue fadeArgs[1];
            fadeArgs[0].SetBoolean(true);
            RaceSexPanelsInstance.Invoke("FadeTextEntry", nullptr, fadeArgs, 1);
        }
    }
    RE::ControlMap::GetSingleton()->AllowTextInput(true);
}

void Manager::ShowRaceMenu() {
    if (!enabled) return;
    if (!doesGameStartedNow) {
        return;
    }

    doesGameStartedNow = false;
    if (!Quest::IsDone(MQ101)) {
        RE::UIMessageQueue::GetSingleton()->AddMessage(RE::RaceSexMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        std::thread([] { 

            while (true) {
                if (RE::UI::GetSingleton()->IsMenuOpen(RE::RaceSexMenu::MENU_NAME)) {
                    ModifyRaceMenu(RE::PlayerCharacter::GetSingleton()->GetName()); 
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
             
            }).detach();
    }
}

void Manager::PlayerHideRaceMenu() {
    if (!enabled) return;
    if (!firstRaceMenuHide) {
        return;
    }

    firstRaceMenuHide = false;

    auto specificItems = StartObjectManager::GetGroupForCharacter(GetPatternFormID());
    if (specificItems) {
        specificItems->AddToPlayer();
    }
    auto genericItems= StartObjectManager::GetGroupForCharacter(0);
    if (genericItems) {
        genericItems->AddToPlayer();
    }
    auto group = IntroVoiceManager::GetRandomGroup(GetPatternFormID());

    if (group) {
        for (auto sequence : group->sequences) {
            SpeechManager::Add({
                sequence.speakerName,
                sequence.subtitiles,
                sequence.duration,
                sequence.audio
            });
        }
        if (group->sequences.size() == 0) {
            OnSpreachEnd();
        }
    } else {
        OnSpreachEnd();
    }
}

void Manager::SetBaseCharacter(RE::Character* character) {
    playerCopyTarget = character; 
}

RE::Character* Manager::GetBaseCharacter() { 
    return playerCopyTarget; 
}
RE::FormID Manager::GetPatternFormID() {
    return GetBaseCharacter() ? GetBaseCharacter()->GetFormID() : 0; 
}

void Manager::StartGame() {
    if (!enabled) return;
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerBase = player->GetActorBase();
    auto possesionBase = playerCopyTarget->GetActorBase();

    CopyData();

    playerBase->playerSkills = possesionBase->playerSkills;
    playerBase->actorData.level = possesionBase->GetLevel();

    auto container = player->extraList.GetByType<RE::ExtraContainerChanges>();
    container->changes->RemoveAllItems(player, nullptr, false, false, true);
    auto outfitItems = playerBase->defaultOutfit->outfitItems;
    for (auto& [key, value] : playerCopyTarget->GetInventory()) {

        bool found = false;
        auto obj = value.second->object;

        if (!obj) {
            continue;
        }

        for (auto item : outfitItems) {
            if (item && item->GetFormID() == obj->GetFormID()) {
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }
        if (player->GetInventory().contains(obj)) {
            continue;
        }
        player->AddObjectToContainer(obj, nullptr, value.first, nullptr);
    }

    std::vector<RE::ActorValue> copyActorValues = {
        RE::ActorValue::kOneHanded,
        RE::ActorValue::kTwoHanded,
        RE::ActorValue::kArchery,
        RE::ActorValue::kBlock,
        RE::ActorValue::kSmithing,
        RE::ActorValue::kHeavyArmor,
        RE::ActorValue::kLightArmor,
        RE::ActorValue::kPickpocket,
        RE::ActorValue::kLockpicking,
        RE::ActorValue::kSneak,
        RE::ActorValue::kAlchemy,
        RE::ActorValue::kSpeech,
        RE::ActorValue::kAlteration,
        RE::ActorValue::kConjuration,
        RE::ActorValue::kDestruction,
        RE::ActorValue::kIllusion,
        RE::ActorValue::kRestoration,
        RE::ActorValue::kEnchanting,
        RE::ActorValue::kHealth,
        RE::ActorValue::kMagicka,
        RE::ActorValue::kStamina,
        RE::ActorValue::kCarryWeight,
    };


    for (auto av : copyActorValues){
        auto value = playerCopyTarget->AsActorValueOwner()->GetActorValue(av);
        if (value != 0) {
            player->AsActorValueOwner()->SetActorValue(av, value);
        }
    }

    if (startAtNPCLocation) {
        SKSE::GetTaskInterface()->AddTask([]() {
            auto player = RE::PlayerCharacter::GetSingleton();
            player->MoveTo(playerCopyTarget);
        });
    } else {
        SKSE::GetTaskInterface()->AddTask([]() {
            auto player = RE::PlayerCharacter::GetSingleton();
            player->CenterOnCell(Configuration::StartLocation.c_str());
        });
    }
    
}

static inline void SwitchPlayerSkeleton() {
    using func_t = void(RE::PlayerCharacter*, bool);
    const REL::Relocation<func_t> func{REL::RelocationID(39401, 40476)};
    func(RE::PlayerCharacter::GetSingleton(), true);
}

RE::TintMask* CreateMaskFromAsset(RE::TESRace::FaceRelatedData::TintAsset* asset, std::uint16_t presetIndex) {
    if (!asset) return nullptr;

    auto* mask = static_cast<RE::TintMask*>(RE::MemoryManager::GetSingleton()->Allocate(sizeof(RE::TintMask), alignof(RE::TintMask), true));

    mask->texture = &asset->texture.file;
    mask->alpha = 1.0f;

    auto skinTone = asset->texture.skinTone.underlying();
    if (skinTone == 0) return nullptr;

    mask->type.set(static_cast<RE::TintMask::Type>(skinTone));

    if (presetIndex < asset->presets.colors.size()) {
        mask->color = asset->presets.colors[presetIndex]->color;
    }

    return mask;
}

void Manager::CopyData() {
    if (!playerCopyTarget) {
        return;
    }
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerBase = player->GetActorBase();
    auto possesionBase = playerCopyTarget->GetActorBase();
    if (!possesionBase) {
        return;
    }

    if (!playerBase->headRelatedData) {
        playerBase->headRelatedData = new RE::TESNPC::HeadRelatedData();
    }
    if (!playerBase->faceData) {
        playerBase->faceData = new RE::TESNPC::FaceData();
    }

    if (playerBase->headRelatedData && possesionBase->headRelatedData) {
        playerBase->headRelatedData->faceDetails = possesionBase->headRelatedData->faceDetails;
        playerBase->headRelatedData->hairColor = possesionBase->headRelatedData->hairColor;
    }
    if (playerBase->faceData && possesionBase->faceData) 
    {
        std::memcpy(playerBase->faceData->morphs, possesionBase->faceData->morphs, sizeof(playerBase->faceData->morphs));
        std::memcpy(playerBase->faceData->parts, possesionBase->faceData->parts, sizeof(playerBase->faceData->parts) );
    }

    playerBase->height = possesionBase->height;
    playerBase->weight = possesionBase->weight;
    playerBase->defaultOutfit = possesionBase->defaultOutfit;
    playerBase->race = possesionBase->race;

    if ((static_cast<std::uint32_t>(possesionBase->actorData.actorBaseFlags.get()) & static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kFemale)) != 0) 
    {
        playerBase->actorData.actorBaseFlags = static_cast<RE::ACTOR_BASE_DATA::Flag>(static_cast<std::uint32_t>(playerBase->actorData.actorBaseFlags.get()) | static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kFemale));
    } 
    else 
    {
        playerBase->actorData.actorBaseFlags = static_cast<RE::ACTOR_BASE_DATA::Flag>(static_cast<std::uint32_t>(playerBase->actorData.actorBaseFlags.get()) & ~static_cast<std::uint32_t>(RE::ACTOR_BASE_DATA::Flag::kFemale));
    }

    auto old = playerBase->headParts;
    auto current = new RE::BSTArray<RE::BGSHeadPart*>();

    for (auto i = 0; i < possesionBase->numHeadParts; i++) {
        current->push_back(possesionBase->headParts[i]);
    }

    playerBase->headParts = current->data();
    playerBase->numHeadParts = possesionBase->numHeadParts;

    RE::free(old);




    for (auto& copySourceFaction : possesionBase->factions) {
        if (!copySourceFaction.faction) continue;

        auto faction = copySourceFaction.faction;
        auto rank = copySourceFaction.rank;

        if (!player->IsInFaction(faction)) {
            player->AddToFaction(faction, rank);
            addedFactions.push_back(copySourceFaction);
        }
    }

    playerBase->SetFullName(possesionBase->fullName.c_str());
    player->SetDisplayName(possesionBase->fullName, true);

    auto playerSpells = playerBase->GetSpellList();
    auto otherSpells = possesionBase->GetSpellList();

    for (int i = 0; i < playerSpells->numSpells; i++) {
        playerSpells->RemoveSpell(playerSpells->spells[i]);
    }
    for (int i = 0; i < playerSpells->numShouts; i++) {
        playerSpells->RemoveShout(playerSpells->shouts[i]);
    }
    for (int i = 0; i < playerSpells->numlevSpells; i++) {
        playerSpells->RemoveLevSpell(playerSpells->levSpells[i]);
    }

    for (int i = 0; i < otherSpells->numSpells; i++) {
        if (auto spell = otherSpells->spells[i]) {
            if (spell->GetFormID() == GHOST_SPELL) {
                continue;
            }
            auto name = spell->GetName();

            if (spellTranslations.contains(name)) {
                spell = spellTranslations[name];
            } 

            if (!playerSpells->GetIndex(spell)) {
                playerSpells->AddSpell(spell);
            }
        }
    }
    for (int i = 0; i < otherSpells->numShouts; i++) {
        playerSpells->AddShout(otherSpells->shouts[i]);
    }
    for (int i = 0; i < otherSpells->numlevSpells; i++) {
        if (auto spell = otherSpells->levSpells[i]) {
            auto name = spell->GetName();
            if (spellTranslations.contains(name)) {
                auto spellToAdd = spellTranslations[name];
                if (!playerSpells->GetIndex(spellToAdd)) {
                    playerSpells->AddSpell(spellToAdd);
                }
            } 
        }
    }

    if (!possesionBase) {
        return;
    }

    std::string suffix = "'s Soul";
    if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
        name.erase(name.size() - suffix.size());
    }

    name = possesionBase->fullName;
    possesionBase->fullName = std::string(possesionBase->fullName) + "'s Soul";

    if (!possesionBase->actorEffects) {
        possesionBase->actorEffects = new RE::TESSpellList::SpellData();
    }

    possesionBase->actorEffects->AddSpell(RE::TESForm::LookupByID<RE::SpellItem>(GHOST_SPELL));

    oldCopyTarget = playerCopyTarget;
}

std::vector<RE::Character*>& Manager::GetAllCharacters() { 
    return allNpcs; 
}

void Manager::OnNewGame() {
    if (!enabled) return;
    if (!addedFactions.empty()) {
        auto player = RE::PlayerCharacter::GetSingleton();

        for (auto& addedFaction : addedFactions) {
            if (!addedFaction.faction) continue;

            if (player->IsInFaction(addedFaction.faction)) {
                player->RemoveFromFaction(addedFaction.faction);
            }
        }

        addedFactions.clear();
    }
    if (oldCopyTarget != nullptr) {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto playerBase = player->GetActorBase();
        auto possesionBase = oldCopyTarget->GetActorBase();
        if (possesionBase) {
            name = possesionBase->fullName;
            std::string suffix = "'s Soul";

            if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
                name.erase(name.size() - suffix.size());
            }
            if (possesionBase->actorEffects) {
                possesionBase->actorEffects->RemoveSpell(RE::TESForm::LookupByID<RE::SpellItem>(GHOST_SPELL));
            }
            possesionBase->SetFullName(name.c_str());
        }
        oldCopyTarget = nullptr;
    }
    doesGameStartedNow = true;
    firstRaceMenuHide = true;
}
