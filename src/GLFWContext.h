#pragma once

#include <memory>
#include <functional>
#include <GLFW/glfw3.h>

#include "OpenGLContext.h"

class GLFWContext : public OpenGLContext {
public:
  GLFWwindow* window;
public:
  static std::shared_ptr<GLFWContext> create(size_t width, size_t height,
					     size_t majorGLVersion, size_t minorGLVersion, bool invisible);
  GLFWContext(GLFWwindow* window, int width, int height);

  bool isOffscreen() const override { return false; }

  bool makeCurrent() override {
    glfwMakeContextCurrent(this->window);
    return true;
  }

  void loop(std::function<void()> render);
};
