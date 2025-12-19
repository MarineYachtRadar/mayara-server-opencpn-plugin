/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar PPI (Plan Position Indicator) renderer
 */

#ifndef _RADAR_PPI_RENDERER_H_
#define _RADAR_PPI_RENDERER_H_

#include "RadarRenderer.h"
#include "MayaraClient.h"
#include <vector>

PLUGIN_BEGIN_NAMESPACE

class RadarPPIRenderer : public RadarRenderer {
public:
    RadarPPIRenderer();
    ~RadarPPIRenderer() override;

    // Initialize with radar parameters
    bool Init(size_t spokes, size_t maxSpokeLen) override;

    // Render PPI display
    void DrawPPI(wxGLContext* context,
                 int width, int height,
                 double range_meters,
                 double heading);

    // Draw overlay elements
    void DrawRangeRings(int width, int height, double range_meters, int num_rings = 4);
    void DrawHeadingLine(int width, int height, double heading);
    void DrawTargets(int width, int height, double range_meters,
                     const std::vector<ArpaTarget>& targets);

    // Settings
    void SetShowRangeRings(bool show) { m_show_range_rings = show; }
    void SetShowHeadingLine(bool show) { m_show_heading_line = show; }
    void SetShowTargets(bool show) { m_show_targets = show; }

private:
    // Shader sources for PPI rendering
    static const char* GetVertexShaderSource();
    static const char* GetFragmentShaderSource();

    // Drawing helpers
    void DrawCircle(float cx, float cy, float radius, int segments = 64);
    void DrawLine(float x1, float y1, float x2, float y2);
    void DrawTriangle(float x, float y, float size, float rotation);

    // Shader uniform locations
    GLint m_loc_center;
    GLint m_loc_radius;
    GLint m_loc_rotation;
    GLint m_loc_texture;
    GLint m_loc_palette;

    // Display options
    bool m_show_range_rings;
    bool m_show_heading_line;
    bool m_show_targets;
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_PPI_RENDERER_H_
