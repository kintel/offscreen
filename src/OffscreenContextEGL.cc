#include "OffscreenContextEGL.h"

#include <iostream>

#include "EGL/egl.h"
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

#include "system-gl.h"

class OffscreenContextEGLImpl : public OffscreenContextEGL {
public:
  EGLDisplay eglDisplay;
  EGLSurface eglSurface;
  EGLContext eglContext;

  OffscreenContextEGLImpl(int width, int height) : OffscreenContextEGL(width, height) {}
  
  bool makeCurrent() override {
    eglMakeCurrent(this->eglDisplay, this->eglSurface, this->eglSurface, this->eglContext);
    return true;
  }
  bool destroy() override {
    return true;
  }
};

std::shared_ptr<OffscreenContextEGL> OffscreenContextEGL::create(size_t width, size_t height,
								 size_t majorGLVersion, size_t minorGLVersion, bool compatibilityProfile)
{
  auto ctx = std::make_shared<OffscreenContextEGLImpl>(width, height);

 static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_CONFORMANT, EGL_OPENGL_BIT,
    EGL_NONE
  };

  const EGLint pbufferAttribs[] = {
    EGL_WIDTH, width,
    EGL_HEIGHT, height,
    EGL_NONE,
  };
  
  ctx->eglDisplay = EGL_NO_DISPLAY;
  auto eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");
  auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
  if (eglQueryDevicesEXT && eglGetPlatformDisplayEXT) {
    const int MAX_DEVICES = 10;
    EGLDeviceEXT eglDevs[MAX_DEVICES];
    EGLint numDevices = 0;

    eglQueryDevicesEXT(MAX_DEVICES, eglDevs, &numDevices);
    std::cout << "Found " << numDevices << " EGL devices" << std::endl;
    for (int idx = 0; idx < numDevices; idx++) {
      EGLDisplay disp = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevs[idx], 0);
      if (disp != EGL_NO_DISPLAY) {
        std::cout << "  selecting #" << idx << std::endl;
        ctx->eglDisplay = disp;
        break;
      }
    }
  } else {
    std::cout << "Trying default EGL display..." << std::endl;
    ctx->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  }
  if (ctx->eglDisplay == EGL_NO_DISPLAY) {
    std::cerr << "No EGL display found" << std::endl;
    return nullptr;
  }


  EGLint major, minor;
  if (!eglInitialize(ctx->eglDisplay, &major, &minor)) {
    std::cerr << "Unable to initialize EGL" << std::endl;
    return nullptr;
  }

  std::cout << "EGL Version: " << major << "." << minor << " (" << eglQueryString(ctx->eglDisplay, EGL_VENDOR) << ")" << std::endl;

  auto eglGetDisplayDriverName = (PFNEGLGETDISPLAYDRIVERNAMEPROC) eglGetProcAddress("eglGetDisplayDriverName");
  if (eglGetDisplayDriverName) {
    const char *name = eglGetDisplayDriverName(ctx->eglDisplay);
    if (name) {
      std::cout << "Got EGL display with driver name " << name << std::endl;
    }
  }



 EGLint numConfigs;
  EGLConfig config;
  if (!eglChooseConfig(ctx->eglDisplay, configAttribs, &config, 1, &numConfigs)) {
    std::cerr << "Failed to choose config (eglError: " << std::hex << eglGetError() << ")" << std::endl;
    return nullptr;
  }
  if (!eglBindAPI(EGL_OPENGL_API)) {
    std::cerr << "Bind EGL_OPENGL_API failed!" << std::endl;
    return nullptr;
  }
  ctx->eglSurface = eglCreatePbufferSurface(ctx->eglDisplay, config, pbufferAttribs);
  if (ctx->eglSurface == EGL_NO_SURFACE) {
    std::cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << std::endl;
    return nullptr;
  }

  EGLint ctxattr[] = {
    EGL_CONTEXT_MAJOR_VERSION, majorGLVersion,
    EGL_CONTEXT_MINOR_VERSION, minorGLVersion,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, compatibilityProfile ? EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT : EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    EGL_NONE
  };
  ctx->eglContext = eglCreateContext(ctx->eglDisplay, config, EGL_NO_CONTEXT, ctxattr);
  if (ctx->eglContext == EGL_NO_CONTEXT) {
    std::cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")" << std::endl;
    return nullptr;
  }

  return ctx;
}
