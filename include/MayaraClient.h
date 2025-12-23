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
#include <optional>

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

// ============================================================
// Capability Types (mirrors mayara-core/src/capabilities/mod.rs)
// ============================================================

/// Control type determines what UI widget to render
enum class ControlType {
    Boolean,    // On/off toggle
    Number,     // Numeric value with range
    Enum,       // Selection from fixed values
    Compound,   // Complex object with multiple properties
    String      // Text value (typically read-only)
};

/// Control category
enum class ControlCategory {
    Base,           // Base controls available on all radars
    Extended,       // Extended controls specific to certain models
    Installation    // Installation/setup controls
};

/// Range specification for number controls
struct RangeSpec {
    double min;
    double max;
    std::optional<double> step;
    std::optional<std::string> unit;
};

/// Enum value with label and optional description
struct EnumValue {
    std::string value;      // The actual value (as string, even if numeric)
    std::string label;      // Human-readable label
    std::optional<std::string> description;
    bool readOnly = false;  // Whether this value is read-only (can be reported but not set)
};

/// Property definition for compound controls
struct PropertyDefinition {
    std::string propType;   // "number", "enum", "boolean", etc.
    std::optional<std::string> description;
    std::optional<RangeSpec> range;
    std::vector<EnumValue> values;  // For enum properties
};

/// Control definition (schema, not value)
struct ControlDefinition {
    std::string id;             // Semantic control ID (e.g., "gain", "sea")
    std::string name;           // Human-readable name
    std::string description;    // Description for tooltips
    ControlCategory category;   // Base, Extended, or Installation
    ControlType controlType;    // Determines UI widget

    // For number types
    std::optional<RangeSpec> range;

    // For enum types
    std::vector<EnumValue> values;

    // For compound types
    std::map<std::string, PropertyDefinition> properties;

    // For controls with auto/manual modes
    std::vector<std::string> modes;
    std::optional<std::string> defaultMode;

    // Whether this control is read-only
    bool readOnly = false;

    // Default value (as JSON string for complex types)
    std::optional<std::string> defaultValue;
};

/// Hardware characteristics of the radar
struct Characteristics {
    uint32_t maxRange;
    uint32_t minRange;
    std::vector<uint32_t> supportedRanges;
    uint16_t spokesPerRevolution;
    uint16_t maxSpokeLength;
    bool hasDoppler;
    bool hasDualRange;
    uint32_t maxDualRange;
    uint8_t noTransmitZoneCount;
};

/// Optional features a radar provider may implement
enum class SupportedFeature {
    Arpa,       // ARPA target tracking
    GuardZones, // Guard zone alerting
    Trails,     // Target history/trail data
    DualRange   // Dual-range simultaneous display
};

/// Capability manifest from /capabilities endpoint (full schema)
struct CapabilityManifest {
    std::string id;
    std::optional<std::string> key;
    std::string make;
    std::string model;
    std::optional<std::string> modelFamily;
    std::optional<std::string> serialNumber;
    std::optional<std::string> firmwareVersion;

    Characteristics characteristics;
    std::vector<ControlDefinition> controls;
    std::vector<SupportedFeature> supportedFeatures;

    // Helper methods
    bool hasControl(const std::string& controlId) const;
    const ControlDefinition* getControl(const std::string& controlId) const;
    bool hasFeature(SupportedFeature feature) const;

    // Legacy compatibility
    int spokesPerRevolution() const { return characteristics.spokesPerRevolution; }
    int maxSpokeLength() const { return characteristics.maxSpokeLength; }
};

// Control value - supports all control types
// The value can be: bool, number, string (enum), or JSON (compound)
struct ControlValue {
    ControlType type = ControlType::Number;

    // For simple boolean controls
    bool boolValue = false;

    // For number controls (or manual value in compound)
    double numericValue = 0.0;

    // For enum controls (string value)
    std::string stringValue;

    // For compound controls with mode
    std::string mode;  // "auto" or "manual" or empty

    // Raw JSON representation (for compound types)
    std::string jsonValue;

    // Convenience constructors
    static ControlValue Boolean(bool v) {
        ControlValue cv;
        cv.type = ControlType::Boolean;
        cv.boolValue = v;
        return cv;
    }

    static ControlValue Number(double v) {
        ControlValue cv;
        cv.type = ControlType::Number;
        cv.numericValue = v;
        return cv;
    }

    static ControlValue Enumeration(const std::string& v) {
        ControlValue cv;
        cv.type = ControlType::Enum;
        cv.stringValue = v;
        return cv;
    }

    static ControlValue Compound(const std::string& mode, double value) {
        ControlValue cv;
        cv.type = ControlType::Compound;
        cv.mode = mode;
        cv.numericValue = value;
        return cv;
    }
};

// Radar state from /state endpoint
struct RadarState {
    RadarStatus status;
    double rangeMeters;
    std::map<std::string, ControlValue> controls;

    // Helper to get a control value by ID
    const ControlValue* getControl(const std::string& id) const {
        auto it = controls.find(id);
        return it != controls.end() ? &it->second : nullptr;
    }
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

    // -------- Controls (generic) --------
    // Set any control using ControlValue
    bool SetControl(const std::string& radarId,
                    const std::string& controlId,
                    const ControlValue& value);

    // Set boolean control
    bool SetControlBool(const std::string& radarId,
                        const std::string& controlId,
                        bool value);

    // Set numeric control
    bool SetControlNumber(const std::string& radarId,
                          const std::string& controlId,
                          double value);

    // Set enum control (string value)
    bool SetControlEnum(const std::string& radarId,
                        const std::string& controlId,
                        const std::string& value);

    // Set compound control with mode (auto/manual) and optional value
    bool SetControlCompound(const std::string& radarId,
                            const std::string& controlId,
                            const std::string& mode,
                            double value = 0.0);

    // -------- Controls (legacy convenience methods) --------
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
