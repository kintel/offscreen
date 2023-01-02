#pragma once

#include <memory>

#include "OffscreenContext.h"

class OffscreenContextNSOpenGL : public OffscreenContext {
public:
  OffscreenContextNSOpenGL(int width, int height) : OffscreenContext(width, height) {}
  static std::shared_ptr<OffscreenContextNSOpenGL> create(size_t width, size_t height,
							  size_t majorGLVersion, size_t minorGLVersion);
};
