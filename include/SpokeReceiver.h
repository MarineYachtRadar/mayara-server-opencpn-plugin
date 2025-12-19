/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * WebSocket client for receiving spoke data from mayara-server
 */

#ifndef _SPOKE_RECEIVER_H_
#define _SPOKE_RECEIVER_H_

#include "pi_common.h"
#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <memory>

// Forward declare IXWebSocket types
namespace ix { class WebSocket; }

PLUGIN_BEGIN_NAMESPACE

// Spoke data received from WebSocket
struct SpokeData {
    uint32_t angle;          // 0 to spokesPerRevolution-1
    uint32_t bearing;        // Optional true bearing
    uint32_t rangeMeters;    // Range of last pixel
    uint64_t timestamp;      // Unix timestamp in ms
    std::vector<uint8_t> data;  // Pixel intensities
};

// Callback type for received spokes
using SpokeCallback = std::function<void(const SpokeData& spoke)>;

class SpokeReceiver {
public:
    SpokeReceiver(const std::string& url,
                  SpokeCallback callback,
                  int reconnect_interval_ms = 5000);
    ~SpokeReceiver();

    // Start/stop the WebSocket connection
    void Start();
    void Stop();

    // Connection status
    bool IsConnected() const { return m_connected.load(); }

    // Get statistics
    uint64_t GetSpokesReceived() const { return m_spokes_received.load(); }
    uint64_t GetBytesReceived() const { return m_bytes_received.load(); }

private:
    void OnMessage(const std::string& data);
    void OnOpen();
    void OnClose();
    void OnError(const std::string& error);
    void ScheduleReconnect();
    bool DecodeProtobuf(const std::string& data);

    std::unique_ptr<ix::WebSocket> m_websocket;
    SpokeCallback m_callback;
    std::string m_url;
    int m_reconnect_interval_ms;

    std::atomic<bool> m_connected;
    std::atomic<bool> m_should_run;
    std::atomic<uint64_t> m_spokes_received;
    std::atomic<uint64_t> m_bytes_received;

    std::thread m_reconnect_thread;
    wxCriticalSection m_lock;
};

PLUGIN_END_NAMESPACE

#endif  // _SPOKE_RECEIVER_H_
