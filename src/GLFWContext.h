#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <functional>
#include "system-gl.h"
#include <GLFW/glfw3.h>

#include "OpenGLContext.h"

class GLFWContext : public OpenGLContext {
public:
  GLFWwindow* window;

  GLFWContext(GLFWwindow* window, uint32_t width, uint32_t height);
  ~GLFWContext() override {
    if (this->window) {
      glfwDestroyWindow(this->window);
    }
  }

  bool isOffscreen() const override { return false; }

  std::string getInfo() const override {
    return "GL context creator: GLFW\n";
  }

  bool makeCurrent() const override {
    glfwMakeContextCurrent(this->window);
    return true;
  }

  void loop(std::function<void()> render);
};

std::shared_ptr<GLFWContext> CreateGLFWContext(size_t width, size_t height,
                                               size_t majorGLVersion, size_t minorGLVersion, bool invisible);

