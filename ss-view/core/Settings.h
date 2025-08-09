#pragma once

#include "AppData.h"

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

class Settings final
{
    Settings() {}

public:

    static Settings& Get() { static Settings instance; return instance; }

    ~Settings() = default;
    Settings(const Settings& other) = delete;
    Settings(const Settings&& other) = delete;
    Settings operator=(const Settings& other) = delete;
    Settings operator=(const Settings&& other) = delete;

    std::string fontPath = "C:/Windows/Fonts/CourierPrime-Regular.ttf";

    void Load()
    {
        std::filesystem::path path = _APPDATA_ + "\\SimpleScript\\view.ini";

        if (!std::filesystem::exists(path))
        {
            SaveDefault();
            return;
        }

        std::ifstream file(path);

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty())
                continue;

            std::stringstream linestream(line);
            std::string cell;
            std::getline(linestream, cell, '=');
            if (cell == "fontPath")
            {
                std::getline(linestream, cell);
                fontPath = cell;
            }
            else
            {
                std::cout << cell << " -- was not a recognized settings key" << std::endl;
            }
        }
    }

    void SaveDefault()
    {
        if (!std::filesystem::exists(_APPDATA_ + "\\SimpleScript"))
            std::filesystem::create_directories(_APPDATA_ + "\\SimpleScript");

        std::ofstream file(_APPDATA_ + "\\SimpleScript\\view.ini");

        file << "fontPath=" << fontPath << std::endl;
    }
};
