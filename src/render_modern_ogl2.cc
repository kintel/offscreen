#include "render_modern_ogl2.h"
#include "state.h"
#include "system-gl.h"

const char *vertexShaderSource_120 = R"(#version 120
    attribute vec3 aPos;
    attribute vec3 aColor;

    varying vec3 ourColor;

    void main() {
      gl_Position = vec4(aPos, 1.0);
      ourColor = aColor;
    }
  )";

const char *fragmentShaderSource_120 = R"(#version 120
    varying vec3 ourColor;

    void main() {
      gl_FragColor = vec4(ourColor, 1.0);
    }
  )";

void setupModernOGL2(MyState &state, const std::string &glslVersion) {
  float vertices[] = {
    -0.8f, -0.8f, 0.0f, 1, 0, 0,
    0.8f, -0.8f, 0.0f,  0, 1, 0,
    0.0f,  0.9f, 0.0f,  0, 0, 1,
  };

  GLuint vbo;
  glGenBuffers(1, &vbo);

  std::cout << "Using GLSL " << glslVersion << std::endl;
  const char *vertexShaderSource = vertexShaderSource_120;
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

  const char *fragmentShaderSource = fragmentShaderSource_120;
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
  glBindAttribLocation(state.shaderProgram, 0, "aPos");
  glBindAttribLocation(state.shaderProgram, 1, "aColor");
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
  printf("OpenGL glGenVertexArrays: %p\n", glGenVertexArrays);
  printf("OpenGL glGenVertexArraysAPPLE: %p\n", glGenVertexArraysAPPLE);
  //  GL_CHECK(glGenVertexArraysAPPLE(1, &state.vao));
  //  GL_CHECK(glBindVertexArrayAPPLE(state.vao));
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  GL_CHECK();
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  GL_CHECK();
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  unsigned int indices[] = {
    0, 1, 2,
  };
  glGenBuffers(1, &state.ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  GL_CHECK();
}

void renderModernOGL2(const MyState& state) {
  GL_CHECK(glClearColor(0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 0.4 + 0.6*std::rand()/RAND_MAX, 1.0));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  glUseProgram(state.shaderProgram);
  //  glBindVertexArray(state.vao);
  //  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
