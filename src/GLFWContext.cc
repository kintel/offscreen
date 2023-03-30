#include "GLFWContext.h"

#include <iostream>

#include "system-gl.h"
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

std::shared_ptr<GLFWContext> CreateGLFWContext(size_t width, size_t height,
					       size_t majorGLVersion, size_t minorGLVersion, bool invisible)
{
  if (!glfwInit()) {
    std::cerr << "glfwInit() failed" << std::endl;
    return nullptr;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorGLVersion);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorGLVersion);
  if (majorGLVersion >= 3) {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  }
  // Make window invisible for "offscreen" rendering
  if (invisible) {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  }

  GLFWwindow *window = glfwCreateWindow(width, height, "offscreen", NULL, NULL);
  if (!window) {
    std::cerr << "glfwCreateWindow() failed" << std::endl;
    glfwTerminate();
    return nullptr;
  }
  glfwSetKeyCallback(window, key_callback);

  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  auto context = std::make_shared<GLFWContext>(window, fbWidth, fbHeight);

  return context;
}

GLFWContext::GLFWContext(GLFWwindow* window, int width, int height) : OpenGLContext(width, height), window(window) {
}

void GLFWContext::loop(std::function<void()> render)
{
  while (!glfwWindowShouldClose(this->window)) {
    render();
    glfwSwapBuffers(this->window);
    glfwPollEvents();
  }
}
