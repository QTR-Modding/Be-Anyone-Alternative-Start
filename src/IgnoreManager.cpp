#include "IgnoreManager.h"
#include "Folder.h"
#include "Form.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool IgnoreManager::DoesIgnore(RE::TESObjectREFR* reference) {

    if (!reference) {
        return true;
    }
    if (igonreReferences.contains(reference->GetFormID())){
        return true;
    } 
    if (reference->GetParentCell()) {
        if (ignroeCells.contains(reference->GetParentCell()->GetFormID())) {
            return true;
        } 
    }
    return false;
}

void IgnoreManager::Install() {
    igonreReferences.clear();
    ignroeCells.clear();
    auto files = Folder::GetAllFiles(".\\Data\\", "_BA_IGNORE.json");
    for (const auto& path : files) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) continue;
            logger::info("Found config file: {}", path.string());
            json data;
            file >> data;


        if (data.contains("IgnoreCells") && !data["IgnoreCells"].is_null()) {
            auto list = data["IgnoreCells"];
            if (!list.is_array()) {
                throw std::runtime_error("IgnoreCells must be an array");
            }
            for (std::string item : list) {
                if (item.empty()) {
                    logger::error("string is empty");
                    continue;
                }
                auto formId = Form::GetIdFromString(item);

                logger::info("ignore cell {:x}", formId);

                ignroeCells.insert(formId);
            }
        }
        if (data.contains("IgnoreReferences") && !data["IgnoreReferences"].is_null()) {
            auto list = data["IgnoreReferences"];
            if (!list.is_array()) {
                throw std::runtime_error("IgnoreReferences must be an array");
            }
            for (std::string item : list) {
                if (item.empty()) {
                    logger::error("string is empty");
                    continue;
                }
                auto formId = Form::GetIdFromString(item);

                logger::info("ignore reference {:x}", formId);

                igonreReferences.insert(formId);
            }
        }
        } catch (const json::parse_error& e) {
            logger::error("On file: {}; {}", path.string(), e.what());
        } catch (const std::exception& e) {
            logger::error("On file: {}; Exception: {}", path.string(), e.what());
        }
    }
}