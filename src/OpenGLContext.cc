#include "OpenGLContext.h"

#include <iostream>

#include "system-gl.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ext/stb/stb_image_write.h"

//bool OpenGLContext::saveFramebuffer(std::ostream &output)
bool OpenGLContext::saveFramebuffer(const char *filename)
{
  // Read pixels from OpenGL
  int samplesPerPixel = 4; // R, G, B and A
  int rowBytes = samplesPerPixel * this->width_;
  unsigned char *bufferData = (unsigned char *)malloc(rowBytes * this->height_);
  if (!bufferData) {
    std::cerr << "Unable to allocate buffer for image extraction.";
    return 1;
  }
  GL_CHECK(glReadPixels(0, 0, this->width_, this->height_, GL_RGBA, GL_UNSIGNED_BYTE, bufferData));

  stbi_flip_vertically_on_write(true);
  if (stbi_write_png(filename, this->width_, this->height_, samplesPerPixel, bufferData, 0) != 1) {
    std::cerr << "stbi_write_png(\"" << filename << "\") failed" << std::endl;
    return false;
  }
  free(bufferData);

  return true;
}
