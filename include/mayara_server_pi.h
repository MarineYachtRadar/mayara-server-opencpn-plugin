/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Main plugin class - connects to mayara-server for radar display
 */

#ifndef _MAYARA_SERVER_PI_H_
#define _MAYARA_SERVER_PI_H_

#include "pi_common.h"
#include <memory>
#include <string>

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class RadarManager;
class PreferencesDialog;

class mayara_server_pi : public opencpn_plugin_118, public wxEvtHandler {
public:
    mayara_server_pi(void* ppimgr);
    ~mayara_server_pi() override;

    // -------- Required plugin methods --------
    int Init() override;
    bool DeInit() override;

    int GetAPIVersionMajor() override { return 1; }
    int GetAPIVersionMinor() override { return 18; }
    int GetPlugInVersionMajor() override { return VERSION_MAJOR; }
    int GetPlugInVersionMinor() override { return VERSION_MINOR; }

    wxBitmap* GetPlugInBitmap() override;
    wxString GetCommonName() override { return _("MaYaRa Server"); }
    wxString GetShortDescription() override;
    wxString GetLongDescription() override;

    // -------- Toolbar --------
    int GetToolbarToolCount() override { return 1; }
    void OnToolbarToolCallback(int id) override;
    void SetToolbarItemState(int id, bool enable);
    void UpdateToolbarIcon();

    // -------- Preferences --------
    void ShowPreferencesDialog(wxWindow* parent) override;

    // -------- OpenGL overlay rendering --------
    bool RenderGLOverlayMultiCanvas(wxGLContext* pcontext,
                                     PlugIn_ViewPort* vp,
                                     int canvasIndex,
                                     int priority) override;

    // -------- Position updates --------
    void SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) override;

    // -------- Configuration --------
    bool LoadConfig();
    bool SaveConfig();

    // -------- Accessors --------
    wxWindow* GetParentWindow() { return m_parent_window; }
    wxString GetDataDir() { return m_data_dir; }

    // Settings accessors
    std::string GetServerHost() const { return m_server_host; }
    int GetServerPort() const { return m_server_port; }
    int GetDiscoveryPollInterval() const { return m_discovery_poll_interval; }
    int GetReconnectInterval() const { return m_reconnect_interval; }
    bool GetShowOverlay() const { return m_show_overlay; }
    bool GetShowPPIWindow() const { return m_show_ppi_window; }

    void SetServerHost(const std::string& host) { m_server_host = host; }
    void SetServerPort(int port) { m_server_port = port; }
    void SetDiscoveryPollInterval(int interval) { m_discovery_poll_interval = interval; }
    void SetReconnectInterval(int interval) { m_reconnect_interval = interval; }
    void SetShowOverlay(bool show) { m_show_overlay = show; }
    void SetShowPPIWindow(bool show) { m_show_ppi_window = show; }

    // Position accessors
    GeoPosition GetOwnPosition() const { return m_own_position; }
    double GetHeading() const { return m_heading; }
    bool IsPositionValid() const { return m_position_valid; }

    // Radar manager accessor
    RadarManager* GetRadarManager() { return m_radar_manager.get(); }

private:
    void OnTimerNotify(wxTimerEvent& event);

    wxWindow* m_parent_window;
    wxFileConfig* m_config;
    wxString m_data_dir;
    int m_tool_id;
    wxBitmap* m_icon;
    wxTimer* m_timer;

    // Settings
    std::string m_server_host;
    int m_server_port;
    int m_discovery_poll_interval;
    int m_reconnect_interval;
    bool m_show_overlay;
    bool m_show_ppi_window;

    // Radar management
    std::unique_ptr<RadarManager> m_radar_manager;

    // Position data from OpenCPN
    GeoPosition m_own_position;
    double m_heading;
    double m_cog;
    double m_sog;
    bool m_position_valid;

    DECLARE_EVENT_TABLE()
};

PLUGIN_END_NAMESPACE

#endif  // _MAYARA_SERVER_PI_H_
