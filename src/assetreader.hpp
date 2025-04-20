#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <map>
#include <filesystem>

enum PSK_FileChunkName
{
    PSK_CHUNK_HEADER = 0,
    PSK_CHUNK_POINTS,
    PSK_CHUNK_WEDGES,
    PSK_CHUNK_FACES,
    PSK_CHUNK_FACES32,
    PSK_CHUNK_MATERIALS,
    PSK_CHUNK_BONES,
    PSK_CHUNK_WEIGHTS,
    PSK_CHUNK_EXTRA_UV0,
    PSK_CHUNK_EXTRA_UV1,
    PSK_CHUNK_EXTRA_UV2
};


struct PSK_ChunkHeader
{
    std::string chunkId;
    int32_t typeFlag;
    int32_t dataSize;
    int32_t dataCount;
};

struct PSK_Point
{
    float x;
    float y;
    float z;
};

struct PSK_Wedge
{
    uint32_t pointIndex;
    float u;
    float v;
    int32_t materialIndex;
};

struct PSK_Face
{
    int32_t wedge0;
    int32_t wedge1;
    int32_t wedge2;
    
    int8_t materialIndex;
    int8_t auxMaterialIndex;
    int32_t smoothingGroups;
};

struct PSK_Material
{
    std::string name;

    int32_t textureIndex;
    int32_t polyFlags;
    int32_t auxMaterial;
    int32_t auxFlags;
    int32_t lodBias;
    int32_t lodStyle;
};

struct PSK_MeshData
{
    std::string directory;

    std::vector<PSK_Point> points;
    std::vector<PSK_Wedge> wedges;
    std::vector<PSK_Face> faces;
    std::vector<PSK_Material> materials;
};



PSK_MeshData* ReadPSKFile(const std::string &pskPath);
std::map<std::string, std::string> ReadKeyValueFile(const std::filesystem::path &filePath);