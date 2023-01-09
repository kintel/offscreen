cmake_minimum_required(VERSION 3.20)

project(offscreen)

find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_egl_LIBRARY}")
find_package(glfw3 REQUIRED)

add_executable(offscreen)

if(APPLE)
  find_library(COCOA Cocoa)
  target_link_libraries(offscreen ${COCOA})
  set(SRCS_APPLE
      src/OffscreenContextNSOpenGL.mm
      src/OffscreenContextCGL.cc
      )
endif(APPLE)

set(SRCS_EGL
    src/OffscreenContextEGL.cc
)
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
target_link_libraries(offscreen glfw OpenGL::OpenGL OpenGL::EGL OpenGL::GLU)
set_property(TARGET offscreen PROPERTY CXX_STANDARD 17)
