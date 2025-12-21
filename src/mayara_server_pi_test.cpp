/*
 * MaYaRa Server Plugin - Test version
 * Tests: wxEvtHandler inheritance + timer
 * Used to isolate DLL load crash
 */

#include "config.h"
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <wx/fileconf.h>
#include "ocpn_plugin.h"

// Simple GeoPosition struct
struct GeoPosition {
    double lat;
    double lon;
    GeoPosition() : lat(0), lon(0) {}
    GeoPosition(double la, double lo) : lat(la), lon(lo) {}
};

// Timer ID
enum { ID_TIMER = wxID_HIGHEST + 1 };

// Plugin icon
static wxBitmap* g_pPluginIcon = nullptr;

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

    // Preferences
    void ShowPreferencesDialog(wxWindow* parent) override;

    // OpenGL overlay - API 1.16 signature
    bool RenderGLOverlayMultiCanvas(wxGLContext* pcontext,
                                     PlugIn_ViewPort* vp,
                                     int canvasIndex) override;

    // Position updates
    void SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) override;

private:
    void OnTimerNotify(wxTimerEvent& event);

    wxWindow* m_parent_window;
    wxFileConfig* m_config;
    int m_tool_id;
    wxTimer* m_timer;

    // Settings
    std::string m_server_host;
    int m_server_port;
    bool m_show_overlay;

    // Position
    GeoPosition m_own_position;
    double m_heading;
    bool m_position_valid;

    DECLARE_EVENT_TABLE()
};

// Event table
BEGIN_EVENT_TABLE(mayara_server_pi, wxEvtHandler)
    EVT_TIMER(ID_TIMER, mayara_server_pi::OnTimerNotify)
END_EVENT_TABLE()

mayara_server_pi::mayara_server_pi(void* ppimgr)
    : opencpn_plugin_116(ppimgr)
    , m_parent_window(nullptr)
    , m_config(nullptr)
    , m_tool_id(-1)
    , m_timer(nullptr)
    , m_server_host("localhost")
    , m_server_port(8080)
    , m_show_overlay(true)
    , m_heading(0.0)
    , m_position_valid(false)
{
    // Create icon
    g_pPluginIcon = new wxBitmap(16, 16);
}

mayara_server_pi::~mayara_server_pi() {
    delete g_pPluginIcon;
    g_pPluginIcon = nullptr;
}

int mayara_server_pi::Init() {
    m_parent_window = GetOCPNCanvasWindow();
    return WANTS_PREFERENCES;
}

bool mayara_server_pi::DeInit() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
        m_timer = nullptr;
    }
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
             "Connects to mayara-server for radar display.");
}

void mayara_server_pi::OnToolbarToolCallback(int id) {
    if (id == m_tool_id) {
        m_show_overlay = !m_show_overlay;
    }
}

void mayara_server_pi::ShowPreferencesDialog(wxWindow* parent) {
    wxMessageBox(_("Preferences not implemented in test version"),
                 _("MaYaRa Server"), wxOK | wxICON_INFORMATION, parent);
}

bool mayara_server_pi::RenderGLOverlayMultiCanvas(
    wxGLContext* pcontext,
    PlugIn_ViewPort* vp,
    int canvasIndex)
{
    return false;  // No rendering in test version
}

void mayara_server_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) {
    m_own_position = GeoPosition(pfix.Lat, pfix.Lon);
    m_heading = pfix.Hdt;
    m_position_valid = true;
}

void mayara_server_pi::OnTimerNotify(wxTimerEvent& event) {
    // Nothing in test version
}

// Plugin factory functions
extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr) {
    return new mayara_server_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
    delete p;
}
