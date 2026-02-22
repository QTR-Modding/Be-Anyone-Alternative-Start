#include "FileWritter.h"

FileWritter::FileWritter(std::string& fileName) {
    if (!std::filesystem::exists(fileName)) {
        std::ofstream createFile(fileName);
    }
    file.open(fileName, std::ios::binary | std::ios::app);
}

FileWritter::~FileWritter() {
    if (file.is_open()) {
        file.close();
    }
}

void FileWritter::WriteUInt32(uint32_t value) { file.write(reinterpret_cast<const char*>(&value), sizeof(uint32_t)); }
void FileWritter::WriteUInt8(uint8_t value) { file.write(reinterpret_cast<const char*>(&value), sizeof(uint8_t)); }
void FileWritter::WriteBool(bool value) {
    uint8_t v = value ? 1 : 0;
    file.write(reinterpret_cast<const char*>(&v), sizeof(uint8_t));
}

bool FileWritter::IsOpen() { return file.is_open(); }

void FileWritter::WriteString(const std::string& value) {
    WriteUInt32(value.size());
    file.write(value.data(), value.size());
}

void FileWritter::WriteFormId(RE::FormID id) {
    if (id == 0) {
        WriteUInt8(0);
        return;
    }
    const auto dataHandler = RE::TESDataHandler::GetSingleton();
    auto modId = (id >> 24) & 0xff;

    if (modId == 0xff) {
        logger::trace("modId: {:x}", modId);
        logger::trace("dynamicform: {:x}", id);
        WriteUInt8(2);
        WriteUInt32(id);
        return;
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
            WriteUInt8(1);
            WriteString(fileName.c_str());
            WriteUInt32(localId);
            return;
        }
    } else {
        auto file = dataHandler->LookupLoadedModByIndex(modId);

        if (file) {
            auto localId = id & 0xFFFFFF;
            logger::trace("fileName: {}", file->fileName);
            logger::trace("localId: {:x}", localId);
            std::string fileName = file->fileName;
            WriteUInt8(1);
            WriteString(fileName.c_str());
            WriteUInt32(localId);
            return;
        }
    }

    WriteUInt8(0);
}
