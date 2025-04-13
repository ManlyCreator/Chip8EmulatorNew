#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <string>

class Shader {
  private:
    unsigned ID;

    std::string readFile(std::ifstream *file);
    int shaderCompilationSuccess(unsigned shader);
    int programLinkSuccess(unsigned program);

  public:
    Shader(const char *vsPath, const char *fsPath);

    void use();
    unsigned getID() { return ID; };
    void setInt(const char *uniform, int value);
    void setFloat(const char *uniform, float value);
    void setVector3f(const char *uniform, glm::vec3 value);
    void setMatrix4(const char *uniform, glm::mat4 value);
};

#endif
