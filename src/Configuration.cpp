#include "Configuration.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void Configuration::Load() {
    std::ifstream file(".\\Data\\SKSE\\Plugins\\BeAnyoneAlternativeStart.json");
    if (!file.is_open()) {
        logger::error("Config file not found");
        return;
    }
    json j;
    file >> j;

    if (j.contains("EnableDevMode")) {
        EnableDevMode = j["EnableDevMode"];
        logger::info("EnableDevMode: {}", EnableDevMode);
    }
    if (j.contains("StartLocation")) {
        StartLocation = j["StartLocation"];
        logger::info("StartLocation: {}", StartLocation);
    }
}

void Configuration::Save() {
    std::filesystem::create_directories(".\\Data\\SKSE\\Plugins");

    nlohmann::json j;
    j["EnableDevMode"] = EnableDevMode;
    j["StartLocation"] = StartLocation;

    std::ofstream file(".\\Data\\SKSE\\Plugins\\BeAnyoneAlternativeStart.json");
    if (!file.is_open()) {
        logger::error("Failed to write config file");
        return;
    }

    file << j.dump(4);
}