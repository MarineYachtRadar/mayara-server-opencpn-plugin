/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 */

#ifndef _PI_COMMON_H_
#define _PI_COMMON_H_

#include "config.h"

#include <wx/wx.h>
#include <wx/glcanvas.h>

// OpenCPN plugin API
#ifdef __WXOSX__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
#include "ocpn_plugin.h"
#ifdef __WXOSX__
#pragma clang diagnostic pop
#endif

// Plugin namespace
#define PLUGIN_BEGIN_NAMESPACE namespace mayara {
#define PLUGIN_END_NAMESPACE }

// Version string
#define PLUGIN_VERSION_STRING \
    wxString::Format("%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)

// Default settings
#define DEFAULT_SERVER_HOST "localhost"
#define DEFAULT_SERVER_PORT 6502
#define DEFAULT_DISCOVERY_INTERVAL 10  // seconds
#define DEFAULT_RECONNECT_INTERVAL 5   // seconds

// Geographic position
struct GeoPosition {
    double lat;
    double lon;

    GeoPosition() : lat(0.0), lon(0.0) {}
    GeoPosition(double latitude, double longitude)
        : lat(latitude), lon(longitude) {}

    bool IsValid() const {
        return lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0;
    }
};

// Radar status enum
enum class RadarStatus {
    Off,
    Standby,
    Transmit,
    Unknown
};

// Convert status to string
inline wxString RadarStatusToString(RadarStatus status) {
    switch (status) {
        case RadarStatus::Off: return "off";
        case RadarStatus::Standby: return "standby";
        case RadarStatus::Transmit: return "transmit";
        default: return "unknown";
    }
}

// Convert string to status
inline RadarStatus StringToRadarStatus(const wxString& str) {
    if (str == "off") return RadarStatus::Off;
    if (str == "standby") return RadarStatus::Standby;
    if (str == "transmit") return RadarStatus::Transmit;
    return RadarStatus::Unknown;
}

// Degrees to radians
inline double DegToRad(double deg) {
    return deg * M_PI / 180.0;
}

// Radians to degrees
inline double RadToDeg(double rad) {
    return rad * 180.0 / M_PI;
}

#endif  // _PI_COMMON_H_
