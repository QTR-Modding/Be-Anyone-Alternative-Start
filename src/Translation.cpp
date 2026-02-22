#include <nlohmann/json.hpp>

#include "Translation.h"

using json = nlohmann::json;

namespace Translation {
    constexpr const char* translationsFolder = ".\\Data\\SKSE\\Plugins\\BeAnyoneAlternativeStartStrings.json";
    const char* defaultTranslation = "missing translation";
    static inline std::map<std::string, const char*> translations;
}

void Translation::Install() {
    try {
        std::ifstream file(translationsFolder);
        nlohmann::json j;
        if (!file.is_open()) {
            logger::error("Failed to read translation file");
            return;
        }
        file >> j;
        logger::trace("reading translation");
        if (!j.is_object()) {
            logger::trace("translation json: {} must be an object", translationsFolder);
        }
        for (auto& [key, value] : j.items()) {
            logger::trace("{} -> {}", key, value);
            std::string v = value;
            translations[key] = strdup(v.c_str());
        }
    } catch (const json::parse_error& e) {
        logger::error("Failed to parse translation file: {}; {}", translationsFolder, e.what());
    } catch (const std::exception& e) {
        logger::error("Failed to parse translation file: {}; Exception: {}", translationsFolder, e.what());
    }
}

const const char* Translation::Get(std::string key) {
    if (translations.contains(key)) {
        return translations[key];
    }
    return defaultTranslation;
}