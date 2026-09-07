#include "OffscreenContextWGL.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <windows.h>
#ifdef USE_GLAD
#define GLAD_WGL
#define GLAD_WGL_IMPLEMENTATION
#include <glad/wgl.h>
#include <glad/gl.h>
#endif

#include "OffscreenContext.h"
#include "scope_guard.hpp"

class OffscreenContextWGL : public OffscreenContext {

public:
  HWND window = nullptr;
  HDC devContext = nullptr;
  HGLRC renderContext = nullptr;

  OffscreenContextWGL(int width, int height) : OffscreenContext(width, height) {}
  ~OffscreenContextWGL() {
    wglMakeCurrent(nullptr, nullptr);
    if (this->renderContext) wglDeleteContext(this->renderContext);
    if (this->devContext) ReleaseDC(this->window, this->devContext);
    if (this->window) DestroyWindow(this->window);
  }

  std::string getInfo() const override {
    std::ostringstream result;
    result << "GL context creator: WGL\n";
    return result.str();
  }

  bool makeCurrent() const override {
    return wglMakeCurrent(this->devContext, this->renderContext);
  }
};


std::shared_ptr<OffscreenContext> CreateOffscreenContextWGL(size_t width, size_t height,
							    size_t majorGLVersion, size_t minorGLVersion, bool compatibilityProfile)
{
  auto ctx = std::make_shared<OffscreenContextWGL>(width, height);

  WNDCLASSEX wndClass = {};
  wndClass.cbSize = sizeof(WNDCLASSEX);
  wndClass.style = CS_OWNDC;
  wndClass.lpfnWndProc = &DefWindowProc;
  wndClass.lpszClassName = "OffscreenClass";
  // FIXME: Check for ERROR_CLASS_ALREADY_EXISTS ?
  RegisterClassEx(&wndClass);
  // Create the window. Position and size it.
  // Style the window and remove the caption bar (WS_POPUP)
  ctx->window = CreateWindowEx(0, "OffscreenClass", "offscreen", WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, 0, 0);
  ctx->devContext = GetDC(ctx->window);

  PIXELFORMATDESCRIPTOR pixelFormatDesc = {};
  pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pixelFormatDesc.nVersion = 1;
  // FIXME: Can we remove PFD_DOUBLEBUFFER for offscreen rendering?
  pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
  pixelFormatDesc.cColorBits = 32;
  pixelFormatDesc.cDepthBits = 24;
  pixelFormatDesc.cStencilBits = 8;

  int pixelFormat = ChoosePixelFormat(ctx->devContext, &pixelFormatDesc);
  SetPixelFormat(ctx->devContext, pixelFormat, &pixelFormatDesc);
  // FIXME: Use wglChoosePixelFormatARB() if appropriate

  const auto tmpRenderContext = wglCreateContext(ctx->devContext);
  if (tmpRenderContext == nullptr) {
    std::cerr << "wglCreateContext() failed: " << GetLastError() << std::endl;
    return nullptr;
  }
  auto guard = sg::make_scope_guard([tmpRenderContext]() {
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tmpRenderContext);
  });

  wglMakeCurrent(ctx->devContext, tmpRenderContext);
  gladLoaderLoadWGL(ctx->devContext);

  int attributes[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, static_cast<int>(majorGLVersion),
    WGL_CONTEXT_MINOR_VERSION_ARB, static_cast<int>(minorGLVersion),
    WGL_CONTEXT_PROFILE_MASK_ARB,
    compatibilityProfile ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB,         
    0
  };
  ctx->renderContext = wglCreateContextAttribsARB(ctx->devContext, nullptr, attributes);
  if (ctx->renderContext == nullptr) {
    std::cerr << "wglCreateContextAttribsARB() failed: " << GetLastError() << std::endl;
    return nullptr;
  }

  return ctx;
}
