#pragma once

#include <memory>
#include <string>

#include "OffscreenContext.h"

class OffscreenContextGLX : public OffscreenContext {
public:
  OffscreenContextGLX(int width, int height) : OffscreenContext(width, height) {}
  static std::shared_ptr<OffscreenContextGLX> create(
    size_t width, size_t height, size_t majorGLVersion,
    size_t minorGLVersion, bool gles, bool compatibilityProfile);
};
