#pragma once

#include <cstddef>

#include "system-gl.h"

class FBO
{
 public:
  GLuint fbo_id;
  GLuint old_fbo_id;
  GLuint renderbuf_id;
  GLuint depthbuf_id;

  bool resize(size_t width, size_t height);
  GLuint bind();
  void unbind();
  void destroy();
};


FBO *createFBO(size_t width, size_t height);
