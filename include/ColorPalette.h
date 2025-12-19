/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Color palette for radar display
 */

#ifndef _COLOR_PALETTE_H_
#define _COLOR_PALETTE_H_

#include "pi_common.h"
#include <array>

PLUGIN_BEGIN_NAMESPACE

// Color scheme types
enum class ColorScheme {
    Day,
    Dusk,
    Night
};

class ColorPalette {
public:
    ColorPalette();
    ~ColorPalette();

    // Set color scheme
    void SetScheme(ColorScheme scheme);
    ColorScheme GetScheme() const { return m_scheme; }

    // Set intensity thresholds
    void SetThresholds(int weak, int medium, int strong);

    // Map radar intensity (0-255) to RGBA color
    void GetColor(uint8_t intensity, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;

    // Get pre-computed lookup table for shader (256 x 4 bytes RGBA)
    const uint8_t* GetLUT() const { return m_lut.data(); }
    size_t GetLUTSize() const { return m_lut.size(); }

private:
    void BuildLUT();

    std::array<uint8_t, 256 * 4> m_lut;  // RGBA for each intensity
    ColorScheme m_scheme;

    int m_threshold_weak;
    int m_threshold_medium;
    int m_threshold_strong;
};

PLUGIN_END_NAMESPACE

#endif  // _COLOR_PALETTE_H_
