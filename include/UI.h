#pragma once

#include "SKSEMenuFramework.h"
#include "Configuration.h"

namespace UI {
    void Register();
    void __stdcall RenderMenu();
    namespace NewGame {
        inline REL::Relocation<void(*)()> startNewGame;
        inline int index = 0;
        inline int maxIndex = 5;
        void Next();
        void Previus();
        void Update();
        inline MENU_WINDOW startAsNPCWindow = nullptr;
        inline MENU_WINDOW chooseCharacterWindow = nullptr;
        inline MENU_WINDOW chooseSideWindow = nullptr;
        inline MENU_WINDOW reviewWindow = nullptr;
        void __stdcall RenderStartAsNPC();
        void __stdcall RenderChooseCharacterWindow();
        void __stdcall RenderChooseSideWindow();
        void __stdcall RenderReviewWindow();
        void __stdcall RenderBackground();
    }
};