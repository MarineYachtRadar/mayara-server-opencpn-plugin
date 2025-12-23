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
#include <wx/url.h>
#include <wx/sstream.h>
#include <wx/protocol/http.h>
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
    // Use wxHTTP for PUT/POST/DELETE, wxURL for GET
    // IXWebSocket HTTP crashes on Windows, so we use wx classes

    try {
        if (method == "GET") {
            // Use wxURL for simple GET requests
            wxString url = wxString::Format("http://%s:%d%s",
                wxString(m_host), m_port, wxString(path));

            wxURL wxurl(url);
            if (wxurl.GetError() != wxURL_NOERR) {
                m_connected = false;
                m_last_error = "Invalid URL";
                return "";
            }

            int timeout = std::min(m_timeout_ms / 1000, 2);
            wxurl.GetProtocol().SetTimeout(timeout);

            wxInputStream* stream = wxurl.GetInputStream();
            if (!stream) {
                m_connected = false;
                m_last_error = "Connection failed";
                return "";
            }

            wxStringOutputStream out;
            stream->Read(out);
            delete stream;

            wxString response = out.GetString();
            m_connected = true;
            return response.ToStdString();
        } else {
            // Use wxHTTP for PUT/POST/DELETE
            // NOTE: wxHTTP is limited - SetPostBuffer + GetInputStream should do POST
            wxHTTP http;
            http.SetTimeout(std::min(m_timeout_ms / 1000, 2));
            http.SetHeader("Content-Type", "application/json");

            // Add method override header for servers that support it
            if (method == "PUT") {
                http.SetHeader("X-HTTP-Method-Override", "PUT");
            } else if (method == "DELETE") {
                http.SetHeader("X-HTTP-Method-Override", "DELETE");
            }

            if (!http.Connect(wxString(m_host), m_port)) {
                m_connected = false;
                m_last_error = "Connection failed";
                wxLogMessage("MaYaRa: HTTP Connect failed to %s:%d", m_host.c_str(), m_port);
                return "";
            }

            // Set request body - this should make it a POST request
            if (!body.empty()) {
                wxMemoryBuffer buf;
                buf.AppendData(body.c_str(), body.size());
                http.SetPostBuffer("application/json", buf);
                wxLogMessage("MaYaRa: HTTP %s %s body=%s", method.c_str(), path.c_str(), body.c_str());
            }

            wxInputStream* stream = http.GetInputStream(wxString(path));

            int responseCode = http.GetResponse();
            wxLogMessage("MaYaRa: HTTP response code: %d", responseCode);

            if (!stream || responseCode >= 400) {
                m_connected = (responseCode > 0);
                m_last_error = wxString::Format("HTTP %d", responseCode).ToStdString();
                wxLogMessage("MaYaRa: HTTP request failed: %s", m_last_error.c_str());
                delete stream;
                return "";
            }

            wxStringOutputStream out;
            stream->Read(out);
            delete stream;

            wxString response = out.GetString();
            wxLogMessage("MaYaRa: HTTP response: %s", response.Left(200).c_str());

            m_connected = true;
            return response.ToStdString();
        }
    } catch (const std::exception& e) {
        m_connected = false;
        m_last_error = std::string("Exception: ") + e.what();
        return "";
    } catch (...) {
        m_connected = false;
        m_last_error = "Unknown error";
        return "";
    }
}

std::vector<std::string> MayaraClient::GetRadarIds() {
    std::vector<std::string> ids;

    std::string response = Request("GET", "/v2/api/radars");
    if (response.empty()) return ids;

    try {
        json j = json::parse(response);

        // Handle both formats:
        // 1. Object format: {"radar-2": {...}, "radar-3": {...}}
        // 2. Array format: ["radar-2", "radar-3"]
        if (j.is_object()) {
            for (auto& [key, value] : j.items()) {
                ids.push_back(key);
            }
        } else if (j.is_array()) {
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
        info.spokesPerRevolution = caps.spokesPerRevolution();
        info.maxSpokeLength = caps.maxSpokeLength();
        info.rangeMeters = state.rangeMeters;

        radars[id] = info;
    }

    return radars;
}

// Helper to convert JSON control type string to enum
static ControlType ParseControlType(const std::string& typeStr) {
    if (typeStr == "boolean") return ControlType::Boolean;
    if (typeStr == "number") return ControlType::Number;
    if (typeStr == "enum") return ControlType::Enum;
    if (typeStr == "compound") return ControlType::Compound;
    if (typeStr == "string") return ControlType::String;
    return ControlType::String;  // Default fallback
}

// Helper to convert JSON category string to enum
static ControlCategory ParseControlCategory(const std::string& catStr) {
    if (catStr == "base") return ControlCategory::Base;
    if (catStr == "extended") return ControlCategory::Extended;
    if (catStr == "installation") return ControlCategory::Installation;
    return ControlCategory::Base;  // Default
}

// Helper to convert JSON feature string to enum
static std::optional<SupportedFeature> ParseSupportedFeature(const std::string& feat) {
    if (feat == "arpa") return SupportedFeature::Arpa;
    if (feat == "guardZones") return SupportedFeature::GuardZones;
    if (feat == "trails") return SupportedFeature::Trails;
    if (feat == "dualRange") return SupportedFeature::DualRange;
    return std::nullopt;
}

// Helper to parse RangeSpec from JSON
static std::optional<RangeSpec> ParseRangeSpec(const json& j) {
    if (!j.is_object()) return std::nullopt;
    RangeSpec spec;
    spec.min = j.value("min", 0.0);
    spec.max = j.value("max", 100.0);
    if (j.contains("step")) spec.step = j["step"].get<double>();
    if (j.contains("unit")) spec.unit = j["unit"].get<std::string>();
    return spec;
}

// Helper to parse EnumValue from JSON
static EnumValue ParseEnumValue(const json& j) {
    EnumValue ev;
    if (j.contains("value")) {
        if (j["value"].is_string()) {
            ev.value = j["value"].get<std::string>();
        } else if (j["value"].is_number()) {
            ev.value = std::to_string(j["value"].get<double>());
        }
    }
    ev.label = j.value("label", ev.value);
    if (j.contains("description")) ev.description = j["description"].get<std::string>();
    ev.readOnly = j.value("readOnly", false);
    return ev;
}

// Helper to parse PropertyDefinition from JSON
static PropertyDefinition ParsePropertyDefinition(const json& j) {
    PropertyDefinition prop;
    prop.propType = j.value("type", "string");
    if (j.contains("description")) prop.description = j["description"].get<std::string>();
    if (j.contains("range")) prop.range = ParseRangeSpec(j["range"]);
    if (j.contains("values") && j["values"].is_array()) {
        for (const auto& v : j["values"]) {
            prop.values.push_back(ParseEnumValue(v));
        }
    }
    return prop;
}

// Helper to parse ControlDefinition from JSON
static ControlDefinition ParseControlDefinition(const json& j) {
    ControlDefinition ctrl;

    ctrl.id = j.value("id", "");
    ctrl.name = j.value("name", ctrl.id);
    ctrl.description = j.value("description", "");
    ctrl.category = ParseControlCategory(j.value("category", "base"));
    ctrl.controlType = ParseControlType(j.value("type", "string"));
    ctrl.readOnly = j.value("readOnly", false);

    // Parse range for number types
    if (j.contains("range")) {
        ctrl.range = ParseRangeSpec(j["range"]);
    }

    // Parse values for enum types
    if (j.contains("values") && j["values"].is_array()) {
        for (const auto& v : j["values"]) {
            ctrl.values.push_back(ParseEnumValue(v));
        }
    }

    // Parse properties for compound types
    if (j.contains("properties") && j["properties"].is_object()) {
        for (auto& [key, value] : j["properties"].items()) {
            ctrl.properties[key] = ParsePropertyDefinition(value);
        }
    }

    // Parse modes (for controls with auto/manual)
    if (j.contains("modes") && j["modes"].is_array()) {
        for (const auto& m : j["modes"]) {
            ctrl.modes.push_back(m.get<std::string>());
        }
    }

    // Parse default mode
    if (j.contains("defaultMode")) {
        ctrl.defaultMode = j["defaultMode"].get<std::string>();
    }

    // Parse default value as JSON string
    if (j.contains("default")) {
        ctrl.defaultValue = j["default"].dump();
    }

    return ctrl;
}

CapabilityManifest MayaraClient::GetCapabilities(const std::string& radarId) {
    CapabilityManifest caps;

    // Initialize characteristics with defaults
    caps.characteristics.spokesPerRevolution = 2048;
    caps.characteristics.maxSpokeLength = 512;
    caps.characteristics.maxRange = 96000;  // 96km
    caps.characteristics.minRange = 50;
    caps.characteristics.hasDoppler = false;
    caps.characteristics.hasDualRange = false;
    caps.characteristics.maxDualRange = 0;
    caps.characteristics.noTransmitZoneCount = 0;

    std::string response = Request("GET", "/v2/api/radars/" + radarId + "/capabilities");
    if (response.empty()) return caps;

    try {
        json j = json::parse(response);

        // Parse top-level fields
        caps.id = j.value("id", radarId);
        if (j.contains("key")) caps.key = j["key"].get<std::string>();
        caps.make = j.value("make", "");
        caps.model = j.value("model", "");
        if (j.contains("modelFamily")) caps.modelFamily = j["modelFamily"].get<std::string>();
        if (j.contains("serialNumber")) caps.serialNumber = j["serialNumber"].get<std::string>();
        if (j.contains("firmwareVersion")) caps.firmwareVersion = j["firmwareVersion"].get<std::string>();

        // Parse characteristics
        if (j.contains("characteristics")) {
            auto& ch = j["characteristics"];
            caps.characteristics.maxRange = ch.value("maxRange", 96000u);
            caps.characteristics.minRange = ch.value("minRange", 50u);
            caps.characteristics.spokesPerRevolution = ch.value("spokesPerRevolution", 2048);
            caps.characteristics.maxSpokeLength = ch.value("maxSpokeLength", 512);
            caps.characteristics.hasDoppler = ch.value("hasDoppler", false);
            caps.characteristics.hasDualRange = ch.value("hasDualRange", false);
            caps.characteristics.maxDualRange = ch.value("maxDualRange", 0u);
            caps.characteristics.noTransmitZoneCount = ch.value("noTransmitZoneCount", 0);

            // Parse supported ranges
            if (ch.contains("supportedRanges") && ch["supportedRanges"].is_array()) {
                for (const auto& r : ch["supportedRanges"]) {
                    caps.characteristics.supportedRanges.push_back(r.get<uint32_t>());
                }
            }
        }

        // Parse controls array - THIS IS THE KEY ADDITION
        if (j.contains("controls") && j["controls"].is_array()) {
            wxLogMessage("MaYaRa: Parsing %u controls from capabilities", (unsigned)j["controls"].size());
            for (const auto& ctrl : j["controls"]) {
                ControlDefinition def = ParseControlDefinition(ctrl);
                wxLogMessage("MaYaRa: Parsed control: id=%s type=%d",
                    def.id.c_str(), static_cast<int>(def.controlType));
                caps.controls.push_back(def);
            }
        }

        // Parse supported features
        if (j.contains("supportedFeatures") && j["supportedFeatures"].is_array()) {
            for (const auto& f : j["supportedFeatures"]) {
                auto feat = ParseSupportedFeature(f.get<std::string>());
                if (feat.has_value()) {
                    caps.supportedFeatures.push_back(feat.value());
                }
            }
        }

    } catch (const std::exception& e) {
        m_last_error = std::string("JSON parse error: ") + e.what();
        wxLogMessage("MaYaRa: GetCapabilities parse error: %s", e.what());
    } catch (...) {
        m_last_error = "JSON parse error";
    }

    return caps;
}

// CapabilityManifest helper methods
bool CapabilityManifest::hasControl(const std::string& controlId) const {
    for (const auto& ctrl : controls) {
        if (ctrl.id == controlId) return true;
    }
    return false;
}

const ControlDefinition* CapabilityManifest::getControl(const std::string& controlId) const {
    for (const auto& ctrl : controls) {
        if (ctrl.id == controlId) return &ctrl;
    }
    return nullptr;
}

bool CapabilityManifest::hasFeature(SupportedFeature feature) const {
    for (const auto& f : supportedFeatures) {
        if (f == feature) return true;
    }
    return false;
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

            // Extract range first (for convenience field)
            if (controls.contains("range")) {
                if (controls["range"].is_number()) {
                    state.rangeMeters = controls["range"].get<double>();
                } else if (controls["range"].is_object() && controls["range"].contains("value")) {
                    state.rangeMeters = controls["range"]["value"].get<double>();
                }
            }

            // Parse all controls with proper type detection
            for (auto& [key, value] : controls.items()) {
                ControlValue cv;
                cv.jsonValue = value.dump();  // Store raw JSON for reference

                if (value.is_boolean()) {
                    // Boolean control
                    cv.type = ControlType::Boolean;
                    cv.boolValue = value.get<bool>();
                } else if (value.is_number()) {
                    // Simple numeric control
                    cv.type = ControlType::Number;
                    cv.numericValue = value.get<double>();
                } else if (value.is_string()) {
                    // Enum control (string value)
                    cv.type = ControlType::Enum;
                    cv.stringValue = value.get<std::string>();
                } else if (value.is_object()) {
                    // Compound control with mode and value
                    cv.type = ControlType::Compound;
                    if (value.contains("mode")) {
                        cv.mode = value["mode"].get<std::string>();
                    }
                    if (value.contains("value")) {
                        if (value["value"].is_number()) {
                            cv.numericValue = value["value"].get<double>();
                        } else if (value["value"].is_boolean()) {
                            cv.boolValue = value["value"].get<bool>();
                        }
                    }
                }

                state.controls[key] = cv;
            }
        }
    } catch (const std::exception& e) {
        m_last_error = std::string("JSON parse error: ") + e.what();
        wxLogMessage("MaYaRa: GetState parse error: %s", e.what());
    } catch (...) {
        m_last_error = "JSON parse error";
    }

    return state;
}

// Generic SetControl using ControlValue
// API expects: {"value": <actual_value>} for all control types
bool MayaraClient::SetControl(const std::string& radarId,
                               const std::string& controlId,
                               const ControlValue& value)
{
    json body;
    json actualValue;

    switch (value.type) {
        case ControlType::Boolean:
            actualValue = value.boolValue;
            break;
        case ControlType::Number:
            actualValue = value.numericValue;
            break;
        case ControlType::Enum:
        case ControlType::String:
            actualValue = value.stringValue;
            break;
        case ControlType::Compound:
            // Compound controls have mode and value as an object
            if (!value.mode.empty()) {
                actualValue["mode"] = value.mode;
                actualValue["value"] = value.numericValue;
            } else {
                actualValue = value.numericValue;
            }
            break;
    }

    // Wrap in {"value": ...} as required by the API
    body["value"] = actualValue;

    wxLogMessage("MaYaRa: SetControl %s/%s = %s", radarId.c_str(), controlId.c_str(), body.dump().c_str());

    std::string response = Request("PUT",
        "/v2/api/radars/" + radarId + "/controls/" + controlId,
        body.dump());

    return !response.empty() || m_connected;
}

// Set boolean control
bool MayaraClient::SetControlBool(const std::string& radarId,
                                   const std::string& controlId,
                                   bool value)
{
    return SetControl(radarId, controlId, ControlValue::Boolean(value));
}

// Set numeric control
bool MayaraClient::SetControlNumber(const std::string& radarId,
                                     const std::string& controlId,
                                     double value)
{
    return SetControl(radarId, controlId, ControlValue::Number(value));
}

// Set enum control (string value)
bool MayaraClient::SetControlEnum(const std::string& radarId,
                                   const std::string& controlId,
                                   const std::string& value)
{
    return SetControl(radarId, controlId, ControlValue::Enumeration(value));
}

// Set compound control with mode
bool MayaraClient::SetControlCompound(const std::string& radarId,
                                       const std::string& controlId,
                                       const std::string& mode,
                                       double value)
{
    return SetControl(radarId, controlId, ControlValue::Compound(mode, value));
}

// Legacy convenience methods
bool MayaraClient::SetPower(const std::string& radarId, RadarStatus status) {
    std::string value = RadarStatusToString(status).ToStdString();
    return SetControlEnum(radarId, "power", value);
}

bool MayaraClient::SetRange(const std::string& radarId, double rangeMeters) {
    return SetControlNumber(radarId, "range", rangeMeters);
}

bool MayaraClient::SetGain(const std::string& radarId, int value, bool autoMode) {
    return SetControlCompound(radarId, "gain", autoMode ? "auto" : "manual", value);
}

bool MayaraClient::SetSea(const std::string& radarId, int value, bool autoMode) {
    return SetControlCompound(radarId, "sea", autoMode ? "auto" : "manual", value);
}

bool MayaraClient::SetRain(const std::string& radarId, int value) {
    return SetControlNumber(radarId, "rain", value);
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
