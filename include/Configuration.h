#pragma once

namespace Configuration {
    inline bool EnableDevMode = false;
    inline std::string StartLocation = "";
    inline int GroupsPerPage = 30;
    void Load();
    void Save();
};