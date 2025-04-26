#include <string>
#include <cstring> // for memcpy

#include <map>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "assetreader.hpp"

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

PSK_ChunkHeader readChunkHeader(std::ifstream &file)
{
    PSK_ChunkHeader header;

    // First 20 bytes header
    header.chunkId.resize(20);
    file.read(header.chunkId.data(), 20);
    
    // 4 bytes type flag
    // 4 bytes data size
    // 4 bytes data count
    // memcpy to avoid undefined behavior

    char intBuffer[4];

    file.read(intBuffer, 4);
    std::memcpy(&header.typeFlag, intBuffer, 4);


    file.read(intBuffer, 4);
    std::memcpy(&header.dataSize, intBuffer, 4);

    file.read(intBuffer, 4);
    std::memcpy(&header.dataCount, intBuffer, 4);

    std::cout << "Header: " << header.chunkId << " | Type Flag: " << header.typeFlag << " | Data Size: "
        << header.dataSize << " | Data Count: " << header.dataCount << std::endl;

    return header;

}

std::vector<PSK_Point> readPointsChunk(std::ifstream &pskFile, int32_t dataSize, int32_t dataCount)
{
    std::vector<PSK_Point> points(dataCount);

    char buffer[4];

    int64_t bytesRead = 0;
    for (PSK_Point &point : points)
    {
        pskFile.read(buffer, 4);
        std::memcpy(&point.x, buffer, 4);
        

        pskFile.read(buffer, 4);
        std::memcpy(&point.z, buffer, 4);
        
        pskFile.read(buffer, 4);
        std::memcpy(&point.y, buffer, 4);
        
        bytesRead += 12;
    }

    if (bytesRead != (dataSize * dataCount))
    {
        std::cout << "Warning: incorrect number of bytes read from points chunk" << std::endl;
    }

    return points;
}

std::vector<PSK_Wedge> readWedgesChunk(std::ifstream &pskFile, int32_t dataSize, int32_t dataCount)
{

    std::vector<PSK_Wedge> wedges(dataCount);

    char buffer[4];
    int64_t bytesRead = 0;
    for (PSK_Wedge &wedge : wedges)
    {
        pskFile.read(buffer, 4);
        std::memcpy(&wedge.pointIndex, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&wedge.u, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&wedge.v, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&wedge.materialIndex, buffer, 4);

        bytesRead += 16;
    }

    if (bytesRead != (dataSize * dataCount))
    {
        std::cout << "Warning: incorrect number of bytes read from wedges chunk(Expected: " << dataSize * dataCount << " | Read: "<< bytesRead << ")" << std::endl;
    }

    return wedges;

}

std::vector<PSK_Face> readFacesChunk(std::ifstream &pskFile, std::string headerId, int32_t dataSize, int32_t dataCount)
{

    std::vector<PSK_Face> faces(dataCount);

    char buffer[4] = {0, 0, 0, 0};
    int64_t bytesRead = 0;

    if (headerId == "FACE0000")
    {
        for (PSK_Face &face : faces)
        {
            pskFile.read(buffer, 2); 
            std::memcpy(&face.wedge0, buffer, 4);            

            pskFile.read(buffer, 2); 
            std::memcpy(&face.wedge1, buffer, 4);            

            pskFile.read(buffer, 2); 
            std::memcpy(&face.wedge2, buffer, 4);

            buffer[1] = 0;
            pskFile.read(buffer, 1);
            std::memcpy(&face.materialIndex, buffer, 1);

            pskFile.read(buffer, 1);
            std::memcpy(&face.auxMaterialIndex, buffer, 1);

            pskFile.read(buffer, 4);
            std::memcpy(&face.smoothingGroups, buffer, 4);


            bytesRead += 12;
        }
    }
    else if (headerId == "FACE0032")
    {
        std::cout << "Umodel face chunk detected" << std::endl;
        for (PSK_Face &face : faces)
        {
            pskFile.read(buffer, 4); 
            std::memcpy(&face.wedge0, buffer, 4);            

            pskFile.read(buffer, 4); 
            std::memcpy(&face.wedge1, buffer, 4);            

            pskFile.read(buffer, 4); 
            std::memcpy(&face.wedge2, buffer, 4);

            buffer[1] = 0;
            buffer[2] = 0;
            buffer[3] = 0;

            pskFile.read(buffer, 1);
            std::memcpy(&face.materialIndex, buffer, 1);

            pskFile.read(buffer, 1);
            std::memcpy(&face.auxMaterialIndex, buffer, 1);

            pskFile.read(buffer, 4);
            std::memcpy(&face.smoothingGroups, buffer, 4);
            bytesRead += 18;
        }
    }

    if (bytesRead != (dataSize * dataCount))
    {
        std::cout << "Warning: incorrect number of bytes read from faces chunk(Expected: " << dataSize * dataCount << " | Read: "<< bytesRead << ")" << std::endl;
    }

    return faces;

}

std::vector<PSK_Material> readMaterialsChunk(std::ifstream &pskFile, std::string headerId, int32_t dataSize, int32_t dataCount)
{
    std::vector<PSK_Material> materials(dataCount);

    char buffer[4];
    for (PSK_Material &material : materials)
    {
        material.name.resize(64);
        pskFile.read(material.name.data(), 64);
        rtrim(material.name);

        pskFile.read(buffer, 4);
        std::memcpy(&material.textureIndex, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.polyFlags, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.auxMaterial, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.auxFlags, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.lodBias, buffer, 4);

        pskFile.read(buffer, 4);
        std::memcpy(&material.lodStyle, buffer, 4);
    }

    return materials;
}

PSK_MeshData* ReadPSKFile(const std::string &pskPath)
{
    std::ifstream pskFile(pskPath, std::ios::ate | std::ios::binary);

    
    if (!pskFile.is_open())
    {
        std::cout << "Failed to open file: " << pskPath << "\n";
        throw std::runtime_error("failed to open psk file");
    }
    
    size_t filesize = (size_t) pskFile.tellg();
    pskFile.seekg(0);
    std::cout << "File \"" << pskPath << "\" loaded (" << filesize / 1000 << " KB)\n";
    
    
    PSK_MeshData *data = new PSK_MeshData;
    
    // Set directory
    int lastSlashIndex = 0;
    for (int i = 0; i < pskPath.size(); i++)
    {
        if (pskPath[i] == '/') lastSlashIndex = i;
    }
    data->directory = pskPath.substr(0, lastSlashIndex);

    // Load data
    while (pskFile.tellg() != filesize)
    {
        PSK_ChunkHeader header = readChunkHeader(pskFile);
        std::string id = header.chunkId.substr(0, 8);
        if (id == "PNTS0000")
        {
            data->points = readPointsChunk(pskFile, header.dataSize, header.dataCount);
            continue;
        };

        if (id == "VTXW0000")
        {
            data->wedges = readWedgesChunk(pskFile, header.dataSize, header.dataCount);
            continue;
        }

        if (id == "FACE0000" || id == "FACE3200")
        {
            data->faces = readFacesChunk(pskFile, id, header.dataSize, header.dataCount);
            continue;
        }

        if (id == "MATT0000")
        {
            data->materials = readMaterialsChunk(pskFile, id, header.dataSize, header.dataCount);
            continue;
        }

        pskFile.seekg( (size_t) pskFile.tellg() + (header.dataCount * header.dataSize) );
    }

    pskFile.close();

    return data;
}

std::map<std::string, std::string> ReadKeyValueFile(const std::filesystem::path &filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    size_t filesize = (size_t) file.tellg();
    file.seekg(0);
     
    if (!file.is_open())
    {
        std::cout << "Failed to open file: " << filePath << "\n";
        throw std::runtime_error("failed to open key value file");
    }

    std::map<std::string, std::string> kvMap;

    char buffer;
    while (file.tellg() < filesize)
    {
        std::string key, value;

        // Read key
        while (file.tellg() < filesize)
        {
            file.read(&buffer, 1);
            if (buffer == '=') break;

            key += buffer;
        }

        while (file.tellg() < filesize)
        {
            file.read(&buffer, 1);
            if (buffer == '\n') break;

            value += buffer;
        }

        // fucking windows line enders
        rtrim(value);

        kvMap[key] = value;
    }

    file.close();
    return kvMap;
}