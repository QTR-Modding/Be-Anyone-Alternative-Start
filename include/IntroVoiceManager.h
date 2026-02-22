#pragma once

class IntroVoiceManager {
public:

    struct Sequence {
        std::string speakerName;
        std::string subtitiles;
        float duration = 0;
        RE::BGSSoundDescriptorForm* audio = nullptr;
    };
    struct SequenceGroup {
        std::vector<Sequence> sequences;
    };
    static void Install();
    static SequenceGroup* GetRandomGroup(RE::FormID character);

private:
    static inline std::unordered_map<RE::FormID, std::vector<SequenceGroup>> groups;
};