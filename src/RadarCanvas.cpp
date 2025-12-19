/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * wxGLCanvas for separate PPI window
 */

#include "RadarCanvas.h"
#include "mayara_server_pi.h"
#include "RadarManager.h"
#include "RadarDisplay.h"
#include "RadarPPIRenderer.h"
#include "SpokeBuffer.h"

using namespace mayara;

// OpenGL attributes
static int gl_attribs[] = {
    WX_GL_RGBA,
    WX_GL_DOUBLEBUFFER,
    WX_GL_DEPTH_SIZE, 16,
    0
};

BEGIN_EVENT_TABLE(RadarCanvas, wxGLCanvas)
    EVT_PAINT(RadarCanvas::OnPaint)
    EVT_SIZE(RadarCanvas::OnSize)
    EVT_MOUSEWHEEL(RadarCanvas::OnMouseWheel)
    EVT_LEFT_DOWN(RadarCanvas::OnLeftDown)
    EVT_RIGHT_DOWN(RadarCanvas::OnRightDown)
    EVT_KEY_DOWN(RadarCanvas::OnKeyDown)
END_EVENT_TABLE()

RadarCanvas::RadarCanvas(wxWindow* parent,
                         mayara_server_pi* plugin,
                         RadarDisplay* radar)
    : wxGLCanvas(parent, wxID_ANY, gl_attribs, wxDefaultPosition,
                 wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
    , m_plugin(plugin)
    , m_radar(radar)
    , m_context(nullptr)
    , m_zoom(1.0)
    , m_dragging(false)
    , m_drag_start_x(0)
    , m_drag_start_y(0)
{
    m_context = new wxGLContext(this);
}

RadarCanvas::~RadarCanvas() {
    delete m_context;
}

void RadarCanvas::Render() {
    Refresh(false);
}

void RadarCanvas::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);

    if (!m_context || !m_radar) return;

    SetCurrent(*m_context);

    int width, height;
    GetClientSize(&width, &height);

    glViewport(0, 0, width, height);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update and render
    auto* renderer = m_radar->GetPPIRenderer();
    if (renderer) {
        renderer->UpdateTexture(m_radar->GetSpokeBuffer());
        renderer->DrawPPI(m_context, width, height,
                          m_radar->GetRangeMeters(),
                          m_plugin->GetHeading());

        // Draw targets
        renderer->DrawTargets(width, height,
                              m_radar->GetRangeMeters(),
                              m_radar->GetTargets());
    }

    SwapBuffers();
}

void RadarCanvas::OnSize(wxSizeEvent& event) {
    Refresh(false);
    event.Skip();
}

void RadarCanvas::OnMouseWheel(wxMouseEvent& event) {
    int rotation = event.GetWheelRotation();

    if (rotation > 0) {
        m_zoom *= 1.1;
    } else if (rotation < 0) {
        m_zoom /= 1.1;
    }

    // Clamp zoom
    if (m_zoom < 0.5) m_zoom = 0.5;
    if (m_zoom > 5.0) m_zoom = 5.0;

    Refresh(false);
}

void RadarCanvas::OnLeftDown(wxMouseEvent& event) {
    // Acquire target at click position
    if (!m_radar) return;

    double bearing, distance;
    if (MouseToRadar(event.GetX(), event.GetY(), bearing, distance)) {
        auto* manager = m_plugin->GetRadarManager();
        if (manager && manager->GetClient()) {
            manager->GetClient()->AcquireTarget(m_radar->GetId(), bearing, distance);
        }
    }
}

void RadarCanvas::OnRightDown(wxMouseEvent& event) {
    // TODO: Context menu
}

void RadarCanvas::OnKeyDown(wxKeyEvent& event) {
    int keycode = event.GetKeyCode();

    switch (keycode) {
        case '+':
        case WXK_NUMPAD_ADD:
            m_zoom *= 1.1;
            break;
        case '-':
        case WXK_NUMPAD_SUBTRACT:
            m_zoom /= 1.1;
            break;
        case '0':
            m_zoom = 1.0;
            break;
        default:
            event.Skip();
            return;
    }

    // Clamp zoom
    if (m_zoom < 0.5) m_zoom = 0.5;
    if (m_zoom > 5.0) m_zoom = 5.0;

    Refresh(false);
}

void RadarCanvas::OnClose(wxCloseEvent& event) {
    // Notify radar display that window is closing
    if (m_radar) {
        m_radar->SetPPIWindow(nullptr);
    }
    event.Skip();
}

bool RadarCanvas::MouseToRadar(int x, int y, double& bearing, double& distance) {
    if (!m_radar) return false;

    int width, height;
    GetClientSize(&width, &height);

    // Calculate center and radius
    int display_size = std::min(width, height) - 40;
    float radius = display_size / 2.0f;
    float cx = width / 2.0f;
    float cy = height / 2.0f;

    // Convert to relative coordinates
    float dx = x - cx;
    float dy = y - cy;

    // Calculate distance ratio
    float dist_pixels = sqrt(dx * dx + dy * dy);
    float dist_ratio = dist_pixels / radius;

    if (dist_ratio > 1.0f) return false;  // Outside radar display

    // Calculate bearing (0 = north, clockwise)
    float angle = atan2(dx, -dy);  // Adjust for north-up
    bearing = angle * 180.0 / M_PI;
    if (bearing < 0) bearing += 360.0;

    // Adjust for heading
    bearing += m_plugin->GetHeading();
    while (bearing >= 360.0) bearing -= 360.0;
    while (bearing < 0) bearing += 360.0;

    // Calculate distance
    distance = dist_ratio * m_radar->GetRangeMeters();

    return true;
}

// ========== RadarFrame ==========

BEGIN_EVENT_TABLE(RadarFrame, wxFrame)
    EVT_CLOSE(RadarFrame::OnClose)
END_EVENT_TABLE()

RadarFrame::RadarFrame(wxWindow* parent,
                       mayara_server_pi* plugin,
                       RadarDisplay* radar)
    : wxFrame(parent, wxID_ANY,
              wxString::Format(_("Radar: %s"), radar->GetName()),
              wxDefaultPosition, wxSize(600, 600),
              wxDEFAULT_FRAME_STYLE)
    , m_plugin(plugin)
    , m_radar(radar)
{
    m_canvas = new RadarCanvas(this, plugin, radar);

    // Register with radar display
    radar->SetPPIWindow(m_canvas);

    // Set minimum size
    SetMinSize(wxSize(300, 300));
}

RadarFrame::~RadarFrame() {
    if (m_radar) {
        m_radar->SetPPIWindow(nullptr);
    }
}

void RadarFrame::OnClose(wxCloseEvent& event) {
    if (m_radar) {
        m_radar->SetPPIWindow(nullptr);
    }
    Destroy();
}
