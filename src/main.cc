#include <iostream>
#include <locale>

#ifdef USE_GLAD
#define GLAD_GL_IMPLEMENTATION
#endif
#include "system-gl.h"

#include "CommandLine.h"
#ifdef __APPLE__
#include "OffscreenContextNSOpenGL.h"
#include "OffscreenContextCGL.h"
#endif
#ifdef HAS_EGL
#include "OffscreenContextEGL.h"
#endif
#include "GLFWContext.h"
#include "FBO.h"
#include "state.h"
#include "render_immediate.h"
#include "render_modern_ogl2.h"
#include "render_modern_ogl3.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <stdlib.h>
#include <string.h>
void * MyNSGLGetProcAddress(const char *name)
{
    NSSymbol symbol;
    char *symbolName = static_cast<char*>(malloc(strlen (name) + 2));
    strcpy(symbolName + 1, name);
    symbolName[0] = '_';
    symbol = NULL;
    if (NSIsSymbolNameDefined(symbolName)) {
      symbol = NSLookupAndBindSymbol(symbolName);
    }
    free(symbolName);
    return symbol ? NSAddressOfSymbol (symbol) : NULL;
}
#endif // __APPLE__

int main(int argc, char *argv[])
{
  std::srand(std::time(nullptr));

  uint32_t argWidth = 512;
  uint32_t argHeight = 512;
  std::string argGLVersion = "2.1";
  std::string argContextProvider = "NSOpenGL";
  std::string argProfile = "compatibility";
  bool argInvisible = false;
  std::string argRenderMode = "auto";
  bool argPrintHelp = false;

  // First configure all possible command line options.
  CommandLine args("OpenGL context tester.");
  args.addArgument({"--width"}, &argWidth, "Framebuffer width");
  args.addArgument({"--height"}, &argHeight, "Framebuffer height");
  args.addArgument({"--opengl"}, &argGLVersion, "OpenGL version");
  args.addArgument({"--context"}, &argContextProvider, "OpenGL context provider");
  args.addArgument({"--profile"}, &argProfile, "OpenGL profile [core | compatibility]");
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
#ifdef __APPLE__
  if (argContextProvider == "nsopengl") {
    ctx = OffscreenContextNSOpenGL::create(argWidth, argHeight, major, minor);
  }
  else if (argContextProvider == "cgl") {
    ctx = OffscreenContextCGL::create(argWidth, argHeight, major, minor);
  }
  else
#endif
#if HAS_EGL
  if (argContextProvider == "egl") {
    ctx = OffscreenContextEGL::create(argWidth, argHeight, major, minor, argProfile == "compatibility");
  }
  else
#endif
  if (argContextProvider == "glfw") {
    ctx = GLFWContext::create(argWidth, argHeight, major, minor, argInvisible);
  }
  else {
    std::cerr << "Unknown OpenGL context provider \"" << argContextProvider << "\"" << std::endl;
    return 1;
  }

  if (!ctx) {
    std::cerr << "Error: Unable to create GL context" << std::endl;
    return 1;
  }

  ctx->makeCurrent();

#ifdef USE_GLAD
  int version;
  if (argContextProvider == "glfw") {
    version = gladLoadGL(glfwGetProcAddress);
  }
  else {
    version = gladLoaderLoadGL();
  }
  if (version == 0) {
    std::cout << "GLAD: Failed to initialize OpenGL context" << std::endl;
    return 1;
  }
  printf("GLAD: Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
  printf("GLAD glFramebufferTexture: %p\n", glFramebufferTexture);
#endif

  // FIXME: Add flag to control verbosity or extension output
  if (major == 2) {
    const auto *extensions = glGetString(GL_EXTENSIONS);
    std::cout << extensions << std::endl;
  }
  else {
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for(auto i = 0; i < numExtensions; ++i) {
      std::cout << glGetStringi(GL_EXTENSIONS, i) << " ";
    }
    std::cout << std::endl;
  }

#ifdef __APPLE__
// FIXME: This can probably be removed: It was just some code to prove that MyNSGLGetProcAddress() returned the same function pointer as the OpenGL library itself provides.
  GL_CHECK();
  printf("NSLookupAndBindSymbol glFramebufferTexture: %p\n", MyNSGLGetProcAddress("glFramebufferTexture"));
  printf("OpenGL glFramebufferTexture: %p\n", glFramebufferTexture);
  GL_CHECK();
#endif

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
  if (argRenderMode == "immediate" && glMajor > 2 && argProfile != "compatibility") {
    std::cerr << "Error: OpenGL " << glVersion << " doesn't support immediate mode" << std::endl;
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

    if (argProfile == "compatibility" && !(profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)) {
      std::cerr << "Error: Requested compatibility profile, got context without compatibility profile" << std::endl;
      return 1;
    }

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


  std::vector<MyState> states;

  std::function<void()> setup;
  std::function<void()> render;
  if (argRenderMode == "immediate") {
    setup = [](){};
    render = renderImmediate;
  } else {
    std::string glslVersion = "120";
    if (glMajor >= 4 || glMajor == 3 && glMinor >= 3) {
      glslVersion = "330";
    } else if (glMajor == 3) {
      glslVersion = "140";
    }
    if (glMajor >= 3) {
      setup = [&states, &glslVersion]() {
	setupModernOGL3(states, glslVersion);
      };
    render = [&states]() {
      renderModernOGL3(states);
    };
    } else {
      setup = [&states, &glslVersion]() {
	setupModernOGL2(states, glslVersion);
      };
      render = [&states]() {
	renderModernOGL2(states);
      };
    }
  }

  GL_CHECK(setup());
  if (const auto glfwContext = std::dynamic_pointer_cast<GLFWContext>(ctx)) {
    glfwContext->loop(render);
  }
  else {
    GL_CHECK(render());
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
