#include "Folder.h"

std::vector<std::filesystem::path> Folder::GetAllFiles(const char* folderPath, const char* fileEnding) {
    std::vector<std::filesystem::path> result;

    std::string ending = fileEnding;
    std::transform(ending.begin(), ending.end(), ending.begin(), [](unsigned char c) { return std::tolower(c); });

    auto length = ending.length();

    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) continue;

        std::string filename = entry.path().filename().string();
        std::transform(filename.begin(), filename.end(), filename.begin(), [](unsigned char c) { return std::tolower(c); });

        if (filename.size() >= length && filename.compare(filename.size() - length, length, ending) == 0) {
            result.push_back(entry.path());
        }
    }

    return result;
}