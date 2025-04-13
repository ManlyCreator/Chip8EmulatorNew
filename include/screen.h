#ifndef SCREEN_H
#define SCREEN_H

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define WIDTH 1920
#define HEIGHT 960

class Screen {
  private:
    GLuint texture;
    GLuint VAO;
    std::unique_ptr<Shader> shader;

  public:
    GLFWwindow *window;

    Screen(const char *vsPath, const char *fsPath, unsigned texWidth, unsigned texHeight, unsigned char *texData);
    void draw(unsigned char *displayBuffer);
};

#endif
