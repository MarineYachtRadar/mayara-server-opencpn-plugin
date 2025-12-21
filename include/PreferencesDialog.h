/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Plugin preferences dialog
 */

#ifndef _PREFERENCES_DIALOG_H_
#define _PREFERENCES_DIALOG_H_

#include "pi_common.h"
#include <wx/spinctrl.h>

// Forward declaration - plugin class is in global namespace
class mayara_server_pi;

PLUGIN_BEGIN_NAMESPACE

class PreferencesDialog : public wxDialog {
public:
    PreferencesDialog(wxWindow* parent, ::mayara_server_pi* plugin);
    ~PreferencesDialog();

    // Get values from dialog
    wxString GetServerHost() const;
    int GetServerPort() const;
    int GetDiscoveryPollInterval() const;
    int GetReconnectInterval() const;
    bool GetShowOverlay() const;
    bool GetShowPPIWindow() const;

private:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnTestConnection(wxCommandEvent& event);

    void CreateControls();
    void LoadSettings();
    void SaveSettings();

    ::mayara_server_pi* m_plugin;

    // Server connection
    wxTextCtrl* m_host_ctrl;
    wxSpinCtrl* m_port_ctrl;

    // Timing
    wxSpinCtrl* m_discovery_interval_ctrl;
    wxSpinCtrl* m_reconnect_interval_ctrl;

    // Display options
    wxCheckBox* m_overlay_checkbox;
    wxCheckBox* m_ppi_checkbox;

    // Status
    wxStaticText* m_status_text;

    DECLARE_EVENT_TABLE()
};

PLUGIN_END_NAMESPACE

#endif  // _PREFERENCES_DIALOG_H_
