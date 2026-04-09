#include "../include/texture.h"
#include <GL/glew.h>
#include <iostream>

Texture::Texture() 
    : m_id(0)
    , m_width(0)
    , m_height(0)
    , m_format(Format::RGBA8)
    , m_filter(Filter::Nearest)
    , m_wrap(Wrap::ClampToEdge) {
}

Texture::~Texture() {
    destroy();
}

bool Texture::create(int width, int height, Format format, Filter filter, Wrap wrap) {
    destroy();
    
    glGenTextures(1, &m_id);
    if (m_id == 0) {
        std::cerr << "Failed to generate texture ID" << std::endl;
        return false;
    }
    
    bind(0);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getGLFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getGLFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGLWrap());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGLWrap());
    
    // Allocate texture storage
    glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(), width, height, 0, 
                 getGLFormat(), getGLType(), nullptr);
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Failed to allocate texture: OpenGL error " << error << std::endl;
        destroy();
        return false;
    }
    
    m_width = width;
    m_height = height;
    m_format = format;
    m_filter = filter;
    m_wrap = wrap;
    
    return true;
}

bool Texture::createFromData(int width, int height, Format format, const void* data, Filter filter) {
    if (!create(width, height, format, filter)) {
        return false;
    }
    
    if (data) {
        update(data);
    }
    
    return true;
}

bool Texture::loadFromFile(const std::string& filename) {
    // TODO: Implement texture loading from file
    std::cerr << "Texture loading from file not implemented: " << filename << std::endl;
    return false;
}

void Texture::update(const void* data) {
    if (!isValid()) return;
    
    bind(0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, 
                    getGLFormat(), getGLType(), data);
}

void Texture::updateRegion(int x, int y, int width, int height, const void* data) {
    if (!isValid()) return;
    
    bind(0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, 
                    getGLFormat(), getGLType(), data);
}

void Texture::bind(unsigned int unit) const {
    if (!isValid()) return;
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bindAsImage(unsigned int unit, unsigned int access) const {
    if (!isValid()) return;
    
    // Bind texture as image for compute shader access
    glBindImageTexture(unit, m_id, 0, GL_FALSE, 0, access, getGLInternalFormat());
}

void Texture::destroy() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
    m_width = 0;
    m_height = 0;
}

unsigned int Texture::getGLFormat() const {
    switch (m_format) {
        case Format::RGBA8: return GL_RGBA;
        case Format::RGBA32F: return GL_RGBA;
        case Format::R32F: return GL_RED;
        case Format::RGB8: return GL_RGB;
        default: return GL_RGBA;
    }
}

unsigned int Texture::getGLInternalFormat() const {
    switch (m_format) {
        case Format::RGBA8: return GL_RGBA8;
        case Format::RGBA32F: return GL_RGBA32F;
        case Format::R32F: return GL_R32F;
        case Format::RGB8: return GL_RGB8;
        default: return GL_RGBA8;
    }
}

unsigned int Texture::getGLType() const {
    switch (m_format) {
        case Format::RGBA8: return GL_UNSIGNED_BYTE;
        case Format::RGBA32F: return GL_FLOAT;
        case Format::R32F: return GL_FLOAT;
        case Format::RGB8: return GL_UNSIGNED_BYTE;
        default: return GL_UNSIGNED_BYTE;
    }
}

unsigned int Texture::getGLFilter() const {
    switch (m_filter) {
        case Filter::Nearest: return GL_NEAREST;
        case Filter::Linear: return GL_LINEAR;
        default: return GL_NEAREST;
    }
}

unsigned int Texture::getGLWrap() const {
    switch (m_wrap) {
        case Wrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case Wrap::Repeat: return GL_REPEAT;
        case Wrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        default: return GL_CLAMP_TO_EDGE;
    }
}