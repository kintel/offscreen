#include "OffscreenContextNSOpenGL.h"

#include <iostream>

#include <OpenGL/OpenGL.h>

#import <AppKit/AppKit.h>

class OffscreenContextNSOpenGLImpl : public OffscreenContextNSOpenGL {

public:
 OffscreenContextNSOpenGLImpl(int width, int height) : OffscreenContextNSOpenGL(width, height) { }
  NSOpenGLContext *openGLContext;
  NSAutoreleasePool *pool;

  bool makeCurrent() override {
    [this->openGLContext makeCurrentContext];
    return true;
  }
  bool destroy() override {
    [this->pool release];
    return true;
  }
};

std::shared_ptr<OffscreenContextNSOpenGL> OffscreenContextNSOpenGL::create(size_t width, size_t height,
									   size_t majorGLVersion, size_t minorGLVersion)
{
  auto ctx = std::make_shared<OffscreenContextNSOpenGLImpl>(width, height);

  ctx->pool = [NSAutoreleasePool new];

  // Create an OpenGL context just so that OpenGL calls will work.
  // Will not be used for actual rendering.

  NSOpenGLPixelFormatAttribute glVersion = NSOpenGLProfileVersionLegacy;
  if (majorGLVersion >= 4) glVersion = NSOpenGLProfileVersion4_1Core;
  else if (majorGLVersion >= 3) glVersion = NSOpenGLProfileVersion3_2Core;

  NSOpenGLPixelFormatAttribute attributes[] = {
    NSOpenGLPFAOpenGLProfile, glVersion,
    NSOpenGLPFANoRecovery,
    NSOpenGLPFADepthSize, 24,
    NSOpenGLPFAStencilSize, 8,
// Enable this to force software rendering
// NSOpenGLPFARendererID, kCGLRendererGenericID,
// Took out the acceleration requirement to be able to run the tests
// in a non-accelerated VM.
// NSOpenGLPFAAccelerated,
    (NSOpenGLPixelFormatAttribute) 0
  };
  NSOpenGLPixelFormat *pixFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];

  // Create and make current the OpenGL context to render with (with color and depth buffers)
  ctx->openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixFormat shareContext:nil];
  if (!ctx->openGLContext) {
    std::cerr << "Unable to create NSOpenGLContext" << std::endl;
    [ctx->pool release];
    return nullptr;
  }

  return ctx;
}
