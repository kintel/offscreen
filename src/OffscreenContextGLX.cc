#include "OffscreenContextGLX.h"
#include <GL/glx.h>

#include <iostream>

#include "scope_guard.hpp"

namespace {

int xlibLastError = 0;
int xlibErrorHandler(Display *dpy, XErrorEvent *event) {
  xlibLastError = event->error_code;
  return 0;
}

}  // namespace

class OffscreenContextGLXImpl : public OffscreenContextGLX {
public:
  GLXContext glxContext = nullptr;
  Display *display = nullptr;
  Window xWindow = 0;
  OffscreenContextGLXImpl(int width, int height) : OffscreenContextGLX(width, height) {}

  bool makeCurrent() override {
    return glXMakeContextCurrent(this->display, this->xWindow, this->xWindow, this->glxContext);
  }
  bool destroy() override {
    if (this->display) {
      if (this->glxContext) glXDestroyContext(this->display, this->glxContext);
      if (this->xWindow) XDestroyWindow(this->display, this->xWindow);
      XCloseDisplay(this->display);
    }
    return true;
  }

  /*
   create a dummy X window without showing it. (without 'mapping' it)
   and save information to the ctx.

   This purposely does not use glxCreateWindow, to avoid crashes,
   "failed to create drawable" errors, and Mesa "WARNING: Application calling
   GLX 1.3 function when GLX 1.3 is not supported! This is an application bug!"

   This function will alter ctx.openGLContext and ctx.xwindow if successful
 */
  bool createGLXContext() {
    const int attributes[] = {
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT | GLX_PBUFFER_BIT, //support all 3, for OpenCSG
      GLX_RENDER_TYPE, GLX_RGBA_BIT,
      GLX_RED_SIZE, 8,
      GLX_GREEN_SIZE, 8,
      GLX_BLUE_SIZE, 8,
      GLX_ALPHA_SIZE, 8,
      GLX_DEPTH_SIZE, 24, // depth-stencil for OpenCSG
      GLX_STENCIL_SIZE, 8,
      GLX_DOUBLEBUFFER, true, // FIXME: Do we need this?
      None
    };

    int numConfigs = 0;
    GLXFBConfig *fbconfigs = nullptr;
    XVisualInfo *visinfo = nullptr;
    auto guard = sg::make_scope_guard([fbconfigs, visinfo]() {
      if (fbconfigs) XFree(fbconfigs);
      if (visinfo)XFree(visinfo);
    });
    fbconfigs = glXChooseFBConfig(this->display, DefaultScreen(this->display), attributes, &numConfigs);
    if (fbconfigs == nullptr) {
      std::cerr << "glXChooseFBConfig() failed" << std::endl;
      return false;
    }
    visinfo = glXGetVisualFromFBConfig(this->display, fbconfigs[0]);
    if (visinfo == nullptr) {
      std::cerr << "glXGetVisualFromFBConfig failed" << std::endl;
      return false;
    }

    // We can't depend on XCreateWindow() returning 0 on failure, so we use a custom Xlib error handler
    XErrorHandler originalErrorHandler = XSetErrorHandler(xlibErrorHandler);
    auto errorGuard = sg::make_scope_guard([originalErrorHandler]() {
      XSetErrorHandler(originalErrorHandler);
    });

    const auto root = DefaultRootWindow(this->display);
    XSetWindowAttributes windowAttributes = {
      .event_mask = StructureNotifyMask | ExposureMask | KeyPressMask,
      .colormap = XCreateColormap(this->display, root, visinfo->visual, AllocNone), 
    };
    unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    this->xWindow = 
      XCreateWindow(this->display, root, 0, 0, this->width(), this->height(), 0, 
                    visinfo->depth, InputOutput, visinfo->visual, mask, &windowAttributes);
    XSync(this->display, false);
    if (xlibLastError != Success) {
      char description[1024];
      XGetErrorText(this->display, xlibLastError, description, 1023);
      std::cerr << "XCreateWindow() failed: " << description << std::endl;
      return false;
    }

    // Most programs would call XMapWindow here. But we don't, to keep the window hidden
    this->glxContext = glXCreateNewContext(this->display, fbconfigs[0], GLX_RGBA_TYPE, nullptr, true);
    if (this->glxContext == nullptr) {
      std::cerr << "glXCreateNewContext() failed" << std::endl;
      return false;
    }

    return true;
  }
};


/*
   create a dummy X window without showing it. (without 'mapping' it)
   and save information to the ctx.

   This purposely does not use glxCreateWindow, to avoid crashes,
   "failed to create drawable" errors, and Mesa "WARNING: Application calling
   GLX 1.3 function when GLX 1.3 is not supported! This is an application bug!"

   This function will alter ctx.openGLContext and ctx.xwindow if successful
 */
std::shared_ptr<OffscreenContextGLX> OffscreenContextGLX::create(size_t width, size_t height,
								 size_t majorGLVersion, size_t minorGLVersion, bool gles, bool compatibilityProfile)
{
  auto ctx = std::make_shared<OffscreenContextGLXImpl>(width, height);

  ctx->display = XOpenDisplay(nullptr);
  if (ctx->display == nullptr) {
    std::cerr << "Unable to open a connection to the X server." << std::endl;
    auto dpyenv = getenv("DISPLAY");
    std::cerr << "DISPLAY=" << (dpyenv?dpyenv:"") << std::endl;
    return nullptr;
  }

  // We require GLX >= 1.3.
  // glxQueryVersion sometimes returns an earlier version than is actually available, so 
  // we also accept GLX < 1.3 as long as glXGetVisualFromFBConfig() exists.
  int glxMajor = 0, glxMinor = 0;
  glXQueryVersion(ctx->display, &glxMajor, &glxMinor);
  if (glxMajor < 0 || glxMajor == 1 && glxMinor <= 2 && glXGetVisualFromFBConfig == nullptr) {
    std::cerr << "Error: GLX version 1.3 functions missing. "
              << "Your GLX version: " << glxMajor << "." << glxMinor << std::endl;
    return nullptr;
  }
  std::cout << "GLX version: " << glxMajor << "." << glxMinor << std::endl;
  
  if (!ctx->createGLXContext()) {
    return nullptr;
  }

	return ctx;
}
