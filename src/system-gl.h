#pragma once

#include <iostream>

#ifdef USE_GLAD
#define GLAD_GLES2
#include "glad/gl.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/glu.h>
#endif

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/glu.h>
#include <OpenGL/gl3.h>
#else
#include <GL/glu.h>
#endif

namespace {

void glCheck(const char *stmt, const char *file, int line)
{
  if (GLenum err = glGetError(); err != GL_NO_ERROR) {
    std::cerr << "OpenGL error: " << gluErrorString(err)
              << " (" << err << ") in " << file << ":" << line << "\n"
              << "              " << stmt << std::endl;
  }
}

} // namespace

#ifdef USE_GLAD
#define hasGLExtension(ext) GLAD_GL_##ext
#else
void initGLExtensions(int major, int minor, bool gles);
bool lookupGLExtension(const char *ext);
#define hasGLExtension(ext) lookupGLExtension("GL_" #ext)
#endif

#ifdef DEBUG
  #define GL_CHECK(...) __VA_ARGS__; glCheck(#__VA_ARGS__, __FILE__, __LINE__)
#else
  #define GL_CHECK(...) __VA_ARGS__
#endif


