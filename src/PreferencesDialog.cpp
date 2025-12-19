/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Plugin preferences dialog
 */

#include "PreferencesDialog.h"
#include "mayara_server_pi.h"
#include "MayaraClient.h"

#include <wx/spinctrl.h>

using namespace mayara;

enum {
    ID_TEST_CONNECTION = wxID_HIGHEST + 100
};

BEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PreferencesDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, PreferencesDialog::OnCancel)
    EVT_BUTTON(ID_TEST_CONNECTION, PreferencesDialog::OnTestConnection)
END_EVENT_TABLE()

PreferencesDialog::PreferencesDialog(wxWindow* parent, mayara_server_pi* plugin)
    : wxDialog(parent, wxID_ANY, _("MaYaRa Server Preferences"),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_plugin(plugin)
{
    CreateControls();
    LoadSettings();

    SetMinSize(wxSize(400, 350));
    Fit();
    Centre();
}

PreferencesDialog::~PreferencesDialog() {
}

void PreferencesDialog::CreateControls() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Server connection box
    wxStaticBoxSizer* serverBox = new wxStaticBoxSizer(
        wxVERTICAL, this, _("Server Connection"));

    wxFlexGridSizer* serverGrid = new wxFlexGridSizer(2, 5, 5);
    serverGrid->AddGrowableCol(1);

    // Host
    serverGrid->Add(new wxStaticText(this, wxID_ANY, _("Host:")),
                    0, wxALIGN_CENTER_VERTICAL);
    m_host_ctrl = new wxTextCtrl(this, wxID_ANY);
    serverGrid->Add(m_host_ctrl, 1, wxEXPAND);

    // Port
    serverGrid->Add(new wxStaticText(this, wxID_ANY, _("Port:")),
                    0, wxALIGN_CENTER_VERTICAL);
    m_port_ctrl = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxSP_ARROW_KEYS, 1, 65535);
    serverGrid->Add(m_port_ctrl, 0);

    serverBox->Add(serverGrid, 1, wxEXPAND | wxALL, 5);

    // Test connection button
    wxButton* testBtn = new wxButton(this, ID_TEST_CONNECTION, _("Test Connection"));
    serverBox->Add(testBtn, 0, wxALL, 5);

    // Status text
    m_status_text = new wxStaticText(this, wxID_ANY, wxEmptyString);
    serverBox->Add(m_status_text, 0, wxALL, 5);

    mainSizer->Add(serverBox, 0, wxEXPAND | wxALL, 10);

    // Timing box
    wxStaticBoxSizer* timingBox = new wxStaticBoxSizer(
        wxVERTICAL, this, _("Timing"));

    wxFlexGridSizer* timingGrid = new wxFlexGridSizer(2, 5, 5);
    timingGrid->AddGrowableCol(1);

    // Discovery interval
    timingGrid->Add(new wxStaticText(this, wxID_ANY, _("Discovery Interval (sec):")),
                    0, wxALIGN_CENTER_VERTICAL);
    m_discovery_interval_ctrl = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
                                                wxDefaultPosition, wxDefaultSize,
                                                wxSP_ARROW_KEYS, 5, 60);
    timingGrid->Add(m_discovery_interval_ctrl, 0);

    // Reconnect interval
    timingGrid->Add(new wxStaticText(this, wxID_ANY, _("Reconnect Interval (sec):")),
                    0, wxALIGN_CENTER_VERTICAL);
    m_reconnect_interval_ctrl = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
                                                wxDefaultPosition, wxDefaultSize,
                                                wxSP_ARROW_KEYS, 1, 30);
    timingGrid->Add(m_reconnect_interval_ctrl, 0);

    timingBox->Add(timingGrid, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(timingBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    // Display options box
    wxStaticBoxSizer* displayBox = new wxStaticBoxSizer(
        wxVERTICAL, this, _("Display Options"));

    m_overlay_checkbox = new wxCheckBox(this, wxID_ANY,
                                         _("Show radar overlay on chart"));
    displayBox->Add(m_overlay_checkbox, 0, wxALL, 5);

    m_ppi_checkbox = new wxCheckBox(this, wxID_ANY,
                                     _("Show separate PPI window"));
    displayBox->Add(m_ppi_checkbox, 0, wxALL, 5);

    mainSizer->Add(displayBox, 0, wxEXPAND | wxALL, 10);

    // Buttons
    wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
    btnSizer->AddButton(new wxButton(this, wxID_OK));
    btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
    btnSizer->Realize();

    mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 10);

    SetSizer(mainSizer);
}

void PreferencesDialog::LoadSettings() {
    m_host_ctrl->SetValue(wxString(m_plugin->GetServerHost()));
    m_port_ctrl->SetValue(m_plugin->GetServerPort());
    m_discovery_interval_ctrl->SetValue(m_plugin->GetDiscoveryPollInterval());
    m_reconnect_interval_ctrl->SetValue(m_plugin->GetReconnectInterval());
    m_overlay_checkbox->SetValue(m_plugin->GetShowOverlay());
    m_ppi_checkbox->SetValue(m_plugin->GetShowPPIWindow());
}

void PreferencesDialog::SaveSettings() {
    m_plugin->SetServerHost(m_host_ctrl->GetValue().ToStdString());
    m_plugin->SetServerPort(m_port_ctrl->GetValue());
    m_plugin->SetDiscoveryPollInterval(m_discovery_interval_ctrl->GetValue());
    m_plugin->SetReconnectInterval(m_reconnect_interval_ctrl->GetValue());
    m_plugin->SetShowOverlay(m_overlay_checkbox->GetValue());
    m_plugin->SetShowPPIWindow(m_ppi_checkbox->GetValue());
}

void PreferencesDialog::OnOK(wxCommandEvent& event) {
    SaveSettings();
    EndModal(wxID_OK);
}

void PreferencesDialog::OnCancel(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}

void PreferencesDialog::OnTestConnection(wxCommandEvent& event) {
    wxString host = m_host_ctrl->GetValue();
    int port = m_port_ctrl->GetValue();

    m_status_text->SetLabel(_("Testing connection..."));
    Update();

    MayaraClient client(host.ToStdString(), port, 5000);
    auto ids = client.GetRadarIds();

    if (client.IsConnected()) {
        if (ids.empty()) {
            m_status_text->SetLabel(_("Connected! No radars found."));
        } else {
            m_status_text->SetLabel(wxString::Format(
                _("Connected! Found %zu radar(s)."), ids.size()));
        }
        m_status_text->SetForegroundColour(*wxGREEN);
    } else {
        m_status_text->SetLabel(_("Connection failed: ") +
                                wxString(client.GetLastError()));
        m_status_text->SetForegroundColour(*wxRED);
    }

    Layout();
}

wxString PreferencesDialog::GetServerHost() const {
    return m_host_ctrl->GetValue();
}

int PreferencesDialog::GetServerPort() const {
    return m_port_ctrl->GetValue();
}

int PreferencesDialog::GetDiscoveryPollInterval() const {
    return m_discovery_interval_ctrl->GetValue();
}

int PreferencesDialog::GetReconnectInterval() const {
    return m_reconnect_interval_ctrl->GetValue();
}

bool PreferencesDialog::GetShowOverlay() const {
    return m_overlay_checkbox->GetValue();
}

bool PreferencesDialog::GetShowPPIWindow() const {
    return m_ppi_checkbox->GetValue();
}
