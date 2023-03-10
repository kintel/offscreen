#include "egl_utils.h"

#include <fcntl.h>
#include <iostream>
#ifdef HAS_GBM
#include <gbm.h>
#endif

#include "glad/egl.h"
#include "GL/gl.h"

namespace {

void dumpEGLDisplay(EGLDisplay eglDisplay) {
  const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE
  };

  const EGLint pbufferAttribs[] = {
    EGL_WIDTH, 640,
    EGL_HEIGHT, 480,
    EGL_NONE,
  };

  auto eglGetDisplayDriverName = (PFNEGLGETDISPLAYDRIVERNAMEPROC) eglGetProcAddress("eglGetDisplayDriverName");

  EGLint major, minor;
  if (!eglInitialize(eglDisplay, &major, &minor)) {
    std::cerr << "    Unable to initialize EGL" << std::endl;
    return;
  }

  std::cout << "    Initialized EGL for display: " << major << "." << minor
            << " (" << eglQueryString(eglDisplay, EGL_VENDOR) << ")" << std::endl;

  const auto eglVersion = gladLoaderLoadEGL(eglDisplay);
  if (!eglVersion) {
    std::cerr << "    gladLoaderLoadEGL(eglDisplay): Unable to reload EGL" << std::endl;
    return;
  }
  std::cout << "    Loaded EGL " << GLAD_VERSION_MAJOR(eglVersion) << "." << GLAD_VERSION_MINOR(eglVersion) << " after reload" << std::endl;

  if (eglGetDisplayDriverName) {
    if (const char *name = eglGetDisplayDriverName(eglDisplay); name) {
      std::cout << "    Display driver name: " << name << std::endl;
    }
  }
  std::cout << "    Display extensions: " << eglQueryString(eglDisplay, EGL_EXTENSIONS) << std::endl;

  EGLint numConfigs;
  if (!eglChooseConfig(eglDisplay, configAttribs, nullptr, 0, &numConfigs)) {
    std::cerr << "    eglChooseConfig(): Failed to get number of configs. eglError: " << std::hex << eglGetError() << ")" << std::endl;
    return;
  }

  // if (!eglGetConfigs(disp, nullptr, 0, &numConfigs)) {
  //   std::cerr << "    Failed to get number of configs. eglError: " << std::hex << eglGetError() << ")" << std::endl;
  // }
  // else {
  //   std::cout << "    Got " << numConfigs << " configs from eglGetConfigs()" << std::endl;
  // }

  auto configs = new EGLConfig[numConfigs];
  if (!eglChooseConfig(eglDisplay, configAttribs, configs, numConfigs, &numConfigs)) {
    std::cerr << "    eglChooseConfig(): Failed to choose configs. eglError: " << std::hex << eglGetError() << ")" << std::endl;
    return;
  }
  std::cout << "    Got " << numConfigs << " configs from eglChooseConfig()" << std::endl;

  if (!eglBindAPI(EGL_OPENGL_API)) {
    std::cerr << "Bind EGL_OPENGL_API failed!" << std::endl;
    return;
  }

  for (int i=0;i<numConfigs;++i) {
    std::cout << "    Config #" << i << ":" << std::endl;
    EGLint val;
    eglGetConfigAttrib(eglDisplay, configs[i], EGL_CONFIG_ID, &val);
    std::cout << "      EGL_CONFIG_ID: " << val << std::endl;
    eglGetConfigAttrib(eglDisplay, configs[i], EGL_CONFORMANT, &val);
    std::cout << "      "
      << (val & EGL_OPENGL_BIT ? "OpenGL " : "")
      << (val & EGL_OPENGL_ES_BIT ? "GLES " : "")
      << (val & EGL_OPENGL_ES2_BIT ? "GLES2 " : "")
      << (val & EGL_OPENGL_ES3_BIT ? "GLES3" : "") << std::endl;
    eglGetConfigAttrib(eglDisplay, configs[i], EGL_CONFIG_CAVEAT, &val);
    if (val != EGL_NONE) {
      std::cout << "      EGL_CONFIG_CAVEAT: " << val << std::endl;
    }

    const auto& config = configs[i];
    const auto eglSurface = eglCreatePbufferSurface(eglDisplay, config, pbufferAttribs);
    if (eglSurface == EGL_NO_SURFACE) {
      std::cerr << "      Unable to create EGL surface (eglError: " << eglGetError() << ")" << std::endl;
      continue;
    }

  // FIXME: Query multiple GL versions and profiles?
      EGLint ctxattr[] = {
        EGL_CONTEXT_MAJOR_VERSION, 2,
        EGL_CONTEXT_MINOR_VERSION, 0,
        EGL_NONE
      };
    // EGLint ctxattr[] = {
    //   EGL_CONTEXT_MAJOR_VERSION, majorGLVersion,
    //   EGL_CONTEXT_MINOR_VERSION, minorGLVersion,
    //   EGL_CONTEXT_OPENGL_PROFILE_MASK, compatibilityProfile ? EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT : EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    //   EGL_NONE
    // };
    const auto eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxattr);
    if (eglContext == EGL_NO_CONTEXT) {
      std::cerr << "      Unable to create EGL context (eglError: " << eglGetError() << ")" << std::endl;
    }
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    std::cout << "      OpenGL version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION)) << std::endl;
    std::cout << "      renderer: " << glGetString(GL_RENDERER) << std::endl;
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
  }
  eglTerminate(eglDisplay);
}

void dumpEGLDevicePlatform() {
  std::cout << "=== Device Platform ===" << std::endl;
  auto eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");
  auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");

  if (eglQueryDevicesEXT && eglGetPlatformDisplayEXT) {
    const int MAX_DEVICES = 10;
    EGLDeviceEXT eglDevices[MAX_DEVICES];
    EGLint numDevices = 0;

    eglQueryDevicesEXT(MAX_DEVICES, eglDevices, &numDevices);
    std::cout << "Found " << numDevices << " EGL devices:" << std::endl;
    for (int idx = 0; idx < numDevices; idx++) {
      EGLDisplay eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevices[idx], 0);
      if (eglDisplay != EGL_NO_DISPLAY) {
        std::cout << "  Display for device #" << idx << ": OK" << std::endl;
        dumpEGLDisplay(eglDisplay);
      }
      else {
        std::cout << "  Display for device #" << idx << ": Invalid" << std::endl;
      }
    }
  }
  else {
    std::cout << "Display query extensions not available" << std::endl;
  }
}

#ifdef HAS_GBM
void dumpEGLGBMPlatform(const std::string& drmNode) {
  std::cout << "=== GBM Platform ===" << std::endl;

  const int fd = open(drmNode.c_str(), O_RDWR);
  if (fd < 0) {
    std::cerr << "Unable to open DRM node " << drmNode << std::endl;
    return;
  }

  const auto gbmDevice = gbm_create_device(fd);
  if (!gbmDevice) {
    std::cerr << "Unable to create GDM device" << std::endl;
    return;
  }
  std::cout << "GBM backend: " << gbm_device_get_backend_name(gbmDevice) << std::endl;

  // FIXME: Check EGL extension before passing the identifier to this function
  EGLDisplay eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbmDevice, nullptr);
  if (eglDisplay == EGL_NO_DISPLAY) {
    std::cerr << "Unable to get EGL Display from GBM device" << std::endl;
    return;
  }

  EGLint major, minor;
  if (!eglInitialize(eglDisplay, &major, &minor)) {
    std::cerr << "Unable to initialize EGL" << std::endl;
    return;
  }

  std::cout << "  Initialized EGL for GBM display: " << major << "." << minor
            << " (" << eglQueryString(eglDisplay, EGL_VENDOR) << ")" << std::endl;

  const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, drmNode.empty()? EGL_PBUFFER_BIT : EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_CONFORMANT, EGL_OPENGL_BIT,
    EGL_CONFIG_CAVEAT, EGL_NONE,
    EGL_NONE
  };

  EGLint numConfigs;
  EGLConfig config;
  if (!eglChooseConfig(eglDisplay, configAttribs, &config, 1, &numConfigs)) {
    std::cerr << "Failed to choose config (eglError: " << std::hex << eglGetError() << ")" << std::endl;
    return;
  }

  const auto gbmSurface = gbm_surface_create(gbmDevice,
                                  256, 256,
                                  GBM_FORMAT_ARGB8888,
  // FIXME: For some reason, we have to pass 0 as flags for the nvidia GBM backend
                                  0);                                  
  //                                  GBM_BO_USE_RENDERING);
  if (!gbmSurface) {
    std::cerr << "Unable to create GBM surface" << std::endl;
    return;
  }

  EGLSurface eglSurface =
    eglCreatePlatformWindowSurface(eglDisplay, config, gbmSurface, nullptr);

  EGLint ctxattr[] = {
    EGL_CONTEXT_MAJOR_VERSION, 2,
    EGL_CONTEXT_MINOR_VERSION, 0,
    EGL_NONE
  };
  const auto eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxattr);
  if (eglContext == EGL_NO_CONTEXT) {
    std::cerr << "      Unable to create EGL context (eglError: " << eglGetError() << ")" << std::endl;
  }
  eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
  std::cout << "  OpenGL version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION)) << std::endl;
  std::cout << "         renderer: " << glGetString(GL_RENDERER) << std::endl;
  eglDestroyContext(eglDisplay, eglContext);
  eglDestroySurface(eglDisplay, eglSurface);
  gbm_surface_destroy(gbmSurface);
  eglTerminate(eglDisplay);
}
#endif

}  // namespace

void dumpEGLInfo(const std::string& drmNode) {

  int initialEglVersion = gladLoaderLoadEGL(nullptr);
  if (!initialEglVersion) {
    std::cerr << "gladLoaderLoadEGL(nullptr): Unable to load EGL" << std::endl;
    return;
  }
  std::cout << "Initial EGL loaded: " << GLAD_VERSION_MAJOR(initialEglVersion) << "."
    << GLAD_VERSION_MINOR(initialEglVersion) << " on first load." << std::endl;
  std::cout << "Client extensions: " << eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS) << std::endl;

  dumpEGLDevicePlatform();
#ifdef HAS_GBM
  dumpEGLGBMPlatform(drmNode);
#endif

  gladLoaderUnloadEGL();
}