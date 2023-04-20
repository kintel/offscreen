#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

class OpenGLContext {
 protected:
  int width_;
  int height_;
  int major_;
  int minor_;
  int gles_;

 public:
  OpenGLContext(int width, int height) : width_(width), height_(height) {}
  void setVersion(int major, int minor, bool gles) {
    this->major_ = major;
    this->minor_ = minor;
    this->gles_ = gles;
  }
  int width() const { return this->width_; }
  int height() const { return this->height_; }
  int majorVersion() const { return this->major_; }
  int minorVersion() const { return this->minor_; }
  bool isGLES() const { return this->gles_; }
  virtual bool isOffscreen() const = 0;
  virtual bool makeCurrent() {return false;}
  std::vector<uint8_t> getFramebuffer() const;
};
