#pragma once

#include <memory>
#include <string>

#include "OffscreenContext.h"

class OffscreenContextEGL : public OffscreenContext {
public:
  OffscreenContextEGL(int width, int height) : OffscreenContext(width, height) {}
  // FIXME: Query/enumerate GPUSs/devices?
  static void dumpEGLInfo(const std::string& drmNode);
  static std::shared_ptr<OffscreenContextEGL> create(
    size_t width, size_t height, size_t majorGLVersion, 
    size_t minorGLVersion, bool gles, bool compatibilityProfile,
    const std::string& drmNode = "");
};
