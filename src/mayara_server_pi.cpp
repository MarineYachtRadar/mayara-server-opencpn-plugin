/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Main plugin implementation
 */

#include "mayara_server_pi.h"
#include "RadarManager.h"
#include "RadarDisplay.h"
#include "RadarOverlayRenderer.h"
#include "PreferencesDialog.h"
#include "icons.h"

#include <wx/fileconf.h>

using namespace mayara_server_pi;

// Timer ID
enum { ID_TIMER = wxID_HIGHEST + 1 };

BEGIN_EVENT_TABLE(mayara_server_pi::mayara_server_pi, wxEvtHandler)
    EVT_TIMER(ID_TIMER, mayara_server_pi::mayara_server_pi::OnTimerNotify)
END_EVENT_TABLE()

// Plugin factory functions
extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr) {
    return new mayara_server_pi::mayara_server_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
    delete p;
}

mayara_server_pi::mayara_server_pi(void* ppimgr)
    : opencpn_plugin_118(ppimgr)
    , m_parent_window(nullptr)
    , m_config(nullptr)
    , m_tool_id(-1)
    , m_icon(nullptr)
    , m_timer(nullptr)
    , m_server_host(DEFAULT_SERVER_HOST)
    , m_server_port(DEFAULT_SERVER_PORT)
    , m_discovery_poll_interval(DEFAULT_DISCOVERY_INTERVAL)
    , m_reconnect_interval(DEFAULT_RECONNECT_INTERVAL)
    , m_show_overlay(true)
    , m_show_ppi_window(false)
    , m_heading(0.0)
    , m_cog(0.0)
    , m_sog(0.0)
    , m_position_valid(false)
{
}

mayara_server_pi::~mayara_server_pi() {
}

int mayara_server_pi::Init() {
    // Get parent window and data directory
    m_parent_window = GetOCPNCanvasWindow();
    m_data_dir = GetPluginDataDir("mayara_server_pi");

    // Initialize icons
    InitializeIcons();

    // Load configuration
    m_config = GetOCPNConfigObject();
    LoadConfig();

    // Add toolbar button
    m_icon = GetToolbarIcon(IconState::Disconnected);
    m_tool_id = InsertPlugInToolSVG(
        _("MaYaRa Server"),
        wxEmptyString,  // SVG not used
        wxEmptyString,
        wxEmptyString,
        wxITEM_CHECK,
        _("MaYaRa Server Radar"),
        wxEmptyString,
        m_icon,
        wxS("MaYaRa Server"),
        -1,
        0
    );

    // Create radar manager
    m_radar_manager = std::make_unique<RadarManager>(this);

    // Start polling timer (100ms)
    m_timer = new wxTimer(this, ID_TIMER);
    m_timer->Start(100);

    // Start radar manager
    m_radar_manager->Start();

    // Return capabilities
    return WANTS_OVERLAY_CALLBACK |
           WANTS_OPENGL_OVERLAY_CALLBACK |
           WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK |
           INSTALLS_TOOLBAR_TOOL |
           WANTS_PREFERENCES |
           WANTS_CONFIG;
}

bool mayara_server_pi::DeInit() {
    // Stop timer
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
        m_timer = nullptr;
    }

    // Stop radar manager
    if (m_radar_manager) {
        m_radar_manager->Stop();
        m_radar_manager.reset();
    }

    // Save configuration
    SaveConfig();

    return true;
}

wxBitmap* mayara_server_pi::GetPlugInBitmap() {
    return GetPluginIcon();
}

wxString mayara_server_pi::GetShortDescription() {
    return _("Displays radar data from mayara-server");
}

wxString mayara_server_pi::GetLongDescription() {
    return _("MaYaRa Server Plugin for OpenCPN\n\n"
             "Connects to a running mayara-server instance to display radar data "
             "from Furuno, Navico, Raymarine, and Garmin radars.\n\n"
             "Features chart overlay and separate PPI window display modes.");
}

void mayara_server_pi::OnToolbarToolCallback(int id) {
    if (id == m_tool_id) {
        // Toggle radar display
        m_show_overlay = !m_show_overlay;
        SetToolbarItemState(id, m_show_overlay);
        SaveConfig();
    }
}

void mayara_server_pi::SetToolbarItemState(int id, bool enable) {
    SetToolbarItemState(m_tool_id, enable);
}

void mayara_server_pi::UpdateToolbarIcon() {
    IconState state = IconState::Disconnected;

    if (m_radar_manager && m_radar_manager->IsConnected()) {
        auto radars = m_radar_manager->GetActiveRadars();
        bool transmitting = false;

        for (auto* radar : radars) {
            if (radar->GetStatus() == RadarStatus::Transmit) {
                transmitting = true;
                break;
            }
        }

        state = transmitting ? IconState::Transmit : IconState::Standby;
    }

    wxBitmap* icon = GetToolbarIcon(state);
    if (icon && icon != m_icon) {
        m_icon = icon;
        SetToolbarToolBitmaps(m_tool_id, m_icon, m_icon);
    }
}

void mayara_server_pi::ShowPreferencesDialog(wxWindow* parent) {
    PreferencesDialog dlg(parent, this);
    if (dlg.ShowModal() == wxID_OK) {
        // Settings are saved by dialog
        SaveConfig();

        // Restart radar manager with new settings
        if (m_radar_manager) {
            m_radar_manager->Stop();
            m_radar_manager->Start();
        }
    }
}

bool mayara_server_pi::RenderGLOverlayMultiCanvas(
    wxGLContext* pcontext,
    PlugIn_ViewPort* vp,
    int canvasIndex,
    int priority)
{
    // Only render at OVERLAY_LEGACY priority
    if (priority != OVERLAY_LEGACY) return false;

    // Check if overlay is enabled
    if (!m_show_overlay) return false;

    // Check if we have valid position
    if (!m_position_valid) return false;

    // Check if radar manager is running
    if (!m_radar_manager || !m_radar_manager->IsConnected()) return false;

    // Render each active radar
    for (auto* radar : m_radar_manager->GetActiveRadars()) {
        if (!radar || radar->GetStatus() != RadarStatus::Transmit) continue;

        auto* renderer = radar->GetOverlayRenderer();
        if (renderer && renderer->IsInitialized()) {
            renderer->UpdateTexture(radar->GetSpokeBuffer());
            renderer->DrawOverlay(
                pcontext,
                vp,
                radar->GetRangeMeters(),
                m_own_position,
                m_heading
            );
        }
    }

    return true;
}

void mayara_server_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) {
    m_own_position = GeoPosition(pfix.Lat, pfix.Lon);
    m_heading = pfix.Hdt;
    m_cog = pfix.Cog;
    m_sog = pfix.Sog;
    m_position_valid = true;
}

void mayara_server_pi::OnTimerNotify(wxTimerEvent& event) {
    // Poll radar manager
    if (m_radar_manager) {
        m_radar_manager->Poll();
    }

    // Update toolbar icon based on connection state
    UpdateToolbarIcon();

    // Request canvas refresh if overlay enabled and connected
    if (m_show_overlay && m_radar_manager && m_radar_manager->IsConnected()) {
        RequestRefresh(GetOCPNCanvasWindow());
    }
}

bool mayara_server_pi::LoadConfig() {
    if (!m_config) return false;

    m_config->SetPath("/PlugIns/MaYaRaServer");

    wxString host;
    if (m_config->Read("ServerHost", &host)) {
        m_server_host = host.ToStdString();
    }
    m_config->Read("ServerPort", &m_server_port, DEFAULT_SERVER_PORT);
    m_config->Read("DiscoveryInterval", &m_discovery_poll_interval, DEFAULT_DISCOVERY_INTERVAL);
    m_config->Read("ReconnectInterval", &m_reconnect_interval, DEFAULT_RECONNECT_INTERVAL);
    m_config->Read("ShowOverlay", &m_show_overlay, true);
    m_config->Read("ShowPPIWindow", &m_show_ppi_window, false);

    return true;
}

bool mayara_server_pi::SaveConfig() {
    if (!m_config) return false;

    m_config->SetPath("/PlugIns/MaYaRaServer");

    m_config->Write("ServerHost", wxString(m_server_host));
    m_config->Write("ServerPort", m_server_port);
    m_config->Write("DiscoveryInterval", m_discovery_poll_interval);
    m_config->Write("ReconnectInterval", m_reconnect_interval);
    m_config->Write("ShowOverlay", m_show_overlay);
    m_config->Write("ShowPPIWindow", m_show_ppi_window);

    return true;
}
