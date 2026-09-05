#pragma once

#include <iostream>

#ifdef USE_GLAD
#define GLAD_GLES2
#include "glad/gl.h"
#else
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/glu.h>
#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "utils/printutils.h"

namespace {

// Returns true on OK, false on error
[[maybe_unused]] bool glCheck(const char *stmt = "", const char *file = "", int line = 0)
{
  if (const auto err = glGetError(); err != GL_NO_ERROR) {
    const char *errStr = reinterpret_cast<const char *>(gluErrorString(err));
    LOG(message_group::Error,
        "OpenGL error: %1$s (0x%2$04x) in %3$s:%4$d\n"
        "              %5$s\n",
        errStr ? errStr : "unknown", err, file, line, stmt);
    return false;
  }
  return true;
}

[[maybe_unused]] inline bool glCheck() { return glCheck("", "", 0); }

// Returns true on OK, false on error
[[maybe_unused]] bool glCheckd(const char *stmt = "", const char *file = "", int line = 0)
{
  if (const auto err = glGetError(); err != GL_NO_ERROR) {
    const char *errStr = reinterpret_cast<const char *>(gluErrorString(err));
    PRINTDB(
      "OpenGL error: %s (0x%04x) in %s:%d\n"
      "              %s\n",
      (errStr ? errStr : "unknown") % err % file % line % stmt);
    return false;
  }
  return true;
}

[[maybe_unused]] inline bool glCheckd() { return glCheckd("", "", 0); }


} // namespace

#ifdef USE_GLAD
#define hasGLExtension(ext) GLAD_GL_##ext
#define hasGLVersion3() (GLAD_GL_VERSION_3_0 != 0)
#define hasGLESVersion2() (GLAD_GL_ES_VERSION_2_0 != 0)
#else
void initGLExtensions(int major, int minor, bool gles);
bool lookupGLExtension(const char *ext);
#define hasGLExtension(ext) lookupGLExtension("GL_" #ext)
bool hasGLVersion3();
bool hasGLESVersion2();
#endif

#define GL_CHECK(...) \
  __VA_ARGS__;        \
  glCheck(#__VA_ARGS__, __FILE__, __LINE__)

#define IF_GL_CHECK(...) \
  __VA_ARGS__;           \
  if (!glCheck(#__VA_ARGS__, __FILE__, __LINE__))

#define GL_CHECKD(...) \
  __VA_ARGS__;         \
  glCheckd(#__VA_ARGS__, __FILE__, __LINE__)

#ifdef DEBUG
#define GL_DEBUG_CHECKD(...) \
  __VA_ARGS__;               \
  glCheckd(#__VA_ARGS__, __FILE__, __LINE__)
#else
#define GL_DEBUG_CHECKD(...) __VA_ARGS__
#endif

