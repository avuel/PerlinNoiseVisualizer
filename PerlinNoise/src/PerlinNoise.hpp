#pragma once

#include "imgui.h"

#include <glm/glm.hpp>

#include <vector>
#include <random>


struct NoiseSettings
{
    bool  Color  = false;
    int   Height = 32;
    float Water  = 0.4f;
    float Sand   = 0.425f;
    float Stone  = 0.55f;
    float Snow   = 0.625f;
};

class PerlinNoiseGenerator
{
public:
    PerlinNoiseGenerator(const int &seed, const int &width, const int &height,
        const int &cellsize, const int &levels, const double &attenuation)
        : m_Seed(seed), m_Width(width), m_Height(height), m_CellSize(cellsize), m_Levels(levels), m_Attenuation(attenuation)
    {
        m_InfluenceVectors.resize((size_t)width * (size_t)height);
        m_PixelData.resize((size_t)width * (size_t)height);
        srand(seed);
    }

public:
    bool GUI(size_t &width, size_t &height);
    const std::vector<double>& const GetNoise() { return m_PixelData; }
    NoiseSettings* GetNoiseSettings() { return &m_NoiseSettings; }
    

public:
    void SetHeight(const int   &height) { m_NoiseSettings.Height = height; }
    void SetWater (const float &water ) { m_NoiseSettings.Water  = water;  }
    void SetSand  (const float &sand  ) { m_NoiseSettings.Sand   = sand;   }
    void SetStone (const float &stone ) { m_NoiseSettings.Stone  = stone;  }
    void SetSnow  (const float &snow  ) { m_NoiseSettings.Snow   = snow;   } 

public:
    const int GetNoiseHeight() const { return m_NoiseSettings.Height; }

private:
    double Noise2D(double x, double y);
    double OctaveNoise2D(const int &x, const int &y);

private:
    int m_Seed = 0;
    int m_Width = 256;
    int m_Height = 256;
    int m_CellSize = 16;
    int m_Levels = 1;
    double m_Attenuation = 1.0f;
    std::vector<glm::vec2> m_InfluenceVectors;
    std::vector<double> m_PixelData;
    NoiseSettings m_NoiseSettings;

private:
    void UpdateInfluenceVectors();
    void DrawInfluenceVectors(ImDrawList *drawlist, const ImVec2 &p0);
    void UpdatePixelData();
    void DrawNoiseHeightMap(ImDrawList *drawlist, const ImVec2 &p0);
};
