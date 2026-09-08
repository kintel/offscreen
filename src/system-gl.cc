#include "system-gl.h"

#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <sstream>

namespace {

std::set<std::string> glExtensions;
int glMajorVersion = 0;
bool glIsGLES = false;

void queryGLVersionIfNeeded() {
  if (glMajorVersion > 0) return;
  const char *v = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  if (!v) return;
  if (std::strncmp(v, "OpenGL ES ", 10) == 0) {
    glIsGLES = true;
    v += 10;
  }
  std::sscanf(v, "%d", &glMajorVersion);
}

} // namespace

#ifndef USE_GLAD

void initGLExtensions(int major, int minor, bool gles)
{
  glExtensions.clear();
  glMajorVersion = major;
  glIsGLES = gles;

  if (major == 2 && !gles) {
    const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
    if (extensions) {
      std::istringstream iss(extensions);
      while (iss) {
        std::string extension;
        iss >> extension;
        glExtensions.insert(extension);
      }
    }
  } else {
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(auto i = 0; i < numExtensions; ++i) {
      const char *ext = reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i));
      if (ext) {
        glExtensions.insert(ext);
      }
    }
  }
}

bool lookupGLExtension(const char *ext)
{
  return glExtensions.find(ext) != glExtensions.end();
}

bool hasGLVersion3()
{
  queryGLVersionIfNeeded();
  return !glIsGLES && glMajorVersion >= 3;
}

bool hasGLESVersion2()
{
  queryGLVersionIfNeeded();
  return glIsGLES && glMajorVersion >= 2;
}

#endif

