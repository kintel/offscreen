#include "render_immediate.h"

#include <numbers>
#include <math.h>

#include "system-gl.h"

using std::numbers::pi;

namespace {

float colorWheelVertices[] = {
  0.0f, 0.0f, 0.0f, 1, 1, 1,
  0.8f * cosf(0*pi/180), 0.8f * sinf(0*pi/180), 0.0f, 1, 0, 0,
  0.8f * cosf(60*pi/180), 0.8f * sinf(60*pi/180), 0.0f, 1, 1, 0,
  0.8f * cosf(120*pi/180), 0.8f * sinf(120*pi/180), 0.0f, 0, 1, 0,
  0.8f * cosf(180*pi/180), 0.8f * sinf(180*pi/180), 0.0f, 0, 1, 1,
  0.8f * cosf(240*pi/180), 0.8f * sinf(240*pi/180), 0.0f, 0, 0, 1,
  0.8f * cosf(300*pi/180), 0.8f * sinf(300*pi/180), 0.0f, 1, 0, 1,
};

uint8_t colorWheelIndices[] = {
  0, 1, 2,
  0, 2, 3,
  0, 3, 4,
  0, 4, 5,
  0, 5, 6,
  0, 6, 1,
};

float centerVertices[] = {
  0.15f * cosf(45*pi/180), 0.15f * sinf(45*pi/180), 0.0f,
  0.15f * cosf(135*pi/180), 0.15f * sinf(135*pi/180), 0.0f,
  0.15f * cosf(225*pi/180), 0.15f * sinf(225*pi/180), 0.0f,
  0.15f * cosf(315*pi/180), 0.15f * sinf(315*pi/180), 0.0f,
  0.0f, 0.0f, 0.0f,
};

uint8_t centerIndices[] = {
  4, 0, 1,
  4, 1, 2,
  4, 2, 3,
  4, 3, 0,
};

} // namespace

void renderImmediate() {
  GL_CHECK(glClearColor(0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 1.0));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  glBegin(GL_TRIANGLES);
  for (int t=0;t<sizeof(colorWheelIndices)/3;++t) {
    for (int i=0;i<3;++i) {
      glColor3fv(colorWheelVertices + 6*colorWheelIndices[3*t+i] + 3);
      glVertex3fv(colorWheelVertices + 6*colorWheelIndices[3*t+i]);
    }
  }
  glEnd();
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_TRIANGLES);
  for (int t=0;t<sizeof(centerIndices)/3;++t) {
    for (int i=0;i<3;++i) {
      glVertex3fv(centerVertices + 3*centerIndices[3*t+i]);
    }
  }
  glEnd();
}
