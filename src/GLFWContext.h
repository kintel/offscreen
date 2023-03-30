#pragma once

#include <memory>
#include <functional>
#include "system-gl.h"
#include <GLFW/glfw3.h>

#include "OpenGLContext.h"

class GLFWContext : public OpenGLContext {
public:
  GLFWwindow* window;
public:
  GLFWContext(GLFWwindow* window, int width, int height);

  bool isOffscreen() const override { return false; }

  bool makeCurrent() override {
    glfwMakeContextCurrent(this->window);
    return true;
  }

  void loop(std::function<void()> render);
};

std::shared_ptr<GLFWContext> CreateGLFWContext(size_t width, size_t height,
					       size_t majorGLVersion, size_t minorGLVersion, bool invisible);
