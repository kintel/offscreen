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
  if (major == 2) {
    const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
    std::istringstream iss(extensions);
    while (iss) {
      std::string extension;
      iss >> extension;
      glExtensions.insert(extension);
    }
  } else {
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(auto i = 0; i < numExtensions; ++i) {
      glExtensions.insert(reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i)));
    }
  }
}

bool lookupGLExtension(const char *ext)
{
  return glExtensions.find(ext) != glExtensions.end();
}

#endif
