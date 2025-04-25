#include "screen.h"
#include "GLFW/glfw3.h"
#include "chip8.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <ostream>
#include <sstream>
#include <vector>
#include <iomanip>

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
  // Debugger Settings
  static int steps = 1;
  static int toggleHex = 1;
  static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
  int freq = static_cast<int>(chip8->instructionFrequency);

  // Screen
  static ImVec2 imageSize(int(WIDTH / 2), int(HEIGHT / 2));
  static ImVec2 screenSize(imageSize.x, imageSize.y + 35);
  ImGui::SetNextWindowPos(ImVec2(WIDTH - screenSize.x, 0));
  ImGui::SetNextWindowSize(screenSize);
  ImGui::Begin("Screen");
  ImGui::Image(texture, imageSize);
  ImGui::End();

  // State
  static ImVec2 stateSize(int(screenSize.x / 2), screenSize.y);
  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(stateSize);
  ImGui::Begin("State");
  ImGui::RadioButton("Hex", &toggleHex, 1); ImGui::SameLine();
  ImGui::RadioButton("Decimal", &toggleHex, 0);
  std::stringstream opcodeStream, pcStream, keyStream;
  opcodeStream << "Opcode: 0x" << std::hex << std::uppercase << chip8->opcode;
  pcStream     << "PC:     ";
  keyStream    << "Key:    ";
  if (toggleHex) {
    pcStream  << "0x" << std::hex << std::uppercase << chip8->pc;
    keyStream << "0x" << std::hex << std::uppercase << int(chip8->keyPressed);
  }
  else {
    pcStream  << chip8->pc;
    keyStream << int(chip8->keyPressed);
  }
  if (chip8->keyPressed == -1) {
    keyStream.str("Key:    NONE");
  }

  ImGui::TextUnformatted(pcStream.str().c_str());
  ImGui::TextUnformatted(keyStream.str().c_str());
  ImGui::TextUnformatted(opcodeStream.str().c_str());

  ImGui::SeparatorText("V-Registers");
  if (ImGui::BeginTable("Registers", 2, tableFlags)) {
    ImGui::TableSetupColumn("Register");
    ImGui::TableSetupColumn("Value");
    ImGui::TableHeadersRow();
    for (int row = 0; row < 16; row ++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("V[%.1X]", row);
      ImGui::TableNextColumn();
      if (toggleHex)
        ImGui::Text("0x%.2X", chip8->V[row]);
      else
        ImGui::Text("%d", chip8->V[row]);
    }
    ImGui::EndTable();
  }
  ImGui::End();

  // Memory
  static char address[4] = "";
  ImVec2 memorySize = ImVec2(screenSize.x, screenSize.y - 70);
  ImU32 activeColor = ImGui::GetColorU32(ImVec4(0.0f, 0.73f, 1.0f, 0.5f));
  ImGui::SetNextWindowSize(memorySize);
  ImGui::SetNextWindowPos(ImVec2(0, HEIGHT - memorySize.y));
  ImGui::Begin("Memory");
  ImGui::Text("Jump to Address:"); ImGui::SameLine();
  ImGui::SetNextItemWidth(100.0f);
  if (ImGui::InputTextWithHint("##Address", "<0xXXX>", address, 4, ImGuiInputTextFlags_EnterReturnsTrue)) {
    std::cout << address << "\n";
  }
  if (ImGui::BeginTable("Memory", 2, tableFlags)) {
    ImGui::TableSetupColumn("Address");
    ImGui::TableSetupColumn("Value");
    ImGui::TableHeadersRow();
    for (int i = 0; i < MEMORY; i += 2) {
      Word word = (chip8->memory[i] << 8) | chip8->memory[i + 1];
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (i == chip8-> pc - 1 || i == chip8->pc || i == chip8->pc + 1) // Accounts for ROM's that set the PC to an odd number
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, activeColor);
      ImGui::Text("0x%.3X", i);
      ImGui::TableNextColumn();
      if (i == chip8-> pc - 1 || i == chip8->pc || i == chip8->pc + 1)
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, activeColor);
      ImGui::Text("0x%.4X", word);
    }
    ImGui::EndTable();
  }
  ImGui::End();
  ImGuiInputTextCallbackData().Buf;

  // Controls
  static ImVec2 controlsSize = stateSize;
  ImGui::SetNextWindowPos(ImVec2(screenSize.x - controlsSize.x, 0.0f));
  ImGui::SetNextWindowSize(controlsSize);
  ImGui::Begin("Controls");
  ImGui::PushItemWidth(100.0f);
  ImGui::InputInt("Instruction Frequency", &freq);
  chip8->instructionFrequency = freq;
  ImGui::InputInt("Step Count", &steps);
  ImGui::PopItemWidth();
  if (ImGui::Button("Pause")) {
    chip8->paused = !chip8->paused;
  }
  if (ImGui::Button("Step")) {
    if (chip8->paused) {
      for (int i = 0; i < steps; i++) {
        chip8->soundTimer = chip8->soundTimer > 0 ? chip8->soundTimer - 1 : 0;
        chip8->delayTimer = chip8->delayTimer > 0 ? chip8->delayTimer - 1 : 0;
        chip8->emulateCycle();
      }
    }
  }
  ImGui::End();
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
