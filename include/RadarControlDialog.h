/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar control dialog for adjusting radar settings
 */

#ifndef _RADAR_CONTROL_DIALOG_H_
#define _RADAR_CONTROL_DIALOG_H_

#include "pi_common.h"
#include "MayaraClient.h"

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class mayara_server_pi;
class RadarDisplay;

class RadarControlDialog : public wxDialog {
public:
    RadarControlDialog(wxWindow* parent,
                       mayara_server_pi* plugin,
                       RadarDisplay* radar);
    ~RadarControlDialog();

    // Refresh state from server
    void RefreshState();

private:
    void CreateControls();
    void UpdateUI(const RadarState& state);

    // Event handlers
    void OnPowerButton(wxCommandEvent& event);
    void OnRangeChanged(wxCommandEvent& event);
    void OnGainChanged(wxScrollEvent& event);
    void OnGainAutoChanged(wxCommandEvent& event);
    void OnSeaChanged(wxScrollEvent& event);
    void OnSeaAutoChanged(wxCommandEvent& event);
    void OnRainChanged(wxScrollEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnTimer(wxTimerEvent& event);

    mayara_server_pi* m_plugin;
    RadarDisplay* m_radar;
    MayaraClient* m_client;

    // Power controls
    wxButton* m_power_off_btn;
    wxButton* m_power_standby_btn;
    wxButton* m_power_transmit_btn;

    // Range control
    wxChoice* m_range_choice;

    // Gain control
    wxSlider* m_gain_slider;
    wxCheckBox* m_gain_auto_checkbox;
    wxStaticText* m_gain_value_text;

    // Sea clutter control
    wxSlider* m_sea_slider;
    wxCheckBox* m_sea_auto_checkbox;
    wxStaticText* m_sea_value_text;

    // Rain clutter control
    wxSlider* m_rain_slider;
    wxStaticText* m_rain_value_text;

    // Status display
    wxStaticText* m_status_text;
    wxStaticText* m_spokes_text;

    // Auto-refresh timer
    wxTimer* m_timer;
    bool m_updating_ui;

    DECLARE_EVENT_TABLE()
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_CONTROL_DIALOG_H_
