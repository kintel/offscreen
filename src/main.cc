#include <iostream>
#include <locale>

#define GLAD_GL_IMPLEMENTATION
#include "system-gl.h"

#include "CommandLine.h"
#include "OffscreenContextNSOpenGL.h"
#include "OffscreenContextCGL.h"
#include "GLFWContext.h"
#include "FBO.h"

void renderImmediate() {
  GL_CHECK(glClearColor(0.8, 0.8, 0.8, 1.0));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  glBegin(GL_TRIANGLES);
  glColor3f(1, 0, 0);
  glVertex2f(-0.8, -0.8);
  glColor3f(0, 1, 0);
  glVertex2f(0.8, -0.8);
  glColor3f(0, 0, 1);
  glVertex2f(0, 0.9);
  glEnd();
}

struct MyState {
  GLuint shaderProgram;
  GLuint vao;
  GLuint ebo;
};

void setupModern(MyState &state) {
  float vertices[] = {
    -0.8f, -0.8f, 0.0f, 1, 0, 0,
    0.8f, -0.8f, 0.0f,  0, 1, 0,
    0.0f,  0.9f, 0.0f,  0, 0, 1,
  };


  GLuint vbo;
  glGenBuffers(1, &vbo);

  const char *vertexShaderSource = R"(#version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = aColor;
    }
  )";
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  int  success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  const char *fragmentShaderSource = R"(#version 330 core
    out vec4 FragColor;
    in vec3 ourColor;

    void main() {
      FragColor = vec4(ourColor, 1.0);
    }
  )";
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  state.shaderProgram = glCreateProgram();
  glAttachShader(state.shaderProgram, vertexShader);
  glAttachShader(state.shaderProgram, fragmentShader);
  glLinkProgram(state.shaderProgram);
  glGetProgramiv(state.shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(state.shaderProgram, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glUseProgram(state.shaderProgram);

  glGenVertexArrays(1, &state.vao);
  glBindVertexArray(state.vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  unsigned int indices[] = {
    0, 1, 2,
  };
  glGenBuffers(1, &state.ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


}

void renderModern(const MyState& state) {
  GL_CHECK(glClearColor(0.8, 0.6, 0.6, 1.0));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  glUseProgram(state.shaderProgram);
  //  glBindVertexArray(state.vao);
  //  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main(int argc, char *argv[])
{
  uint32_t argWidth = 640;
  uint32_t argHeight = 480;
  std::string argGLVersion = "2.1";
  std::string argContextProvider = "NSOpenGL";
  bool argInvisible = false;
  std::string argRenderMode = "auto";
  bool argPrintHelp = false;

  // First configure all possible command line options.
  CommandLine args("OpenGL context tester.");
  args.addArgument({"--width"}, &argWidth, "Framebuffer width");
  args.addArgument({"--height"}, &argHeight, "Framebuffer height");
  args.addArgument({"--opengl"}, &argGLVersion, "OpenGL version");
  args.addArgument({"--context"}, &argContextProvider, "OpenGL context provider");
  args.addArgument({"--invisible"}, &argInvisible, "Make window invisible");
  args.addArgument({"--mode"}, &argRenderMode, "Rendering mode [auto | immediate | modern]");
  args.addArgument({"-h", "--help"}, &argPrintHelp, "Print this help.");

  // Then do the actual parsing.
  try {
    args.parse(argc, argv);
  } catch (std::runtime_error const& e) {
    std::cout << e.what() << std::endl;
    return -1;
  }

  // When oPrintHelp was set to true, we print a help message and exit.
  if (argPrintHelp) {
    args.printHelp();
    return 0;
  }

  int major = 0;
  int minor = 0;
  int numVersionElements = sscanf(argGLVersion.c_str(), "%d.%d", &major, &minor);
  if (numVersionElements == 0) {
    std::cerr << "Error parsing GL version string \"" << argGLVersion << "\"" << std::endl;
    return 1;
  }

  std::cout << "Requesting context and framebuffer:\n";
  std::cout << "  Context provider: " << argContextProvider << "\n";
  std::cout << "  OpenGL: " << major << "." << minor << "\n";
  std::cout << "  Size: " << argWidth << " x " << argHeight << "\n";

  std::shared_ptr<OpenGLContext> ctx;
  std::transform(argContextProvider.begin(), argContextProvider.end(), argContextProvider.begin(), ::tolower);
  if (argContextProvider == "nsopengl") {
    ctx = OffscreenContextNSOpenGL::create(argWidth, argHeight, major, minor);
  }
  else if (argContextProvider == "cgl") {
    ctx = OffscreenContextCGL::create(argWidth, argHeight, major, minor);
  }
  else if (argContextProvider == "glfw") {
    ctx = GLFWContext::create(argWidth, argHeight, major, minor, argInvisible);
  }
  else {
    std::cerr << "Unknown OpenGL context provider \"" << argContextProvider << "\"" << std::endl;
    return 1;
  }

  ctx->makeCurrent();

  int version = gladLoaderLoadGL();
  if (version == 0) {
    std::cout << "GLAD: Failed to initialize OpenGL context" << std::endl;
    return 1;
  }
  printf("GLAD: Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

  const char *glVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  int glMajor, glMinor;
  int numGlVersionElements = sscanf(glVersion, "%d.%d", &glMajor, &glMinor);
  if (numGlVersionElements != 2) {
    std::cerr << "Unable to parse OpenGL version \"" << glVersion << "\"" << std::endl;
    return 1;
  }

  if (argRenderMode == "auto") {
    if (glMajor == 2) {
      argRenderMode = "immediate";
    }
    else {
      argRenderMode = "modern";
    }
  }
  if (argRenderMode == "immediate" && glMajor > 2) {
    // FIXME: ..unless a compatibility profile is used?
    std::cerr << "OpenGL " << glVersion << " doesn't support immediate mode" << std::endl;
    return 1;
  }

  std::cout << "Got context and framebuffer:\n";
  std::cout << "  OpenGL: " << glVersion << " (" << glGetString(GL_VENDOR) << ")" << std::endl;
  std::cout << "  renderer: " << glGetString(GL_RENDERER) << std::endl;

  if (glMajor > 3 || glMajor == 3 && glMinor >= 2) {
    GLint profile = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    std::string profileStr = (profile & GL_CONTEXT_CORE_PROFILE_BIT) ? "Core" : (profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) ? "Compatibility" : "No";
    std::cout << "  profile: " << profileStr << " profile" << std::endl;
    std::cout << "  flags: ";
    std::cout << ((flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) ? "Forward compatible" : "");
    std::cout << std::endl;
  }

  FBO *fbo = nullptr;
  if (ctx->isOffscreen()) {
    fbo = createFBO(ctx->width(), ctx->height());
  }

  glViewport(0, 0, ctx->width(), ctx->height());


  MyState state;

  std::function<void()> setup;
  std::function<void()> render;
  if (argRenderMode == "immediate") {
    setup = [](){};
    render = renderImmediate;
  } else {
    setup = [&state]() {
      setupModern(state);
    };
    render = [&state]() {
      renderModern(state);
    };
  }

  setup();
  if (const auto glfwContext = std::dynamic_pointer_cast<GLFWContext>(ctx)) {
    glfwContext->loop(render);
  }
  else {
    render();
  }

  glFinish();
  ctx->saveFramebuffer();

  if (fbo) {
    fbo->destroy();
    delete fbo;
  }
  //ctx->destroy();

  //  glfwTerminate();

  // FIXME: Resize buffer
  //glfwGetFramebufferSize(window, &width, &height);

  return 0;
}
