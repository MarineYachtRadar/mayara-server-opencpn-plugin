/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar overlay renderer for chart canvas
 */

#ifndef _RADAR_OVERLAY_RENDERER_H_
#define _RADAR_OVERLAY_RENDERER_H_

#include "RadarRenderer.h"

PLUGIN_BEGIN_NAMESPACE

class RadarOverlayRenderer : public RadarRenderer {
public:
    RadarOverlayRenderer();
    ~RadarOverlayRenderer() override;

    // Initialize with radar parameters
    bool Init(size_t spokes, size_t maxSpokeLen) override;

    // Render radar overlay on chart
    void DrawOverlay(wxGLContext* context,
                     PlugIn_ViewPort* vp,
                     double range_meters,
                     const GeoPosition& radar_pos,
                     double heading);

private:
    // Shader sources for overlay rendering
    static const char* GetVertexShaderSource();
    static const char* GetFragmentShaderSource();

    // Shader uniform locations
    GLint m_loc_center;
    GLint m_loc_scale;
    GLint m_loc_rotation;
    GLint m_loc_texture;
    GLint m_loc_palette;
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_OVERLAY_RENDERER_H_
