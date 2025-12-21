/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * wxGLCanvas for separate PPI window
 */

#ifndef _RADAR_CANVAS_H_
#define _RADAR_CANVAS_H_

#include "pi_common.h"

// Forward declaration - plugin class is in global namespace
class mayara_server_pi;

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class RadarDisplay;

class RadarCanvas : public wxGLCanvas {
public:
    RadarCanvas(wxWindow* parent,
                ::mayara_server_pi* plugin,
                RadarDisplay* radar);
    ~RadarCanvas();

    // Force a redraw
    void Render();

    // Get the radar display
    RadarDisplay* GetRadarDisplay() { return m_radar; }

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnRightDown(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnClose(wxCloseEvent& event);

    // Convert mouse position to radar coordinates (bearing, distance)
    bool MouseToRadar(int x, int y, double& bearing, double& distance);

    ::mayara_server_pi* m_plugin;
    RadarDisplay* m_radar;
    wxGLContext* m_context;

    // Zoom level (1.0 = fit to window)
    double m_zoom;

    // Drag state
    bool m_dragging;
    int m_drag_start_x;
    int m_drag_start_y;

    DECLARE_EVENT_TABLE()
};

// Frame window containing the canvas
class RadarFrame : public wxFrame {
public:
    RadarFrame(wxWindow* parent,
               ::mayara_server_pi* plugin,
               RadarDisplay* radar);
    ~RadarFrame();

    RadarCanvas* GetCanvas() { return m_canvas; }

private:
    void OnClose(wxCloseEvent& event);

    ::mayara_server_pi* m_plugin;
    RadarDisplay* m_radar;
    RadarCanvas* m_canvas;

    DECLARE_EVENT_TABLE()
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_CANVAS_H_
