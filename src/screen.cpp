#include "screen.h"
#include "GLFW/glfw3.h"
#include "chip8.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

void framebufferSizeCallback(GLFWwindow *window, int width, int height);

Screen::Screen(const char *vsPath, const char *fsPath, Chip8 *chip8) {
  GLuint VBO;
  float plane[] = {
    // Vertices   // Texture Coordinates
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f
  };
  this->chip8 = chip8;
  textureData = new std::vector<unsigned char>(DISPLAY_WIDTH * DISPLAY_HEIGHT * 4);

  // GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Chip8", NULL, NULL);

  if (!window) {
    printf("Window creation failed\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  // GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to load GLAD\n");
    exit(EXIT_FAILURE);
  }

  glViewport(0, 0, WIDTH, HEIGHT);

  // Shader
  shader = std::make_unique<Shader>(vsPath, fsPath);

  // Texture
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  updateTextureData();
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData->data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Frame Buffer Object
  glGenFramebuffers(1, &FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  // Attaching Texture to FBO
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer creation failed\n";
    std::exit(EXIT_FAILURE);
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  // Vertex Array
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Vertex Buffer
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  // Callbacks
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  // ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();
}

void Screen::draw() {
  glfwPollEvents();

  /*ImGui_ImplOpenGL3_NewFrame();*/
  /*ImGui_ImplGlfw_NewFrame();*/
  /*ImGui::NewFrame();*/
  /*ImGui::ShowDemoWindow();*/
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Draw to FBO
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  glBindVertexArray(VAO);
  shader->use();
  shader->setInt("texSample", 0);
  updateTextureData();
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, textureData->data());
  glDrawArrays(GL_TRIANGLES, 0, 6);
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) std::cout << "GL Error: " << err << "\n";
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Draw
  glViewport(0, 0, WIDTH, HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  debugger();

  // ImGui Render
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Poll Events & Swap Buffers
  glfwSwapBuffers(window);
}

void Screen::updateTextureData() {
  for (unsigned int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
    (*textureData)[i * 4] = chip8->display[i];
    (*textureData)[i * 4 + 1] = chip8->display[i];
    (*textureData)[i * 4 + 2] = chip8->display[i];
    (*textureData)[i * 4 + 3] = 255;
  }
}

void Screen::debugger() {
  int freq = static_cast<int>(chip8->instructionFrequency);

  // Screen
  int screenWidth = WIDTH / 2;
  int screenHeight = HEIGHT / 2;
  ImGui::SetNextWindowPos(ImVec2(WIDTH - screenWidth, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100.0f, 0.0f));
  // Window label is ~35 pixels
  ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight + 35));
  ImGui::Begin("Screen");
  ImGui::Image(texture, ImVec2(screenWidth, screenHeight));
  ImGui::End();
  ImGui::PopStyleVar();
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

Screen::~Screen() {
  delete textureData;
  glDeleteFramebuffers(1, &FBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteShader(shader->getID());
  glDeleteTextures(1, &texture);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
}
