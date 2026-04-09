#pragma once

#include <string>
#include <unordered_map>

class Shader {
public:
    Shader();
    ~Shader();
    
    // Load and compile shader from files
    bool loadFromFiles(const std::string& vertexPath, 
                      const std::string& fragmentPath);
    bool loadFromFiles(const std::string& vertexPath,
                      const std::string& geometryPath,
                      const std::string& fragmentPath);
    bool loadFromFiles(const std::string& computePath);
    
    // Load and compile shader from source strings
    bool loadFromSource(const std::string& vertexSource,
                       const std::string& fragmentSource);
    bool loadFromSource(const std::string& computeSource);
    
    // Use shader program
    void bind() const;
    void unbind() const;
    
    // Uniform setters
    void setUniformInt(const std::string& name, int value);
    void setUniformFloat(const std::string& name, float value);
    void setUniformVec2(const std::string& name, float x, float y);
    void setUniformVec3(const std::string& name, float x, float y, float z);
    void setUniformVec4(const std::string& name, float x, float y, float z, float w);
    void setUniformMat4(const std::string& name, const float* matrix);
    
    // Texture uniform
    void setUniformTexture(const std::string& name, unsigned int textureId, int unit);
    
    // Check if shader is valid
    bool isValid() const { return m_programId != 0; }
    
    // Get OpenGL program ID
    unsigned int getId() const { return m_programId; }
    
    // Cleanup
    void destroy();
    
private:
    unsigned int m_programId;
    std::unordered_map<std::string, int> m_uniformCache;
    
    // Shader compilation
    unsigned int compileShader(const std::string& source, unsigned int type);
    bool linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    bool linkProgram(unsigned int vertexShader, unsigned int geometryShader, unsigned int fragmentShader);
    bool linkProgram(unsigned int computeShader);
    
    // Uniform location caching
    int getUniformLocation(const std::string& name);
    
    // File reading
    std::string readFile(const std::string& path);
    
    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
};