#include "Racemenu.h"
#include "Patch.h"

void Racemenu::Modifty(const char* name) {
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

    if (Patch::IsRaceMenuInstalled()) {
        RE::GFxValue textEntryField;
        if (RaceSexPanelsInstance.GetMember("textEntry", &textEntryField)) {
            RE::GFxValue textInput;
            if (textEntryField.GetMember("TextInputInstance", &textInput)) {
                textInput.SetText(name);
                textInput.SetMember("focused", true); 
            }

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
                textInput.SetMember("focused", true);
            }

            RE::GFxValue fadeArgs[1];
            fadeArgs[0].SetBoolean(true);
            RaceSexPanelsInstance.Invoke("FadeTextEntry", nullptr, fadeArgs, 1);
        }
    }
    RE::ControlMap::GetSingleton()->AllowTextInput(true);
}

void Racemenu::ModiftyWhenOpen() {
    std::thread([] {
        while (true) {
            if (RE::UI::GetSingleton()->IsMenuOpen(RE::RaceSexMenu::MENU_NAME)) {
                Modifty(RE::PlayerCharacter::GetSingleton()->GetName());
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}
