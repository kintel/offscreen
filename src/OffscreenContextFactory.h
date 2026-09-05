#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "OffscreenContext.h"

namespace OffscreenContextFactory {

struct ContextAttributes {
  uint32_t width;
  uint32_t height;
  unsigned int majorGLVersion;
  unsigned int minorGLVersion;
  bool gles;
  bool compatibilityProfile;
  std::string gpu;
  bool invisible;
};

const char *defaultProvider();
std::shared_ptr<OpenGLContext> create(const std::string& provider, const ContextAttributes& attrib);

}  // namespace OffscreenContextFactory

