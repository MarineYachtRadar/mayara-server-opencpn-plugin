/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Plugin toolbar icons
 */

#ifndef _ICONS_H_
#define _ICONS_H_

#include <wx/wx.h>

PLUGIN_BEGIN_NAMESPACE

// Icon states
enum class IconState {
    Disconnected,   // Gray - no connection to mayara-server
    Standby,        // Yellow - connected, radar in standby
    Transmit        // Green - connected, radar transmitting
};

// Initialize icons from embedded data
void InitializeIcons();

// Get toolbar icon for given state
wxBitmap* GetToolbarIcon(IconState state);

// Get plugin icon for preferences/about dialogs
wxBitmap* GetPluginIcon();

PLUGIN_END_NAMESPACE

#endif  // _ICONS_H_
