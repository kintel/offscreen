#pragma once

#include <memory>

#include "OffscreenContext.h"

class OffscreenContextCGL : public OffscreenContext {
public:
  OffscreenContextCGL(int width, int height) : OffscreenContext(width, height) {}
  static std::shared_ptr<OffscreenContextCGL> create(size_t width, size_t height,
						     size_t majorGLVersion, size_t minorGLVersion);
};
