#include "IntroVoiceManager.h"
#include "Folder.h"
#include "Form.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void IntroVoiceManager::Install() {
    try {
    groups.clear();

    auto files = Folder::GetAllFiles("Data/", "_BA_INTRO_SPEACH.json");

    for (const auto& path : files) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) continue;

            logger::info("Found config file: {}", path.string());

            json data;
            file >> data;

            SequenceGroup group;
            if (data.contains("SpeachSequence") && data["SpeachSequence"].is_array()) {
                for (const auto& entry : data["SpeachSequence"]) {
                    if (!entry.is_object()) continue;

                    Sequence seq{};

                    seq.speakerName = entry.value("SpeakerName", "");
                    seq.subtitiles = entry.value("Subtitles", "");
                    seq.duration = entry.value("Duration", 0.0f);

                    seq.audio = nullptr;

                    if (entry.contains("Audio") && entry["Audio"].is_string()) {
                        std::string audioStr = entry["Audio"];
                        if (!audioStr.empty()) {
                            auto formId = Form::GetIdFromString(audioStr);
                            if (formId) {
                                seq.audio = RE::TESForm::LookupByID<RE::BGSSoundDescriptorForm>(formId);
                            }
                        }
                    }

                    group.sequences.push_back(seq);
                }
            }

            logger::info("---- Dumping SequenceGroup from file: {}", path.string());

            for (std::size_t i = 0; i < group.sequences.size(); ++i) {
                const auto& seq = group.sequences[i];

                logger::info("  Sequence [{}]", i);
                logger::info("    SpeakerName: {}", seq.speakerName);
                logger::info("    Subtitles: {}", seq.subtitiles);
                logger::info("    Duration: {}", seq.duration);
                logger::info("    Audio: {}", seq.audio ? seq.audio->GetFormID() : 0);
            }

            if (data.contains("RequiredActor") && !data["RequiredActor"].is_null()) {
                auto requiredArray = data["RequiredActor"];
                if (requiredArray.is_array()) {
                    logger::error("string is empty");
                    throw std::runtime_error("RequiredActor must be an array");
                }
                for (std::string requiredStr : requiredArray) {
                    if (!requiredStr.empty()) {
                        continue;
                    }
                    auto formId = Form::GetIdFromString(requiredStr);
                    if (formId) {
                        groups[formId].push_back(group);
                    } else {
                        logger::error("form not found");
                    }
                }
            } else {
                groups[0].push_back(group);
            }


        } catch (const json::parse_error& e) {
            logger::error("On file: {}; {}", path.string(), e.what());
        } catch (const std::exception& e) {
            logger::error("On file: {}; Exception: {}", path.string(), e.what());
        }
    }
    } catch (const std::exception& e) {
        logger::error("On StartObjectManager; Exception: {}", e.what());
    }
}


IntroVoiceManager::SequenceGroup* IntroVoiceManager::GetRandomGroup(RE::FormID character) {
    auto it = groups.find(character);

    if (it == groups.end()) {
        it = groups.find(0);
        if (it == groups.end()) return nullptr;
    }

    auto& vec = it->second;
    if (vec.empty()) return nullptr;

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, vec.size() - 1);

    return &vec[dist(rng)];
}
