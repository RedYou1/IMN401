
#include "GLProgram.h"
#include "ImGuiLogger.h"
#include "utils.hpp"
#include <cctype>
#include <iostream>

GLProgram::GLProgram(std::string filename, GLenum type) : m_Type(type), m_filename(filename) {
    info_text = "";

    std::string shaderSource = readFile(filename);
    if (shaderSource.size() >= 3 &&
        static_cast<unsigned char>(shaderSource[0]) == 0xEF &&
        static_cast<unsigned char>(shaderSource[1]) == 0xBB &&
        static_cast<unsigned char>(shaderSource[2]) == 0xBF) {
        shaderSource = shaderSource.substr(3);
    }

    size_t firstNonWhitespace = 0;
    while (firstNonWhitespace < shaderSource.size() &&
           std::isspace(static_cast<unsigned char>(shaderSource[firstNonWhitespace]))) {
        ++firstNonWhitespace;
    }

    // Check if the shader source already contains a version directive otherwise prepend the version specifications
    std::string str;
    if (firstNonWhitespace < shaderSource.size() && shaderSource.compare(firstNonWhitespace, 8, "#version") == 0) {
        str = shaderSource;
    } else {
        str = m_version_specs + shaderSource;
    }
    const GLchar *srcCode = str.c_str();

    m_Id = glCreateShaderProgramv(type, 1, &srcCode);

    printInfoLog();
}

GLProgram::~GLProgram() {
    glDeleteProgram(m_Id);
}
GLuint GLProgram::getId() {
    return m_Id;
}
bool GLProgram::printInfoLog() {
    GLuint isLinked;
    glGetProgramiv(m_Id, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(m_Id, GL_INFO_LOG_LENGTH, &maxLength);
        // The maxLength includes the NULL character
        char *infoLog = (char *)malloc(maxLength);
        glGetProgramInfoLog(m_Id, maxLength, &maxLength, &infoLog[0]);

        std::string df(infoLog);
        info_text += df;

        LOG_ERROR << "Error in Shader " + m_filename + "\n " + info_text << std::endl;
        Logger::getInstance()->show_interface = true;
        // We don't need the program anymore.
        glDeleteProgram(m_Id);
        m_Id = 0;
        return false;
    }
    return true;
}
