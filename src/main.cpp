#include <libloaderapi.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <optional>
#include <map>
#include <fstream>

#include "assetreader.hpp"

std::filesystem::path GetExecutablePath()
{
    char buffer[1024];
    GetModuleFileNameA(NULL, buffer, 1024);
    return std::filesystem::path(buffer);
}

std::string GetRelativePath(std::filesystem::path p)
{
    std::string input = GetExecutablePath().parent_path().append("in").generic_string();
    std::string s = p.generic_string();

    return s.substr(input.size() + 1, std::string::npos);
}

inline void rtrim(std::string &s)
{
    for (int i = s.size() - 1; i >= 0; i--)
    {
        if (s[i] != 0 && s[i] != '\n' && s[i] != '\r')
        {
            if (i != s.size() - 1) s.erase(i + 1);
            break;
        }
    }
}

void writeMappingFile(std::filesystem::path &meshPath, std::map<std::string, std::filesystem::path> &materialMap, std::map<std::string, std::filesystem::path> &textureMap)
{
    std::ofstream mappingFile;

    std::filesystem::path outputFileName = meshPath;
    outputFileName.replace_extension(".skmap");

    std::cout << "Writing mapping file (" << outputFileName << ")\n";


    mappingFile.open(outputFileName, std::ostream::binary);
    if (!mappingFile.is_open())
    {
        std::cout << "Error creating mapping file: " << outputFileName << std::endl;
        throw std::runtime_error("error creating file");
    }

    for (auto &kvPair : materialMap)
    {
        std::string value = kvPair.second.string();
        if (value.size() > 0)
        {
            value = GetRelativePath(value);
        }

        std::string line = kvPair.first + "=" + value;
        mappingFile.write(line.c_str(), line.size());
        mappingFile.write("\n", 1);
    }

    for (auto &kvPair : textureMap)
    {
        std::string value = kvPair.second.string();
        if (value.size() > 0)
        {
            value = GetRelativePath(value);
        }

        std::string line = kvPair.first + "=" + value;
        mappingFile.write(line.c_str(), line.size());
        mappingFile.write("\n", 1);
    }

    std::cout << "Write successful!" << std::endl;

    mappingFile.close();
}


int main(int argc, char *argv[])
{
    bool autoMappingEnabled = false;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp("--auto", argv[i]) == 0)
        {
            autoMappingEnabled = true;
            std::cout << "Automapping Enabled\n";
        }
    }

    std::map<std::string, std::filesystem::path> materialsMap;
    std::map<std::string, std::filesystem::path> texturesMap;

    std::map<std::string, std::optional<std::filesystem::path>> autoMap;

    std::filesystem::path input = GetExecutablePath().parent_path().append("in");
    
    // Find Mesh file to pack
    std::cout << "Select file to load: \n";
    std::vector<std::filesystem::path> meshPaths;
    std::vector<std::filesystem::path> materialPaths;
    std::vector<std::filesystem::path> texturePaths;

    for (const std::filesystem::directory_entry &dirEntry : std::filesystem::recursive_directory_iterator(input))
    {
        if (!dirEntry.is_regular_file()) continue;

        std::string extension = dirEntry.path().extension().string();
        if (autoMappingEnabled && (extension == ".psk" || extension == ".mat" || extension == ".tga"))
        {
            std::string stem = dirEntry.path().stem().string();
            if (autoMap.count(stem))
            {
                // Duplicate detected
                // this will set hasValue to false
                autoMap[stem].reset();
            }
            else 
            {
                autoMap[stem] = dirEntry.path();
            }
        }

        if (dirEntry.path().extension() == ".psk")
        {
            meshPaths.push_back(dirEntry.path());
            std::cout << meshPaths.size() << ") " << GetRelativePath(dirEntry.path()) << "\n";
        }

        if (dirEntry.path().extension() == ".mat")
        {
            materialPaths.push_back(dirEntry.path());
        }

        if (dirEntry.path().extension() == ".tga")
        {
            texturePaths.push_back(dirEntry.path());
        }
    }

    int selection;
    std::cin >> selection;
    if (selection < 1 || selection > meshPaths.size())
    {
        std::cout << "Invalid input noob\n";
        return 0;
    }
    
    std::filesystem::path mesh = meshPaths[selection - 1];


    std::cout << "\nPacking Mesh: " << mesh.filename() << std::endl;

    PSK_MeshData *mData = ReadPSKFile(mesh.string());
    std::cout << std::endl;
    for (int i = 0; i < materialPaths.size(); i++)
    {
        if (autoMappingEnabled && autoMap.count(materialPaths[i].stem().string()) && autoMap[materialPaths[i].stem().string()].has_value())
            continue;

        std::cout << i+1 << ") " << materialPaths[i].filename() << " (" << GetRelativePath(materialPaths[i]) << ")\n";
    }

    std::cout << "\n" << mData->materials.size() << " material keys found inside mesh file.\n";

    std::cout << "Mapping material keys...\n";
    for (auto &material : mData->materials)
    {
        if (autoMappingEnabled && autoMap.count(material.name) && autoMap[material.name].has_value())
        {
            materialsMap[material.name] = autoMap[material.name].value();
            continue;
        }

        selection = -1;
        std::cout << material.name << ": ";
        std::cin >> selection;

        while (selection < 1 || selection > materialPaths.size())
        {
            std::cout << "INVALID ENTRY! TRY AGAIN: ";
            std::cin >> selection;
        }


        materialsMap[material.name] = materialPaths[selection - 1];
    }
    std::cout << "Material mapping complete!\n";

    std::cout << std::endl;


    // Mapping texture keys
    for (int i = 0; i < texturePaths.size(); i++)
    {
        if (autoMappingEnabled && autoMap.count(texturePaths[i].stem().string()) && autoMap[texturePaths[i].stem().string()].has_value())
            continue;

        std::cout << i+1 << ") " << texturePaths[i].filename() << " (" << GetRelativePath(texturePaths[i]) << ")" << std::endl;
    }

    std::cout << "Mapping textures..." << std::endl;
    for (const auto &materialPair : materialsMap)
    {
        std::map<std::string, std::string> materialFileData = ReadKeyValueFile(materialPair.second);

        for (const auto &materialFileLine : materialFileData)
        {
            if (texturesMap.count(materialFileLine.second))
                continue;

            if (autoMappingEnabled && autoMap.count(materialFileLine.second) && autoMap[materialFileLine.second].has_value())
            {
                texturesMap[materialFileLine.second] = autoMap[materialFileLine.second].value();
                continue;
            }

            // The fucking cubes
            if (materialFileLine.first == "Cube")
            {
                std::cout << "Warning! Cube " << materialFileLine.second << " will need manual definition for mesh " << GetRelativePath(mesh) << std::endl;
                texturesMap[materialFileLine.second] = "";
                continue;
            }

            if (autoMappingEnabled)
            {
                std::cout << "Manual mapping required for: ";
            }

            std::cout << materialFileLine.second << "(" << GetRelativePath(mesh) << ")" << std::endl;
            selection = -1;
            std::cin >> selection;

            std::cout << materialFileLine.second << " = " << texturePaths[selection - 1] << std::endl;
            while (selection < 1 || selection > texturePaths.size())
            {
                std::cout << "INVALID ENTRY! TRY AGAIN: ";
                std::cin >> selection;
            }

            texturesMap[materialFileLine.second] = texturePaths[selection - 1];
        }
    }

    std::cout << "Texture mapping complete!\n" << std::endl;

    writeMappingFile(mesh, materialsMap, texturesMap);

    return 0;
}
