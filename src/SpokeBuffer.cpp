/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Ring buffer for storing spoke data
 */

#include "SpokeBuffer.h"
#include <cstring>

using namespace mayara;

SpokeBuffer::SpokeBuffer(size_t spokes, size_t max_spoke_len)
    : m_spokes(spokes)
    , m_max_spoke_len(max_spoke_len)
{
    // Allocate texture data (RGBA per pixel)
    m_texture_data.resize(spokes * max_spoke_len, 0);

    // Allocate metadata
    m_spoke_lengths.resize(spokes, 0);
    m_spoke_ranges.resize(spokes, 0);
    m_timestamps.resize(spokes, 0);
}

SpokeBuffer::~SpokeBuffer() {
}

void SpokeBuffer::WriteSpoke(uint32_t angle, const uint8_t* data, size_t len, uint32_t range_meters) {
    if (angle >= m_spokes) return;

    wxCriticalSectionLocker lock(m_lock);

    // Calculate offset in texture
    size_t offset = angle * m_max_spoke_len;

    // Copy data (truncate if too long)
    size_t copy_len = std::min(len, m_max_spoke_len);
    std::memcpy(&m_texture_data[offset], data, copy_len);

    // Zero remaining if shorter
    if (copy_len < m_max_spoke_len) {
        std::memset(&m_texture_data[offset + copy_len], 0, m_max_spoke_len - copy_len);
    }

    // Update metadata
    m_spoke_lengths[angle] = copy_len;
    m_spoke_ranges[angle] = range_meters;
    m_timestamps[angle] = wxGetLocalTimeMillis();
}

const uint8_t* SpokeBuffer::GetSpoke(uint32_t angle) const {
    if (angle >= m_spokes) return nullptr;

    wxCriticalSectionLocker lock(m_lock);
    return &m_texture_data[angle * m_max_spoke_len];
}

size_t SpokeBuffer::GetSpokeLength(uint32_t angle) const {
    if (angle >= m_spokes) return 0;

    wxCriticalSectionLocker lock(m_lock);
    return m_spoke_lengths[angle];
}

uint32_t SpokeBuffer::GetSpokeRange(uint32_t angle) const {
    if (angle >= m_spokes) return 0;

    wxCriticalSectionLocker lock(m_lock);
    return m_spoke_ranges[angle];
}

wxLongLong SpokeBuffer::GetSpokeTime(uint32_t angle) const {
    if (angle >= m_spokes) return 0;

    wxCriticalSectionLocker lock(m_lock);
    return m_timestamps[angle];
}

void SpokeBuffer::Clear() {
    wxCriticalSectionLocker lock(m_lock);

    std::memset(m_texture_data.data(), 0, m_texture_data.size());
    std::fill(m_spoke_lengths.begin(), m_spoke_lengths.end(), 0);
    std::fill(m_spoke_ranges.begin(), m_spoke_ranges.end(), 0);
    std::fill(m_timestamps.begin(), m_timestamps.end(), 0);
}
