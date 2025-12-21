/*
 * Minimal plugin using API 1.16 - this is what radar_pi uses successfully
 */

#include "config.h"
#include <wx/wx.h>
#include <wx/bitmap.h>
#include "ocpn_plugin.h"

// Dummy bitmap to ensure wxWidgets core is linked
// OpenCPN checks for wxmsw32u_core_vc14x.dll dependency to verify compatibility
static wxBitmap* g_pPluginIcon = nullptr;

// Use opencpn_plugin_116 like radar_pi does - it works with OpenCPN 5.12
class mayara_server_pi_minimal : public opencpn_plugin_116 {
public:
    mayara_server_pi_minimal(void* ppimgr) : opencpn_plugin_116(ppimgr) {
        // Create a tiny bitmap to force wxWidgets core linkage
        g_pPluginIcon = new wxBitmap(16, 16);
    }
    ~mayara_server_pi_minimal() override {
        delete g_pPluginIcon;
        g_pPluginIcon = nullptr;
    }

    int Init() override { return 0; }  // No special capabilities for now
    bool DeInit() override { return true; }

    int GetAPIVersionMajor() override { return 1; }
    int GetAPIVersionMinor() override { return 16; }
    int GetPlugInVersionMajor() override { return 1; }
    int GetPlugInVersionMinor() override { return 0; }

    wxBitmap* GetPlugInBitmap() override { return g_pPluginIcon; }
    wxString GetCommonName() override { return wxString("MaYaRa Server"); }
    wxString GetShortDescription() override { return wxString("Radar display plugin"); }
    wxString GetLongDescription() override { return wxString("Connects to mayara-server for radar display"); }
};

extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr) {
    return new mayara_server_pi_minimal(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
    delete p;
}
