#include "OffscreenContextWGL.h"

#include <iostream>

#include <windows.h>
#ifdef USE_GLAD
#define GLAD_WGL
#define GLAD_WGL_IMPLEMENTATION
#include <glad/wgl.h>
#include <glad/gl.h>
#endif

class OffscreenContextWGL : public OffscreenContext {

public:
  HWND window = nullptr;
  HDC devContext = nullptr;
  HGLRC renderContext = nullptr;

  OffscreenContextWGL(int width, int height) : OffscreenContext(width, height) {}
  
  bool makeCurrent() override {
    wglMakeCurrent(this->devContext, this->renderContext);
    return true;
  }
  bool destroy() override {
    wglMakeCurrent(nullptr, nullptr);
    bool ok = true;
    if (this->renderContext) {
      ok = wglDeleteContext(this->renderContext);
      if (!ok) {
        std::cerr << "wglDeleteContext() failed: " << GetLastError() << std::endl;
      }
    }
    if (this->devContext) {
      ok = ReleaseDC(this->window, this->devContext);
      if (!ok) {
        std::cerr << "ReleaseDC() failed: " << GetLastError() << std::endl;
      }
    }
    if (this->window) {
      ok = DestroyWindow(this->window);
      if (!ok) {
        std::cerr << "DestroyWindow() failed: " << GetLastError() << std::endl;
      }
    }
    return ok;
  }
};

std::shared_ptr<OffscreenContext> CreateOffscreenContextWGL(size_t width, size_t height,
							    size_t majorGLVersion, size_t minorGLVersion, bool compatibilityProfile)
{
  auto ctx = std::make_shared<OffscreenContextWGL>(width, height);

  WNDCLASSEX wndClass = {
    .cbSize = sizeof(WNDCLASSEX),
    .style = CS_OWNDC,
    .lpfnWndProc = &DefWindowProc,
    .lpszClassName = "OffscreenClass"
  };
  // FIXME: Check for ERROR_CLASS_ALREADY_EXISTS ?
  RegisterClassEx(&wndClass);
  // Create the window. Position and size it.
  // Style the window and remove the caption bar (WS_POPUP)
  ctx->window = CreateWindowEx(0, "OffscreenClass", "offscreen", WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, 0, 0);
  ctx->devContext = GetDC(ctx->window);

  PIXELFORMATDESCRIPTOR pixelFormatDesc = {
    .nSize = sizeof(PIXELFORMATDESCRIPTOR),
    .nVersion = 1,
    // FIXME: Can we remove PFD_DOUBLEBUFFER for offscreen rendering?
    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 32,
    .cDepthBits = 24,
    .cStencilBits = 8
  };
  int pixelFormat = ChoosePixelFormat(ctx->devContext, &pixelFormatDesc);
  SetPixelFormat(ctx->devContext, pixelFormat, &pixelFormatDesc);
  // FIXME: Use wglChoosePixelFormatARB() if appropriate

  const auto tmpRenderContext = wglCreateContext(ctx->devContext);
  if (tmpRenderContext == nullptr) {
    std::cerr << "wglCreateContext() failed: " << GetLastError() << std::endl;
    ctx->destroy();
    return nullptr;
  }

  wglMakeCurrent(ctx->devContext, tmpRenderContext);
  gladLoaderLoadWGL(ctx->devContext);

  int attributes[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, majorGLVersion,
    WGL_CONTEXT_MINOR_VERSION_ARB, minorGLVersion,
    WGL_CONTEXT_PROFILE_MASK_ARB,
    compatibilityProfile ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB,         
    0
  };
  ctx->renderContext = wglCreateContextAttribsARB(ctx->devContext, nullptr, attributes);
  if (ctx->renderContext == nullptr) {
    std::cerr << "wglCreateContextAttribsARB() failed: " << GetLastError() << std::endl;
    ctx->destroy();
    return nullptr;
  }
  wglMakeCurrent(nullptr, nullptr);
  if (!wglDeleteContext(tmpRenderContext)) {
    std::cerr << "wglDeleteContext() failed: " << GetLastError() << std::endl;
    ctx->destroy();
    return nullptr;
  }

  return ctx;
}
