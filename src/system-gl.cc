#include "system-gl.h"

#include <set>
#include <string>
#include <sstream>

namespace {

std::set<std::string> glExtensions;

}

#ifndef USE_GLAD

void initGLExtensions(int major, int minor, bool gles)
{
  glExtensions.clear();
  // Framebuffer Objects were promoted to core functionality in OpenGL 3.0 and GLES 2.0.
  // Core Profile drivers (e.g. macOS Core Profile, Mesa) do not list promoted core features
  // in the GL_EXTENSIONS string list, so we explicitly insert GL_ARB_framebuffer_object here
  // to maintain compatibility with hasGLExtension(ARB_framebuffer_object) checks in non-GLAD builds.
  if (major >= 3 || (gles && major >= 2)) {
    glExtensions.insert("GL_ARB_framebuffer_object");
  }
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

#endif
