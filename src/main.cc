#include <iostream>
#include <locale>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>

// FIXME: Disable check in non-debug mode
#define GL_CHECK(expr)						\
  expr;								\
  if (const auto err = glGetError(); err != GL_NO_ERROR) {	\
    std::cout << "OpenGL error: " << gluErrorString(err)	\
	      << " (" << err << ") in "				\
	      << __FILE__ << ":" << __LINE__ << "\n"		\
	      << "              " << #expr << "\n";		\
  }

#include "CommandLine.h"
#include "OffscreenContextNSOpenGL.h"
#include "OffscreenContextCGL.h"
#include "GLFWContext.h"
#include "FBO.h"


void render() {
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

int main(int argc, char *argv[])
{
  uint32_t argWidth = 640;
  uint32_t argHeight = 480;
  std::string argGLVersion = "2.1";
  std::string argContextProvider = "NSOpenGL";
  bool argInvisible = false;
  bool argPrintHelp = false;

  // First configure all possible command line options.
  CommandLine args("OpenGL context tester.");
  args.addArgument({"--width"}, &argWidth, "Framebuffer width");
  args.addArgument({"--height"}, &argHeight, "Framebuffer height");
  args.addArgument({"--opengl"}, &argGLVersion, "OpenGL version");
  args.addArgument({"--context"}, &argContextProvider, "OpenGL context provider");
  args.addArgument({"--invisible"}, &argInvisible, "Make window invisible");
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

  const char *glVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  int glMajor, glMinor;
  int numGlVersionElements = sscanf(glVersion, "%d.%d", &glMajor, &glMinor);
  if (numGlVersionElements != 2) {
    std::cerr << "Unable to parse OpenGL version \"" << glVersion << "\"" << std::endl;
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
