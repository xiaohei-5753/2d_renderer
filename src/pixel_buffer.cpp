#include "../include/pixel_buffer.h"
#include "../include/texture.h"
#include <algorithm>
#include <cmath>
#include <iostream>

PixelBuffer::PixelBuffer(int width, int height) 
    : m_width(width)
    , m_height(height)
    , m_pixels(width * height)
    , m_dirty(width * height, false) {
}

PixelBuffer::~PixelBuffer() {
}

void PixelBuffer::initialize() {
    // Initialize all pixels to transparent black
    std::fill(m_pixels.begin(), m_pixels.end(), Pixel());
    std::fill(m_dirty.begin(), m_dirty.end(), true); // All pixels need ray tracing
}

Pixel PixelBuffer::getPixel(int x, int y) const {
    if (!inBounds(x, y)) {
        return Pixel();
    }
    return m_pixels[index(x, y)];
}

Pixel PixelBuffer::getPixel(const CoordI& coord) const {
    return getPixel(coord.x, coord.y);
}

void PixelBuffer::setPixel(int x, int y, const Pixel& pixel) {
    if (!inBounds(x, y)) return;
    m_pixels[index(x, y)] = pixel;
    markDirty(x, y);
}

void PixelBuffer::setPixel(const CoordI& coord, const Pixel& pixel) {
    setPixel(coord.x, coord.y, pixel);
}

void PixelBuffer::clear(const Pixel& fill) {
    std::fill(m_pixels.begin(), m_pixels.end(), fill);
    std::fill(m_dirty.begin(), m_dirty.end(), true);
}

void PixelBuffer::fillRegion(int x1, int y1, int x2, int y2, const Pixel& fill) {
    // Clamp coordinates
    x1 = std::max(0, std::min(x1, m_width - 1));
    x2 = std::max(0, std::min(x2, m_width - 1));
    y1 = std::max(0, std::min(y1, m_height - 1));
    y2 = std::max(0, std::min(y2, m_height - 1));
    
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            m_pixels[index(x, y)] = fill;
            markDirty(x, y);
        }
    }
}

void PixelBuffer::drawLine(const CoordI& a, const CoordI& b, const Pixel& pen) {
    // Bresenham's line algorithm (simplified)
    int x1 = a.x, y1 = a.y;
    int x2 = b.x, y2 = b.y;
    
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        setPixel(x1, y1, pen);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void PixelBuffer::drawCircle(const CoordI& center, int radius, const Pixel& pen) {
    // Midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        setPixel(center.x + x, center.y + y, pen);
        setPixel(center.x + y, center.y + x, pen);
        setPixel(center.x - y, center.y + x, pen);
        setPixel(center.x - x, center.y + y, pen);
        setPixel(center.x - x, center.y - y, pen);
        setPixel(center.x - y, center.y - x, pen);
        setPixel(center.x + y, center.y - x, pen);
        setPixel(center.x + x, center.y - y, pen);
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

Pixel PixelBuffer::rayTrace(const CoordI& origin, const CoordI& direction) const {
    // Simplified ray tracing - just return pixel color at origin
    if (!inBounds(origin.x, origin.y)) {
        return m_wallColor;
    }
    
    // For now, just return the pixel color
    // In original, this does complex light transport
    return m_pixels[index(origin.x, origin.y)];
}

uint32_t PixelBuffer::samplePixel(const CoordI& coord) const {
    // Convert pixel to 32-bit RGBA (8-bit per channel)
    Pixel pixel = getPixel(coord);
    
    // Custom clamp function since std::clamp might not be available
    auto clamp = [](float value, float min, float max) -> float {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    };
    
    uint32_t r = static_cast<uint32_t>(clamp(pixel.r * 255.0f, 0.0f, 255.0f));
    uint32_t g = static_cast<uint32_t>(clamp(pixel.g * 255.0f, 0.0f, 255.0f));
    uint32_t b = static_cast<uint32_t>(clamp(pixel.b * 255.0f, 0.0f, 255.0f));
    uint32_t a = static_cast<uint32_t>(clamp(pixel.a * 255.0f, 0.0f, 255.0f));
    return (r << 24) | (g << 16) | (b << 8) | a;
}

void PixelBuffer::updateTexture(Texture& texture) const {
    // Convert pixel data to float array for texture upload
    // Texture expects RGBA32F format (4 floats per pixel: r,g,b,a)
    // Pixel structure has 7 floats: r,g,b,a,lr,lg,lb
    // We need to extract just the RGBA components
    
    std::vector<float> rgbaData(m_width * m_height * 4);
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int srcIdx = index(x, y);
            int dstIdx = (y * m_width + x) * 4;
            
            const Pixel& pixel = m_pixels[srcIdx];
            rgbaData[dstIdx + 0] = pixel.r;
            rgbaData[dstIdx + 1] = pixel.g;
            rgbaData[dstIdx + 2] = pixel.b;
            rgbaData[dstIdx + 3] = pixel.a;
        }
    }
    
    texture.update(rgbaData.data());
}

void PixelBuffer::updateTextureRegion(Texture& texture, int x, int y, int width, int height) const {
    // Clamp region to valid bounds
    int x1 = std::max(0, std::min(x, m_width - 1));
    int y1 = std::max(0, std::min(y, m_height - 1));
    int x2 = std::max(0, std::min(x + width - 1, m_width - 1));
    int y2 = std::max(0, std::min(y + height - 1, m_height - 1));
    
    if (x1 > x2 || y1 > y2) return;
    
    int regionWidth = x2 - x1 + 1;
    int regionHeight = y2 - y1 + 1;
    
    // Extract RGBA data for the region
    std::vector<float> rgbaData(regionWidth * regionHeight * 4);
    
    for (int py = y1; py <= y2; ++py) {
        for (int px = x1; px <= x2; ++px) {
            int srcIdx = index(px, py);
            int dstIdx = ((py - y1) * regionWidth + (px - x1)) * 4;
            
            const Pixel& pixel = m_pixels[srcIdx];
            rgbaData[dstIdx + 0] = pixel.r;
            rgbaData[dstIdx + 1] = pixel.g;
            rgbaData[dstIdx + 2] = pixel.b;
            rgbaData[dstIdx + 3] = pixel.a;
        }
    }
    
    texture.updateRegion(x1, y1, regionWidth, regionHeight, rgbaData.data());
}

void PixelBuffer::updateTextures(Texture& colorTexture, Texture& lightTexture) {
    // Check if we have dirty region bounds
    int x1, y1, x2, y2;
    bool hasDirtyRegion = getDirtyBounds(x1, y1, x2, y2);
    
    if (!hasDirtyRegion) {
        // No dirty pixels, nothing to upload
        return;
    }
    
    // If dirty region covers most of the texture, update the whole texture
    // This avoids overhead of multiple small updates
    int dirtyWidth = x2 - x1 + 1;
    int dirtyHeight = y2 - y1 + 1;
    int totalPixels = m_width * m_height;
    int dirtyPixels = dirtyWidth * dirtyHeight;
    
    const float DIRTY_THRESHOLD = 0.3f; // If more than 30% of pixels are dirty, update whole texture
    
    if (dirtyPixels > totalPixels * DIRTY_THRESHOLD) {
        // Update whole texture for large dirty regions
        std::vector<float> colorData(m_width * m_height * 4);
        std::vector<float> lightData(m_width * m_height * 4);
        
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                int srcIdx = index(x, y);
                int dstIdx = (y * m_width + x) * 4;
                
                const Pixel& pixel = m_pixels[srcIdx];
                
                // Color data (RGBA)
                colorData[dstIdx + 0] = pixel.r;
                colorData[dstIdx + 1] = pixel.g;
                colorData[dstIdx + 2] = pixel.b;
                colorData[dstIdx + 3] = pixel.a;
                
                // Light data (RGB + unused alpha)
                lightData[dstIdx + 0] = pixel.lr;
                lightData[dstIdx + 1] = pixel.lg;
                lightData[dstIdx + 2] = pixel.lb;
                lightData[dstIdx + 3] = 0.0f; // unused
            }
        }
        
        colorTexture.update(colorData.data());
        lightTexture.update(lightData.data());
    } else {
        // Update only the dirty region
        int regionWidth = dirtyWidth;
        int regionHeight = dirtyHeight;
        
        std::vector<float> colorData(regionWidth * regionHeight * 4);
        std::vector<float> lightData(regionWidth * regionHeight * 4);
        
        for (int y = y1; y <= y2; ++y) {
            for (int x = x1; x <= x2; ++x) {
                int srcIdx = index(x, y);
                int dstIdx = ((y - y1) * regionWidth + (x - x1)) * 4;
                
                const Pixel& pixel = m_pixels[srcIdx];
                
                // Color data (RGBA)
                colorData[dstIdx + 0] = pixel.r;
                colorData[dstIdx + 1] = pixel.g;
                colorData[dstIdx + 2] = pixel.b;
                colorData[dstIdx + 3] = pixel.a;
                
                // Light data (RGB + unused alpha)
                lightData[dstIdx + 0] = pixel.lr;
                lightData[dstIdx + 1] = pixel.lg;
                lightData[dstIdx + 2] = pixel.lb;
                lightData[dstIdx + 3] = 0.0f; // unused
            }
        }
        
        colorTexture.updateRegion(x1, y1, regionWidth, regionHeight, colorData.data());
        lightTexture.updateRegion(x1, y1, regionWidth, regionHeight, lightData.data());
    }
    
    // Clear dirty flags after upload
    clearDirty();
}

void PixelBuffer::markDirty(int x, int y) {
    if (!inBounds(x, y)) return;
    m_dirty[index(x, y)] = true;
}

void PixelBuffer::markDirtyRegion(int x1, int y1, int x2, int y2) {
    // Clamp coordinates
    x1 = std::max(0, std::min(x1, m_width - 1));
    x2 = std::max(0, std::min(x2, m_width - 1));
    y1 = std::max(0, std::min(y1, m_height - 1));
    y2 = std::max(0, std::min(y2, m_height - 1));
    
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            m_dirty[index(x, y)] = true;
        }
    }
}

bool PixelBuffer::isDirty(int x, int y) const {
    if (!inBounds(x, y)) return false;
    return m_dirty[index(x, y)];
}

void PixelBuffer::clearDirty() {
    std::fill(m_dirty.begin(), m_dirty.end(), false);
}

bool PixelBuffer::inBounds(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

bool PixelBuffer::getDirtyBounds(int& x1, int& y1, int& x2, int& y2) const {
    bool found = false;
    x1 = m_width;
    y1 = m_height;
    x2 = -1;
    y2 = -1;
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (m_dirty[index(x, y)]) {
                if (!found) {
                    found = true;
                    x1 = x2 = x;
                    y1 = y2 = y;
                } else {
                    if (x < x1) x1 = x;
                    if (x > x2) x2 = x;
                    if (y < y1) y1 = y;
                    if (y > y2) y2 = y;
                }
            }
        }
    }
    
    return found;
}