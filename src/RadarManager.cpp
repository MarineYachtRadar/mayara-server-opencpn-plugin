/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Manages radar discovery, lifecycle, and connections
 */

#include "RadarManager.h"
#include "mayara_server_pi.h"
#include "RadarDisplay.h"

using namespace mayara;

RadarManager::RadarManager(mayara_server_pi* plugin)
    : m_plugin(plugin)
    , m_running(false)
    , m_connected(false)
    , m_notification_shown(false)
{
}

RadarManager::~RadarManager() {
    Stop();
}

void RadarManager::Start() {
    wxCriticalSectionLocker lock(m_lock);

    if (m_running) return;

    // Create REST client
    m_client = std::make_unique<MayaraClient>(
        m_plugin->GetServerHost(),
        m_plugin->GetServerPort()
    );

    m_running = true;
    // Set last_discovery to 0 so first Poll() triggers discovery
    m_last_discovery = 0;
    m_last_reconnect_attempt = 0;

    // Don't do initial discovery here - let Poll() handle it
    // This avoids IXWebSocket HTTP issues during plugin Init()
}

void RadarManager::Stop() {
    wxCriticalSectionLocker lock(m_lock);

    if (!m_running) return;

    m_running = false;

    // Stop all radars
    for (auto& [id, radar] : m_radars) {
        radar->Stop();
    }
    m_radars.clear();
    m_known_radar_ids.clear();

    m_client.reset();
    m_connected = false;
}

void RadarManager::Poll() {
    if (!m_running) return;

    wxLongLong now = wxGetLocalTimeMillis();

    if (m_connected) {
        // Discovery polling
        int interval = m_plugin->GetDiscoveryPollInterval() * 1000;
        if ((now - m_last_discovery).GetValue() >= interval) {
            wxLogMessage("MaYaRa: Running discovery poll");
            DoDiscovery();
            m_last_discovery = now;
        }
    } else {
        // Reconnection attempts
        int interval = m_plugin->GetReconnectInterval() * 1000;
        if ((now - m_last_reconnect_attempt).GetValue() >= interval) {
            wxLogMessage("MaYaRa: Attempting to connect to %s:%d",
                m_plugin->GetServerHost(), m_plugin->GetServerPort());
            TryReconnect();
            m_last_reconnect_attempt = now;
        }
    }
}

void RadarManager::DoDiscovery() {
    if (!m_client) return;

    auto radars = m_client->GetRadars();

    if (!m_client->IsConnected()) {
        if (m_connected) {
            m_connected = false;
            ShowConnectionNotification(false);
        }
        return;
    }

    if (!m_connected) {
        m_connected = true;
        m_notification_shown = false;  // Reset for next disconnect
    }

    // Find new radars
    std::set<std::string> current_ids;
    for (const auto& [id, info] : radars) {
        current_ids.insert(id);

        if (m_known_radar_ids.find(id) == m_known_radar_ids.end()) {
            HandleNewRadar(id, info);
        } else {
            // Update existing radar state
            if (m_radars.count(id)) {
                auto state = m_client->GetState(id);
                m_radars[id]->UpdateState(state);
            }
        }
    }

    // Find removed radars
    std::vector<std::string> removed;
    for (const auto& id : m_known_radar_ids) {
        if (current_ids.find(id) == current_ids.end()) {
            removed.push_back(id);
        }
    }
    for (const auto& id : removed) {
        HandleRemovedRadar(id);
    }
}

void RadarManager::HandleNewRadar(const std::string& id, const RadarInfo& info) {
    wxCriticalSectionLocker lock(m_lock);

    m_known_radar_ids.insert(id);

    // Create radar display
    auto radar = std::make_unique<RadarDisplay>(m_plugin, id, info);

    // Fetch capabilities
    auto caps = m_client->GetCapabilities(id);
    radar->UpdateCapabilities(caps);

    // Start receiving spokes
    radar->Start();

    m_radars[id] = std::move(radar);
}

void RadarManager::HandleRemovedRadar(const std::string& id) {
    wxCriticalSectionLocker lock(m_lock);

    m_known_radar_ids.erase(id);

    if (m_radars.count(id)) {
        m_radars[id]->Stop();
        m_radars.erase(id);
    }
}

void RadarManager::TryReconnect() {
    if (!m_client) return;

    // Try to get radars
    auto ids = m_client->GetRadarIds();

    if (m_client->IsConnected()) {
        wxLogMessage("MaYaRa: Connected! Found %zu radar(s)", ids.size());
        m_connected = true;
        DoDiscovery();
    } else {
        wxLogMessage("MaYaRa: Connection failed: %s", m_client->GetLastError());
    }
}

void RadarManager::ShowConnectionNotification(bool connected) {
    if (!connected && !m_notification_shown) {
        // Don't show popup - it can cause crashes when called from timer context
        // Just log the error and set the flag
        wxLogMessage("MaYaRa Server: Cannot connect to %s:%d",
            m_plugin->GetServerHost(), m_plugin->GetServerPort());
        m_notification_shown = true;
    }
}

bool RadarManager::IsConnected() const {
    return m_connected;
}

std::string RadarManager::GetConnectionStatus() const {
    if (m_connected) {
        return "Connected";
    } else {
        return "Disconnected";
    }
}

std::vector<RadarDisplay*> RadarManager::GetActiveRadars() {
    wxCriticalSectionLocker lock(m_lock);

    std::vector<RadarDisplay*> result;
    for (auto& [id, radar] : m_radars) {
        result.push_back(radar.get());
    }
    return result;
}

RadarDisplay* RadarManager::GetRadar(const std::string& id) {
    wxCriticalSectionLocker lock(m_lock);

    if (m_radars.count(id)) {
        return m_radars[id].get();
    }
    return nullptr;
}
