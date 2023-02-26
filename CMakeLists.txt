cmake_minimum_required(VERSION 3.16)

project(offscreen)

add_executable(offscreen)

option(USE_GLAD "Use GLAD for OpenGL function wrangling")
if(USE_GLAD)
  target_compile_definitions(offscreen PRIVATE USE_GLAD)
endif()

find_package(OpenGL REQUIRED)
if (OpenGL_EGL_FOUND)
  if (TARGET OpenGL::EGL)
    target_link_libraries(offscreen OpenGL::OpenGL OpenGL::EGL)
  else()
    # Some systems (e.g. Raspberry Pi) seem to not produce the OpenGL::* targets
    target_link_libraries(offscreen ${OPENGL_LIBRARIES} ${OPENGL_egl_LIBRARY})
  endif()
  set(HAS_EGL TRUE)
else()
  target_link_libraries(offscreen OpenGL::GL)
endif()
target_link_libraries(offscreen OpenGL::GLU)

find_package(glfw3 REQUIRED)
target_link_libraries(offscreen glfw)

# Needed for Raspberry pi:
target_link_libraries(offscreen dl)

if(APPLE)
  find_library(COCOA Cocoa)
  target_link_libraries(offscreen ${COCOA})
  set(SRCS_APPLE
      src/OffscreenContextNSOpenGL.mm
      src/OffscreenContextCGL.cc
      )
endif(APPLE)

if(HAS_EGL)
  set(SRCS_EGL
      src/OffscreenContextEGL.cc
      )
  add_compile_definitions(HAS_EGL)
endif()

set(SRCS
    src/main.cc
    src/CommandLine.cc
    src/OpenGLContext.cc
    src/OffscreenContext.cc
    src/GLFWContext.cc
    src/FBO.cc
    src/render_immediate.cc
    src/render_modern_ogl2.cc
    src/render_modern_ogl3.cc
    ${SRCS_EGL}
    ${SRCS_APPLE}
    )

target_sources(offscreen PRIVATE ${SRCS})
set_property(TARGET offscreen PROPERTY CXX_STANDARD 20)

message(STATUS " ")
message(STATUS "====================================")
message(STATUS "Build Configuration Summary")
message(STATUS "====================================")
message(STATUS " ")
message(STATUS "Using GLAD:          ${USE_GLAD}")
