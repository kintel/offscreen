#pragma once

class OpenGLContext {
 protected:
  int width_;
  int height_;

 public:
 OpenGLContext(int width, int height) : width_(width), height_(height) {}
  int width() const { return this->width_; }
  int height() const { return this->height_; }
  virtual bool isOffscreen() const = 0;
  virtual bool makeCurrent() {return false;}
  //  bool saveFramebuffer(std::ostream &output);
  bool saveFramebuffer();
};
