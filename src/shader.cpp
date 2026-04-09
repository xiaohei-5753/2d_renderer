#include "../include/shader.h"
#include <GL/glew.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() : m_programId(0) {
}

Shader::~Shader() {
    destroy();
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "Failed to read shader files: " << vertexPath << ", " << fragmentPath << std::endl;
        return false;
    }
    
    return loadFromSource(vertexSource, fragmentSource);
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& geometryPath, const std::string& fragmentPath) {
    // Not implemented
    std::cerr << "Geometry shader not implemented" << std::endl;
    return false;
}

bool Shader::loadFromFiles(const std::string& computePath) {
    destroy();
    
    std::string computeSource = readFile(computePath);
    if (computeSource.empty()) {
        std::cerr << "Failed to read compute shader file: " << computePath << std::endl;
        return false;
    }
    
    unsigned int computeShader = compileShader(computeSource, GL_COMPUTE_SHADER);
    if (computeShader == 0) {
        return false;
    }
    
    m_programId = glCreateProgram();
    glAttachShader(m_programId, computeShader);
    
    if (!linkProgram(computeShader)) {
        glDeleteShader(computeShader);
        glDeleteProgram(m_programId);
        m_programId = 0;
        return false;
    }
    
    glDeleteShader(computeShader);
    return true;
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    destroy();
    
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        return false;
    }
    
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    if (!linkProgram(vertexShader, fragmentShader)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

bool Shader::loadFromSource(const std::string& computeSource) {
    destroy();
    
    unsigned int computeShader = compileShader(computeSource, GL_COMPUTE_SHADER);
    if (computeShader == 0) {
        return false;
    }
    
    m_programId = glCreateProgram();
    glAttachShader(m_programId, computeShader);
    
    if (!linkProgram(computeShader)) {
        glDeleteShader(computeShader);
        glDeleteProgram(m_programId);
        m_programId = 0;
        return false;
    }
    
    glDeleteShader(computeShader);
    return true;
}

void Shader::bind() const {
    if (m_programId != 0) {
        glUseProgram(m_programId);
    }
}

void Shader::unbind() const {
    glUseProgram(0);
}

void Shader::setUniformInt(const std::string& name, int value) {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void Shader::setUniformFloat(const std::string& name, float value) {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Shader::setUniformVec2(const std::string& name, float x, float y) {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform2f(location, x, y);
    }
}

void Shader::setUniformVec3(const std::string& name, float x, float y, float z) {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void Shader::setUniformVec4(const std::string& name, float x, float y, float z, float w) {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform4f(location, x, y, z, w);
    }
}

void Shader::setUniformMat4(const std::string& name, const float* matrix) {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

void Shader::setUniformTexture(const std::string& name, unsigned int textureId, int unit) {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, unit);
    }
}

void Shader::destroy() {
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
        m_programId = 0;
    }
    m_uniformCache.clear();
}

unsigned int Shader::compileShader(const std::string& source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    if (shader == 0) {
        std::cerr << "Failed to create shader" << std::endl;
        return 0;
    }
    
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    m_programId = glCreateProgram();
    if (m_programId == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        return false;
    }
    
    glAttachShader(m_programId, vertexShader);
    glAttachShader(m_programId, fragmentShader);
    glLinkProgram(m_programId);
    
    GLint success;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(m_programId, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
        glDeleteProgram(m_programId);
        m_programId = 0;
        return false;
    }
    
    return true;
}

bool Shader::linkProgram(unsigned int vertexShader, unsigned int geometryShader, unsigned int fragmentShader) {
    // Not implemented
    return false;
}

bool Shader::linkProgram(unsigned int computeShader) {
    glLinkProgram(m_programId);
    
    // Check link status
    int success;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programId, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    // Validate program (optional but useful for debugging)
    glValidateProgram(m_programId);
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programId, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program validation failed: " << infoLog << std::endl;
        // Don't return false here - validation can fail on some drivers even with correct shaders
    }
    
    return true;
}

int Shader::getUniformLocation(const std::string& name) {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    
    int location = glGetUniformLocation(m_programId, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: uniform '" << name << "' not found in shader" << std::endl;
    }
    
    m_uniformCache[name] = location;
    return location;
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}