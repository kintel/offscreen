#include "OffscreenContextEGL.h"

#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <gbm.h>

#include "EGL/egl.h"
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

#include "GL/gl.h"

namespace {

#define CASE_STR( value ) case value: return #value; 
const char* eglGetErrorString( EGLint error )
{
    switch( error )
    {
    CASE_STR( EGL_SUCCESS             )
    CASE_STR( EGL_NOT_INITIALIZED     )
    CASE_STR( EGL_BAD_ACCESS          )
    CASE_STR( EGL_BAD_ALLOC           )
    CASE_STR( EGL_BAD_ATTRIBUTE       )
    CASE_STR( EGL_BAD_CONTEXT         )
    CASE_STR( EGL_BAD_CONFIG          )
    CASE_STR( EGL_BAD_CURRENT_SURFACE )
    CASE_STR( EGL_BAD_DISPLAY         )
    CASE_STR( EGL_BAD_SURFACE         )
    CASE_STR( EGL_BAD_MATCH           )
    CASE_STR( EGL_BAD_PARAMETER       )
    CASE_STR( EGL_BAD_NATIVE_PIXMAP   )
    CASE_STR( EGL_BAD_NATIVE_WINDOW   )
    CASE_STR( EGL_CONTEXT_LOST        )
    default: return "Unknown";
    }
}
#undef CASE_STR

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
  if (eglGetDisplayDriverName) {
    if (const char *name = eglGetDisplayDriverName(eglDisplay); name) {
      std::cout << "    Display driver name " << name << std::endl;
    }
  }
  std::cout << "    Extensions: " << eglQueryString(eglDisplay, EGL_EXTENSIONS) << std::endl;

  EGLint numConfigs;
  if (!eglChooseConfig(eglDisplay, configAttribs, nullptr, 0, &numConfigs)) {
    std::cerr << "    eglChooseConfig(): Failed to get number of configs. eglError: " << std::hex << eglGetError() << ")" << std::endl;
    return;
  }
  std::cout << "    " << numConfigs << " configs available from eglChooseConfig()" << std::endl;

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
    std::cout << "Found " << numDevices << " EGL devices" << std::endl;
    for (int idx = 0; idx < numDevices; idx++) {
      EGLDisplay eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevices[idx], 0);
      if (eglDisplay != EGL_NO_DISPLAY) {
        std::cout << "  Display for device #" << idx << " OK" << std::endl;
        dumpEGLDisplay(eglDisplay);
      }
      else {
        std::cout << "  Display for device #" << idx << " Invalid" << std::endl;
      }
    }
  }
  else {
    std::cout << "Display query extensions not available" << std::endl;
  }
}

void dumpEGLGBMPlatform(const std::string& drmNode) {
  std::cout << "=== GBM Platform ===" << std::endl;
  
  const int fd = open(drmNode.c_str(), O_RDWR);
  if (fd < 0) {
    std::cerr << "Unable to open " << drmNode << std::endl;
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

} // namespace

class OffscreenContextEGLImpl : public OffscreenContextEGL {

public:
  EGLDisplay eglDisplay;
  EGLSurface eglSurface;
  EGLContext eglContext;

// If eglDisplay is backed by a GBM device.
  struct gbm_device *gbmDevice = nullptr;

  OffscreenContextEGLImpl(int width, int height) : OffscreenContextEGL(width, height) {}
  
  bool makeCurrent() override {
    eglMakeCurrent(this->eglDisplay, this->eglSurface, this->eglSurface, this->eglContext);
    return true;
  }
  bool destroy() override {
    return true;
  }

    void getDisplayFromDrmNode(const std::string& drmNode) {
    this->eglDisplay = EGL_NO_DISPLAY;
    const int fd = open(drmNode.c_str(), O_RDWR);
    if (fd < 0) {
      std::cerr << "Unable to open DRM node " << drmNode << std::endl;
      return;
    }

    this->gbmDevice = gbm_create_device(fd);
    if (!this->gbmDevice) {
      std::cerr << "Unable to create GDM device" << std::endl;
      return;
    }

    // FIXME: Check EGL extension before passing the identifier to this function
    this->eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, this->gbmDevice, nullptr);
  }

  void findPlatformDisplay() {
    std::set<std::string> clientExtensions;
    std::string ext = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    std::cout << ext << std::endl;
    std::istringstream iss(ext);
    while (iss) {
      std::string extension;
      iss >> extension;
      clientExtensions.insert(extension);
    }

    if (clientExtensions.find("EGL_EXT_platform_device") == clientExtensions.end()) {
      return;
    }

    std::cout << "Trying Platform display..." << std::endl;
    auto eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) eglGetProcAddress("eglQueryDevicesEXT");
    auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (eglQueryDevicesEXT && eglGetPlatformDisplayEXT) {
      EGLDeviceEXT eglDevice;
      EGLint numDevices = 0;
      eglQueryDevicesEXT(1, &eglDevice, &numDevices);
      if (numDevices > 0) {
      // FIXME: Attribs
        this->eglDisplay =  eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevice, nullptr);
      }
    }
  }

  void createSurface(const EGLConfig& config,size_t width, size_t height) {
    if (this->gbmDevice) {
// FIXME: For some reason, we have to pass 0 as flags for the nvidia GBM backend
      const auto gbmSurface =
        gbm_surface_create(this->gbmDevice, width, height,
                           GBM_FORMAT_ARGB8888, 
                           0); // GBM_BO_USE_RENDERING
      if (!gbmSurface) {
        std::cerr << "Unable to create GBM surface" << std::endl;
        this->eglSurface = EGL_NO_SURFACE;
        return;
      }

      this->eglSurface =
        eglCreatePlatformWindowSurface(this->eglDisplay, config, gbmSurface, nullptr);
    }
    else {
      const EGLint pbufferAttribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE,
      };
      this->eglSurface = eglCreatePbufferSurface(this->eglDisplay, config, pbufferAttribs);
    }
  }
};


void OffscreenContextEGL::dumpEGLInfo(const std::string& drmNode) {

  dumpEGLDevicePlatform();
  dumpEGLGBMPlatform(drmNode);


}

// Typical variants:
// OpenGL core major.minor
// OpenGL compatibility major.minor
// OpenGL ES major.minor
std::shared_ptr<OffscreenContextEGL> OffscreenContextEGL::create(size_t width, size_t height,
								 size_t majorGLVersion, size_t minorGLVersion, bool gles, bool compatibilityProfile,
                 const std::string &drmNode)
{
  auto ctx = std::make_shared<OffscreenContextEGLImpl>(width, height);

  EGLint conformant;
  if (!gles) conformant = EGL_OPENGL_BIT;
  else if (majorGLVersion >= 3) conformant = EGL_OPENGL_ES3_BIT;
  else if (majorGLVersion >= 2) conformant = EGL_OPENGL_ES2_BIT;
  else conformant = EGL_OPENGL_ES_BIT;

  const EGLint configAttribs[] = {
    // For some reason, we have to request a "window" surface when using GBM, although
    // we're rendering offscreen
    EGL_SURFACE_TYPE, drmNode.empty() ? EGL_PBUFFER_BIT : EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_CONFORMANT, conformant,
    EGL_CONFIG_CAVEAT, EGL_NONE,
    EGL_NONE
  };

  if (!drmNode.empty()) {
    std::cout << "Using GBM..." << std::endl;
    ctx->getDisplayFromDrmNode(drmNode);
  } else {
    // FIXME: Should we try default display first?
    // If so, we also have to try initializing it
    ctx->findPlatformDisplay();
    if (ctx->eglDisplay == EGL_NO_DISPLAY) {
      std::cout << "Trying default EGL display..." << std::endl;
      ctx->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }
  } 

  if (ctx->eglDisplay == EGL_NO_DISPLAY) {
    std::cerr << "No EGL display found" << std::endl;
    return nullptr;
  }

  EGLint major, minor;
  if (!eglInitialize(ctx->eglDisplay, &major, &minor)) {
    std::cerr << "Unable to initialize EGL: " << eglGetErrorString(eglGetError()) << std::endl;
    return nullptr;
  }

  std::cout << "EGL Version: " << major << "." << minor << " (" << eglQueryString(ctx->eglDisplay, EGL_VENDOR) << ")" << std::endl;

  auto eglGetDisplayDriverName = (PFNEGLGETDISPLAYDRIVERNAMEPROC) eglGetProcAddress("eglGetDisplayDriverName");
  if (eglGetDisplayDriverName) {
    const char *name = eglGetDisplayDriverName(ctx->eglDisplay);
    if (name) {
      std::cout << "Got EGL display with driver name: " << name << std::endl;
    }
  }

  EGLint numConfigs;
  EGLConfig config;
  bool gotConfig = eglChooseConfig(ctx->eglDisplay, configAttribs, &config, 1, &numConfigs);
  if (!gotConfig || numConfigs == 0) {
    std::cerr << "Failed to choose config (eglError: " << std::hex << eglGetError() << ")" << std::endl;
    return nullptr;
  }
  if (!eglBindAPI(gles ? EGL_OPENGL_ES_API : EGL_OPENGL_API)) {
    std::cerr << "eglBindAPI() failed!" << std::endl;
    return nullptr;
  }

  ctx->createSurface(config, width, height);    
  if (ctx->eglSurface == EGL_NO_SURFACE) {
    std::cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << std::endl;
    return nullptr;
  }

  std::vector<EGLint> ctxattr = {
    EGL_CONTEXT_MAJOR_VERSION, static_cast<EGLint>(majorGLVersion),
    EGL_CONTEXT_MINOR_VERSION, static_cast<EGLint>(minorGLVersion),
  };
  if (!gles) {
    ctxattr.push_back(EGL_CONTEXT_OPENGL_PROFILE_MASK);
    ctxattr.push_back(compatibilityProfile ? EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT : EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT);
  }
  ctxattr.push_back(EGL_NONE);
  ctx->eglContext = eglCreateContext(ctx->eglDisplay, config, EGL_NO_CONTEXT, ctxattr.data());
  if (ctx->eglContext == EGL_NO_CONTEXT) {
    std::cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")" << std::endl;
    return nullptr;
  }

  return ctx;
}
