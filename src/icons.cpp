/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Plugin toolbar icons
 */

#include "pi_common.h"
#include "icons.h"

using namespace mayara;

// Static icon storage
static wxBitmap* s_icon_disconnected = nullptr;
static wxBitmap* s_icon_standby = nullptr;
static wxBitmap* s_icon_transmit = nullptr;
static wxBitmap* s_plugin_icon = nullptr;

// Simple icon data (32x32 gray for disconnected, yellow for standby, green for transmit)
// In production, these would be proper PNG/SVG resources

static void CreateSimpleIcon(wxBitmap** icon, wxColour color) {
    if (*icon) return;

    wxImage img(32, 32);
    img.InitAlpha();

    // Draw a simple radar circle
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            int dx = x - 16;
            int dy = y - 16;
            int dist = (int)sqrt(dx*dx + dy*dy);

            if (dist < 14 && dist > 10) {
                // Ring
                img.SetRGB(x, y, color.Red(), color.Green(), color.Blue());
                img.SetAlpha(x, y, 255);
            } else if (dist <= 10) {
                // Center fill (darker)
                img.SetRGB(x, y, color.Red()/2, color.Green()/2, color.Blue()/2);
                img.SetAlpha(x, y, 200);
            } else {
                // Transparent
                img.SetRGB(x, y, 0, 0, 0);
                img.SetAlpha(x, y, 0);
            }
        }
    }

    *icon = new wxBitmap(img);
}

void mayara::InitializeIcons() {
    CreateSimpleIcon(&s_icon_disconnected, wxColour(128, 128, 128));  // Gray
    CreateSimpleIcon(&s_icon_standby, wxColour(255, 200, 0));         // Yellow
    CreateSimpleIcon(&s_icon_transmit, wxColour(0, 200, 0));          // Green
    CreateSimpleIcon(&s_plugin_icon, wxColour(0, 150, 200));          // Blue
}

wxBitmap* mayara::GetToolbarIcon(IconState state) {
    switch (state) {
        case IconState::Disconnected:
            return s_icon_disconnected;
        case IconState::Standby:
            return s_icon_standby;
        case IconState::Transmit:
            return s_icon_transmit;
        default:
            return s_icon_disconnected;
    }
}

wxBitmap* mayara::GetPluginIcon() {
    return s_plugin_icon;
}
