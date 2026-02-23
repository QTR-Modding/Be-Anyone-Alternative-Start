#pragma once

class Quest {
public:
    static void CompleteQuest(RE::FormID quest);
    static bool IsDone(RE::FormID formId);
    static void SetQuestSage(RE::FormID formId, std::vector<uint32_t> stages);
    static void CallQuestVoidFunction(RE::FormID formId, const char* className, const char* functionName);
    static void ForceAliasRef(RE::FormID questFormId, uint32_t aliasId, RE::FormID targetRefId);
};