#pragma once

namespace FormID {
    RE::FormID GetLocal(RE::FormID id);
    std::string GetFileName(RE::TESForm* form);
    void Copy(RE::TESForm* item);
}