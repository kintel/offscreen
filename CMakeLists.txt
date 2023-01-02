cmake_minimum_required(VERSION 3.20)

project(offscreen)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(SRCS
    src/main.cc
    src/CommandLine.cc
    src/OpenGLContext.cc
    src/OffscreenContext.cc
    src/OffscreenContextNSOpenGL.mm
    src/OffscreenContextCGL.cc
    src/GLFWContext.cc
    src/FBO.cc
    )

add_executable(offscreen ${SRCS})
target_link_libraries(offscreen glfw OpenGL::GL "-framework Cocoa")
set_property(TARGET offscreen PROPERTY CXX_STANDARD 17)
