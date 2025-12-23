/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * WebSocket client for receiving spoke data from mayara-server
 */

// Include wx headers first to get ssize_t defined before IXWebSocket
#include "pi_common.h"

#include "SpokeReceiver.h"
#include <ixwebsocket/IXWebSocket.h>

using namespace mayara;

SpokeReceiver::SpokeReceiver(const std::string& url,
                             SpokeCallback callback,
                             int reconnect_interval_ms)
    : m_callback(callback)
    , m_url(url)
    , m_reconnect_interval_ms(reconnect_interval_ms)
    , m_connected(false)
    , m_should_run(false)
    , m_spokes_received(0)
    , m_bytes_received(0)
{
    wxLogMessage("MaYaRa: SpokeReceiver ctor - about to create WebSocket");
    wxLog::FlushActive();

    // Delay WebSocket creation - just store URL for now
    // WebSocket will be created in Start()
    wxLogMessage("MaYaRa: SpokeReceiver ctor - done (WebSocket creation deferred)");
    wxLog::FlushActive();
}

SpokeReceiver::~SpokeReceiver() {
    Stop();
}

void SpokeReceiver::Start() {
    wxLogMessage("MaYaRa: SpokeReceiver::Start() entry");
    wxLog::FlushActive();

    m_should_run = true;

    // Create WebSocket here (deferred from constructor)
    if (!m_websocket) {
        wxLogMessage("MaYaRa: SpokeReceiver::Start() - creating WebSocket now");
        wxLog::FlushActive();
        try {
            m_websocket = std::make_unique<ix::WebSocket>();
            wxLogMessage("MaYaRa: SpokeReceiver::Start() - WebSocket created OK");
            wxLog::FlushActive();
        } catch (const std::exception& e) {
            wxLogMessage("MaYaRa: SpokeReceiver::Start() - WebSocket creation FAILED: %s", e.what());
            wxLog::FlushActive();
            return;
        } catch (...) {
            wxLogMessage("MaYaRa: SpokeReceiver::Start() - WebSocket creation FAILED (unknown)");
            wxLog::FlushActive();
            return;
        }
    }

    wxLogMessage("MaYaRa: SpokeReceiver::Start() - setting URL: %s", m_url.c_str());
    wxLog::FlushActive();
    m_websocket->setUrl(m_url);

    wxLogMessage("MaYaRa: SpokeReceiver::Start() - setting callback");
    wxLog::FlushActive();
    m_websocket->setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg) {
            switch (msg->type) {
                case ix::WebSocketMessageType::Open:
                    OnOpen();
                    break;
                case ix::WebSocketMessageType::Close:
                    OnClose();
                    break;
                case ix::WebSocketMessageType::Error:
                    OnError(msg->errorInfo.reason);
                    break;
                case ix::WebSocketMessageType::Message:
                    if (msg->binary) {
                        OnMessage(msg->str);
                    }
                    break;
                default:
                    break;
            }
        }
    );

    wxLogMessage("MaYaRa: SpokeReceiver::Start() - calling websocket->start()");
    wxLog::FlushActive();
    m_websocket->start();
    wxLogMessage("MaYaRa: SpokeReceiver::Start() - complete");
    wxLog::FlushActive();
}

void SpokeReceiver::Stop() {
    m_should_run = false;
    if (m_websocket) {
        m_websocket->stop();
    }
    m_connected = false;
}

void SpokeReceiver::OnOpen() {
    m_connected = true;
}

void SpokeReceiver::OnClose() {
    m_connected = false;

    if (m_should_run) {
        ScheduleReconnect();
    }
}

void SpokeReceiver::OnError(const std::string& error) {
    m_connected = false;

    if (m_should_run) {
        ScheduleReconnect();
    }
}

void SpokeReceiver::OnMessage(const std::string& data) {
    m_bytes_received += data.size();

    if (DecodeProtobuf(data)) {
        m_spokes_received++;
    }
}

void SpokeReceiver::ScheduleReconnect() {
    // IXWebSocket handles reconnection internally
    // We just need to restart
    if (m_should_run && !m_connected) {
        m_websocket->start();
    }
}

bool SpokeReceiver::DecodeProtobuf(const std::string& data) {
    // TODO: Implement protobuf decoding
    // For now, create a simple spoke from raw data

    if (data.size() < 16) return false;  // Minimum header size

    // Parse basic header (this is a placeholder - real implementation
    // needs proper protobuf decoding)
    SpokeData spoke;
    spoke.angle = 0;
    spoke.bearing = 0;
    spoke.rangeMeters = 1000;
    spoke.timestamp = wxGetLocalTimeMillis().GetValue();

    // Copy data
    spoke.data.assign(data.begin(), data.end());

    // Call callback
    if (m_callback) {
        m_callback(spoke);
    }

    return true;
}
