#include "FormID.h"
#include "SKSEMenuFramework.h"

RE::FormID FormID::GetLocal(RE::FormID id) {
    if (id == 0) {
        return 0;
    }
    const auto dataHandler = RE::TESDataHandler::GetSingleton();
    auto modId = (id >> 24) & 0xff;

    if (modId == 0xff) {
        logger::trace("modId: {:x}", modId);
        logger::trace("dynamicform: {:x}", id);
        return id;
    }

    else if (modId == 0xfe) {
        auto lightId = (id >> 12) & 0xFFF;
        logger::trace("lightId: {:x}", lightId);
        auto file = dataHandler->LookupLoadedLightModByIndex(lightId);
        if (file) {
            auto localId = id & 0xFFF;
            logger::trace("fileName: {}", file->fileName);
            logger::trace("localId: {:x}", localId);
            std::string fileName = file->fileName;
            return localId;
        }
    } else {
        auto file = dataHandler->LookupLoadedModByIndex(modId);

        if (file) {
            auto localId = id & 0xFFFFFF;
            logger::trace("fileName: {}", file->fileName);
            logger::trace("localId: {:x}", localId);
            std::string fileName = file->fileName;
            return localId;
        }
    }

    return 0;
}

std::string FormID::GetFileName(RE::TESForm* form) {
    if (form == 0) {
        return "unknown";
    }
    auto id = form->GetFormID();
    if (id == 0) {
        return "unknown";
    }
    const auto dataHandler = RE::TESDataHandler::GetSingleton();
    auto modId = (id >> 24) & 0xff;

    if (modId == 0xff) {
        logger::trace("modId: {:x}", modId);
        logger::trace("dynamicform: {:x}", id);
        return "unknown";
    }

    else if (modId == 0xfe) {
        auto lightId = (id >> 12) & 0xFFF;
        logger::trace("lightId: {:x}", lightId);
        auto file = dataHandler->LookupLoadedLightModByIndex(lightId);
        if (file) {
            auto localId = id & 0xFFF;
            logger::trace("fileName: {}", file->fileName);
            logger::trace("localId: {:x}", localId);
            std::string fileName = file->fileName;
            return fileName;
        }
    } else {
        auto file = dataHandler->LookupLoadedModByIndex(modId);

        if (file) {
            auto localId = id & 0xFFFFFF;
            logger::trace("fileName: {}", file->fileName);
            logger::trace("localId: {:x}", localId);
            std::string fileName = file->fileName;
            return fileName;
        }
    }

    return "unknown";
}

bool HasFileName(RE::TESForm* form) {
    if (!form) {
        return false;
    }

    if (!form->sourceFiles.array || form->sourceFiles.array->empty()) {
        return false;
    }

    return true;
}

void FormID::Copy(RE::TESForm* item) {
    if (!HasFileName(item)) {
        return;
    }
    std::string formIdStr = std::format("{:08X}", GetLocal(item->GetFormID()));
    while (!formIdStr.empty() && formIdStr.front() == '0') formIdStr.erase(formIdStr.begin());
    ImGuiMCP::SetClipboardText(std::format("{}~0x{}", GetFileName(item), formIdStr.c_str()).c_str());
}
