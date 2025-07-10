#pragma once
#include "pch.h"

static std::vector<std::string> loadPlayerNames(const std::string &filename)
{
    std::vector<std::string> players;
    std::ifstream file(filename);
    std::string line;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            if (!line.empty())
            {
                players.push_back(line);
            }
        }
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }

    return players;
}