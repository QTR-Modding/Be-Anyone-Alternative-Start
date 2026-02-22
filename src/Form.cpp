#include "Form.h"

RE::FormID Form::GetIdFromString(const std::string input) {
    auto pos = input.find('~');
    if (pos == std::string::npos) return 0;

    std::string modName = input.substr(0, pos);
    std::string formPart = input.substr(pos + 1);

    std::uint32_t formID = 0;
    if (formPart.rfind("0x", 0) == 0 || formPart.rfind("0X", 0) == 0) {
        formID = static_cast<std::uint32_t>(std::stoul(formPart, nullptr, 16));
    }
    else {
        formID = static_cast<std::uint32_t>(std::stoul(formPart));
    }

    auto id = RE::TESDataHandler::GetSingleton()->LookupFormID(formID, modName);
    return id;
}
