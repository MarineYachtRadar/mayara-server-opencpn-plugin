/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Color palette for radar display
 */

#include "ColorPalette.h"

using namespace mayara;

ColorPalette::ColorPalette()
    : m_scheme(ColorScheme::Day)
    , m_threshold_weak(50)
    , m_threshold_medium(100)
    , m_threshold_strong(200)
{
    BuildLUT();
}

ColorPalette::~ColorPalette() {
}

void ColorPalette::SetScheme(ColorScheme scheme) {
    if (m_scheme != scheme) {
        m_scheme = scheme;
        BuildLUT();
    }
}

void ColorPalette::SetThresholds(int weak, int medium, int strong) {
    m_threshold_weak = weak;
    m_threshold_medium = medium;
    m_threshold_strong = strong;
    BuildLUT();
}

void ColorPalette::GetColor(uint8_t intensity, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
    size_t idx = intensity * 4;
    r = m_lut[idx];
    g = m_lut[idx + 1];
    b = m_lut[idx + 2];
    a = m_lut[idx + 3];
}

void ColorPalette::BuildLUT() {
    // Color schemes based on intensity
    // Day: Green-Yellow-Red
    // Dusk: Blue-Cyan-Yellow
    // Night: Dark Blue-Cyan-White

    for (int i = 0; i < 256; i++) {
        size_t idx = i * 4;
        uint8_t r, g, b, a;

        if (i == 0) {
            // Transparent for zero intensity
            r = g = b = a = 0;
        } else if (i < m_threshold_weak) {
            // Weak returns
            switch (m_scheme) {
                case ColorScheme::Day:
                    r = 0; g = 100; b = 0; a = 128;
                    break;
                case ColorScheme::Dusk:
                    r = 0; g = 50; b = 100; a = 128;
                    break;
                case ColorScheme::Night:
                    r = 0; g = 50; b = 80; a = 128;
                    break;
            }
        } else if (i < m_threshold_medium) {
            // Medium returns
            switch (m_scheme) {
                case ColorScheme::Day:
                    r = 0; g = 200; b = 0; a = 180;
                    break;
                case ColorScheme::Dusk:
                    r = 0; g = 150; b = 150; a = 180;
                    break;
                case ColorScheme::Night:
                    r = 0; g = 100; b = 150; a = 180;
                    break;
            }
        } else if (i < m_threshold_strong) {
            // Strong returns
            switch (m_scheme) {
                case ColorScheme::Day:
                    r = 200; g = 200; b = 0; a = 220;
                    break;
                case ColorScheme::Dusk:
                    r = 150; g = 200; b = 50; a = 220;
                    break;
                case ColorScheme::Night:
                    r = 100; g = 200; b = 200; a = 220;
                    break;
            }
        } else {
            // Very strong returns
            switch (m_scheme) {
                case ColorScheme::Day:
                    r = 255; g = 100; b = 0; a = 255;
                    break;
                case ColorScheme::Dusk:
                    r = 255; g = 255; b = 100; a = 255;
                    break;
                case ColorScheme::Night:
                    r = 200; g = 255; b = 255; a = 255;
                    break;
            }
        }

        m_lut[idx] = r;
        m_lut[idx + 1] = g;
        m_lut[idx + 2] = b;
        m_lut[idx + 3] = a;
    }
}
