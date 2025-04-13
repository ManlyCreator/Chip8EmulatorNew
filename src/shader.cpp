#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include "shader.h"

Shader::Shader(const char *vsPath, const char *fsPath) {
  unsigned vertexShader, fragmentShader;
  std::string vsSource, fsSource;
  std::ifstream vsFile, fsFile; 
  std::stringstream vsStream, fsStream;

  vsFile.open(vsPath);
  fsFile.open(fsPath);

  // Vertex Shader Compilation
  if (vsFile.is_open()) {
    vsSource = readFile(&vsFile);
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char **)&vsSource, NULL);
    glCompileShader(vertexShader);
    if (!shaderCompilationSuccess(vertexShader))
      std::cerr << "Failed to compile Vertex Shader\n";
  } else
    std::perror("Could not open Vertex Shader\n");

  // Fragment Shader Compilation
  if (fsFile.is_open()) {
    fsSource = readFile(&fsFile);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char **)&fsSource, NULL);
    glCompileShader(fragmentShader);
    if (!shaderCompilationSuccess(fragmentShader))
      std::cerr << "Failed to compile Fragment Shader\n";
  } else 
    std::perror("Could not open Fragment Shader\n");

  // Program Linking
  ID = glCreateProgram();
  glAttachShader(ID, vertexShader);
  glAttachShader(ID, fragmentShader);
  glLinkProgram(ID);
  if (!programLinkSuccess(ID))
    std::cerr << "Failed to link shader program\n";

  // Free Memory
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  vsFile.close();
  fsFile.close();
}

void Shader::use() {
  glUseProgram(ID);
}

void Shader::setFloat(const char *uniform, float value) {
  glUniform1f(glGetUniformLocation(ID, uniform), value);
}

void Shader::setInt(const char *uniform, int value) {
  glUniform1i(glGetUniformLocation(ID, uniform), value);
}

void Shader::setVector3f(const char *uniform, glm::vec3 value) {
  glUniform3f(glGetUniformLocation(ID, uniform), value.x, value.y, value.z);
}

void Shader::setMatrix4(const char *uniform, glm::mat4 value) {
  glUniformMatrix4fv(glGetUniformLocation(ID, uniform), 1, GL_FALSE, &value[0][0]);
}

std::string Shader::readFile(std::ifstream *file) {
  std::stringstream stream;
  stream << file->rdbuf();
  return stream.str();
}

int Shader::shaderCompilationSuccess(unsigned shader) {
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr  << "*** ERROR COMPILING SHADER ***\n" << infoLog;
    return 0;
  }
  return 1;
}

int Shader::programLinkSuccess(unsigned program) {
  int success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cerr << "*** ERROR LINKING PROGRAM ***\n" << infoLog;
    return 0;
  }
  return 1;
}
