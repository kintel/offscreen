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
#define hasGLExtension(ext) GLAD_##ext
#else
void initGLExtensions(int major, int minor, bool gles);
bool lookupGLExtension(const char *ext);
#define hasGLExtension(ext) lookupGLExtension(#ext)
#endif

#ifdef DEBUG
  #define GL_CHECK(stmt) stmt; glCheck(#stmt, __FILE__, __LINE__)
#else
  #define GL_CHECK(stmt) stmt
#endif
