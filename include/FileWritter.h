#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

class FileWritter {
private:
    std::ofstream file;
public:
    FileWritter(std::string& fileName);
    ~FileWritter();
    void WriteUInt32(uint32_t value);
    void WriteUInt8(uint8_t value);
    void WriteBool(bool value);
    bool IsOpen();
    void WriteString(const std::string& value);
    void WriteFormId(RE::FormID id);
};