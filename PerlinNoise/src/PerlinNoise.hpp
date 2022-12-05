#pragma once

#include "imgui.h"

#include <glm/glm.hpp>

#include <vector>
#include <random>

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
    const std::vector<double>& const GetNoise() { return m_PixelData; };

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

private:
    void UpdateInfluenceVectors();
    void DrawInfluenceVectors(ImDrawList *drawlist, const ImVec2 &p0);
    void UpdatePixelData();
    void DrawNoiseHeightMap(ImDrawList *drawlist, const ImVec2 &p0);
};
