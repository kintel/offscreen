#include "render_immediate.h"

#include "system-gl.h"

void renderImmediate() {
  GL_CHECK(glClearColor(0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 1.0));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  glBegin(GL_TRIANGLES);
  glColor3f(1, 0, 0);
  glVertex2f(-0.8, -0.8);
  glColor3f(0, 1, 0);
  glVertex2f(0.8, -0.8);
  glColor3f(0, 0, 1);
  glVertex2f(0, 0.9);
  glEnd();
}
