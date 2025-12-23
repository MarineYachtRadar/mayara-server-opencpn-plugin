/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Dynamic control panel - builds UI from capability manifest
 */

#include "DynamicControlPanel.h"

using namespace mayara;

// Helper to format range value display
static wxString FormatRangeValue(double meters) {
    if (meters < 1000) {
        return wxString::Format("%.0f m", meters);
    } else {
        return wxString::Format("%.1f km", meters / 1000.0);
    }
}

// Helper to format numeric value with unit
static wxString FormatValue(double value, const std::optional<RangeSpec>& range) {
    if (range && range->unit) {
        if (*range->unit == "percent") {
            return wxString::Format("%.0f%%", value);
        } else if (*range->unit == "meters") {
            return FormatRangeValue(value);
        } else {
            return wxString::Format("%.1f %s", value, range->unit->c_str());
        }
    }
    return wxString::Format("%.0f", value);
}

DynamicControlPanel::DynamicControlPanel(wxWindow* parent,
                                         MayaraClient* client,
                                         const std::string& radarId,
                                         const CapabilityManifest& capabilities)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxVSCROLL | wxHSCROLL)
    , m_client(client)
    , m_radarId(radarId)
    , m_capabilities(capabilities)
    , m_updating_ui(false)
    , m_nextId(wxID_HIGHEST + 1000)
{
    SetScrollRate(5, 5);
    BuildControls();
}

DynamicControlPanel::~DynamicControlPanel() {
}

bool DynamicControlPanel::HasControl(const std::string& controlId) const {
    return m_controls.find(controlId) != m_controls.end();
}

void DynamicControlPanel::BuildControls() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxLogMessage("MaYaRa: Building dynamic controls for %u capabilities",
                 (unsigned)m_capabilities.controls.size());

    // Group controls by category
    std::vector<const ControlDefinition*> baseControls;
    std::vector<const ControlDefinition*> extendedControls;
    std::vector<const ControlDefinition*> installControls;

    for (const auto& def : m_capabilities.controls) {
        switch (def.category) {
            case ControlCategory::Base:
                baseControls.push_back(&def);
                break;
            case ControlCategory::Extended:
                extendedControls.push_back(&def);
                break;
            case ControlCategory::Installation:
                installControls.push_back(&def);
                break;
        }
    }

    // Create base controls section
    if (!baseControls.empty()) {
        wxStaticBoxSizer* baseBox = new wxStaticBoxSizer(
            wxVERTICAL, this, _("Basic Controls"));

        for (const auto* def : baseControls) {
            CreateControlWidget(*def, baseBox);
        }

        mainSizer->Add(baseBox, 0, wxEXPAND | wxALL, 5);
    }

    // Create extended controls section
    if (!extendedControls.empty()) {
        wxStaticBoxSizer* extBox = new wxStaticBoxSizer(
            wxVERTICAL, this, _("Extended Controls"));

        for (const auto* def : extendedControls) {
            CreateControlWidget(*def, extBox);
        }

        mainSizer->Add(extBox, 0, wxEXPAND | wxALL, 5);
    }

    // Create installation controls section (collapsed by default in real UI)
    if (!installControls.empty()) {
        wxStaticBoxSizer* installBox = new wxStaticBoxSizer(
            wxVERTICAL, this, _("Installation Settings"));

        for (const auto* def : installControls) {
            CreateControlWidget(*def, installBox);
        }

        mainSizer->Add(installBox, 0, wxEXPAND | wxALL, 5);
    }

    SetSizer(mainSizer);
    FitInside();
}

void DynamicControlPanel::CreateControlWidget(const ControlDefinition& def, wxSizer* parentSizer) {
    wxSizer* controlSizer = nullptr;

    switch (def.controlType) {
        case ControlType::Boolean:
            controlSizer = CreateBooleanControl(def);
            break;
        case ControlType::Number:
            controlSizer = CreateNumberControl(def);
            break;
        case ControlType::Enum:
            controlSizer = CreateEnumControl(def);
            break;
        case ControlType::Compound:
            controlSizer = CreateCompoundControl(def);
            break;
        case ControlType::String:
            controlSizer = CreateStringControl(def);
            break;
    }

    if (controlSizer) {
        parentSizer->Add(controlSizer, 0, wxEXPAND | wxALL, 2);
    }
}

wxSizer* DynamicControlPanel::CreateBooleanControl(const ControlDefinition& def) {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    int id = m_nextId++;
    wxCheckBox* checkbox = new wxCheckBox(this, id, wxString(def.name));
    checkbox->SetToolTip(wxString(def.description));

    if (def.readOnly) {
        checkbox->Enable(false);
    } else {
        // Bind event dynamically
        checkbox->Bind(wxEVT_CHECKBOX, [this, controlId = def.id](wxCommandEvent& event) {
            OnCheckboxChanged(event, controlId);
        });
    }

    sizer->Add(checkbox, 1, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Store control reference
    DynamicControl dc;
    dc.controlId = def.id;
    dc.type = ControlType::Boolean;
    dc.definition = def;
    dc.checkbox = checkbox;
    dc.containerSizer = sizer;
    m_controls[def.id] = dc;

    wxLogMessage("MaYaRa: Created boolean control: %s", def.id.c_str());

    return sizer;
}

wxSizer* DynamicControlPanel::CreateNumberControl(const ControlDefinition& def) {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    // Label
    wxStaticText* label = new wxStaticText(this, wxID_ANY, wxString(def.name) + ":");
    label->SetToolTip(wxString(def.description));
    sizer->Add(label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Slider
    int id = m_nextId++;
    int minVal = 0, maxVal = 100;
    if (def.range) {
        minVal = static_cast<int>(def.range->min);
        maxVal = static_cast<int>(def.range->max);
    }

    wxSlider* slider = new wxSlider(this, id, minVal, minVal, maxVal,
                                     wxDefaultPosition, wxSize(120, -1));

    if (def.readOnly) {
        slider->Enable(false);
    } else {
        slider->Bind(wxEVT_SCROLL_CHANGED, [this, controlId = def.id](wxScrollEvent& event) {
            OnSliderChanged(event, controlId);
        });
        slider->Bind(wxEVT_SCROLL_THUMBRELEASE, [this, controlId = def.id](wxScrollEvent& event) {
            OnSliderChanged(event, controlId);
        });
    }

    sizer->Add(slider, 1, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Value display
    wxStaticText* valueText = new wxStaticText(this, wxID_ANY, "0",
                                                wxDefaultPosition, wxSize(50, -1),
                                                wxALIGN_RIGHT);
    sizer->Add(valueText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Store control reference
    DynamicControl dc;
    dc.controlId = def.id;
    dc.type = ControlType::Number;
    dc.definition = def;
    dc.slider = slider;
    dc.valueLabel = valueText;
    dc.containerSizer = sizer;
    m_controls[def.id] = dc;

    wxLogMessage("MaYaRa: Created number control: %s (range %d-%d)", def.id.c_str(), minVal, maxVal);

    return sizer;
}

wxSizer* DynamicControlPanel::CreateEnumControl(const ControlDefinition& def) {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    // Label
    wxStaticText* label = new wxStaticText(this, wxID_ANY, wxString(def.name) + ":");
    label->SetToolTip(wxString(def.description));
    sizer->Add(label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Choice dropdown
    int id = m_nextId++;
    wxArrayString choices;
    for (const auto& ev : def.values) {
        choices.Add(wxString(ev.label));
    }

    wxChoice* choice = new wxChoice(this, id, wxDefaultPosition, wxDefaultSize, choices);

    if (def.readOnly) {
        choice->Enable(false);
    } else {
        choice->Bind(wxEVT_CHOICE, [this, controlId = def.id](wxCommandEvent& event) {
            OnChoiceChanged(event, controlId);
        });
    }

    sizer->Add(choice, 1, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Store control reference
    DynamicControl dc;
    dc.controlId = def.id;
    dc.type = ControlType::Enum;
    dc.definition = def;
    dc.choice = choice;
    dc.containerSizer = sizer;
    m_controls[def.id] = dc;

    wxLogMessage("MaYaRa: Created enum control: %s with %u values", def.id.c_str(), (unsigned)def.values.size());

    return sizer;
}

wxSizer* DynamicControlPanel::CreateCompoundControl(const ControlDefinition& def) {
    wxStaticBoxSizer* sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxString(def.name));
    sizer->GetStaticBox()->SetToolTip(wxString(def.description));

    wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

    // Auto checkbox (if modes include "auto")
    wxCheckBox* autoCheck = nullptr;
    bool hasAuto = false;
    for (const auto& mode : def.modes) {
        if (mode == "auto") {
            hasAuto = true;
            break;
        }
    }

    if (hasAuto) {
        int autoId = m_nextId++;
        autoCheck = new wxCheckBox(this, autoId, _("Auto"));
        autoCheck->Bind(wxEVT_CHECKBOX, [this, controlId = def.id](wxCommandEvent& event) {
            OnAutoCheckboxChanged(event, controlId);
        });
        rowSizer->Add(autoCheck, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    }

    // Slider for the value
    int sliderId = m_nextId++;
    int minVal = 0, maxVal = 100;

    // Check if there's a "value" property with range
    auto it = def.properties.find("value");
    if (it != def.properties.end() && it->second.range) {
        minVal = static_cast<int>(it->second.range->min);
        maxVal = static_cast<int>(it->second.range->max);
    }

    wxSlider* slider = new wxSlider(this, sliderId, minVal, minVal, maxVal,
                                     wxDefaultPosition, wxSize(120, -1));

    if (def.readOnly) {
        slider->Enable(false);
        if (autoCheck) autoCheck->Enable(false);
    } else {
        slider->Bind(wxEVT_SCROLL_CHANGED, [this, controlId = def.id](wxScrollEvent& event) {
            OnSliderChanged(event, controlId);
        });
        slider->Bind(wxEVT_SCROLL_THUMBRELEASE, [this, controlId = def.id](wxScrollEvent& event) {
            OnSliderChanged(event, controlId);
        });
    }

    rowSizer->Add(slider, 1, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Value display
    wxStaticText* valueText = new wxStaticText(this, wxID_ANY, "0%",
                                                wxDefaultPosition, wxSize(50, -1),
                                                wxALIGN_RIGHT);
    rowSizer->Add(valueText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    sizer->Add(rowSizer, 0, wxEXPAND);

    // Store control reference
    DynamicControl dc;
    dc.controlId = def.id;
    dc.type = ControlType::Compound;
    dc.definition = def;
    dc.slider = slider;
    dc.autoCheckbox = autoCheck;
    dc.valueLabel = valueText;
    dc.containerSizer = sizer;
    m_controls[def.id] = dc;

    wxLogMessage("MaYaRa: Created compound control: %s (hasAuto=%d)", def.id.c_str(), hasAuto);

    return sizer;
}

wxSizer* DynamicControlPanel::CreateStringControl(const ControlDefinition& def) {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    // Label
    wxStaticText* label = new wxStaticText(this, wxID_ANY, wxString(def.name) + ":");
    label->SetToolTip(wxString(def.description));
    sizer->Add(label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Read-only text control
    int id = m_nextId++;
    wxTextCtrl* textCtrl = new wxTextCtrl(this, id, "", wxDefaultPosition, wxDefaultSize,
                                          wxTE_READONLY);
    sizer->Add(textCtrl, 1, wxALL | wxALIGN_CENTER_VERTICAL, 3);

    // Store control reference
    DynamicControl dc;
    dc.controlId = def.id;
    dc.type = ControlType::String;
    dc.definition = def;
    dc.textCtrl = textCtrl;
    dc.containerSizer = sizer;
    m_controls[def.id] = dc;

    wxLogMessage("MaYaRa: Created string control: %s", def.id.c_str());

    return sizer;
}

void DynamicControlPanel::UpdateFromState(const RadarState& state) {
    m_updating_ui = true;

    for (auto& [controlId, ctrl] : m_controls) {
        auto it = state.controls.find(controlId);
        if (it == state.controls.end()) continue;

        const ControlValue& value = it->second;

        switch (ctrl.type) {
            case ControlType::Boolean:
                if (ctrl.checkbox) {
                    ctrl.checkbox->SetValue(value.boolValue);
                }
                break;

            case ControlType::Number:
                if (ctrl.slider) {
                    ctrl.slider->SetValue(static_cast<int>(value.numericValue));
                }
                if (ctrl.valueLabel) {
                    ctrl.valueLabel->SetLabel(FormatValue(value.numericValue, ctrl.definition.range));
                }
                break;

            case ControlType::Enum:
                if (ctrl.choice) {
                    // Find matching value in the enum list
                    for (size_t i = 0; i < ctrl.definition.values.size(); i++) {
                        if (ctrl.definition.values[i].value == value.stringValue) {
                            ctrl.choice->SetSelection(static_cast<int>(i));
                            break;
                        }
                    }
                }
                break;

            case ControlType::Compound:
                if (ctrl.autoCheckbox) {
                    bool isAuto = (value.mode == "auto");
                    ctrl.autoCheckbox->SetValue(isAuto);
                    if (ctrl.slider) {
                        ctrl.slider->Enable(!isAuto && !ctrl.definition.readOnly);
                    }
                }
                if (ctrl.slider) {
                    ctrl.slider->SetValue(static_cast<int>(value.numericValue));
                }
                if (ctrl.valueLabel) {
                    // Check for "value" property range
                    std::optional<RangeSpec> range;
                    auto it = ctrl.definition.properties.find("value");
                    if (it != ctrl.definition.properties.end()) {
                        range = it->second.range;
                    }
                    ctrl.valueLabel->SetLabel(FormatValue(value.numericValue, range));
                }
                break;

            case ControlType::String:
                if (ctrl.textCtrl) {
                    ctrl.textCtrl->SetValue(wxString(value.stringValue));
                }
                break;
        }
    }

    m_updating_ui = false;
}

void DynamicControlPanel::OnCheckboxChanged(wxCommandEvent& event, const std::string& controlId) {
    if (m_updating_ui) return;

    auto it = m_controls.find(controlId);
    if (it == m_controls.end()) return;

    bool value = it->second.checkbox->GetValue();
    SendControlValue(controlId, ControlValue::Boolean(value));
}

void DynamicControlPanel::OnSliderChanged(wxScrollEvent& event, const std::string& controlId) {
    if (m_updating_ui) return;

    auto it = m_controls.find(controlId);
    if (it == m_controls.end()) return;

    DynamicControl& ctrl = it->second;
    double value = ctrl.slider->GetValue();

    // Update value label
    if (ctrl.valueLabel) {
        if (ctrl.type == ControlType::Compound) {
            std::optional<RangeSpec> range;
            auto propIt = ctrl.definition.properties.find("value");
            if (propIt != ctrl.definition.properties.end()) {
                range = propIt->second.range;
            }
            ctrl.valueLabel->SetLabel(FormatValue(value, range));
        } else {
            ctrl.valueLabel->SetLabel(FormatValue(value, ctrl.definition.range));
        }
    }

    if (ctrl.type == ControlType::Compound) {
        std::string mode = "manual";
        if (ctrl.autoCheckbox && ctrl.autoCheckbox->GetValue()) {
            mode = "auto";
        }
        SendControlValue(controlId, ControlValue::Compound(mode, value));
    } else {
        SendControlValue(controlId, ControlValue::Number(value));
    }
}

void DynamicControlPanel::OnChoiceChanged(wxCommandEvent& event, const std::string& controlId) {
    if (m_updating_ui) return;

    auto it = m_controls.find(controlId);
    if (it == m_controls.end()) return;

    DynamicControl& ctrl = it->second;
    int selection = ctrl.choice->GetSelection();

    if (selection >= 0 && selection < static_cast<int>(ctrl.definition.values.size())) {
        std::string value = ctrl.definition.values[selection].value;
        SendControlValue(controlId, ControlValue::Enumeration(value));
    }
}

void DynamicControlPanel::OnAutoCheckboxChanged(wxCommandEvent& event, const std::string& controlId) {
    if (m_updating_ui) return;

    auto it = m_controls.find(controlId);
    if (it == m_controls.end()) return;

    DynamicControl& ctrl = it->second;
    bool isAuto = ctrl.autoCheckbox->GetValue();

    // Enable/disable slider based on auto mode
    if (ctrl.slider) {
        ctrl.slider->Enable(!isAuto);
    }

    double value = ctrl.slider ? ctrl.slider->GetValue() : 0;
    std::string mode = isAuto ? "auto" : "manual";
    SendControlValue(controlId, ControlValue::Compound(mode, value));
}

void DynamicControlPanel::SendControlValue(const std::string& controlId, const ControlValue& value) {
    if (m_client) {
        m_client->SetControl(m_radarId, controlId, value);
    }

    if (m_callback) {
        m_callback(controlId, value);
    }
}
