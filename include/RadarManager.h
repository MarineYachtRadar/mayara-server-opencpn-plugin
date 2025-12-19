/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Manages radar discovery, lifecycle, and connections
 */

#ifndef _RADAR_MANAGER_H_
#define _RADAR_MANAGER_H_

#include "pi_common.h"
#include "MayaraClient.h"
#include <memory>
#include <map>
#include <set>

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class mayara_server_pi;
class RadarDisplay;

class RadarManager {
public:
    RadarManager(mayara_server_pi* plugin);
    ~RadarManager();

    // Start/stop discovery and connections
    void Start();
    void Stop();

    // Called periodically by plugin timer
    void Poll();

    // Connection status
    bool IsConnected() const;
    std::string GetConnectionStatus() const;

    // Get active radars
    std::vector<RadarDisplay*> GetActiveRadars();
    RadarDisplay* GetRadar(const std::string& id);

    // Get the REST client (for control dialogs)
    MayaraClient* GetClient() { return m_client.get(); }

private:
    void DoDiscovery();
    void HandleNewRadar(const std::string& id, const RadarInfo& info);
    void HandleRemovedRadar(const std::string& id);
    void TryReconnect();
    void ShowConnectionNotification(bool connected);

    mayara_server_pi* m_plugin;
    std::unique_ptr<MayaraClient> m_client;

    // Known radars
    std::map<std::string, std::unique_ptr<RadarDisplay>> m_radars;
    std::set<std::string> m_known_radar_ids;

    // State
    bool m_running;
    bool m_connected;
    bool m_notification_shown;

    // Timing
    wxLongLong m_last_discovery;
    wxLongLong m_last_reconnect_attempt;

    wxCriticalSection m_lock;
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_MANAGER_H_
