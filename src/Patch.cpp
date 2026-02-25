#include "Patch.h"


bool Patch::IsLiveAnotherLifeInstalled() {
    constexpr auto dllPath = "Data/alternate start - live another life.esp";
    return std::filesystem::exists(dllPath);
}

bool Patch::IsRaceMenuInstalled() {
    constexpr auto dllPath = "Data/RaceMenu.esp";
    return std::filesystem::exists(dllPath);
}

bool Patch::IsSubtitlesInstalled() {
    constexpr auto dllPath = "Data/SKSE/Plugins/Subtitles.dll";
    return std::filesystem::exists(dllPath);
}