cmake_minimum_required(VERSION 3.20)

project(offscreen)

add_executable(offscreen)

find_package(OpenGL REQUIRED)
if (OpenGL_EGL_FOUND)
  target_link_libraries(offscreen OpenGL::OpenGL OpenGL::EGL)
  set(HAS_EGL TRUE)
else()
  target_link_libraries(offscreen OpenGL::GL)
endif()
target_link_libraries(offscreen OpenGL::GLU)

find_package(glfw3 REQUIRED)
target_link_libraries(offscreen glfw)

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
  add_compile_definitions(-DHAS_EGL)
endif()

set(SRCS
    src/main.cc
    src/CommandLine.cc
    src/OpenGLContext.cc
    src/OffscreenContext.cc
    src/GLFWContext.cc
    src/FBO.cc
    ${SRCS_EGL}
    ${SRCS_APPLE}
    )

target_sources(offscreen PRIVATE ${SRCS})
set_property(TARGET offscreen PROPERTY CXX_STANDARD 17)
