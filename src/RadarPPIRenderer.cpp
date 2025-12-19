/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar PPI (Plan Position Indicator) renderer
 */

#include "RadarPPIRenderer.h"
#include "SpokeBuffer.h"

using namespace mayara_server_pi;

RadarPPIRenderer::RadarPPIRenderer()
    : RadarRenderer()
    , m_loc_center(-1)
    , m_loc_radius(-1)
    , m_loc_rotation(-1)
    , m_loc_texture(-1)
    , m_loc_palette(-1)
    , m_show_range_rings(true)
    , m_show_heading_line(true)
    , m_show_targets(true)
{
}

RadarPPIRenderer::~RadarPPIRenderer() {
}

bool RadarPPIRenderer::Init(size_t spokes, size_t maxSpokeLen) {
    if (!RadarRenderer::Init(spokes, maxSpokeLen)) {
        return false;
    }

    return true;
}

void RadarPPIRenderer::DrawPPI(wxGLContext* context,
                                int width, int height,
                                double range_meters,
                                double heading)
{
    if (!m_initialized) return;

    // Calculate display radius (fit to window with margin)
    int margin = 20;
    int display_size = std::min(width, height) - 2 * margin;
    float radius = display_size / 2.0f;
    float cx = width / 2.0f;
    float cy = height / 2.0f;

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw background circle
    glColor4f(0.1f, 0.1f, 0.15f, 1.0f);
    DrawCircle(cx, cy, radius, 64);

    // Draw radar texture
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glEnable(GL_TEXTURE_2D);

    // Draw as polar sectors
    // This is a simplified version - production would use shaders
    int segments = m_spokes;
    float angle_step = 2.0f * M_PI / segments;

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < segments; i++) {
        float angle1 = i * angle_step - heading * M_PI / 180.0f - M_PI / 2.0f;
        float angle2 = (i + 1) * angle_step - heading * M_PI / 180.0f - M_PI / 2.0f;

        // Center vertex
        glTexCoord2f(0, (float)i / segments);
        glColor4f(0, 0.5f, 0, 0.5f);
        glVertex2f(cx, cy);

        // Edge vertices
        glTexCoord2f(1, (float)i / segments);
        glColor4f(0, 0.8f, 0, 0.3f);
        glVertex2f(cx + cos(angle1) * radius, cy + sin(angle1) * radius);

        glTexCoord2f(1, (float)(i + 1) / segments);
        glVertex2f(cx + cos(angle2) * radius, cy + sin(angle2) * radius);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Draw overlays
    if (m_show_range_rings) {
        DrawRangeRings(width, height, range_meters, 4);
    }

    if (m_show_heading_line) {
        DrawHeadingLine(width, height, heading);
    }

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void RadarPPIRenderer::DrawRangeRings(int width, int height, double range_meters, int num_rings) {
    int display_size = std::min(width, height) - 40;
    float radius = display_size / 2.0f;
    float cx = width / 2.0f;
    float cy = height / 2.0f;

    glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
    glLineWidth(1.0f);

    for (int i = 1; i <= num_rings; i++) {
        float ring_radius = radius * i / num_rings;

        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < 64; j++) {
            float angle = j * 2.0f * M_PI / 64;
            glVertex2f(cx + cos(angle) * ring_radius, cy + sin(angle) * ring_radius);
        }
        glEnd();
    }
}

void RadarPPIRenderer::DrawHeadingLine(int width, int height, double heading) {
    int display_size = std::min(width, height) - 40;
    float radius = display_size / 2.0f;
    float cx = width / 2.0f;
    float cy = height / 2.0f;

    float angle = -heading * M_PI / 180.0f - M_PI / 2.0f;

    glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(cx, cy);
    glVertex2f(cx + cos(angle) * radius, cy + sin(angle) * radius);
    glEnd();
}

void RadarPPIRenderer::DrawTargets(int width, int height, double range_meters,
                                    const std::vector<ArpaTarget>& targets)
{
    int display_size = std::min(width, height) - 40;
    float radius = display_size / 2.0f;
    float cx = width / 2.0f;
    float cy = height / 2.0f;

    for (const auto& target : targets) {
        // Convert bearing/distance to screen coordinates
        float bearing_rad = target.bearing * M_PI / 180.0f - M_PI / 2.0f;
        float dist_ratio = target.distance / range_meters;
        if (dist_ratio > 1.0f) continue;  // Out of range

        float tx = cx + cos(bearing_rad) * radius * dist_ratio;
        float ty = cy + sin(bearing_rad) * radius * dist_ratio;

        // Draw target symbol
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        DrawTriangle(tx, ty, 8, target.course * M_PI / 180.0f);

        // Draw velocity vector
        if (target.speed > 0.1) {
            float course_rad = target.course * M_PI / 180.0f - M_PI / 2.0f;
            float vx = tx + cos(course_rad) * target.speed * 5;  // Scale for visibility
            float vy = ty + sin(course_rad) * target.speed * 5;

            glBegin(GL_LINES);
            glVertex2f(tx, ty);
            glVertex2f(vx, vy);
            glEnd();
        }
    }
}

void RadarPPIRenderer::DrawCircle(float cx, float cy, float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float angle = i * 2.0f * M_PI / segments;
        glVertex2f(cx + cos(angle) * radius, cy + sin(angle) * radius);
    }
    glEnd();
}

void RadarPPIRenderer::DrawLine(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void RadarPPIRenderer::DrawTriangle(float x, float y, float size, float rotation) {
    float c = cos(rotation);
    float s = sin(rotation);

    // Triangle pointing in direction of rotation
    float p1x = 0, p1y = -size;
    float p2x = -size * 0.6f, p2y = size * 0.5f;
    float p3x = size * 0.6f, p3y = size * 0.5f;

    // Rotate
    glBegin(GL_TRIANGLES);
    glVertex2f(x + p1x * c - p1y * s, y + p1x * s + p1y * c);
    glVertex2f(x + p2x * c - p2y * s, y + p2x * s + p2y * c);
    glVertex2f(x + p3x * c - p3y * s, y + p3x * s + p3y * c);
    glEnd();
}

const char* RadarPPIRenderer::GetVertexShaderSource() {
    return R"(
        #version 120
        attribute vec2 position;
        varying vec2 v_texcoord;

        void main() {
            gl_Position = vec4(position, 0.0, 1.0);
            v_texcoord = position * 0.5 + 0.5;
        }
    )";
}

const char* RadarPPIRenderer::GetFragmentShaderSource() {
    return R"(
        #version 120
        varying vec2 v_texcoord;
        uniform sampler2D radar_texture;
        uniform sampler1D palette;
        uniform float rotation;

        void main() {
            vec2 pos = v_texcoord * 2.0 - 1.0;
            float angle = atan(pos.y, pos.x) + rotation;
            float dist = length(pos);

            if (dist > 1.0) discard;

            float u = mod(angle + 3.14159, 2.0 * 3.14159) / (2.0 * 3.14159);
            float v = dist;

            float intensity = texture2D(radar_texture, vec2(v, u)).r;
            gl_FragColor = texture1D(palette, intensity);
        }
    )";
}
