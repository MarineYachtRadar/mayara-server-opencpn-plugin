/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Main plugin implementation - inline class definition to avoid DLL static init issues
 */

// Use same includes as working test version
#include "config.h"
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <wx/fileconf.h>
#include "ocpn_plugin.h"

// Include support files AFTER basic wx includes
#include "RadarManager.h"
#include "RadarDisplay.h"
#include "RadarOverlayRenderer.h"
#include "RadarControlDialog.h"
#include "PreferencesDialog.h"
#include "icons.h"

#include <ixwebsocket/IXNetSystem.h>
#include <memory>
#include <string>

// Timer ID
enum {
    ID_TIMER = wxID_HIGHEST + 1
};

// Default settings (copied from pi_common.h to avoid include)
#define DEFAULT_SERVER_HOST "localhost"
#define DEFAULT_SERVER_PORT 6502
#define DEFAULT_DISCOVERY_INTERVAL 10
#define DEFAULT_RECONNECT_INTERVAL 5

// Plugin icon
static wxBitmap* g_pPluginIcon = nullptr;

// Plugin class defined inline (not from header) to avoid DLL static init crash
class mayara_server_pi : public opencpn_plugin_116, public wxEvtHandler {
public:
    mayara_server_pi(void* ppimgr);
    ~mayara_server_pi() override;

    // Required plugin methods
    int Init() override;
    bool DeInit() override;

    int GetAPIVersionMajor() override { return 1; }
    int GetAPIVersionMinor() override { return 16; }
    int GetPlugInVersionMajor() override { return VERSION_MAJOR; }
    int GetPlugInVersionMinor() override { return VERSION_MINOR; }

    wxBitmap* GetPlugInBitmap() override;
    wxString GetCommonName() override { return _("MaYaRa Server"); }
    wxString GetShortDescription() override;
    wxString GetLongDescription() override;

    // Toolbar
    int GetToolbarToolCount() override { return 1; }
    void OnToolbarToolCallback(int id) override;
    void SetToolbarItemState(int id, bool enable);
    void UpdateToolbarIcon();

    // Preferences
    void ShowPreferencesDialog(wxWindow* parent) override;

    // OpenGL overlay - API 1.16 signature
    bool RenderGLOverlayMultiCanvas(wxGLContext* pcontext,
                                     PlugIn_ViewPort* vp,
                                     int canvasIndex) override;

    // Position updates
    void SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) override;

    // Configuration
    bool LoadConfig();
    bool SaveConfig();

    // Accessors
    wxWindow* GetParentWindow() { return m_parent_window; }
    wxString GetDataDir() { return m_data_dir; }

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

    GeoPosition GetOwnPosition() const { return m_own_position; }
    double GetHeading() const { return m_heading; }
    bool IsPositionValid() const { return m_position_valid; }

    mayara::RadarManager* GetRadarManager() { return m_radar_manager.get(); }

private:
    void OnTimerNotify(wxTimerEvent& event);
    void ShowRadarControlDialog();

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
    std::unique_ptr<mayara::RadarManager> m_radar_manager;

    // Position data from OpenCPN
    GeoPosition m_own_position;
    double m_heading;
    double m_cog;
    double m_sog;
    bool m_position_valid;

    DECLARE_EVENT_TABLE()
};

// Event table
BEGIN_EVENT_TABLE(mayara_server_pi, wxEvtHandler)
    EVT_TIMER(ID_TIMER, mayara_server_pi::OnTimerNotify)
END_EVENT_TABLE()

// Plugin factory functions
extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr) {
    return new mayara_server_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
    delete p;
}

mayara_server_pi::mayara_server_pi(void* ppimgr)
    : opencpn_plugin_116(ppimgr)
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
    g_pPluginIcon = new wxBitmap(16, 16);
}

mayara_server_pi::~mayara_server_pi() {
    delete g_pPluginIcon;
    g_pPluginIcon = nullptr;
}

int mayara_server_pi::Init() {
    // Initialize IXWebSocket network system (required for Windows WSAStartup)
    ix::initNetSystem();

    m_parent_window = GetOCPNCanvasWindow();
    m_data_dir = GetPluginDataDir("MaYaRaServer");

    // Load configuration
    m_config = GetOCPNConfigObject();
    LoadConfig();

    // Initialize icons
    mayara::InitializeIcons();

    // Add toolbar button - overlay toggle (starts disabled/gray)
    wxBitmap* icon = mayara::GetToolbarIcon(mayara::IconState::Disconnected);
    m_tool_id = InsertPlugInTool(
        _T(""),                 // Label
        icon, icon,             // Normal and rollover bitmaps
        wxITEM_CHECK,           // Check item (toggle on/off)
        _("MaYaRa Radar"),      // Short help
        _("Click to connect, right-click for controls"), // Long help
        nullptr,                // Client data
        -1,                     // Position
        0,                      // Tool selection
        this                    // Owner
    );

    // Don't start radar manager or timer here - wait for user to click toolbar
    // User must click the toolbar icon to start radar connection

    return WANTS_PREFERENCES | WANTS_OPENGL_OVERLAY_CALLBACK | WANTS_NMEA_EVENTS | INSTALLS_TOOLBAR_TOOL;
}

bool mayara_server_pi::DeInit() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
        m_timer = nullptr;
    }

    if (m_radar_manager) {
        m_radar_manager->Stop();
        m_radar_manager.reset();
    }

    SaveConfig();
    ix::uninitNetSystem();

    return true;
}

wxBitmap* mayara_server_pi::GetPlugInBitmap() {
    return g_pPluginIcon;
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
    wxLogMessage("MaYaRa: OnToolbarToolCallback id=%d, m_tool_id=%d", id, m_tool_id);
    if (id == m_tool_id) {
        wxLogMessage("MaYaRa: Toolbar match, overlay=%d, manager=%p",
                     m_show_overlay, (void*)m_radar_manager.get());
        // If already ON and connected, show control dialog instead of toggling off
        if (m_show_overlay && m_radar_manager && m_radar_manager->IsConnected()) {
            wxLogMessage("MaYaRa: Already connected, showing controls");
            ShowRadarControlDialog();
            return;
        }

        // Toggle overlay on/off
        m_show_overlay = !m_show_overlay;
        SetToolbarItemState(id, m_show_overlay);

        wxLogMessage("MaYaRa: Toolbar clicked, overlay=%s", m_show_overlay ? "ON" : "OFF");

        if (m_show_overlay) {
            // Start radar connection
            wxLogMessage("MaYaRa: Starting radar manager...");
            if (!m_radar_manager) {
                m_radar_manager = std::make_unique<mayara::RadarManager>(this);
            }
            m_radar_manager->Start();

            if (!m_timer) {
                m_timer = new wxTimer(this, ID_TIMER);
            }
            m_timer->Start(100);  // 10 Hz refresh
            wxLogMessage("MaYaRa: Timer started");
        } else {
            // Stop radar connection
            wxLogMessage("MaYaRa: Stopping radar...");
            if (m_timer) {
                m_timer->Stop();
            }
            if (m_radar_manager) {
                m_radar_manager->Stop();
            }
        }

        SaveConfig();
    }
}

void mayara_server_pi::ShowRadarControlDialog() {
    wxLogMessage("MaYaRa: ShowRadarControlDialog - entry");
    if (!m_radar_manager) {
        wxLogMessage("MaYaRa: ShowRadarControlDialog - no radar manager");
        return;
    }

    wxLogMessage("MaYaRa: ShowRadarControlDialog - getting active radars");
    auto radars = m_radar_manager->GetActiveRadars();
    if (radars.empty()) {
        wxLogMessage("MaYaRa: No active radars to control");
        return;
    }

    wxLogMessage("MaYaRa: ShowRadarControlDialog - have %u radars, parent=%p",
                 (unsigned)radars.size(), (void*)m_parent_window);

    // Show dialog for first radar (could extend to show selection if multiple)
    wxLogMessage("MaYaRa: ShowRadarControlDialog - creating dialog");
    mayara::RadarControlDialog dlg(m_parent_window, this, radars[0]);
    wxLogMessage("MaYaRa: ShowRadarControlDialog - showing modal");
    dlg.ShowModal();
    wxLogMessage("MaYaRa: ShowRadarControlDialog - done");
}

void mayara_server_pi::SetToolbarItemState(int id, bool enable) {
    ::SetToolbarItemState(m_tool_id, enable);
}

void mayara_server_pi::UpdateToolbarIcon() {
    mayara::IconState state = mayara::IconState::Disconnected;

    if (m_radar_manager && m_radar_manager->IsConnected()) {
        auto radars = m_radar_manager->GetActiveRadars();
        bool transmitting = false;

        for (auto* radar : radars) {
            if (radar->GetStatus() == RadarStatus::Transmit) {
                transmitting = true;
                break;
            }
        }

        state = transmitting ? mayara::IconState::Transmit : mayara::IconState::Standby;
    }

    wxBitmap* icon = mayara::GetToolbarIcon(state);
    if (icon && icon != m_icon) {
        m_icon = icon;
        SetToolbarToolBitmaps(m_tool_id, m_icon, m_icon);
    }
}

void mayara_server_pi::ShowPreferencesDialog(wxWindow* parent) {
    mayara::PreferencesDialog dlg(parent, this);
    if (dlg.ShowModal() == wxID_OK) {
        SaveConfig();
        if (m_radar_manager) {
            m_radar_manager->Stop();
            m_radar_manager->Start();
        }
    }
}

bool mayara_server_pi::RenderGLOverlayMultiCanvas(
    wxGLContext* pcontext,
    PlugIn_ViewPort* vp,
    int canvasIndex)
{
    static int log_counter = 0;
    log_counter++;

    // Log first 5 renders AND every 100 frames
    bool do_log = (log_counter <= 5) || (log_counter % 100 == 0);

    try {
        if (log_counter <= 5) {
            wxLogMessage("MaYaRa: RenderGLOverlay #%d entry, overlay=%d, vp=%p",
                         log_counter, m_show_overlay, (void*)vp);
            wxLog::FlushActive();
        }

        if (!m_show_overlay) return false;

        if (!m_position_valid) {
            if (do_log) wxLogMessage("MaYaRa: No position fix yet");
            return false;
        }
        if (!m_radar_manager) {
            if (do_log) wxLogMessage("MaYaRa: No radar manager");
            return false;
        }
        if (!m_radar_manager->IsConnected()) {
            if (do_log) wxLogMessage("MaYaRa: Not connected to server");
            return false;
        }

        if (do_log) wxLogMessage("MaYaRa: RenderGLOverlay - getting active radars");
        auto radars = m_radar_manager->GetActiveRadars();
        if (do_log) {
            wxLogMessage("MaYaRa: Got %u active radar(s)", (unsigned)radars.size());
            wxLog::FlushActive();
        }

        for (auto* radar : radars) {
            if (!radar) {
                if (do_log) wxLogMessage("MaYaRa: Skipping null radar");
                continue;
            }

            if (do_log) {
                wxLogMessage("MaYaRa: Processing radar %s", radar->GetId().c_str());
                wxLog::FlushActive();
            }
            RadarStatus status = radar->GetStatus();
            if (do_log) {
                wxLogMessage("MaYaRa: Radar status=%d (Transmit=2)", (int)status);
                wxLog::FlushActive();
            }

            // Skip if not transmitting - radar loop continues but no overlay drawn
            if (status != RadarStatus::Transmit) continue;

            // NOTE: Don't start spoke receiver here - it's started from OnTimerNotify
            // to avoid threading issues with IXWebSocket on OpenGL thread

            if (do_log) {
                wxLogMessage("MaYaRa: Getting overlay renderer");
                wxLog::FlushActive();
            }
            auto* renderer = radar->GetOverlayRenderer();
            if (do_log) {
                wxLogMessage("MaYaRa: GetOverlayRenderer returned %p", (void*)renderer);
                wxLog::FlushActive();
            }
            if (!renderer) {
                if (do_log) wxLogMessage("MaYaRa: No overlay renderer");
                continue;
            }

            // TEMPORARY: Return early - got renderer but skip OpenGL init/rendering
            if (do_log) {
                wxLogMessage("MaYaRa: Got renderer=%p, returning early for debug", (void*)renderer);
                wxLog::FlushActive();
            }
            return false;

            // Initialize renderer on first use (when GL context is active)
            if (!renderer->IsInitialized()) {
                if (do_log) wxLogMessage("MaYaRa: Initializing renderer");
                renderer->Init(radar->GetSpokesPerRevolution(), radar->GetMaxSpokeLength());
                if (do_log) wxLogMessage("MaYaRa: Renderer Init() returned");
            }
            if (!renderer->IsInitialized()) {
                if (do_log) wxLogMessage("MaYaRa: Renderer init failed");
                continue;
            }

            if (do_log) wxLogMessage("MaYaRa: Drawing overlay at range %.0fm", radar->GetRangeMeters());

            if (do_log) wxLogMessage("MaYaRa: Calling UpdateTexture");
            renderer->UpdateTexture(radar->GetSpokeBuffer());
            if (do_log) wxLogMessage("MaYaRa: Calling DrawOverlay");
            renderer->DrawOverlay(
                pcontext,
                vp,
                radar->GetRangeMeters(),
                m_own_position,
                m_heading
            );
            if (do_log) wxLogMessage("MaYaRa: DrawOverlay returned");
        }

        if (do_log) wxLogMessage("MaYaRa: RenderGLOverlay complete");
        return true;
    } catch (const std::exception& e) {
        wxLogMessage("MaYaRa: RenderGLOverlay EXCEPTION: %s", e.what());
        return false;
    } catch (...) {
        wxLogMessage("MaYaRa: RenderGLOverlay UNKNOWN EXCEPTION");
        return false;
    }
}

void mayara_server_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) {
    m_own_position = GeoPosition(pfix.Lat, pfix.Lon);
    m_heading = pfix.Hdt;
    m_cog = pfix.Cog;
    m_sog = pfix.Sog;
    m_position_valid = true;
}

void mayara_server_pi::OnTimerNotify(wxTimerEvent& event) {
    static int timer_count = 0;
    timer_count++;
    if (timer_count <= 3) {
        wxLogMessage("MaYaRa: OnTimerNotify #%d", timer_count);
    }

    if (m_radar_manager) {
        if (timer_count <= 3) wxLogMessage("MaYaRa: Calling Poll");
        m_radar_manager->Poll();

        // DISABLED: Spoke receiver causes crash in IXWebSocket constructor on Windows
        // TODO: Fix IXWebSocket linking/initialization issue
        // For now, just test the control dialog without spoke streaming
        /*
        if (m_show_overlay && m_radar_manager->IsConnected()) {
            auto radars = m_radar_manager->GetActiveRadars();
            for (auto* radar : radars) {
                if (radar && radar->GetStatus() == RadarStatus::Transmit && !radar->IsReceiving()) {
                    if (timer_count <= 5) {
                        wxLogMessage("MaYaRa: Starting spoke receiver for %s from timer", radar->GetId().c_str());
                        wxLog::FlushActive();
                    }
                    radar->Start();
                    if (timer_count <= 5) {
                        wxLogMessage("MaYaRa: Spoke receiver started");
                        wxLog::FlushActive();
                    }
                }
            }
        }
        */
    }
    if (timer_count <= 3) wxLogMessage("MaYaRa: Calling UpdateToolbarIcon");
    UpdateToolbarIcon();
    if (m_show_overlay && m_radar_manager && m_radar_manager->IsConnected()) {
        wxWindow* canvas = GetOCPNCanvasWindow();
        if (timer_count <= 3) wxLogMessage("MaYaRa: Canvas window=%p", (void*)canvas);
        if (canvas) {
            if (timer_count <= 3) {
                wxLogMessage("MaYaRa: Requesting refresh");
                wxLog::FlushActive();
            }
            RequestRefresh(canvas);
            if (timer_count <= 3) {
                wxLogMessage("MaYaRa: Refresh requested OK");
                wxLog::FlushActive();
            }
        }
    }
    if (timer_count <= 3) {
        wxLogMessage("MaYaRa: OnTimerNotify #%d complete", timer_count);
        wxLog::FlushActive();
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
    // Don't load ShowOverlay from config - always start with overlay OFF
    // User must click toolbar to activate
    m_show_overlay = false;
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
