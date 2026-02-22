#pragma once
#include <unordered_set>

class IgnoreManager {
public:
    static bool DoesIgnore(RE::TESObjectREFR* reference);
    static void Install();
private:
    static inline std::unordered_set<RE::FormID> igonreReferences;
    static inline std::unordered_set<RE::FormID> ignroeCells;

};