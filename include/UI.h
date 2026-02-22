#pragma once

#include "SKSEMenuFramework.h"
#include "Configuration.h"

namespace UI {
    void Register();
    void __stdcall RenderMenu();
    namespace NewGame {
        inline int index = 0;
        inline int maxIndex = 4;
        void Next();
        void Previus();
        void Update();
        inline MENU_WINDOW chooseCharacterWindow = nullptr;
        inline MENU_WINDOW chooseSideWindow = nullptr;
        inline MENU_WINDOW reviewWindow = nullptr;
        void __stdcall RenderChooseCharacterWindow();
        void __stdcall RenderChooseSideWindow();
        void __stdcall RenderReviewWindow();
        void __stdcall RenderBackground();
    }
};