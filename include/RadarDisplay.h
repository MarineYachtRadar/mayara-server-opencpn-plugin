/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Per-radar data container - owns receiver, buffer, and renderers
 */

#ifndef _RADAR_DISPLAY_H_
#define _RADAR_DISPLAY_H_

#include "pi_common.h"
#include "MayaraClient.h"
#include "SpokeReceiver.h"
#include "SpokeBuffer.h"
#include <memory>

// Forward declaration - plugin class is in global namespace
class mayara_server_pi;

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class RadarOverlayRenderer;
class RadarPPIRenderer;
class RadarCanvas;

class RadarDisplay {
public:
    RadarDisplay(::mayara_server_pi* plugin,
                 const std::string& id,
                 const RadarInfo& info);
    ~RadarDisplay();

    // Start/stop spoke reception
    void Start();
    void Stop();

    // Update capabilities and state from server
    void UpdateCapabilities(const CapabilityManifest& caps);
    void UpdateState(const RadarState& state);

    // Accessors
    std::string GetId() const { return m_id; }
    std::string GetName() const { return m_info.name; }
    std::string GetBrand() const { return m_info.brand; }
    std::string GetModel() const { return m_info.model; }
    RadarStatus GetStatus() const { return m_status; }
    double GetRangeMeters() const { return m_range_meters; }
    int GetSpokesPerRevolution() const { return m_spokes_per_revolution; }
    int GetMaxSpokeLength() const { return m_max_spoke_length; }

    // Connection status
    bool IsReceiving() const;

    // Get renderers
    RadarOverlayRenderer* GetOverlayRenderer() { return m_overlay_renderer.get(); }
    RadarPPIRenderer* GetPPIRenderer() { return m_ppi_renderer.get(); }

    // Get spoke buffer (for renderers)
    SpokeBuffer* GetSpokeBuffer() { return m_spoke_buffer.get(); }

    // Get/set PPI window
    RadarCanvas* GetPPIWindow() { return m_ppi_window; }
    void SetPPIWindow(RadarCanvas* window) { m_ppi_window = window; }

    // ARPA targets
    std::vector<ArpaTarget> GetTargets() const { return m_targets; }
    void UpdateTargets(const std::vector<ArpaTarget>& targets);

private:
    void OnSpokeReceived(const SpokeData& spoke);

    ::mayara_server_pi* m_plugin;
    std::string m_id;
    RadarInfo m_info;

    // Current state
    RadarStatus m_status;
    double m_range_meters;
    int m_spokes_per_revolution;
    int m_max_spoke_length;

    // Components
    std::unique_ptr<SpokeReceiver> m_receiver;
    std::unique_ptr<SpokeBuffer> m_spoke_buffer;
    std::unique_ptr<RadarOverlayRenderer> m_overlay_renderer;
    std::unique_ptr<RadarPPIRenderer> m_ppi_renderer;

    // Optional PPI window (owned by wxWidgets, not us)
    RadarCanvas* m_ppi_window;

    // ARPA targets
    std::vector<ArpaTarget> m_targets;

    wxCriticalSection m_lock;
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_DISPLAY_H_
