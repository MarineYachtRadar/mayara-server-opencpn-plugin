/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * REST API client for mayara-server
 */

// Include wx headers first to get ssize_t defined before IXWebSocket
#include "pi_common.h"

#include "MayaraClient.h"
#include <ixwebsocket/IXHttpClient.h>
#include <nlohmann/json.hpp>

using namespace mayara;
using json = nlohmann::json;

MayaraClient::MayaraClient(const std::string& host, int port, int timeout_ms)
    : m_host(host)
    , m_port(port)
    , m_timeout_ms(timeout_ms)
    , m_connected(false)
{
}

MayaraClient::~MayaraClient() {
}

std::string MayaraClient::Request(const std::string& method,
                                   const std::string& path,
                                   const std::string& body)
{
    ix::HttpClient client;

    std::string url = "http://" + m_host + ":" + std::to_string(m_port) + path;

    ix::HttpRequestArgsPtr args = client.createRequest();
    args->connectTimeout = m_timeout_ms / 1000;
    args->transferTimeout = m_timeout_ms / 1000;

    ix::HttpResponsePtr response;

    if (method == "GET") {
        response = client.get(url, args);
    } else if (method == "PUT") {
        args->body = body;
        args->extraHeaders["Content-Type"] = "application/json";
        response = client.request(url, "PUT", body, args);
    } else if (method == "POST") {
        args->body = body;
        args->extraHeaders["Content-Type"] = "application/json";
        response = client.post(url, body, args);
    } else if (method == "DELETE") {
        response = client.request(url, "DELETE", "", args);
    }

    if (!response) {
        m_connected = false;
        m_last_error = "Request failed";
        return "";
    }

    if (response->statusCode >= 200 && response->statusCode < 300) {
        m_connected = true;
        return response->body;
    }

    m_connected = false;
    m_last_error = "HTTP " + std::to_string(response->statusCode);
    return "";
}

std::vector<std::string> MayaraClient::GetRadarIds() {
    std::vector<std::string> ids;

    std::string response = Request("GET", "/v2/api/radars");
    if (response.empty()) return ids;

    try {
        json j = json::parse(response);
        if (j.is_array()) {
            for (const auto& id : j) {
                if (id.is_string()) {
                    ids.push_back(id.get<std::string>());
                }
            }
        }
    } catch (...) {
        m_last_error = "JSON parse error";
    }

    return ids;
}

std::map<std::string, RadarInfo> MayaraClient::GetRadars() {
    std::map<std::string, RadarInfo> radars;

    auto ids = GetRadarIds();
    for (const auto& id : ids) {
        auto caps = GetCapabilities(id);
        auto state = GetState(id);

        RadarInfo info;
        info.id = id;
        info.name = caps.model.empty() ? id : caps.model;
        info.brand = caps.make;
        info.model = caps.model;
        info.status = state.status;
        info.spokesPerRevolution = caps.spokesPerRevolution;
        info.maxSpokeLength = caps.maxSpokeLength;
        info.rangeMeters = state.rangeMeters;

        radars[id] = info;
    }

    return radars;
}

CapabilityManifest MayaraClient::GetCapabilities(const std::string& radarId) {
    CapabilityManifest caps;
    caps.spokesPerRevolution = 2048;  // Default
    caps.maxSpokeLength = 512;

    std::string response = Request("GET", "/v2/api/radars/" + radarId + "/capabilities");
    if (response.empty()) return caps;

    try {
        json j = json::parse(response);

        if (j.contains("make")) caps.make = j["make"].get<std::string>();
        if (j.contains("model")) caps.model = j["model"].get<std::string>();

        if (j.contains("characteristics")) {
            auto& ch = j["characteristics"];
            if (ch.contains("spokesPerRevolution")) {
                caps.spokesPerRevolution = ch["spokesPerRevolution"].get<int>();
            }
            if (ch.contains("maxSpokeLength")) {
                caps.maxSpokeLength = ch["maxSpokeLength"].get<int>();
            }
        }
    } catch (...) {
        m_last_error = "JSON parse error";
    }

    return caps;
}

RadarState MayaraClient::GetState(const std::string& radarId) {
    RadarState state;
    state.status = RadarStatus::Unknown;
    state.rangeMeters = 0;

    std::string response = Request("GET", "/v2/api/radars/" + radarId + "/state");
    if (response.empty()) return state;

    try {
        json j = json::parse(response);

        if (j.contains("status")) {
            std::string status = j["status"].get<std::string>();
            state.status = StringToRadarStatus(wxString(status));
        }

        if (j.contains("controls")) {
            auto& controls = j["controls"];

            if (controls.contains("range")) {
                state.rangeMeters = controls["range"].get<double>();
            }

            // Parse other controls
            for (auto& [key, value] : controls.items()) {
                ControlValue cv;
                if (value.is_object()) {
                    if (value.contains("mode")) {
                        cv.mode = value["mode"].get<std::string>();
                    }
                    if (value.contains("value")) {
                        cv.value = value["value"].get<int>();
                    }
                } else if (value.is_number()) {
                    cv.value = value.get<int>();
                }
                state.controls[key] = cv;
            }
        }
    } catch (...) {
        m_last_error = "JSON parse error";
    }

    return state;
}

bool MayaraClient::SetControl(const std::string& radarId,
                               const std::string& controlId,
                               int value,
                               const std::string& mode)
{
    json body;
    if (!mode.empty()) {
        body["mode"] = mode;
        body["value"] = value;
    } else {
        body = value;
    }

    std::string response = Request("PUT",
        "/v2/api/radars/" + radarId + "/controls/" + controlId,
        body.dump());

    return !response.empty() || m_connected;
}

bool MayaraClient::SetPower(const std::string& radarId, RadarStatus status) {
    std::string value = RadarStatusToString(status).ToStdString();
    json body = value;
    std::string response = Request("PUT",
        "/v2/api/radars/" + radarId + "/controls/power",
        body.dump());
    return !response.empty() || m_connected;
}

bool MayaraClient::SetRange(const std::string& radarId, double rangeMeters) {
    json body = rangeMeters;
    std::string response = Request("PUT",
        "/v2/api/radars/" + radarId + "/controls/range",
        body.dump());
    return !response.empty() || m_connected;
}

bool MayaraClient::SetGain(const std::string& radarId, int value, bool autoMode) {
    return SetControl(radarId, "gain", value, autoMode ? "auto" : "manual");
}

bool MayaraClient::SetSea(const std::string& radarId, int value, bool autoMode) {
    return SetControl(radarId, "sea", value, autoMode ? "auto" : "manual");
}

bool MayaraClient::SetRain(const std::string& radarId, int value) {
    return SetControl(radarId, "rain", value, "");
}

TargetList MayaraClient::GetTargets(const std::string& radarId) {
    TargetList result;

    std::string response = Request("GET", "/v2/api/radars/" + radarId + "/targets");
    if (response.empty()) return result;

    try {
        json j = json::parse(response);
        if (j.contains("targets") && j["targets"].is_array()) {
            for (const auto& t : j["targets"]) {
                ArpaTarget target;
                if (t.contains("targetId")) target.targetId = t["targetId"].get<int>();
                if (t.contains("bearing")) target.bearing = t["bearing"].get<double>();
                if (t.contains("distance")) target.distance = t["distance"].get<double>();
                if (t.contains("speed")) target.speed = t["speed"].get<double>();
                if (t.contains("course")) target.course = t["course"].get<double>();
                if (t.contains("cpa")) target.cpa = t["cpa"].get<double>();
                if (t.contains("tcpa")) target.tcpa = t["tcpa"].get<double>();
                result.targets.push_back(target);
            }
        }
    } catch (...) {
        m_last_error = "JSON parse error";
    }

    return result;
}

int MayaraClient::AcquireTarget(const std::string& radarId, double bearing, double distance) {
    json body;
    body["bearing"] = bearing;
    body["distance"] = distance;

    std::string response = Request("POST",
        "/v2/api/radars/" + radarId + "/targets",
        body.dump());

    if (response.empty()) return -1;

    try {
        json j = json::parse(response);
        if (j.contains("targetId")) {
            return j["targetId"].get<int>();
        }
    } catch (...) {
    }

    return -1;
}

bool MayaraClient::CancelTarget(const std::string& radarId, int targetId) {
    std::string response = Request("DELETE",
        "/v2/api/radars/" + radarId + "/targets/" + std::to_string(targetId));
    return m_connected;
}

std::string MayaraClient::GetSpokeStreamUrl(const std::string& radarId) {
    return "ws://" + m_host + ":" + std::to_string(m_port) +
           "/v2/api/radars/" + radarId + "/spokes";
}

std::string MayaraClient::GetTargetStreamUrl(const std::string& radarId) {
    return "ws://" + m_host + ":" + std::to_string(m_port) +
           "/v2/api/radars/" + radarId + "/targets/stream";
}
