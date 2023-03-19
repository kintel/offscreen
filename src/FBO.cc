#include "FBO.h"

#include <iostream>

#include "system-gl.h"

bool check_fbo_status()
{
  /* This code is based on user V-man code from
     http://www.opengl.org/wiki/GL_EXT_framebuffer_multisample
     See also: http://www.songho.ca/opengl/gl_fbo.html */
  GLenum status;
  auto result = false;
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (status == GL_FRAMEBUFFER_COMPLETE) {
    result = true;
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED\n";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n";
    //  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
    //    std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n";
    //  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_FORMATS) {
    //    std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS\n";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
    std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) {
    std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n";
  } else {
    std::cerr << "Unknown Code: glCheckFramebufferStatus() returned:" << status << "\n";
  }
  return result;
}

// bool hasFBO() {
//   if (GLES2 or 3+) {
//     return true;
//   }
//   return hasGLExtension("GL_ARB_framebuffer_object") || hasGLExtension("GL_EXT_framebuffer_object");
// }

FBO *createFBO(size_t width, size_t height) {

// FIXME: Check if FBO is supported:
// * built-in or ARB
// * EXT




  FBO *fbo = new FBO();
#ifdef DEBUG
  if (!glGenFramebuffers) std::cerr << "glGenFramebuffers() not found" << std::endl;
  if (!glBindFramebuffer) std::cerr << "glBindFramebuffer() not found" << std::endl;
  if (!glGenRenderbuffers) std::cerr << "glGenRenderbuffers() not found" << std::endl;
  if (!glFramebufferRenderbuffer) std::cerr << "glFramebufferRenderbuffer() not found" << std::endl;
  if (!glBindRenderbuffer) std::cerr << "glBindRenderbuffer() not found" << std::endl;
  if (!glRenderbufferStorage) std::cerr << "glRenderbufferStorage() not found" << std::endl;
#endif
  // Generate and bind FBO
  GL_CHECK(glGenFramebuffers(1, &fbo->fbo_id));
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo_id));

  // Generate depth and render buffers
  GL_CHECK(glGenRenderbuffers(1, &fbo->depthbuf_id));
  GL_CHECK(glGenRenderbuffers(1, &fbo->renderbuf_id));

  // Create buffers with correct size
  if (!fbo->resize(width, height)) return nullptr;

  // Attach render and depth buffers
  GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     GL_RENDERBUFFER, fbo->renderbuf_id));

  if (!check_fbo_status()) {
    std::cerr << "Problem with OpenGL framebuffer after specifying color render buffer.\n";
    return nullptr;
  }

  //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
  // to prevent Mesa's software renderer from crashing, do this in two stages.
  // ie. instead of using GL_DEPTH_STENCIL_ATTACHMENT, do DEPTH then STENCIL.
  GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				     GL_RENDERBUFFER, fbo->depthbuf_id));
  GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
				     GL_RENDERBUFFER, fbo->depthbuf_id));

  if (!check_fbo_status()) {
    std::cerr << "Problem with OpenGL framebuffer after specifying depth render buffer.\n";
    return nullptr;
  }

  return fbo;
}


bool FBO::resize(size_t width, size_t height)
{
  GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, this->renderbuf_id));
  GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height));
  GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, this->depthbuf_id));
  GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));

  return true;
}

GLuint FBO::bind()
{
 glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint *>(&this->old_fbo_id));
 glBindFramebuffer(GL_FRAMEBUFFER, this->fbo_id);
 return this->old_fbo_id;
}

void FBO::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, this->old_fbo_id);
}

void FBO::destroy()
{
  this->unbind();
  GL_CHECK(glDeleteRenderbuffers(1, &this->depthbuf_id));
  GL_CHECK(glDeleteRenderbuffers(1, &this->renderbuf_id));
  GL_CHECK(glDeleteFramebuffers(1, &this->fbo_id));
}
