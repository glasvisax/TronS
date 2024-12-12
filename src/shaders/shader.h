#pragma once

#include <string>
#include <glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

class Shader {
public:
    unsigned int ID; // ID ןנמדנאללû רויהונא

    Shader() : vertexShader(0), fragmentShader(0), ID(0) {}
    ~Shader();

    void setVertex(const char* vertexPath) 
    {
        vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
    }

    void setFragment(const char* fragmentPath)
    {
        fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);
    }

    void linkProgram();

    void use() 
    {
        glUseProgram(ID);
    }

public:
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    unsigned int vertexShader;
    unsigned int fragmentShader;

    unsigned int compileShader(const char* path, GLenum shaderType);

    void checkCompileErrors(unsigned int shader, std::string type);
};
