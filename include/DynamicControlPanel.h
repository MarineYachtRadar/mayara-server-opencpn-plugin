/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Dynamic control panel - builds UI from capability manifest
 */

#ifndef _DYNAMIC_CONTROL_PANEL_H_
#define _DYNAMIC_CONTROL_PANEL_H_

#include "pi_common.h"
#include "MayaraClient.h"
#include <map>
#include <functional>

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class MayaraClient;

// Callback when a control value changes
using ControlChangeCallback = std::function<void(const std::string& controlId, const ControlValue& value)>;

// A single dynamic control widget (wraps the actual wxWidget controls)
struct DynamicControl {
    std::string controlId;
    ControlType type;
    ControlDefinition definition;

    // Widget pointers (only one will be valid depending on type)
    wxCheckBox* checkbox = nullptr;      // For boolean
    wxSlider* slider = nullptr;          // For number
    wxChoice* choice = nullptr;          // For enum
    wxTextCtrl* textCtrl = nullptr;      // For string (read-only info)
    wxCheckBox* autoCheckbox = nullptr;  // For compound types with auto mode
    wxStaticText* valueLabel = nullptr;  // For showing current value

    // Parent sizer containing this control
    wxSizer* containerSizer = nullptr;
};

// Panel that dynamically builds controls from CapabilityManifest
class DynamicControlPanel : public wxScrolledWindow {
public:
    DynamicControlPanel(wxWindow* parent,
                        MayaraClient* client,
                        const std::string& radarId,
                        const CapabilityManifest& capabilities);
    ~DynamicControlPanel();

    // Update UI from current state
    void UpdateFromState(const RadarState& state);

    // Set callback for when controls change
    void SetChangeCallback(ControlChangeCallback callback) { m_callback = callback; }

    // Get all dynamically created controls
    const std::map<std::string, DynamicControl>& GetControls() const { return m_controls; }

    // Check if a specific control exists
    bool HasControl(const std::string& controlId) const;

    // Get number of controls created
    size_t GetControlCount() const { return m_controls.size(); }

private:
    void BuildControls();
    void CreateControlWidget(const ControlDefinition& def, wxSizer* parentSizer);

    // Create specific control types
    wxSizer* CreateBooleanControl(const ControlDefinition& def);
    wxSizer* CreateNumberControl(const ControlDefinition& def);
    wxSizer* CreateEnumControl(const ControlDefinition& def);
    wxSizer* CreateCompoundControl(const ControlDefinition& def);
    wxSizer* CreateStringControl(const ControlDefinition& def);

    // Event handlers - using dynamic event binding
    void OnCheckboxChanged(wxCommandEvent& event, const std::string& controlId);
    void OnSliderChanged(wxScrollEvent& event, const std::string& controlId);
    void OnChoiceChanged(wxCommandEvent& event, const std::string& controlId);
    void OnAutoCheckboxChanged(wxCommandEvent& event, const std::string& controlId);

    // Send control value to server
    void SendControlValue(const std::string& controlId, const ControlValue& value);

    MayaraClient* m_client;
    std::string m_radarId;
    CapabilityManifest m_capabilities;
    ControlChangeCallback m_callback;

    std::map<std::string, DynamicControl> m_controls;
    bool m_updating_ui;

    // ID counter for dynamic widgets
    int m_nextId;
};

PLUGIN_END_NAMESPACE

#endif  // _DYNAMIC_CONTROL_PANEL_H_
