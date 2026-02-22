#pragma once

class Folder {
public:
    static std::vector<std::filesystem::path> GetAllFiles(const char* path, const char* ending);
};