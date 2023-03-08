#include "OffscreenContextWGL.h"

#include <iostream>

#include <windows.h>

class OffscreenContextWGLImpl : public OffscreenContextWGL {

public:
  HWND window;
  HDC devContext;
  HGLRC renderContext;

  OffscreenContextWGLImpl(int width, int height) : OffscreenContextWGL(width, height) {}
  
  bool makeCurrent() override {
    wglMakeCurrent(this->devContext, this->renderContext);
    return true;
  }
  bool destroy() override {
    bool ok = wglDeleteContext(this->renderContext);
    if (!ok) {
      std::cerr << "wglDeleteContext() failed: " << GetLastError() << std::endl;
    }
    return ok;
  }
};

std::shared_ptr<OffscreenContextWGL> OffscreenContextWGL::create(size_t width, size_t height,
								 size_t majorGLVersion, size_t minorGLVersion, bool compatibilityProfile)
{
  auto ctx = std::make_shared<OffscreenContextWGLImpl>(width, height);

  WNDCLASSEX wndClass = {
    .cbSize = sizeof(WNDCLASSEX),
    .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
    .lpfnWndProc = &DefWindowProc,
    .hCursor = LoadCursor(0, IDC_ARROW),
    .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
    .lpszClassName = "WndClass"
  };
  RegisterClassEx(&wndClass);
  // style the window and remove the caption bar (WS_POPUP)
  DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
  // Create the window. Position and size it.
  ctx->window = CreateWindowEx(0, "WndClass", "", style,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, 0, 0);
  ctx->devContext = GetDC(ctx->window);

  PIXELFORMATDESCRIPTOR pfd = {
    .nSize = sizeof(PIXELFORMATDESCRIPTOR),
    .nVersion = 1,
    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 32,
    .cDepthBits = 16,
    .cStencilBits = 8,
    .iLayerType = PFD_MAIN_PLANE
  };
  int iPixelFormat = ChoosePixelFormat(ctx->devContext, &pfd);
  SetPixelFormat(ctx->devContext, iPixelFormat, &pfd);

  ctx->renderContext = wglCreateContext(ctx->devContext);
  if (ctx->renderContext == nullptr) {
    std::cerr << "wglCreateContext() failed: " << GetLastError() << std::endl;
    ReleaseDC(ctx->window, ctx->devContext);
    return nullptr;
  }
  return ctx;
}