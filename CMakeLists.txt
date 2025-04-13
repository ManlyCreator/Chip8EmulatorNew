cmake_minimum_required(VERSION 3.5)

project(Chip8Emulator)

include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_BUILD_TYPE Debug)

add_executable(${PROJECT_NAME} main.cpp)

include_directories(include/ imgui/)

add_library(Chip8 src/chip8.cpp)
add_library(Shader src/shader.cpp)
add_library(Screen src/screen.cpp)
add_library(Buzzer src/buzzer.cpp)
add_library(Opcodes src/opcodes.cpp)

target_link_libraries(Screen glad glfw3 GL m Shader)
target_link_libraries(Buzzer openal m)
target_link_libraries(${PROJECT_NAME} Chip8 Screen Buzzer Opcodes)
