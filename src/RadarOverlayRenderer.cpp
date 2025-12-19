/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar overlay renderer for chart canvas
 */

#include "RadarOverlayRenderer.h"
#include "SpokeBuffer.h"

using namespace mayara_server_pi;

RadarOverlayRenderer::RadarOverlayRenderer()
    : RadarRenderer()
    , m_loc_center(-1)
    , m_loc_scale(-1)
    , m_loc_rotation(-1)
    , m_loc_texture(-1)
    , m_loc_palette(-1)
{
}

RadarOverlayRenderer::~RadarOverlayRenderer() {
}

bool RadarOverlayRenderer::Init(size_t spokes, size_t maxSpokeLen) {
    if (!RadarRenderer::Init(spokes, maxSpokeLen)) {
        return false;
    }

    // TODO: Compile shaders
    // For now, use simple immediate mode rendering

    return true;
}

void RadarOverlayRenderer::DrawOverlay(wxGLContext* context,
                                        PlugIn_ViewPort* vp,
                                        double range_meters,
                                        const GeoPosition& radar_pos,
                                        double heading)
{
    if (!m_initialized || !vp) return;

    // Get screen position of radar
    wxPoint radar_screen;
    GetCanvasPixLL(vp, &radar_screen, radar_pos.lat, radar_pos.lon);

    // Calculate scale: pixels per meter
    // Use a point at 1km distance to calculate
    double lat_offset = range_meters / 111320.0;  // approx meters per degree lat
    wxPoint range_point;
    GetCanvasPixLL(vp, &range_point, radar_pos.lat + lat_offset, radar_pos.lon);

    double pixels_per_meter = fabs(range_point.y - radar_screen.y) / range_meters;
    double radius_pixels = range_meters * pixels_per_meter;

    // Save OpenGL state
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Translate to radar center
    glTranslatef(radar_screen.x, radar_screen.y, 0);

    // Rotate for heading (OpenCPN uses north-up, we need to apply heading)
    glRotatef(-heading, 0, 0, 1);

    // Draw radar as a simple colored circle for now
    // TODO: Replace with shader-based polar texture rendering

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glEnable(GL_TEXTURE_2D);

    // Draw as a textured quad that will be transformed
    // This is a placeholder - real implementation uses polar shader
    int segments = 360;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0, 1, 0, 0.5f);  // Semi-transparent green center
    glVertex2f(0, 0);

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i * 2.0f * M_PI / segments;
        float x = cos(angle) * radius_pixels;
        float y = sin(angle) * radius_pixels;
        glColor4f(0, 0.5f, 0, 0.3f);  // Fade at edges
        glVertex2f(x, y);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Restore OpenGL state
    glPopAttrib();
    glPopMatrix();
}

const char* RadarOverlayRenderer::GetVertexShaderSource() {
    return R"(
        #version 120
        attribute vec2 position;
        varying vec2 v_texcoord;
        uniform vec2 center;
        uniform float scale;
        uniform float rotation;

        void main() {
            float c = cos(rotation);
            float s = sin(rotation);
            mat2 rot = mat2(c, -s, s, c);
            vec2 pos = rot * position * scale + center;
            gl_Position = vec4(pos, 0.0, 1.0);
            v_texcoord = position * 0.5 + 0.5;
        }
    )";
}

const char* RadarOverlayRenderer::GetFragmentShaderSource() {
    return R"(
        #version 120
        varying vec2 v_texcoord;
        uniform sampler2D radar_texture;
        uniform sampler1D palette;

        void main() {
            // Convert cartesian to polar
            vec2 pos = v_texcoord * 2.0 - 1.0;
            float angle = atan(pos.y, pos.x);
            float dist = length(pos);

            if (dist > 1.0) discard;

            // Map angle to texture coordinate (0-1)
            float u = (angle + 3.14159) / (2.0 * 3.14159);
            float v = dist;

            // Sample radar data
            float intensity = texture2D(radar_texture, vec2(v, u)).r;

            // Map through palette
            gl_FragColor = texture1D(palette, intensity);
        }
    )";
}
