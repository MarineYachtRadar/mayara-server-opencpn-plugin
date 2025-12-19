/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * REST API client for mayara-server
 */

#ifndef _MAYARA_CLIENT_H_
#define _MAYARA_CLIENT_H_

#include "pi_common.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

// Forward declare nlohmann::json
namespace nlohmann { class json; }

PLUGIN_BEGIN_NAMESPACE

// Radar info from discovery
struct RadarInfo {
    std::string id;
    std::string name;
    std::string brand;
    std::string model;
    RadarStatus status;
    int spokesPerRevolution;
    int maxSpokeLength;
    double rangeMeters;
};

// Capability manifest from /capabilities endpoint
struct CapabilityManifest {
    std::string make;
    std::string model;
    int spokesPerRevolution;
    int maxSpokeLength;
    std::vector<std::string> supportedControls;
};

// Control value (can be simple or with auto mode)
struct ControlValue {
    std::string mode;  // "auto" or "manual" or empty for simple controls
    int value;
};

// Radar state from /state endpoint
struct RadarState {
    RadarStatus status;
    double rangeMeters;
    std::map<std::string, ControlValue> controls;
};

// ARPA target
struct ArpaTarget {
    int targetId;
    double bearing;      // degrees
    double distance;     // meters
    double speed;        // knots
    double course;       // degrees
    double cpa;          // closest point of approach (meters)
    double tcpa;         // time to CPA (minutes)
};

// Target list response
struct TargetList {
    std::vector<ArpaTarget> targets;
};

class MayaraClient {
public:
    MayaraClient(const std::string& host, int port, int timeout_ms = 10000);
    ~MayaraClient();

    // -------- Discovery --------
    std::vector<std::string> GetRadarIds();
    std::map<std::string, RadarInfo> GetRadars();

    // -------- Capabilities & State --------
    CapabilityManifest GetCapabilities(const std::string& radarId);
    RadarState GetState(const std::string& radarId);

    // -------- Controls --------
    bool SetControl(const std::string& radarId,
                    const std::string& controlId,
                    int value,
                    const std::string& mode = "");

    bool SetPower(const std::string& radarId, RadarStatus status);
    bool SetRange(const std::string& radarId, double rangeMeters);
    bool SetGain(const std::string& radarId, int value, bool autoMode);
    bool SetSea(const std::string& radarId, int value, bool autoMode);
    bool SetRain(const std::string& radarId, int value);

    // -------- ARPA Targets --------
    TargetList GetTargets(const std::string& radarId);
    int AcquireTarget(const std::string& radarId, double bearing, double distance);
    bool CancelTarget(const std::string& radarId, int targetId);

    // -------- WebSocket URLs --------
    std::string GetSpokeStreamUrl(const std::string& radarId);
    std::string GetTargetStreamUrl(const std::string& radarId);

    // -------- Connection status --------
    bool IsConnected() const { return m_connected; }
    std::string GetLastError() const { return m_last_error; }

private:
    std::string Request(const std::string& method,
                        const std::string& path,
                        const std::string& body = "");

    std::string m_host;
    int m_port;
    int m_timeout_ms;
    bool m_connected;
    std::string m_last_error;
};

PLUGIN_END_NAMESPACE

#endif  // _MAYARA_CLIENT_H_
