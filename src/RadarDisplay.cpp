/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Per-radar data container - owns receiver, buffer, and renderers
 */

#include "RadarDisplay.h"
#include "mayara_server_pi.h"
#include "RadarOverlayRenderer.h"
#include "RadarPPIRenderer.h"
#include "RadarCanvas.h"

using namespace mayara_server_pi;

RadarDisplay::RadarDisplay(mayara_server_pi* plugin,
                           const std::string& id,
                           const RadarInfo& info)
    : m_plugin(plugin)
    , m_id(id)
    , m_info(info)
    , m_status(info.status)
    , m_range_meters(info.rangeMeters)
    , m_spokes_per_revolution(info.spokesPerRevolution > 0 ? info.spokesPerRevolution : 2048)
    , m_max_spoke_length(info.maxSpokeLength > 0 ? info.maxSpokeLength : 512)
    , m_ppi_window(nullptr)
{
    // Create spoke buffer
    m_spoke_buffer = std::make_unique<SpokeBuffer>(
        m_spokes_per_revolution,
        m_max_spoke_length
    );

    // Create renderers
    m_overlay_renderer = std::make_unique<RadarOverlayRenderer>();
    m_overlay_renderer->Init(m_spokes_per_revolution, m_max_spoke_length);

    m_ppi_renderer = std::make_unique<RadarPPIRenderer>();
    m_ppi_renderer->Init(m_spokes_per_revolution, m_max_spoke_length);
}

RadarDisplay::~RadarDisplay() {
    Stop();
}

void RadarDisplay::Start() {
    if (m_receiver) return;  // Already started

    // Get WebSocket URL from plugin's client
    auto* manager = m_plugin->GetRadarManager();
    if (!manager || !manager->GetClient()) return;

    std::string url = manager->GetClient()->GetSpokeStreamUrl(m_id);

    // Create receiver with callback
    m_receiver = std::make_unique<SpokeReceiver>(
        url,
        [this](const SpokeData& spoke) {
            OnSpokeReceived(spoke);
        }
    );

    m_receiver->Start();
}

void RadarDisplay::Stop() {
    if (m_receiver) {
        m_receiver->Stop();
        m_receiver.reset();
    }
}

void RadarDisplay::UpdateCapabilities(const CapabilityManifest& caps) {
    wxCriticalSectionLocker lock(m_lock);

    if (caps.spokesPerRevolution > 0 && caps.spokesPerRevolution != m_spokes_per_revolution) {
        m_spokes_per_revolution = caps.spokesPerRevolution;
    }
    if (caps.maxSpokeLength > 0 && caps.maxSpokeLength != m_max_spoke_length) {
        m_max_spoke_length = caps.maxSpokeLength;
    }

    // Update info
    m_info.brand = caps.make;
    m_info.model = caps.model;
    m_info.spokesPerRevolution = caps.spokesPerRevolution;
    m_info.maxSpokeLength = caps.maxSpokeLength;
}

void RadarDisplay::UpdateState(const RadarState& state) {
    wxCriticalSectionLocker lock(m_lock);

    m_status = state.status;
    m_range_meters = state.rangeMeters;
}

bool RadarDisplay::IsReceiving() const {
    return m_receiver && m_receiver->IsConnected();
}

void RadarDisplay::UpdateTargets(const std::vector<ArpaTarget>& targets) {
    wxCriticalSectionLocker lock(m_lock);
    m_targets = targets;
}

void RadarDisplay::OnSpokeReceived(const SpokeData& spoke) {
    if (!m_spoke_buffer) return;

    // Write spoke to buffer
    m_spoke_buffer->WriteSpoke(
        spoke.angle,
        spoke.data.data(),
        spoke.data.size(),
        spoke.rangeMeters
    );
}
