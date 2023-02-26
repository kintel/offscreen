#pragma once

#include "system-gl.h"

struct MyState {
  GLuint shaderProgram;
  GLuint vao;
  GLuint ebo;
  int numTris;
};
