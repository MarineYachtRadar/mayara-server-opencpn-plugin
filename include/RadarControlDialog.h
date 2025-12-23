/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar control dialog for adjusting radar settings
 * Now uses dynamic controls from capability manifest
 */

#ifndef _RADAR_CONTROL_DIALOG_H_
#define _RADAR_CONTROL_DIALOG_H_

#include "pi_common.h"
#include "MayaraClient.h"
#include "DynamicControlPanel.h"

// Forward declaration - plugin class is in global namespace
class mayara_server_pi;

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class RadarDisplay;

class RadarControlDialog : public wxDialog {
public:
    RadarControlDialog(wxWindow* parent,
                       ::mayara_server_pi* plugin,
                       RadarDisplay* radar);
    ~RadarControlDialog();

    // Refresh state from server
    void RefreshState();

private:
    void CreateControls();
    void CreatePowerControls(wxSizer* parent);
    void CreateRangeControls(wxSizer* parent);
    void UpdateUI(const RadarState& state);

    // Event handlers
    void OnPowerButton(wxCommandEvent& event);
    void OnRangeChanged(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnTimer(wxTimerEvent& event);

    ::mayara_server_pi* m_plugin;
    RadarDisplay* m_radar;
    MayaraClient* m_client;
    CapabilityManifest m_capabilities;

    // Power controls (special handling - always shown as buttons)
    wxButton* m_power_off_btn;
    wxButton* m_power_standby_btn;
    wxButton* m_power_transmit_btn;

    // Range control (special handling - dropdown with radar's supported ranges)
    wxChoice* m_range_choice;
    std::vector<uint32_t> m_supported_ranges;

    // Dynamic control panel for all other controls
    DynamicControlPanel* m_dynamic_panel;

    // Status display
    wxStaticText* m_status_text;
    wxStaticText* m_model_text;
    wxStaticText* m_spokes_text;

    // Auto-refresh timer
    wxTimer* m_timer;
    bool m_updating_ui;

    DECLARE_EVENT_TABLE()
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_CONTROL_DIALOG_H_
