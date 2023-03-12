#include <string>
#include <iostream>

#define GLAD_EGL_IMPLEMENTATION
#include "glad/egl.h"

EGLDisplay tryDisplay(EGLDisplay display) {
  if (display == EGL_NO_DISPLAY) return EGL_NO_DISPLAY;

  EGLint major, minor;
  if (!eglInitialize(display, &major, &minor)) {
    std::cerr << "Unable to initialize display " << display << ": " << std::hex << eglGetError() << std::endl;
    return EGL_NO_DISPLAY;
  }

  std::cout << "eglInitialize(" << display << "): EGL V" << major << "." << minor << std::endl;
  
  int actualEglVersion = gladLoaderLoadEGL(display);
  if (!actualEglVersion) {
    std::cerr << "gladLoaderLoadEGL(default display): Unable to load EGL" << std::endl;
    return EGL_NO_DISPLAY;
  }
  std::cout << "Loaded EGL " << GLAD_VERSION_MAJOR(actualEglVersion) << "."
    << GLAD_VERSION_MINOR(actualEglVersion) << " (default display)" << std::endl;

  std::string extensions = eglQueryString(display, EGL_EXTENSIONS);
  std::cout << "Display extensions: " << extensions << std::endl;
  return display;
}

int main(int argc, char *argv[]) {

  int initialEglVersion = gladLoaderLoadEGL(NULL);
  if (!initialEglVersion) {
    std::cerr << "gladLoaderLoadEGL(NULL): Unable to load EGL" << std::endl;
    return 1;
  }
  std::cout << "Loaded EGL " << GLAD_VERSION_MAJOR(initialEglVersion) << "."
    << GLAD_VERSION_MINOR(initialEglVersion) << " (no display)" << std::endl;
  std::string extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  std::cout << "Client extensions: " << extensions << std::endl;
  
  std::cout << "Trying default display..." << std::endl;
  EGLDisplay display = tryDisplay(eglGetDisplay(EGL_DEFAULT_DISPLAY));
  if (display == EGL_NO_DISPLAY) {
    std::cout << "Unable to initialize default display" << std::endl;
    if (eglQueryDevicesEXT && eglGetPlatformDisplayEXT) {
      std::cout << "Trying eglQueryDevicesEXT()..." << std::endl;
      EGLDeviceEXT device;
      EGLint numDevices;
      if (!eglQueryDevicesEXT(1, &device, &numDevices)) {
        std::cerr << "eglQueryDevicesEXT() failed: " << std::hex << "0x" << eglGetError() << std::endl;
        return 1;
      }
      if (numDevices == 0) {
        std::cerr << "eglQueryDevicesEXT() returned no devices" << std::endl;
        return 1;
      }
      display = tryDisplay(eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, device, nullptr));
    }
  }

  if (display == EGL_NO_DISPLAY) {
    std::cout << "Unable to initialize display" << std::endl;
  }

  return 0;
}
