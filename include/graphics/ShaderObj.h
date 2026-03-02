#pragma once
#include <string>
#include <GL/glew.h>

class ShaderObj {
public:
    unsigned int ID;
    ShaderObj(const char* vertexPath, const char* fragmentPath);
    void use();
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
};