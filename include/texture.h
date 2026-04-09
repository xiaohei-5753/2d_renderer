#pragma once

#include <cstdint>
#include <string>

class Texture {
public:
    enum class Format {
        RGBA8,      // 8-bit per channel RGBA
        RGBA32F,    // 32-bit float per channel RGBA
        R32F,       // 32-bit float single channel
        RGB8,       // 8-bit per channel RGB
    };
    
    enum class Filter {
        Nearest,    // Pixelated (good for pixel art)
        Linear,     // Smooth interpolation
    };
    
    enum class Wrap {
        ClampToEdge,
        Repeat,
        MirroredRepeat,
    };
    
    Texture();
    ~Texture();
    
    // Create texture with specified format
    bool create(int width, int height, Format format = Format::RGBA8, 
                Filter filter = Filter::Nearest, Wrap wrap = Wrap::ClampToEdge);
    
    // Create texture from pixel data
    bool createFromData(int width, int height, Format format, 
                       const void* data, Filter filter = Filter::Nearest);
    
    // Load texture from file
    bool loadFromFile(const std::string& filename);
    
    // Update texture data
    void update(const void* data);
    void updateRegion(int x, int y, int width, int height, const void* data);
    
    // Bind texture to texture unit
    void bind(unsigned int unit = 0) const;
    void unbind() const;
    
    // Bind texture as image for compute shaders
    void bindAsImage(unsigned int unit, unsigned int access) const;
    
    // Get OpenGL ID
    unsigned int getId() const { return m_id; }
    
    // Get dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Check if texture is valid
    bool isValid() const { return m_id != 0; }
    
    // Cleanup
    void destroy();
    
private:
    unsigned int m_id;
    int m_width;
    int m_height;
    Format m_format;
    Filter m_filter;
    Wrap m_wrap;
    
    // Convert enums to OpenGL constants
    unsigned int getGLFormat() const;
    unsigned int getGLInternalFormat() const;
    unsigned int getGLType() const;
    unsigned int getGLFilter() const;
    unsigned int getGLWrap() const;
    
    // Prevent copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
};