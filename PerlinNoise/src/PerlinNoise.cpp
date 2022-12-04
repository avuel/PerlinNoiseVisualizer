#include <algorithm>

#include "PerlinNoise.hpp"

#include <iostream>

namespace Utils
{
    // From Ken Perlin's 2002 Paper http://mrl.nyu.edu/~perlin/paper445.pdf
    double spline(const double &t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10); // 6*t^5 - 15*t^4 + 10*t^3
    }

    double lerp(const double &t, const double &a, const double &b)
    {
        return a + t*(b - a);
    }

    void DrawGrid(ImDrawList *drawlist, const ImVec2 &p0, const ImVec2 &p1, const ImVec2 &gridsize, const int &cellsize, const ImU32 &linecolor)
    {
        for (float x = cellsize; x < gridsize.x; x += cellsize)
            drawlist->AddLine(ImVec2(p0.x + x, p0.y + cellsize), ImVec2(p0.x + x, p1.y - cellsize), linecolor);
        for (float y = cellsize; y < gridsize.y; y += cellsize)
            drawlist->AddLine(ImVec2(p0.x + cellsize, p0.y + y), ImVec2(p1.x - cellsize, p0.y + y), linecolor);
    }
}

bool PerlinNoiseGenerator::GUI(size_t &width, size_t &height)
{
    ImGuiWindowFlags canvas_window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

    // Dockable window to display the grid and influence vectors
    ImGui::Begin("Influence Vector Grid", nullptr, canvas_window_flags);
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
        if (ImGui::BeginChild("Grid", ImVec2((m_Width * m_Levels) + 2 * m_CellSize, (m_Height * m_Levels) + 2 * m_CellSize), true, ImGuiWindowFlags_NoMove));
        {
            // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            // Draw border and background color
            ImGuiIO &canvasio = ImGui::GetIO();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

            // Draw grid + all lines in the canvas
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);


            // Draw the grid
            ImU32 linecolor = IM_COL32(200, 200, 200, 40);
            Utils::DrawGrid(draw_list, canvas_p0, canvas_p1, canvas_sz, m_CellSize, linecolor);

            // Draw the influence vectors
            DrawInfluenceVectors(draw_list, canvas_p0);

            draw_list->PopClipRect();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::EndChild();
    }
    ImGui::End();

    static int tempSeed = m_Seed;
    static int tempWidth = m_Width;
    static int tempHeight = m_Height;
    static int tempCellSize = m_CellSize;
    static int tempLevels = m_Levels;
    static double tempAttenuation = m_Attenuation;

    bool updated = false;

    // Settings Menu
    ImGui::Begin("Perlin Noise Settings");
    {
        if (ImGui::InputInt("Seed", &tempSeed));

        int oldwidth = tempWidth;
        if (ImGui::InputInt("Width", &tempWidth))
        {
            if (oldwidth < tempWidth)
                tempWidth = oldwidth * 2;
            else
                tempWidth = oldwidth / 2;

            // Clamp width values and update cell size if necessary
            tempWidth = std::clamp(tempWidth, 1, 512);
            tempCellSize = std::min(tempCellSize, tempWidth);
        }


        int oldheight = tempHeight;
        if (ImGui::InputInt("Height", &tempHeight))
        {
            if (oldheight < tempHeight)
                tempHeight = oldheight * 2;
            else
                tempHeight = oldheight / 2;

            // Clamp height values and update cell size if necessary
            tempHeight = std::clamp(tempHeight, 1, 512);
            tempCellSize = std::min(tempCellSize, tempHeight);
        }

        int oldcell = tempCellSize;
        if (ImGui::InputInt("Cell Size", &tempCellSize))
        {
            if (oldcell < tempCellSize)
                tempCellSize = oldcell * 2;
            else
                tempCellSize = oldcell / 2;

            // Clamp our cellsize to be between 1 and the min of width and height
            tempCellSize = std::clamp(tempCellSize, 1, std::min(tempWidth, tempHeight));
        }

        if (ImGui::InputInt("Levels", &tempLevels))
        {
            tempLevels = std::clamp(tempLevels, 1, 8);
        }

        if (ImGui::InputDouble("Attenuation", &tempAttenuation, 0))
        {
            tempAttenuation = std::max(tempAttenuation, std::numeric_limits<double>::min());
        }

        // Add a blank space
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        if (ImGui::Button("Generate Noise"))
        {
            updated = true;

            m_Seed = tempSeed;
            m_Height = tempHeight;
            m_Width = tempWidth;
            m_CellSize = tempCellSize;
            m_Levels = tempLevels;
            m_Attenuation = tempAttenuation;

            width = (size_t)m_Width;
            height = (size_t)m_Height;

            //m_RandomEngine.seed(tempSeed);
            srand(tempSeed);

            UpdateInfluenceVectors();
            UpdatePixelData();
        }
    }
    // End the settings panel
    ImGui::End();


    ImGui::Begin("Perlin Noise Height Map(s)");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
        if (ImGui::BeginChild("noiseheightmap", ImVec2(m_Width, m_Height), true, ImGuiWindowFlags_NoMove));
        {
            // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            // Draw border and background color
            ImGuiIO &canvasio = ImGui::GetIO();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

            // Draw grid + all lines in the canvas
            draw_list->PushClipRect(canvas_p0, canvas_p1, true);

            DrawNoiseHeightMap(draw_list, canvas_p0);
            draw_list->PopClipRect();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::EndChild();
    }
    ImGui::End();


    // End the entire dock window
    //ImGui::End();

    //ImGui::ShowDemoWindow();
    return updated;
}

double PerlinNoiseGenerator::Noise2D(double x, double y)
{
    uint16_t maxLength = std::max(m_Width, m_Height) - 1;
    int X = (int)floor(x) & maxLength; // FIND UNIT SQUARE THAT
    int Y = (int)floor(y) & maxLength; // CONTAINS POINT.

    x -= floor(x); // FIND RELATIVE X,Y
    y -= floor(y); // OF POINT IN CUBE.

    double u = Utils::spline(x); // COMPUTE SPLINE CURVES
    double v = Utils::spline(y); // FOR EACH OF X,Y.

    return Utils::lerp(v, Utils::lerp(u, glm::dot(m_InfluenceVectors[X + Y * m_Width], glm::vec2(x,y)),                  // AND ADD
        glm::dot(m_InfluenceVectors[(X + 1) + Y * m_Width], glm::vec2(x-1.0,y))),       // BLENDED
        Utils::lerp(u, glm::dot(m_InfluenceVectors[X + (Y + 1) * m_Width], glm::vec2(x,y-1.0)),        // RESULTS
            glm::dot(m_InfluenceVectors[(X + 1) + (Y + 1) * m_Width], glm::vec2(x-1.0,y-1.0)))); // FOR 4 CORNERS
}

double PerlinNoiseGenerator::OctaveNoise2D(const int &x, const int &y)
{
    double X = (double)x / (double)m_CellSize;
    double Y = (double)y / (double)m_CellSize;

    double result = 0.0;
    double amplifier = 1.0;
    double amplified = 0.0;

    for (int i = 0; i < m_Levels; i++)
    {
        result += Noise2D(X, Y) * amplifier;
        X *= 2.0;
        Y *= 2.0;

        amplified += amplifier;
        amplifier *= m_Attenuation;
    }

    return result/amplified;
}

void PerlinNoiseGenerator::UpdateInfluenceVectors()
{
    size_t wdth = (size_t)m_Width * (size_t)m_Levels;
    size_t hght = (size_t)m_Height * (size_t)m_Levels;

    m_InfluenceVectors.resize(wdth * hght);

    for (size_t j = 0; j < hght; j++)
    {
        for (size_t i = 0; i < wdth; i++)
        {
            double angle = 2 * std::_Pi * (double)rand() / RAND_MAX;
            m_InfluenceVectors[i + j * wdth] = glm::vec2(cos(angle), sin(angle));
        }
    }
}

void PerlinNoiseGenerator::DrawInfluenceVectors(ImDrawList *drawlist, const ImVec2 &p0)
{
    // Add 1 to rows/cols because we need n + 1 influence vectors for n cells (for 1 cell we need 4 influence vectors, 2 in x and 2 in y)
    size_t rows = 1 + ((size_t)m_Height * (size_t)m_Levels) / (size_t)m_CellSize;
    size_t cols = 1 + ((size_t)m_Width * (size_t)m_Levels) / (size_t)m_CellSize;

    for (size_t j = 0; j < rows; j++)
    {
        double y = p0.y + (j + 1) * m_CellSize;
        for (size_t i = 0; i < cols; i++)
        {
            double x = p0.x + (i + 1) * m_CellSize;
            double dx = m_InfluenceVectors[i + j*m_Width].x;
            double dy = m_InfluenceVectors[i + j*m_Width].y;
            drawlist->AddLine(ImVec2(x, y), ImVec2(x + (m_CellSize / 2)*dx, y + (m_CellSize / 2)*dy), IM_COL32(200, 0, 0, 150));
        }
    }
}

void PerlinNoiseGenerator::UpdatePixelData()
{
    size_t wdth = (size_t)m_Width;
    size_t hght = (size_t)m_Height;

    m_PixelData.resize(wdth * hght);

    for (size_t j = 0; j < hght; j++)
    {
        for (size_t i = 0; i < wdth; i++)
        {
            m_PixelData[i + j * wdth] = OctaveNoise2D(i, j);
        }
    }
}

void PerlinNoiseGenerator::DrawNoiseHeightMap(ImDrawList *drawlist, const ImVec2 &p0)
{
    for (size_t j = 0; j < m_Height; j++)
    {
        for (size_t i = 0; i < m_Width; i++)
        {
            double pixel = 255 * (m_PixelData[i + j * m_Width]*0.5f + 0.5f);
            drawlist->AddRectFilled(ImVec2(p0.x + i, p0.y + j), ImVec2(p0.x + (i + 1), p0.y + (j + 1)),
                IM_COL32(pixel, pixel, pixel, 255));
        }
    }
}

