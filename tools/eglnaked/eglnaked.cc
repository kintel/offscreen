#include <string>
#include <iostream>

#include <EGL/egl.h>

int main(int argc, char *argv[]) {
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

  return 0;
}
