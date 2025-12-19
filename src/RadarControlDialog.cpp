/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar control dialog for adjusting radar settings
 */

#include "RadarControlDialog.h"
#include "mayara_server_pi.h"
#include "RadarManager.h"
#include "RadarDisplay.h"

using namespace mayara_server_pi;

enum {
    ID_POWER_OFF = wxID_HIGHEST + 200,
    ID_POWER_STANDBY,
    ID_POWER_TRANSMIT,
    ID_RANGE_CHOICE,
    ID_GAIN_SLIDER,
    ID_GAIN_AUTO,
    ID_SEA_SLIDER,
    ID_SEA_AUTO,
    ID_RAIN_SLIDER,
    ID_REFRESH,
    ID_TIMER
};

// Common range values in meters
static const double RANGE_VALUES[] = {
    125, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000,
    8000, 12000, 16000, 24000, 36000, 48000, 64000, 96000
};
static const int NUM_RANGES = sizeof(RANGE_VALUES) / sizeof(RANGE_VALUES[0]);

BEGIN_EVENT_TABLE(RadarControlDialog, wxDialog)
    EVT_BUTTON(ID_POWER_OFF, RadarControlDialog::OnPowerButton)
    EVT_BUTTON(ID_POWER_STANDBY, RadarControlDialog::OnPowerButton)
    EVT_BUTTON(ID_POWER_TRANSMIT, RadarControlDialog::OnPowerButton)
    EVT_CHOICE(ID_RANGE_CHOICE, RadarControlDialog::OnRangeChanged)
    EVT_COMMAND_SCROLL(ID_GAIN_SLIDER, RadarControlDialog::OnGainChanged)
    EVT_CHECKBOX(ID_GAIN_AUTO, RadarControlDialog::OnGainAutoChanged)
    EVT_COMMAND_SCROLL(ID_SEA_SLIDER, RadarControlDialog::OnSeaChanged)
    EVT_CHECKBOX(ID_SEA_AUTO, RadarControlDialog::OnSeaAutoChanged)
    EVT_COMMAND_SCROLL(ID_RAIN_SLIDER, RadarControlDialog::OnRainChanged)
    EVT_BUTTON(ID_REFRESH, RadarControlDialog::OnRefresh)
    EVT_CLOSE(RadarControlDialog::OnClose)
    EVT_TIMER(ID_TIMER, RadarControlDialog::OnTimer)
END_EVENT_TABLE()

RadarControlDialog::RadarControlDialog(wxWindow* parent,
                                       mayara_server_pi* plugin,
                                       RadarDisplay* radar)
    : wxDialog(parent, wxID_ANY,
               wxString::Format(_("Radar Controls: %s"), radar->GetName()),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_plugin(plugin)
    , m_radar(radar)
    , m_client(nullptr)
    , m_timer(nullptr)
    , m_updating_ui(false)
{
    auto* manager = plugin->GetRadarManager();
    if (manager) {
        m_client = manager->GetClient();
    }

    CreateControls();
    RefreshState();

    // Start auto-refresh timer (every 2 seconds)
    m_timer = new wxTimer(this, ID_TIMER);
    m_timer->Start(2000);

    SetMinSize(wxSize(350, 400));
    Fit();
    Centre();
}

RadarControlDialog::~RadarControlDialog() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
    }
}

void RadarControlDialog::CreateControls() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Status
    m_status_text = new wxStaticText(this, wxID_ANY, _("Status: Unknown"));
    mainSizer->Add(m_status_text, 0, wxALL, 10);

    // Power control
    wxStaticBoxSizer* powerBox = new wxStaticBoxSizer(
        wxHORIZONTAL, this, _("Power"));

    m_power_off_btn = new wxButton(this, ID_POWER_OFF, _("Off"));
    m_power_standby_btn = new wxButton(this, ID_POWER_STANDBY, _("Standby"));
    m_power_transmit_btn = new wxButton(this, ID_POWER_TRANSMIT, _("Transmit"));

    powerBox->Add(m_power_off_btn, 1, wxALL, 5);
    powerBox->Add(m_power_standby_btn, 1, wxALL, 5);
    powerBox->Add(m_power_transmit_btn, 1, wxALL, 5);

    mainSizer->Add(powerBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Range control
    wxStaticBoxSizer* rangeBox = new wxStaticBoxSizer(
        wxHORIZONTAL, this, _("Range"));

    wxArrayString rangeChoices;
    for (int i = 0; i < NUM_RANGES; i++) {
        double range = RANGE_VALUES[i];
        if (range < 1000) {
            rangeChoices.Add(wxString::Format("%.0f m", range));
        } else {
            rangeChoices.Add(wxString::Format("%.1f km", range / 1000));
        }
    }
    m_range_choice = new wxChoice(this, ID_RANGE_CHOICE, wxDefaultPosition,
                                   wxDefaultSize, rangeChoices);
    rangeBox->Add(m_range_choice, 1, wxALL, 5);

    mainSizer->Add(rangeBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Gain control
    wxStaticBoxSizer* gainBox = new wxStaticBoxSizer(
        wxVERTICAL, this, _("Gain"));

    wxBoxSizer* gainRow = new wxBoxSizer(wxHORIZONTAL);
    m_gain_auto_checkbox = new wxCheckBox(this, ID_GAIN_AUTO, _("Auto"));
    m_gain_slider = new wxSlider(this, ID_GAIN_SLIDER, 50, 0, 100);
    m_gain_value_text = new wxStaticText(this, wxID_ANY, "50%",
                                          wxDefaultPosition, wxSize(40, -1));
    gainRow->Add(m_gain_auto_checkbox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    gainRow->Add(m_gain_slider, 1, wxALL, 5);
    gainRow->Add(m_gain_value_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    gainBox->Add(gainRow, 0, wxEXPAND);

    mainSizer->Add(gainBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Sea clutter control
    wxStaticBoxSizer* seaBox = new wxStaticBoxSizer(
        wxVERTICAL, this, _("Sea Clutter"));

    wxBoxSizer* seaRow = new wxBoxSizer(wxHORIZONTAL);
    m_sea_auto_checkbox = new wxCheckBox(this, ID_SEA_AUTO, _("Auto"));
    m_sea_slider = new wxSlider(this, ID_SEA_SLIDER, 50, 0, 100);
    m_sea_value_text = new wxStaticText(this, wxID_ANY, "50%",
                                         wxDefaultPosition, wxSize(40, -1));
    seaRow->Add(m_sea_auto_checkbox, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    seaRow->Add(m_sea_slider, 1, wxALL, 5);
    seaRow->Add(m_sea_value_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    seaBox->Add(seaRow, 0, wxEXPAND);

    mainSizer->Add(seaBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Rain clutter control
    wxStaticBoxSizer* rainBox = new wxStaticBoxSizer(
        wxVERTICAL, this, _("Rain Clutter"));

    wxBoxSizer* rainRow = new wxBoxSizer(wxHORIZONTAL);
    m_rain_slider = new wxSlider(this, ID_RAIN_SLIDER, 0, 0, 100);
    m_rain_value_text = new wxStaticText(this, wxID_ANY, "0%",
                                          wxDefaultPosition, wxSize(40, -1));
    rainRow->Add(m_rain_slider, 1, wxALL, 5);
    rainRow->Add(m_rain_value_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    rainBox->Add(rainRow, 0, wxEXPAND);

    mainSizer->Add(rainBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Statistics
    m_spokes_text = new wxStaticText(this, wxID_ANY, _("Spokes received: 0"));
    mainSizer->Add(m_spokes_text, 0, wxALL, 10);

    // Refresh button
    wxButton* refreshBtn = new wxButton(this, ID_REFRESH, _("Refresh"));
    mainSizer->Add(refreshBtn, 0, wxALL | wxALIGN_CENTER, 10);

    SetSizer(mainSizer);
}

void RadarControlDialog::RefreshState() {
    if (!m_client || !m_radar) return;

    auto state = m_client->GetState(m_radar->GetId());
    UpdateUI(state);
}

void RadarControlDialog::UpdateUI(const RadarState& state) {
    m_updating_ui = true;

    // Status
    wxString statusStr = _("Status: ");
    switch (state.status) {
        case RadarStatus::Off: statusStr += _("Off"); break;
        case RadarStatus::Standby: statusStr += _("Standby"); break;
        case RadarStatus::Transmit: statusStr += _("Transmitting"); break;
        default: statusStr += _("Unknown"); break;
    }
    m_status_text->SetLabel(statusStr);

    // Power buttons
    m_power_off_btn->Enable(state.status != RadarStatus::Off);
    m_power_standby_btn->Enable(state.status != RadarStatus::Standby);
    m_power_transmit_btn->Enable(state.status != RadarStatus::Transmit);

    // Range
    double range = state.rangeMeters;
    int rangeIdx = 0;
    for (int i = 0; i < NUM_RANGES; i++) {
        if (range >= RANGE_VALUES[i]) {
            rangeIdx = i;
        }
    }
    m_range_choice->SetSelection(rangeIdx);

    // Controls
    auto updateControl = [&](const std::string& name,
                              wxSlider* slider,
                              wxCheckBox* autoBox,
                              wxStaticText* valueText) {
        if (state.controls.count(name)) {
            auto& cv = state.controls.at(name);
            slider->SetValue(cv.value);
            valueText->SetLabel(wxString::Format("%d%%", cv.value));
            if (autoBox) {
                bool isAuto = (cv.mode == "auto");
                autoBox->SetValue(isAuto);
                slider->Enable(!isAuto);
            }
        }
    };

    updateControl("gain", m_gain_slider, m_gain_auto_checkbox, m_gain_value_text);
    updateControl("sea", m_sea_slider, m_sea_auto_checkbox, m_sea_value_text);
    updateControl("rain", m_rain_slider, nullptr, m_rain_value_text);

    m_updating_ui = false;
}

void RadarControlDialog::OnPowerButton(wxCommandEvent& event) {
    if (!m_client || !m_radar) return;

    RadarStatus status;
    switch (event.GetId()) {
        case ID_POWER_OFF: status = RadarStatus::Off; break;
        case ID_POWER_STANDBY: status = RadarStatus::Standby; break;
        case ID_POWER_TRANSMIT: status = RadarStatus::Transmit; break;
        default: return;
    }

    m_client->SetPower(m_radar->GetId(), status);
    RefreshState();
}

void RadarControlDialog::OnRangeChanged(wxCommandEvent& event) {
    if (m_updating_ui || !m_client || !m_radar) return;

    int idx = m_range_choice->GetSelection();
    if (idx >= 0 && idx < NUM_RANGES) {
        m_client->SetRange(m_radar->GetId(), RANGE_VALUES[idx]);
    }
}

void RadarControlDialog::OnGainChanged(wxScrollEvent& event) {
    if (m_updating_ui || !m_client || !m_radar) return;

    int value = m_gain_slider->GetValue();
    m_gain_value_text->SetLabel(wxString::Format("%d%%", value));

    bool autoMode = m_gain_auto_checkbox->GetValue();
    m_client->SetGain(m_radar->GetId(), value, autoMode);
}

void RadarControlDialog::OnGainAutoChanged(wxCommandEvent& event) {
    if (m_updating_ui || !m_client || !m_radar) return;

    bool autoMode = m_gain_auto_checkbox->GetValue();
    m_gain_slider->Enable(!autoMode);

    int value = m_gain_slider->GetValue();
    m_client->SetGain(m_radar->GetId(), value, autoMode);
}

void RadarControlDialog::OnSeaChanged(wxScrollEvent& event) {
    if (m_updating_ui || !m_client || !m_radar) return;

    int value = m_sea_slider->GetValue();
    m_sea_value_text->SetLabel(wxString::Format("%d%%", value));

    bool autoMode = m_sea_auto_checkbox->GetValue();
    m_client->SetSea(m_radar->GetId(), value, autoMode);
}

void RadarControlDialog::OnSeaAutoChanged(wxCommandEvent& event) {
    if (m_updating_ui || !m_client || !m_radar) return;

    bool autoMode = m_sea_auto_checkbox->GetValue();
    m_sea_slider->Enable(!autoMode);

    int value = m_sea_slider->GetValue();
    m_client->SetSea(m_radar->GetId(), value, autoMode);
}

void RadarControlDialog::OnRainChanged(wxScrollEvent& event) {
    if (m_updating_ui || !m_client || !m_radar) return;

    int value = m_rain_slider->GetValue();
    m_rain_value_text->SetLabel(wxString::Format("%d%%", value));

    m_client->SetRain(m_radar->GetId(), value);
}

void RadarControlDialog::OnRefresh(wxCommandEvent& event) {
    RefreshState();
}

void RadarControlDialog::OnClose(wxCloseEvent& event) {
    if (m_timer) {
        m_timer->Stop();
    }
    event.Skip();
}

void RadarControlDialog::OnTimer(wxTimerEvent& event) {
    // Update statistics
    if (m_radar && m_radar->IsReceiving()) {
        // TODO: Get actual spoke count from receiver
        m_spokes_text->SetLabel(_("Spokes received: Active"));
    } else {
        m_spokes_text->SetLabel(_("Spokes received: Not connected"));
    }
}
