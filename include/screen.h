#ifndef SCREEN_H
#define SCREEN_H

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "shader.h"

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define WIDTH 1920
#define HEIGHT 960

class Chip8;

class Screen {
  private:
    GLuint texture;
    GLuint VAO;
    std::unique_ptr<Shader> shader;

  public:
    GLFWwindow *window;

    Screen(const char *vsPath, const char *fsPath, unsigned texWidth, unsigned texHeight, unsigned char *texData);
    ~Screen();
    void draw(Chip8 *chip8);
};

#endif
