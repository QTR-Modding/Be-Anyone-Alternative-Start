#pragma once

typedef void(__stdcall* SpeachEndCallback)();
#include "FileWritter.h"
#include "FileReader.h"
struct Speech {
    std::string speaker;
    std::string text;
    float timeoutInSeconds;
    RE::BGSSoundDescriptorForm* audio;
    std::string GetFullText() const {
        auto iniSettings = RE::INISettingCollection::GetSingleton();
        bool showSpeakerName = iniSettings->GetSetting("bShowSubtitleSpeakerName:Interface")->GetBool();
        std::string fullText;
        if (showSpeakerName) {
            uint32_t speakerNameColor = iniSettings->GetSetting("iSubtitleSpeakerNameColor:Interface")->GetUInt();
            return std::format("<font color='#{:06X}'>{}</font>: {}", speakerNameColor, speaker, text);
        } else {
            return text;
        }
    }
};
class SpeechManager {
    static inline bool doCallbacks = false;
    static inline std::vector<SpeachEndCallback> speechEndCallbacks;
    // The how to display a subtitle is found here, this manager would not be possible
    // without kpvw amazing work
    // https://github.com/WaterFace/subtitles

    static RE::GFxValue GetHudMenu() {
        auto hudMenu = RE::UI::GetSingleton()->GetMenu<RE::HUDMenu>(RE::HUDMenu::MENU_NAME);
        return hudMenu->GetRuntimeData().root;
    }
    static inline std::vector<Speech> subtitleQueue;

    static inline std::thread subtitleThread;
    static inline std::mutex queueMutex;
    static inline std::atomic<bool> threadCreated = false;
    static inline std::atomic<bool> isFirst = false;
    static inline RE::BSSoundHandle a_descriptor;

    static inline std::int64_t audioStartTimeInMS = 0;
    static inline std::int64_t lastAudioTimeInMS = 0;
    static inline std::int64_t audioDurationInMS = 0;
    static inline bool hasAudio = false;

    static inline std::int64_t GetCurrentTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}
    static void ProcessQueue() {
        Speech next;
        bool hasPaused = RE::UI::GetSingleton()->GameIsPaused();
        int64_t lastTimeInMS;
        while (true) {
            const auto currentTimeInMS = GetCurrentTimeMs();
            auto deltaTime = currentTimeInMS - lastTimeInMS;
            lastTimeInMS = currentTimeInMS; 
            const auto elapsedTimeInMS = currentTimeInMS - lastAudioTimeInMS;
            if (isFirst) {
                int waitMs = 0;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    if (subtitleQueue.empty()) {
                        hasPaused = false;
                        waitMs = 1000;
                    }
                }
                if (waitMs != 0) {
                    audioStartTimeInMS += deltaTime;
                    std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
                    continue;
                }
            }

            if (RE::UI::GetSingleton()->GameIsPaused()) {
                if (hasPaused) {
                    hasPaused = false;
                    if (hasAudio) {
                        a_descriptor.Pause();
                    }
                }
                audioStartTimeInMS += deltaTime;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            } else if(!hasPaused) {
                hasPaused = true;
                if (!isFirst) {
                    const auto text = next.GetFullText();
                    RE::GFxValue asStr(text.c_str());
                    GetHudMenu().Invoke("ShowSubtitle", nullptr, &asStr, 1);
                }
                if (hasAudio) {
                    
                    a_descriptor.Play();
                }
            }
            lastAudioTimeInMS = currentTimeInMS;

            if (!isFirst && currentTimeInMS - audioStartTimeInMS < audioDurationInMS) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            {
                std::unique_lock<std::mutex> lock(queueMutex);

                if (!isFirst && !subtitleQueue.empty()) {
                    subtitleQueue.erase(subtitleQueue.begin());
                }
                isFirst = false;

                if (subtitleQueue.empty()) {
                    if (doCallbacks) {
                        for (auto& callback : speechEndCallbacks) {
                            callback();
                        }
                    }
                    SKSE::GetTaskInterface()->AddTask([]() { GetHudMenu().Invoke("HideSubtitle", nullptr, nullptr, 0); });
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    continue;
                }

                next = subtitleQueue.front();
            }

            SKSE::GetTaskInterface()->AddTask([next]() {
                hasAudio = false;
                if (next.audio) {
                    auto* player = RE::PlayerCharacter::GetSingleton();
                    auto* audioManager = RE::BSAudioManager::GetSingleton();
                    if (audioManager->BuildSoundDataFromDescriptor(a_descriptor, next.audio)) {
                        a_descriptor.SetPosition(player->GetPosition());
                        a_descriptor.SetObjectToFollow(player->Get3D());
                        a_descriptor.SetVolume(1.0f);
                        a_descriptor.Play();
                        hasAudio = true;
                    }
                }
                const auto text = next.GetFullText();
                RE::GFxValue asStr(text.c_str());
                GetHudMenu().Invoke("ShowSubtitle", nullptr, &asStr, 1);
            });

            audioStartTimeInMS = GetCurrentTimeMs();
            lastAudioTimeInMS = audioStartTimeInMS;
            audioDurationInMS = next.timeoutInSeconds * 1000;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

public:
    static void SaveGame(FileWritter* fileWritter) {
        std::unique_lock<std::mutex> lock(queueMutex);

        fileWritter->WriteUInt32((uint32_t)subtitleQueue.size());
        for (const auto& speech : subtitleQueue) {
            fileWritter->WriteString(speech.speaker);
            fileWritter->WriteString(speech.text);
            fileWritter->WriteUInt32((uint32_t)(speech.timeoutInSeconds * 1000.0f));
            bool hasAudioDescriptor = speech.audio != nullptr;
            fileWritter->WriteBool(hasAudioDescriptor);
            if (hasAudioDescriptor) {
                fileWritter->WriteFormId(speech.audio->GetFormID());
            }
        }
    }

    static void LoadGame(FileReader* fileReader) {
        doCallbacks = false;
        audioDurationInMS = 0;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            subtitleQueue.clear();
            isFirst = true;
        }

        uint32_t count = fileReader->ReadUInt32();
        for (uint32_t i = 0; i < count; i++) {
            Speech speachFromTheQueue;
            speachFromTheQueue.speaker = fileReader->ReadString();
            speachFromTheQueue.text = fileReader->ReadString();
            uint32_t timeoutInMS = fileReader->ReadUInt32();
            speachFromTheQueue.timeoutInSeconds = timeoutInMS / 1000.0f;
            speachFromTheQueue.audio = nullptr;
            bool hasAudioDescriptor = fileReader->ReadBool();
            if (hasAudioDescriptor) {
                RE::FormID formId = fileReader->ReadFormId();
                speachFromTheQueue.audio = RE::TESForm::LookupByID<RE::BGSSoundDescriptorForm>(formId);
            }
            Add(speachFromTheQueue);
        }
    }
    static void NewGame() { 
        doCallbacks = false;
        audioDurationInMS = 0;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            subtitleQueue.clear();
            isFirst = true;
        }
    }
    static void AddSpeachEndCallback(SpeachEndCallback callback) { return 
        speechEndCallbacks.push_back(callback);
    }
    static void Add(Speech subtitle) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);

            subtitleQueue.push_back(subtitle);

            doCallbacks = true;
        }

        if (!threadCreated) {
            threadCreated = true;
            isFirst = true;
            audioDurationInMS = 0;
            subtitleThread = std::thread(ProcessQueue);
            subtitleThread.detach();
        }
    }
};