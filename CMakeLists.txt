cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(offscreen)

add_executable(offscreen)
target_compile_options(offscreen PRIVATE "$<$<CONFIG:DEBUG>:-DDEBUG>")

if(APPLE)
  set(USE_GLAD_DEFAULT OFF)
else()
  set(USE_GLAD_DEFAULT ON)
endif()

option(USE_GLAD "Use GLAD for OpenGL function wrangling" ${USE_GLAD_DEFAULT})
option(ENABLE_GLFW "Enable on-screen rendering using glfw" ON)

set(HAS_GLVND FALSE)
set(HAS_GLX FALSE)
set(HAS_EGL FALSE)
set(HAS_CGL FALSE)
set(HAS_NSOPENGL FALSE)
set(HAS_WGL FALSE)

if(USE_GLAD)
  target_compile_definitions(offscreen PRIVATE USE_GLAD)
endif()

find_package(OpenGL REQUIRED)
if(TARGET OpenGL::OpenGL)
  # GLVND systems
  target_link_libraries(offscreen OpenGL::OpenGL)
  set(HAS_GLVND TRUE)
else()
  # Non-GLVND systems
  target_link_libraries(offscreen OpenGL::GL)
endif()
target_link_libraries(offscreen OpenGL::GLU)
if (OpenGL_EGL_FOUND)
  if (TARGET OpenGL::EGL)
    # GLVND systems
    target_link_libraries(offscreen OpenGL::EGL)
  else()
    # Non-GLVND systems
    target_link_libraries(offscreen ${OPENGL_egl_LIBRARY})
  endif()
  set(HAS_EGL TRUE)

  find_package(GBM)
  if (GBM_FOUND)
    target_compile_definitions(offscreen PRIVATE HAS_GBM)
    target_link_libraries(offscreen GBM::GBM)
  endif()
endif()
if(OpenGL_GLX_FOUND)
  find_package(X11 REQUIRED)
  if(X11_FOUND)
    if(TARGET OpenGL::OpenGL)
      # For GLVND systems (non-GLVND systems have GLX included in the OpenGL libraries)
      target_link_libraries(offscreen OpenGL::GLX)
    endif()
    set(HAS_GLX TRUE)
    target_link_libraries(offscreen X11::X11)
    target_compile_definitions(offscreen PRIVATE ENABLE_GLX)
  endif()
endif()

if(ENABLE_GLFW)
  find_package(glfw3 REQUIRED)
  target_link_libraries(offscreen glfw)
  target_compile_definitions(offscreen PRIVATE ENABLE_GLFW)
endif()

# Needed for Raspberry pi:
target_link_libraries(offscreen dl)

if(APPLE)
  set(HAS_NSOPENGL TRUE)
  set(HAS_CGL TRUE)
  find_library(COCOA Cocoa)
  target_link_libraries(offscreen ${COCOA})
  set(SRCS_APPLE
      src/OffscreenContextNSOpenGL.mm
      src/OffscreenContextCGL.cc
      )
endif(APPLE)

if(WIN32)
  set(HAS_WGL TRUE)
  set(SRCS_WINDOWS src/OffscreenContextWGL.cc)
endif(WIN32)

if(HAS_EGL)
  set(SRCS_EGL
      src/OffscreenContextEGL.cc
      src/egl_utils.cc
      )
  add_compile_definitions(HAS_EGL)
endif()

if(HAS_GLX)
  set(SRCS_GLX
      src/OffscreenContextGLX.cc
      )
  add_compile_definitions(HAS_GLX)
endif()

if(ENABLE_GLFW)
  set(SRCS_GLFW src/GLFWContext.cc)
endif()

set(SRCS
    src/main.cc
    src/CommandLine.cc
    src/system-gl.cc
    src/OpenGLContext.cc
    src/OffscreenContext.cc
    src/OffscreenContextFactory.cc
    src/FBO.cc
    src/render_immediate.cc
    src/render_modern_ogl2.cc
    src/render_modern_ogl3.cc
    ${SRCS_GLFW}
    ${SRCS_EGL}
    ${SRCS_GLX}
    ${SRCS_APPLE}
    ${SRCS_WINDOWS}
    )
target_sources(offscreen PRIVATE ${SRCS})
target_include_directories(offscreen PUBLIC "${CMAKE_SOURCE_DIR}/src")
# Using C++17 as my RaspberryPi runs gcc=8.3 which doesn't implement <numbers>
set_property(TARGET offscreen PROPERTY CXX_STANDARD 17)

enable_testing()
add_test(NAME default_run COMMAND offscreen)
add_test(NAME fails_on_empty_context_arg COMMAND offscreen --context)
set_property(TEST fails_on_empty_context_arg PROPERTY WILL_FAIL true)

add_test(NAME will_save_framebuffer COMMAND offscreen -o out.png)
add_test(NAME check_file_exists COMMAND ${CMAKE_COMMAND} -E cat out.png)
set_tests_properties(check_file_exists PROPERTIES DEPENDS will_save_framebuffer)

if(APPLE)
add_test(NAME cgl_opengl2_immediate COMMAND offscreen --context cgl --opengl 2 --mode immediate)
add_test(NAME cgl_opengl2_modern COMMAND offscreen --context cgl --opengl 2 --mode modern)
add_test(NAME cgl_opengl3.2_core COMMAND offscreen --context cgl --opengl 3.2 --profile core)
add_test(NAME nsopengl_opengl2_immediate COMMAND offscreen --context nsopengl --opengl 2 --mode immediate)
add_test(NAME nsopengl_opengl2_modern COMMAND offscreen --context nsopengl --opengl 2 --mode modern)
add_test(NAME nsopengl_opengl3.2_core COMMAND offscreen --context nsopengl --opengl 3.2 --profile core)
endif(APPLE)

message(STATUS " ")
message(STATUS "====================================")
message(STATUS "Build Configuration Summary")
message(STATUS "====================================")
message(STATUS " ")
message(STATUS "GLAD:                ${USE_GLAD}")
message(STATUS "GLFW:                ${ENABLE_GLFW}")
message(STATUS "Has GLVND:           ${HAS_GLVND}")
message(STATUS "GLX:                 ${HAS_GLX}")
message(STATUS "EGL:                 ${HAS_EGL}")
message(STATUS "CGL:                 ${HAS_CGL}")
message(STATUS "NSOpenGL:            ${HAS_NSOPENGL}")
message(STATUS "WGL:                 ${HAS_WGL}")
