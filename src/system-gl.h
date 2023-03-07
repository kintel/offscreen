#pragma once

#include <iostream>

#ifdef USE_GLAD
#define GLAD_GLES2
#include "glad/gl.h"
#endif

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/glu.h>
#include <OpenGL/gl3.h>
#else
#include <GL/glu.h>
#endif

// FIXME: Disable check in non-debug mode
#define GL_CHECK(expr)						\
  expr;								\
  if (const auto err = glGetError(); err != GL_NO_ERROR) {	\
    std::cout << "OpenGL error: " << gluErrorString(err)	\
	      << " (" << err << ") in "				\
	      << __FILE__ << ":" << __LINE__ << "\n"		\
	      << "              " << #expr << "\n";		\
  }
