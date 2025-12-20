/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Base class for radar rendering
 */

// Define GL_GLEXT_PROTOTYPES before any headers to enable shader functions on Linux
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include "RadarRenderer.h"
#include "SpokeBuffer.h"

using namespace mayara;

RadarRenderer::RadarRenderer()
    : m_program(0)
    , m_vertex_shader(0)
    , m_fragment_shader(0)
    , m_texture(0)
    , m_palette_texture(0)
    , m_spokes(0)
    , m_spoke_len_max(0)
    , m_initialized(false)
    , m_texture_dirty(true)
{
}

RadarRenderer::~RadarRenderer() {
    Reset();
}

bool RadarRenderer::Init(size_t spokes, size_t maxSpokeLen) {
    wxCriticalSectionLocker lock(m_lock);

    m_spokes = spokes;
    m_spoke_len_max = maxSpokeLen;

    // Create radar texture
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate texture (spokes x spoke_len, single channel)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,
                 maxSpokeLen, spokes, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    // Create palette texture (256 x 1, RGBA)
    glGenTextures(1, &m_palette_texture);
    glBindTexture(GL_TEXTURE_1D, m_palette_texture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 m_palette.GetLUT());

    m_initialized = true;
    return true;
}

void RadarRenderer::Reset() {
    wxCriticalSectionLocker lock(m_lock);

    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_palette_texture) {
        glDeleteTextures(1, &m_palette_texture);
        m_palette_texture = 0;
    }
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    if (m_vertex_shader) {
        glDeleteShader(m_vertex_shader);
        m_vertex_shader = 0;
    }
    if (m_fragment_shader) {
        glDeleteShader(m_fragment_shader);
        m_fragment_shader = 0;
    }

    m_initialized = false;
}

void RadarRenderer::UpdateTexture(SpokeBuffer* buffer) {
    if (!buffer || !m_initialized) return;

    wxCriticalSectionLocker lock(m_lock);

    // Upload texture data
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    buffer->GetMaxSpokeLen(), buffer->GetSpokes(),
                    GL_LUMINANCE, GL_UNSIGNED_BYTE,
                    buffer->GetTextureData());

    m_texture_dirty = false;
}

void RadarRenderer::SetColorPalette(const ColorPalette& palette) {
    wxCriticalSectionLocker lock(m_lock);

    m_palette = palette;

    // Update palette texture
    if (m_palette_texture) {
        glBindTexture(GL_TEXTURE_1D, m_palette_texture);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RGBA, GL_UNSIGNED_BYTE,
                        m_palette.GetLUT());
    }
}

bool RadarRenderer::CompileShaders() {
    // Subclasses override this
    return false;
}

GLuint RadarRenderer::CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool RadarRenderer::LinkProgram() {
    if (!m_vertex_shader || !m_fragment_shader) return false;

    m_program = glCreateProgram();
    glAttachShader(m_program, m_vertex_shader);
    glAttachShader(m_program, m_fragment_shader);
    glLinkProgram(m_program);

    GLint status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[512];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    return true;
}
