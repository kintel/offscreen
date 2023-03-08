#pragma once

#include <memory>
#include <string>

#include "OffscreenContext.h"

class OffscreenContextWGL : public OffscreenContext {
public:
  OffscreenContextWGL(int width, int height) : OffscreenContext(width, height) {}
  static std::shared_ptr<OffscreenContextWGL> create(
    size_t width, size_t height, size_t majorGLVersion, 
    size_t minorGLVersion, bool compatibilityProfile);
};
