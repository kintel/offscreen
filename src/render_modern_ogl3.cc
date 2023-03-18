#include "render_modern_ogl3.h"

#include <math.h>

#include "state.h"

namespace {

constexpr double pi = 3.14159265358979323846;

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

const char *perVertexColor_vert_330 = R"(#version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = aColor;
    }
  )";

const char *perVertexColor_vert_300_es = R"(#version 300 es
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = aColor;
    }
  )";

const char *default_vert_330 = R"(#version 330 core
    layout (location = 0) in vec3 aPos;

    out vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = vec3(1,1,1);
    }
  )";

const char *default_vert_300_es = R"(#version 300 es
    layout (location = 0) in vec3 aPos;

    out vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = vec3(1,1,1);
    }
  )";

const char *default_frag_330 = R"(#version 330 core
    out vec4 FragColor;
    in vec3 ourColor;

    void main() {
      FragColor = vec4(ourColor, 1.0);
    }
  )";

const char *default_frag_300_es = R"(#version 300 es
   precision mediump float;
   out vec4 FragColor;
    in vec3 ourColor;

    void main() {
      FragColor = vec4(ourColor, 1.0);
    }
  )";


const char *perVertexColor_vert_140 = R"(#version 140
    in vec3 aPos;
    in vec3 aColor;

    out vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = aColor;
    }
  )";

const char *default_vert_140 = R"(#version 140
    in vec3 aPos;

    out vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = vec3(1,1,1);
    }
  )";

const char *default_frag_140 = R"(#version 140
    out vec4 FragColor;
    in vec3 ourColor;

    void main() {
      FragColor = vec4(ourColor, 1.0);
    }
  )";

void setupColorWheel(MyState &state, const std::string &glslVersion) {
  const char *vertexShaderSource;
  if (glslVersion == "330") {
    vertexShaderSource = perVertexColor_vert_330;
  } else if (glslVersion == "140") {
    vertexShaderSource = perVertexColor_vert_140;
  } else if (glslVersion == "300 es") {
    vertexShaderSource = perVertexColor_vert_300_es;
  } else {
    std::cerr << "GLSL " << glslVersion << " shaders not implemented" << std::endl;
    return;
  }
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  GLint success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  const char *fragmentShaderSource;
  if (glslVersion == "330") {
    fragmentShaderSource = default_frag_330;
  } else if (glslVersion == "140") {
    fragmentShaderSource = default_frag_140;
  } else if (glslVersion == "300 es") {
    fragmentShaderSource = default_frag_300_es;
  } else {
    std::cerr << "GLSL " << glslVersion << " shaders not implemented" << std::endl;
    return;
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  state.shaderProgram = glCreateProgram();
  glAttachShader(state.shaderProgram, vertexShader);
  glAttachShader(state.shaderProgram, fragmentShader);
  GL_CHECK();
  if (glslVersion != "330") {
    glBindAttribLocation(state.shaderProgram, 0, "aPos");
    glBindAttribLocation(state.shaderProgram, 1, "aColor");
  }
  GL_CHECK();
  glLinkProgram(state.shaderProgram);
  glGetProgramiv(state.shaderProgram, GL_LINK_STATUS, &success);
  if (success != GL_TRUE) {
    glGetProgramInfoLog(state.shaderProgram, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GL_CHECK();
  glUseProgram(state.shaderProgram);
  GL_CHECK(glGenVertexArrays(1, &state.vao));
  GL_CHECK(glBindVertexArray(state.vao));

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colorWheelVertices), colorWheelVertices, GL_STATIC_DRAW);
  GL_CHECK();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  GL_CHECK();
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(colorWheelIndices), colorWheelIndices, GL_STATIC_DRAW);
  GL_CHECK();
  state.numTris = sizeof(colorWheelIndices);
}

void setupCenter(MyState &state, const std::string &glslVersion) {
  const char *vertexShaderSource;
  if (glslVersion == "330") {
    vertexShaderSource = default_vert_330;
  } else if (glslVersion == "140") {
    vertexShaderSource = default_vert_140;
  } else if (glslVersion == "300 es") {
    vertexShaderSource = default_vert_300_es;
  } else {
    std::cerr << "GLSL " << glslVersion << " shaders not implemented" << std::endl;
    return;
  }
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  GLint success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  const char *fragmentShaderSource;
  if (glslVersion == "330") {
    fragmentShaderSource = default_frag_330;
  } else if (glslVersion == "140") {
    fragmentShaderSource = default_frag_140;
  } else if (glslVersion == "300 es") {
    fragmentShaderSource = default_frag_300_es;
  } else {
    std::cerr << "GLSL " << glslVersion << " shaders not implemented" << std::endl;
    return;
  }
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  state.shaderProgram = glCreateProgram();
  glAttachShader(state.shaderProgram, vertexShader);
  glAttachShader(state.shaderProgram, fragmentShader);
  GL_CHECK();
  if (glslVersion != "330") {
    glBindAttribLocation(state.shaderProgram, 0, "aPos");
    glBindAttribLocation(state.shaderProgram, 1, "aColor");
  }
  GL_CHECK();
  glLinkProgram(state.shaderProgram);
  glGetProgramiv(state.shaderProgram, GL_LINK_STATUS, &success);
  if (success != GL_TRUE) {
    glGetProgramInfoLog(state.shaderProgram, 512, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GL_CHECK();
  glUseProgram(state.shaderProgram);
  GL_CHECK(glGenVertexArrays(1, &state.vao));
  GL_CHECK(glBindVertexArray(state.vao));

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(centerVertices), centerVertices, GL_STATIC_DRAW);
  GL_CHECK();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(centerIndices), centerIndices, GL_STATIC_DRAW);
  GL_CHECK();
  state.numTris = sizeof(colorWheelIndices);
}

} // namespace

void setupModernOGL3(std::vector<MyState> &states, const std::string &glslVersion) {
  std::cout << "Rendering using modern OpenGL 3+" << std::endl;
  std::cout << "Using GLSL " << glslVersion << std::endl;
  states.emplace_back();
  setupColorWheel(states.back(), glslVersion);
  states.emplace_back();
  setupCenter(states.back(), glslVersion);
}

void renderModernOGL3(const std::vector<MyState>& states) {
  GL_CHECK(glClearColor(0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 1.0));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  for (const auto& state : states) {
    GL_CHECK(glUseProgram(state.shaderProgram));
    GL_CHECK(glBindVertexArray(state.vao));
    GL_CHECK(glDrawElements(GL_TRIANGLES, state.numTris * 3, GL_UNSIGNED_BYTE, 0));
  }
}
