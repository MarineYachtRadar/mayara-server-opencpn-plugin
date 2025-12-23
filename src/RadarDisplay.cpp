/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Per-radar data container - owns receiver, buffer, and renderers
 */

#include "RadarDisplay.h"
#include "mayara_server_pi.h"
#include "RadarManager.h"
#include "RadarOverlayRenderer.h"
#include "RadarPPIRenderer.h"
#include "RadarCanvas.h"

using namespace mayara;

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
    // Create spoke buffer (no OpenGL needed)
    m_spoke_buffer = std::make_unique<SpokeBuffer>(
        m_spokes_per_revolution,
        m_max_spoke_length
    );

    // Create renderers but DON'T initialize OpenGL yet
    // OpenGL init must happen during rendering when GL context is active
    m_overlay_renderer = std::make_unique<RadarOverlayRenderer>();
    m_ppi_renderer = std::make_unique<RadarPPIRenderer>();
}

RadarDisplay::~RadarDisplay() {
    Stop();
}

void RadarDisplay::Start() {
    try {
        wxLogMessage("MaYaRa: RadarDisplay::Start() entry, receiver=%p", (void*)m_receiver.get());
        wxLog::FlushActive();

        if (m_receiver) {
            wxLogMessage("MaYaRa: RadarDisplay::Start() - already started");
            return;  // Already started
        }

        // Get WebSocket URL from plugin's client
        wxLogMessage("MaYaRa: RadarDisplay::Start() - getting manager");
        wxLog::FlushActive();
        auto* manager = m_plugin->GetRadarManager();
        wxLogMessage("MaYaRa: RadarDisplay::Start() - manager=%p", (void*)manager);
        wxLog::FlushActive();

        if (!manager || !manager->GetClient()) {
            wxLogMessage("MaYaRa: RadarDisplay::Start() - no manager or client");
            return;
        }

        wxLogMessage("MaYaRa: RadarDisplay::Start() - getting spoke URL for %s", m_id.c_str());
        wxLog::FlushActive();
        std::string url = manager->GetClient()->GetSpokeStreamUrl(m_id);
        wxLogMessage("MaYaRa: RadarDisplay::Start() - URL: %s", url.c_str());
        wxLog::FlushActive();

        // Create receiver with callback
        wxLogMessage("MaYaRa: RadarDisplay::Start() - creating SpokeReceiver");
        wxLog::FlushActive();
        m_receiver = std::make_unique<SpokeReceiver>(
            url,
            [this](const SpokeData& spoke) {
                OnSpokeReceived(spoke);
            }
        );
        wxLogMessage("MaYaRa: RadarDisplay::Start() - SpokeReceiver created");
        wxLog::FlushActive();

        wxLogMessage("MaYaRa: RadarDisplay::Start() - calling m_receiver->Start()");
        wxLog::FlushActive();
        m_receiver->Start();
        wxLogMessage("MaYaRa: RadarDisplay::Start() - complete");
        wxLog::FlushActive();
    } catch (const std::exception& e) {
        wxLogMessage("MaYaRa: RadarDisplay::Start() EXCEPTION: %s", e.what());
        wxLog::FlushActive();
    } catch (...) {
        wxLogMessage("MaYaRa: RadarDisplay::Start() UNKNOWN EXCEPTION");
        wxLog::FlushActive();
    }
}

void RadarDisplay::Stop() {
    if (m_receiver) {
        m_receiver->Stop();
        m_receiver.reset();
    }
}

void RadarDisplay::UpdateCapabilities(const CapabilityManifest& caps) {
    wxCriticalSectionLocker lock(m_lock);

    if (caps.spokesPerRevolution() > 0 && caps.spokesPerRevolution() != m_spokes_per_revolution) {
        m_spokes_per_revolution = caps.spokesPerRevolution();
    }
    if (caps.maxSpokeLength() > 0 && caps.maxSpokeLength() != m_max_spoke_length) {
        m_max_spoke_length = caps.maxSpokeLength();
    }

    // Update info
    m_info.brand = caps.make;
    m_info.model = caps.model;
    m_info.spokesPerRevolution = caps.spokesPerRevolution();
    m_info.maxSpokeLength = caps.maxSpokeLength();
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
