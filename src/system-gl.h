#pragma once

#include "glad/gl.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/glu.h>
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
