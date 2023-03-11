#include <string>
#include <iostream>

#define GLAD_EGL_IMPLEMENTATION
#include "glad/egl.h"

int main(int argc, char *argv[]) {

  int initialEglVersion = gladLoaderLoadEGL(NULL);
  if (!initialEglVersion) {
    std::cerr << "gladLoaderLoadEGL(NULL): Unable to load EGL" << std::endl;
    return 1;
  }
  std::cout << "Loaded EGL " << GLAD_VERSION_MAJOR(initialEglVersion) << "."
    << GLAD_VERSION_MINOR(initialEglVersion) << " on first load." << std::endl;
  std::string extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  std::cout << "Client extensions: " << extensions << std::endl;
  
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLint major, minor;
  if (eglInitialize(display, &major, &minor)) {
    std::cout << "eglInitialize: EGL V" << major << "." << minor << std::endl;
  } else {
    std::cerr << "Unable to initialize default display " << display << ": " << std::hex << eglGetError() << std::endl;
    return 1;
  }

  int actualEglVersion = gladLoaderLoadEGL(display);
  if (!actualEglVersion) {
    std::cerr << "gladLoaderLoadEGL(default display): Unable to load EGL" << std::endl;
    return 1;
  }
  std::cout << "Loaded EGL " << GLAD_VERSION_MAJOR(actualEglVersion) << "."
    << GLAD_VERSION_MINOR(actualEglVersion) << " on second load." << std::endl;

  extensions = eglQueryString(display, EGL_EXTENSIONS);
  std::cout << "Display extensions: " << extensions << std::endl;


  return 0;
}
