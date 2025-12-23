/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Radar control dialog - now capability-driven
 */

#include "RadarControlDialog.h"
#include "mayara_server_pi.h"
#include "RadarManager.h"
#include "RadarDisplay.h"

using namespace mayara;

enum {
    ID_POWER_OFF = wxID_HIGHEST + 200,
    ID_POWER_STANDBY,
    ID_POWER_TRANSMIT,
    ID_RANGE_CHOICE,
    ID_REFRESH,
    ID_TIMER
};

// Fallback range values if capabilities don't provide them
static const double FALLBACK_RANGES[] = {
    125, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000,
    8000, 12000, 16000, 24000, 36000, 48000, 64000, 96000
};
static const int NUM_FALLBACK_RANGES = sizeof(FALLBACK_RANGES) / sizeof(FALLBACK_RANGES[0]);

// Helper to format range value
static wxString FormatRange(double meters) {
    if (meters < 1000) {
        return wxString::Format("%.0f m", meters);
    } else {
        return wxString::Format("%.1f km", meters / 1000.0);
    }
}

BEGIN_EVENT_TABLE(RadarControlDialog, wxDialog)
    EVT_BUTTON(ID_POWER_OFF, RadarControlDialog::OnPowerButton)
    EVT_BUTTON(ID_POWER_STANDBY, RadarControlDialog::OnPowerButton)
    EVT_BUTTON(ID_POWER_TRANSMIT, RadarControlDialog::OnPowerButton)
    EVT_CHOICE(ID_RANGE_CHOICE, RadarControlDialog::OnRangeChanged)
    EVT_BUTTON(ID_REFRESH, RadarControlDialog::OnRefresh)
    EVT_CLOSE(RadarControlDialog::OnClose)
    EVT_TIMER(ID_TIMER, RadarControlDialog::OnTimer)
END_EVENT_TABLE()

RadarControlDialog::RadarControlDialog(wxWindow* parent,
                                       mayara_server_pi* plugin,
                                       RadarDisplay* radar)
    : wxDialog(parent, wxID_ANY,
               wxString::Format(_("Radar Controls: %s"), radar ? radar->GetName().c_str() : "Unknown"),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_plugin(plugin)
    , m_radar(radar)
    , m_client(nullptr)
    , m_dynamic_panel(nullptr)
    , m_timer(nullptr)
    , m_updating_ui(false)
{
    wxLogMessage("MaYaRa: RadarControlDialog ctor - entry");

    if (!radar) {
        wxLogMessage("MaYaRa: RadarControlDialog - radar is null!");
        return;
    }

    auto* manager = plugin->GetRadarManager();
    wxLogMessage("MaYaRa: RadarControlDialog - manager=%p", (void*)manager);
    if (manager) {
        m_client = manager->GetClient();
        wxLogMessage("MaYaRa: RadarControlDialog - client=%p", (void*)m_client);
    }

    // Fetch capabilities first
    if (m_client) {
        wxLogMessage("MaYaRa: RadarControlDialog - fetching capabilities for %s", radar->GetId().c_str());
        m_capabilities = m_client->GetCapabilities(radar->GetId());
        wxLogMessage("MaYaRa: Loaded capabilities for %s: %s %s, %u controls",
                     radar->GetId().c_str(),
                     m_capabilities.make.c_str(),
                     m_capabilities.model.c_str(),
                     (unsigned)m_capabilities.controls.size());
    }

    wxLogMessage("MaYaRa: RadarControlDialog - calling CreateControls");
    CreateControls();
    wxLogMessage("MaYaRa: RadarControlDialog - calling RefreshState");
    RefreshState();

    // Start auto-refresh timer (every 2 seconds)
    m_timer = new wxTimer(this, ID_TIMER);
    m_timer->Start(2000);

    SetMinSize(wxSize(380, 500));
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

    // Model and status info
    wxString modelInfo = wxString::Format("%s %s",
        m_capabilities.make.c_str(),
        m_capabilities.model.c_str());
    m_model_text = new wxStaticText(this, wxID_ANY, modelInfo);
    wxFont boldFont = m_model_text->GetFont();
    boldFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_model_text->SetFont(boldFont);
    mainSizer->Add(m_model_text, 0, wxALL, 10);

    m_status_text = new wxStaticText(this, wxID_ANY, _("Status: Unknown"));
    mainSizer->Add(m_status_text, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Power controls (always present, special UI)
    CreatePowerControls(mainSizer);

    // Range controls (always present, special UI with radar-specific ranges)
    CreateRangeControls(mainSizer);

    // Dynamic control panel for all other controls from capabilities
    // Filter out "power" and "range" as they have special handling above
    CapabilityManifest filteredCaps = m_capabilities;
    filteredCaps.controls.erase(
        std::remove_if(filteredCaps.controls.begin(), filteredCaps.controls.end(),
            [](const ControlDefinition& def) {
                return def.id == "power" || def.id == "range";
            }),
        filteredCaps.controls.end()
    );

    if (!filteredCaps.controls.empty()) {
        m_dynamic_panel = new DynamicControlPanel(this, m_client, m_radar->GetId(), filteredCaps);
        mainSizer->Add(m_dynamic_panel, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);
    }

    // Statistics
    m_spokes_text = new wxStaticText(this, wxID_ANY, _("Spokes received: 0"));
    mainSizer->Add(m_spokes_text, 0, wxALL, 10);

    // Refresh button
    wxButton* refreshBtn = new wxButton(this, ID_REFRESH, _("Refresh"));
    mainSizer->Add(refreshBtn, 0, wxALL | wxALIGN_CENTER, 10);

    SetSizer(mainSizer);
}

void RadarControlDialog::CreatePowerControls(wxSizer* parent) {
    wxStaticBoxSizer* powerBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Power"));

    // Check which power values are settable (not read-only) from capabilities
    bool canSetOff = false;
    bool canSetStandby = true;  // Default to true for fallback
    bool canSetTransmit = true;

    const ControlDefinition* powerDef = m_capabilities.getControl("power");
    if (powerDef && !powerDef->values.empty()) {
        // Reset to false, only enable if explicitly settable
        canSetOff = false;
        canSetStandby = false;
        canSetTransmit = false;

        for (const auto& val : powerDef->values) {
            if (!val.readOnly) {
                if (val.value == "off") canSetOff = true;
                else if (val.value == "standby") canSetStandby = true;
                else if (val.value == "transmit") canSetTransmit = true;
            }
        }
    }

    // Only create buttons for settable power states
    m_power_off_btn = nullptr;
    m_power_standby_btn = nullptr;
    m_power_transmit_btn = nullptr;

    if (canSetOff) {
        m_power_off_btn = new wxButton(this, ID_POWER_OFF, _("Off"));
        powerBox->Add(m_power_off_btn, 1, wxALL, 5);
    }
    if (canSetStandby) {
        m_power_standby_btn = new wxButton(this, ID_POWER_STANDBY, _("Standby"));
        powerBox->Add(m_power_standby_btn, 1, wxALL, 5);
    }
    if (canSetTransmit) {
        m_power_transmit_btn = new wxButton(this, ID_POWER_TRANSMIT, _("Transmit"));
        powerBox->Add(m_power_transmit_btn, 1, wxALL, 5);
    }

    parent->Add(powerBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
}

void RadarControlDialog::CreateRangeControls(wxSizer* parent) {
    wxStaticBoxSizer* rangeBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Range"));

    // Use supported ranges from capabilities if available
    m_supported_ranges = m_capabilities.characteristics.supportedRanges;

    // If no ranges from capabilities, use fallback
    if (m_supported_ranges.empty()) {
        for (int i = 0; i < NUM_FALLBACK_RANGES; i++) {
            m_supported_ranges.push_back(static_cast<uint32_t>(FALLBACK_RANGES[i]));
        }
    }

    wxArrayString rangeChoices;
    for (uint32_t range : m_supported_ranges) {
        rangeChoices.Add(FormatRange(static_cast<double>(range)));
    }

    m_range_choice = new wxChoice(this, ID_RANGE_CHOICE, wxDefaultPosition,
                                   wxDefaultSize, rangeChoices);

    rangeBox->Add(m_range_choice, 1, wxALL, 5);
    parent->Add(rangeBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
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

    // Power buttons - enable all except current state (if button exists)
    if (m_power_off_btn) m_power_off_btn->Enable(state.status != RadarStatus::Off);
    if (m_power_standby_btn) m_power_standby_btn->Enable(state.status != RadarStatus::Standby);
    if (m_power_transmit_btn) m_power_transmit_btn->Enable(state.status != RadarStatus::Transmit);

    // Range - find closest match in supported ranges
    if (!m_supported_ranges.empty()) {
        double range = state.rangeMeters;
        int rangeIdx = 0;
        double minDiff = std::abs(static_cast<double>(m_supported_ranges[0]) - range);

        for (size_t i = 1; i < m_supported_ranges.size(); i++) {
            double diff = std::abs(static_cast<double>(m_supported_ranges[i]) - range);
            if (diff < minDiff) {
                minDiff = diff;
                rangeIdx = static_cast<int>(i);
            }
        }
        m_range_choice->SetSelection(rangeIdx);
    }

    // Update dynamic panel
    if (m_dynamic_panel) {
        m_dynamic_panel->UpdateFromState(state);
    }

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

    wxLogMessage("MaYaRa: Setting power to %d", static_cast<int>(status));
    m_client->SetPower(m_radar->GetId(), status);

    // Immediate feedback (if button exists)
    if (m_power_off_btn) m_power_off_btn->Enable(status != RadarStatus::Off);
    if (m_power_standby_btn) m_power_standby_btn->Enable(status != RadarStatus::Standby);
    if (m_power_transmit_btn) m_power_transmit_btn->Enable(status != RadarStatus::Transmit);
}

void RadarControlDialog::OnRangeChanged(wxCommandEvent& event) {
    if (m_updating_ui || !m_client || !m_radar) return;

    int idx = m_range_choice->GetSelection();
    if (idx >= 0 && idx < static_cast<int>(m_supported_ranges.size())) {
        double rangeMeters = static_cast<double>(m_supported_ranges[idx]);
        wxLogMessage("MaYaRa: Setting range to %.0f m", rangeMeters);
        m_client->SetRange(m_radar->GetId(), rangeMeters);
    }
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
        m_spokes_text->SetLabel(_("Spokes received: Active"));
    } else {
        m_spokes_text->SetLabel(_("Spokes received: Not connected"));
    }
}
