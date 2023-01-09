#include "OffscreenContextCGL.h"

#include <iostream>

#include "system-gl.h"
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>

class OffscreenContextCGLImpl : public OffscreenContextCGL {

public:
  OffscreenContextCGLImpl(int width, int height) : OffscreenContextCGL(width, height) {}
  CGLContextObj cglContext = nullptr;

  bool makeCurrent() override {
    if (CGLSetCurrentContext(this->cglContext) != kCGLNoError) {
      std::cerr << "CGLSetCurrentContext() failed" << std::endl;
      return false;
    }
    return true;
  }
  bool destroy() override {
    return true;
  }
};

std::shared_ptr<OffscreenContextCGL> OffscreenContextCGL::create(size_t width, size_t height,
								 size_t majorGLVersion, size_t minorGLVersion)
{
  auto ctx = std::make_shared<OffscreenContextCGLImpl>(width, height);

  CGLOpenGLProfile glVersion = kCGLOGLPVersion_Legacy;
  if (majorGLVersion >= 4) glVersion = kCGLOGLPVersion_GL4_Core;
  else if (majorGLVersion >= 3) glVersion = kCGLOGLPVersion_GL3_Core;

  CGLPixelFormatAttribute attributes[13] = {
    kCGLPFAOpenGLProfile,
    (CGLPixelFormatAttribute)glVersion,
    kCGLPFAAccelerated,
    kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
    kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
    kCGLPFADoubleBuffer,
    kCGLPFASampleBuffers, (CGLPixelFormatAttribute)1,
    kCGLPFASamples,  (CGLPixelFormatAttribute)4,
    (CGLPixelFormatAttribute) 0
  };
  CGLPixelFormatObj pixelFormat = NULL;
  GLint numPixelFormats = 0;
  if (CGLChoosePixelFormat(attributes, &pixelFormat, &numPixelFormats) != kCGLNoError) {
    std::cerr << "CGLChoosePixelFormat() failed" << std::endl;
    return nullptr;
  }
  CGLCreateContext(pixelFormat, NULL, &ctx->cglContext);
  CGLDestroyPixelFormat(pixelFormat); // or CGLReleasePixelFormat()

  return ctx;
}
