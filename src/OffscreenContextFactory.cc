#include "OffscreenContextFactory.h"

#include <iostream>

#ifdef __APPLE__
#include "OffscreenContextNSOpenGL.h"
#include "OffscreenContextCGL.h"
#endif
#ifdef _WIN32
#include "OffscreenContextWGL.h"
#endif
#ifdef HAS_EGL
#include "OffscreenContextEGL.h"
#endif
#ifdef ENABLE_GLX
#include "OffscreenContextGLX.h"
#endif
#ifdef ENABLE_GLFW
#include "GLFWContext.h"
#endif

namespace OffscreenContextFactory {

const char *defaultProvider() {
#ifdef __APPLE__
  return "cgl";
#endif
#if HAS_EGL
  return "egl";
#endif
#ifdef ENABLE_GLX
  return "glx";
#endif
#ifdef _WIN32
  return "wgl";
#endif
#ifdef ENABLE_GLFW
 return "glfw";
#endif
}

std::shared_ptr<OpenGLContext> create(const std::string& provider, const ContextAttributes& attrib) {

  // FIXME: We could log an error if the chosen provider doesn't support all our attribs.
#ifdef __APPLE__
  if (provider == "nsopengl") {
    return OffscreenContextNSOpenGL::create(attrib.width, attrib.height, attrib.majorGLVersion, attrib.minorGLVersion);
  }
  if (provider == "cgl") {
    return OffscreenContextCGL::create(attrib.width, attrib.height, attrib.majorGLVersion, attrib.minorGLVersion);
  }
#endif
#if HAS_EGL
  if (provider == "egl") {
    return OffscreenContextEGL::create(attrib.width, attrib.height, attrib.majorGLVersion, attrib.minorGLVersion,
                                       attrib.gles, attrib.compatibilityProfile, attrib.gpu);
  }
  else
#endif
#ifdef ENABLE_GLX
  if (provider == "glx") {
   return OffscreenContextGLX::create(attrib.width, attrib.height, attrib.majorGLVersion, attrib.minorGLVersion, 
                                      attrib.gles, attrib.compatibilityProfile);
  }
#endif
#ifdef _WIN32
  if (provider == "wgl") {
    return OffscreenContextWGL::create(attrib.width, attrib.height, attrib.majorGLVersion, attrib.minorGLVersion,
                                       attrib.compatibilityProfile);
  }
  else
#endif
#ifdef ENABLE_GLFW
  if (provider == "glfw") {
    return GLFWContext::create(attrib.width, attrib.height, attrib.majorGLVersion, attrib.minorGLVersion,
                               attrib.invisible);
  }
#endif
  std::cerr << "Context provider '" << provider << "' not found" << std::endl;
  return {};
}

}  // namespace OffscreenContextFactory

