/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * OpenGL extension function loader implementation
 */

#include "pi_common.h"
#include "gl_funcs.h"

#ifdef __WXMSW__

// Function pointers - initialized to nullptr
PFNGLCREATESHADERPROC       glCreateShader_ptr = nullptr;
PFNGLSHADERSOURCEPROC       glShaderSource_ptr = nullptr;
PFNGLCOMPILESHADERPROC      glCompileShader_ptr = nullptr;
PFNGLGETSHADERIVPROC        glGetShaderiv_ptr = nullptr;
PFNGLGETSHADERINFOLOGPROC   glGetShaderInfoLog_ptr = nullptr;
PFNGLDELETESHADERPROC       glDeleteShader_ptr = nullptr;
PFNGLCREATEPROGRAMPROC      glCreateProgram_ptr = nullptr;
PFNGLATTACHSHADERPROC       glAttachShader_ptr = nullptr;
PFNGLLINKPROGRAMPROC        glLinkProgram_ptr = nullptr;
PFNGLGETPROGRAMIVPROC       glGetProgramiv_ptr = nullptr;
PFNGLGETPROGRAMINFOLOGPROC  glGetProgramInfoLog_ptr = nullptr;
PFNGLDELETEPROGRAMPROC      glDeleteProgram_ptr = nullptr;
PFNGLUSEPROGRAMPROC         glUseProgram_ptr = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr = nullptr;
PFNGLUNIFORM1IPROC          glUniform1i_ptr = nullptr;
PFNGLUNIFORM1FPROC          glUniform1f_ptr = nullptr;
PFNGLUNIFORM2FPROC          glUniform2f_ptr = nullptr;
PFNGLUNIFORM3FPROC          glUniform3f_ptr = nullptr;
PFNGLUNIFORM4FPROC          glUniform4f_ptr = nullptr;
PFNGLUNIFORMMATRIX4FVPROC   glUniformMatrix4fv_ptr = nullptr;

static bool s_gl_funcs_initialized = false;

bool InitGLFunctions() {
    if (s_gl_funcs_initialized) return true;

    // Load function pointers using wglGetProcAddress
    glCreateShader_ptr = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource_ptr = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader_ptr = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glDeleteShader_ptr = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader_ptr = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glUseProgram_ptr = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1i_ptr = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    glUniform1f_ptr = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
    glUniform2f_ptr = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
    glUniform3f_ptr = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    glUniform4f_ptr = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
    glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");

    // Check if critical functions were loaded
    s_gl_funcs_initialized = (glCreateShader_ptr != nullptr &&
                              glCreateProgram_ptr != nullptr &&
                              glLinkProgram_ptr != nullptr);

    return s_gl_funcs_initialized;
}

#endif  // __WXMSW__
