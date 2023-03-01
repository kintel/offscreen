#pragma once

#include <memory>

#include "OffscreenContext.h"

class OffscreenContextEGL : public OffscreenContext {
public:
  OffscreenContextEGL(int width, int height) : OffscreenContext(width, height) {}
  // FIXME: Query/enumerate GPUSs/devices?
  static void dumpEGLInfo();
  // FIXME: Specify GPU?
  static std::shared_ptr<OffscreenContextEGL> create(
    size_t width, size_t height, size_t majorGLVersion, 
    size_t minorGLVersion, bool compatibilityProfile);
};
