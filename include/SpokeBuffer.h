/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Ring buffer for storing spoke data
 */

#ifndef _SPOKE_BUFFER_H_
#define _SPOKE_BUFFER_H_

#include "pi_common.h"
#include <vector>

PLUGIN_BEGIN_NAMESPACE

class SpokeBuffer {
public:
    SpokeBuffer(size_t spokes, size_t max_spoke_len);
    ~SpokeBuffer();

    // Write a spoke at the given angle
    void WriteSpoke(uint32_t angle, const uint8_t* data, size_t len, uint32_t range_meters);

    // Read a spoke at the given angle (returns nullptr if no data)
    const uint8_t* GetSpoke(uint32_t angle) const;

    // Get spoke metadata
    size_t GetSpokeLength(uint32_t angle) const;
    uint32_t GetSpokeRange(uint32_t angle) const;
    wxLongLong GetSpokeTime(uint32_t angle) const;

    // Buffer properties
    size_t GetSpokes() const { return m_spokes; }
    size_t GetMaxSpokeLen() const { return m_max_spoke_len; }

    // Clear all data
    void Clear();

    // Get raw texture data pointer (for OpenGL upload)
    const uint8_t* GetTextureData() const { return m_texture_data.data(); }
    size_t GetTextureSize() const { return m_texture_data.size(); }

private:
    size_t m_spokes;
    size_t m_max_spoke_len;

    // Main data storage: spokes x max_spoke_len
    std::vector<uint8_t> m_texture_data;

    // Per-spoke metadata
    std::vector<size_t> m_spoke_lengths;
    std::vector<uint32_t> m_spoke_ranges;
    std::vector<wxLongLong> m_timestamps;

    mutable wxCriticalSection m_lock;
};

PLUGIN_END_NAMESPACE

#endif  // _SPOKE_BUFFER_H_
