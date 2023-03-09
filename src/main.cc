#include <iostream>
#include <locale>

#ifdef USE_GLAD
#define GLAD_GL_IMPLEMENTATION
#define GLAD_EGL_IMPLEMENTATION
#endif
#include "system-gl.h"

#include "CommandLine.h"
#ifdef __APPLE__
#include "OffscreenContextNSOpenGL.h"
#include "OffscreenContextCGL.h"
#endif
#ifdef _WIN32
#include "OffscreenContextWGL.h"
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ext/stb/stb_image_write.h"

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

bool saveFramebuffer(const OpenGLContext& ctx, const char *filename)
{
  const auto buffer = ctx.getFramebuffer();
  std::cout << "Buffer size: " << buffer.size() << std::endl;
  stbi_flip_vertically_on_write(true);
  int samplesPerPixel = 4; // R, G, B and A
  if (stbi_write_png(filename, ctx.width(), ctx.height(), samplesPerPixel, buffer.data(), 0) != 1) {
    std::cerr << "stbi_write_png(\"" << filename << "\") failed" << std::endl;
    return false;
  }

  return true;
}


int main(int argc, char *argv[])
{
  std::srand(std::time(nullptr));

  uint32_t argWidth = 512;
  uint32_t argHeight = 512;
  std::string argGLVersion = "";
  std::string argGLESVersion = "";
  std::string argContextProvider;
  std::string argProfile = "compatibility";
  bool argInvisible = false;
  std::string argRenderMode = "auto";
  std::string argGPU = "";
  bool argVerbose = false;
  bool argPrintHelp = false;

  // First configure all possible command line options.
  CommandLine args("OpenGL context tester.");
  args.addArgument({"--width"}, &argWidth, "Framebuffer width");
  args.addArgument({"--height"}, &argHeight, "Framebuffer height");
  args.addArgument({"--opengl"}, &argGLVersion, "OpenGL version");
  args.addArgument({"--gles"}, &argGLESVersion, "OpenGL ES version");
  args.addArgument({"--context"}, &argContextProvider, "OpenGL context provider [glfw | egl | cgl | nsopengl | wgl]");
  args.addArgument({"--profile"}, &argProfile, "OpenGL profile [core | compatibility]");
  args.addArgument({"--invisible"}, &argInvisible, "Make window invisible");
  args.addArgument({"--mode"}, &argRenderMode, "Rendering mode [auto | immediate | modern]");
  args.addArgument({"--gpu"}, &argGPU, "[EGL] Which GPU to use (e.g. /dev/dri/renderD128)");
  args.addArgument({"-v", "--verbose"}, &argVerbose, "Verbose output.");
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

  std::string requestVersion;
  if (!argGLVersion.empty() && !argGLESVersion.empty()) {
    std::cerr << "Only one of --opengl or --gles can be specified" << std::endl;
    return 1;
  } else if (!argGLVersion.empty()) {
    requestVersion = argGLVersion;
  } else if (!argGLESVersion.empty()) {
    requestVersion = argGLESVersion;
  } else {
    argGLVersion = "2.1"; // default to OpenGL 2.1
    requestVersion = argGLVersion;
  }
  bool requestGLES = !argGLESVersion.empty();

  int requestMajor = 0;
  int requestMinor = 0;
  int numVersionElements = sscanf(requestVersion.c_str(), "%d.%d", &requestMajor, &requestMinor);
  if (numVersionElements == 0) {
    std::cerr << "Error parsing requestec GL version string \"" << requestVersion << "\"" << std::endl;
    return 1;
  }

  if (argContextProvider.empty()) {
#ifdef __APPLE__
    argContextProvider = "cgl";
#elif defined(HAS_EGL)
    argContextProvider = "egl";
#elif defined(_WIN32)
    argContextProvider = "wgl";
#endif
  }

#if HAS_EGL
  if (argVerbose && argContextProvider == "egl") {
    OffscreenContextEGL::dumpEGLInfo(argGPU);
  }
#endif

  std::cout << "================:\n";
  std::cout << "Requesting context and framebuffer:\n";
  std::cout << "  Context provider: " << argContextProvider << "\n";
  std::cout << "  " << (requestGLES ? "GLES" : "OpenGL") << ": " << requestMajor << "." << requestMinor << "\n";
  std::cout << "  Size: " << argWidth << " x " << argHeight << "\n";

  std::shared_ptr<OpenGLContext> ctx;

  std::transform(argContextProvider.begin(), argContextProvider.end(), argContextProvider.begin(), ::tolower);
#ifdef __APPLE__
  if (argContextProvider == "nsopengl") {
    ctx = OffscreenContextNSOpenGL::create(argWidth, argHeight, requestMajor, requestMinor);
  }
  else if (argContextProvider == "cgl") {
    ctx = OffscreenContextCGL::create(argWidth, argHeight, requestMajor, requestMinor);
  }
  else
#endif
#if HAS_EGL
  if (argContextProvider == "egl") {
    ctx = OffscreenContextEGL::create(argWidth, argHeight, requestMajor, requestMinor, requestGLES, argProfile == "compatibility",
    argGPU);
  }
  else
#endif
#ifdef _WIN32
  if (argContextProvider == "wgl") {
    ctx = OffscreenContextWGL::create(argWidth, argHeight, requestMajor, requestMinor, argProfile == "compatibility");
  }
  else
#endif
  if (argContextProvider == "glfw") {
    ctx = GLFWContext::create(argWidth, argHeight, requestMajor, requestMinor, argInvisible);
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
    version = requestGLES ? gladLoadGLES2(glfwGetProcAddress) : gladLoadGL(glfwGetProcAddress);
//  } else if (argContextProvider == "egl") {
//  std::cout << "CCC: " << eglGetProcAddress << std::endl;
//    version = requestGLES ? gladLoadGLES2(eglGetProcAddress) : gladLoadGL(eglGetProcAddress);
//  std::cout << "DDD" << std::endl;
  } else {
    version = requestGLES ? gladLoaderLoadGLES2() : gladLoaderLoadGL();
  }
  if (version == 0) {
    std::cout << "GLAD: Failed to initialize " << (requestGLES ? "GLES" : "OpenGL") << " context" << std::endl;
    return 1;
  }
  printf("GLAD: Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
  printf("GLAD glFramebufferTexture: %p\n", glFramebufferTexture);
#endif

  if (argVerbose) {
    if (requestMajor == 2) {
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
  }

#ifdef __APPLE__
// FIXME: This can probably be removed: It was just some code to prove that MyNSGLGetProcAddress() returned the same function pointer as the OpenGL library itself provides.
  GL_CHECK();
  printf("NSLookupAndBindSymbol glFramebufferTexture: %p\n", MyNSGLGetProcAddress("glFramebufferTexture"));
  printf("OpenGL glFramebufferTexture: %p\n", glFramebufferTexture);
  GL_CHECK();
#endif

  std::string glVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  if (!argGLESVersion.empty()) {
    // FIXME: What about "OpenGL ES-CM 1.1 Mesa 22.2.5" ?
    if (glVersion.rfind("OpenGL ES ", 0) != 0) {
      std::cerr << "Unexpected GL_VERSION '" << glVersion << "' for OpenGL ES" << std::endl;
      return 1;
    }
    glVersion = glVersion.substr(10);
  }

  std::cout << "glVersion: " << glVersion << std::endl;
  int glMajor, glMinor;
  int numGlVersionElements = sscanf(glVersion.c_str(), "%d.%d", &glMajor, &glMinor);
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
  std::cout << "  " << (argGLVersion.empty() ? "GLES" : "OpenGL") << ": " << glVersion << " (" << glGetString(GL_VENDOR) << ")" << std::endl;
  std::cout << "  renderer: " << glGetString(GL_RENDERER) << std::endl;

  if (!requestGLES && (glMajor > 3 || glMajor == 3 && glMinor >= 2)) {
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
    if (!requestGLES) {
      if (glMajor >= 4 || glMajor == 3 && glMinor >= 3) {
        glslVersion = "330";
      } else if (glMajor == 3) {
        glslVersion = "140";
      }
    } else {
      if (glMajor >= 3) {
        glslVersion = "300 es";
      } else if (glMajor == 2) {
        glslVersion = "100 es";
      }
    }
    if (requestGLES || glMajor >= 3) {
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
  if (!saveFramebuffer(*ctx, "out.png")) {
    std::cerr << "Unable to write framebuffer" << std::endl;
  }

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
