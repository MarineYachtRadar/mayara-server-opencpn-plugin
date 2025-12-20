/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * OpenGL extension function loader for shader support
 *
 * NOTE: This header should be included AFTER pi_common.h or wx headers
 * to ensure platform detection macros are defined.
 */

#ifndef _GL_FUNCS_H_
#define _GL_FUNCS_H_

// On Linux/GTK, we need GL_GLEXT_PROTOTYPES to get shader function declarations
// This MUST be defined before ANY GL headers are included
#if !defined(__APPLE__) && !defined(_WIN32) && !defined(__WXMSW__)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#endif

// Ensure wx platform detection is done
#include <wx/defs.h>

#ifdef __WXOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#elif defined(__WXMSW__)
// On Windows, wx includes windows.h which defines WINGDIAPI needed by gl.h
#include <wx/msw/wrapwin.h>
#include <GL/gl.h>
#include <GL/glext.h>
#else
// Linux/GTK - GL_GLEXT_PROTOTYPES already defined above
#include <GL/gl.h>
#include <GL/glext.h>
#endif

// On Windows, we need to load these functions dynamically
#ifdef __WXMSW__

// Function pointer types are defined in glext.h with PFNGL* prefix
// We define wrapper macros that use the loaded function pointers

extern PFNGLCREATESHADERPROC       glCreateShader_ptr;
extern PFNGLSHADERSOURCEPROC       glShaderSource_ptr;
extern PFNGLCOMPILESHADERPROC      glCompileShader_ptr;
extern PFNGLGETSHADERIVPROC        glGetShaderiv_ptr;
extern PFNGLGETSHADERINFOLOGPROC   glGetShaderInfoLog_ptr;
extern PFNGLDELETESHADERPROC       glDeleteShader_ptr;
extern PFNGLCREATEPROGRAMPROC      glCreateProgram_ptr;
extern PFNGLATTACHSHADERPROC       glAttachShader_ptr;
extern PFNGLLINKPROGRAMPROC        glLinkProgram_ptr;
extern PFNGLGETPROGRAMIVPROC       glGetProgramiv_ptr;
extern PFNGLGETPROGRAMINFOLOGPROC  glGetProgramInfoLog_ptr;
extern PFNGLDELETEPROGRAMPROC      glDeleteProgram_ptr;
extern PFNGLUSEPROGRAMPROC         glUseProgram_ptr;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr;
extern PFNGLUNIFORM1IPROC          glUniform1i_ptr;
extern PFNGLUNIFORM1FPROC          glUniform1f_ptr;
extern PFNGLUNIFORM2FPROC          glUniform2f_ptr;
extern PFNGLUNIFORM3FPROC          glUniform3f_ptr;
extern PFNGLUNIFORM4FPROC          glUniform4f_ptr;
extern PFNGLUNIFORMMATRIX4FVPROC   glUniformMatrix4fv_ptr;

// Redefine GL functions to use pointers
#define glCreateShader       glCreateShader_ptr
#define glShaderSource       glShaderSource_ptr
#define glCompileShader      glCompileShader_ptr
#define glGetShaderiv        glGetShaderiv_ptr
#define glGetShaderInfoLog   glGetShaderInfoLog_ptr
#define glDeleteShader       glDeleteShader_ptr
#define glCreateProgram      glCreateProgram_ptr
#define glAttachShader       glAttachShader_ptr
#define glLinkProgram        glLinkProgram_ptr
#define glGetProgramiv       glGetProgramiv_ptr
#define glGetProgramInfoLog  glGetProgramInfoLog_ptr
#define glDeleteProgram      glDeleteProgram_ptr
#define glUseProgram         glUseProgram_ptr
#define glGetUniformLocation glGetUniformLocation_ptr
#define glUniform1i          glUniform1i_ptr
#define glUniform1f          glUniform1f_ptr
#define glUniform2f          glUniform2f_ptr
#define glUniform3f          glUniform3f_ptr
#define glUniform4f          glUniform4f_ptr
#define glUniformMatrix4fv   glUniformMatrix4fv_ptr

// Initialize OpenGL extension functions - call once before using shaders
bool InitGLFunctions();

#else
// On other platforms, functions are directly available
inline bool InitGLFunctions() { return true; }
#endif

#endif  // _GL_FUNCS_H_
