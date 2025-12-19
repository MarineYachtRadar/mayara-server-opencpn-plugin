/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Base class for radar rendering
 */

#ifndef _RADAR_RENDERER_H_
#define _RADAR_RENDERER_H_

#include "pi_common.h"
#include "ColorPalette.h"

#ifdef __WXOSX__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

PLUGIN_BEGIN_NAMESPACE

// Forward declarations
class SpokeBuffer;

class RadarRenderer {
public:
    RadarRenderer();
    virtual ~RadarRenderer();

    // Initialize with radar parameters
    virtual bool Init(size_t spokes, size_t maxSpokeLen);

    // Reset/clear the renderer
    virtual void Reset();

    // Update texture from spoke buffer
    virtual void UpdateTexture(SpokeBuffer* buffer);

    // Set color palette
    void SetColorPalette(const ColorPalette& palette);

    // Check if initialized
    bool IsInitialized() const { return m_initialized; }

protected:
    // Shader compilation helpers
    bool CompileShaders();
    GLuint CompileShader(GLenum type, const char* source);
    bool LinkProgram();

    // OpenGL resources
    GLuint m_program;
    GLuint m_vertex_shader;
    GLuint m_fragment_shader;
    GLuint m_texture;
    GLuint m_palette_texture;

    // Radar parameters
    size_t m_spokes;
    size_t m_spoke_len_max;

    // Color palette
    ColorPalette m_palette;

    bool m_initialized;
    bool m_texture_dirty;

    wxCriticalSection m_lock;
};

PLUGIN_END_NAMESPACE

#endif  // _RADAR_RENDERER_H_
