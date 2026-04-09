#pragma once

#include "pixel.h"
#include <vector>
#include <cstdint>
#include <memory>

class Texture; // Forward declaration

class PixelBuffer {
public:
    PixelBuffer(int width = CANVAS_WIDTH, int height = CANVAS_HEIGHT);
    ~PixelBuffer();
    
    // Initialize buffer with default pixels
    void initialize();
    
    // Pixel access
    Pixel getPixel(int x, int y) const;
    Pixel getPixel(const CoordI& coord) const;
    void setPixel(int x, int y, const Pixel& pixel);
    void setPixel(const CoordI& coord, const Pixel& pixel);
    
    // Batch operations
    void clear(const Pixel& fill = Pixel());
    void fillRegion(int x1, int y1, int x2, int y2, const Pixel& fill);
    
    // Line drawing (matching original line_cvs function)
    void drawLine(const CoordI& a, const CoordI& b, const Pixel& pen);
    
    // Circle drawing (matching original circle_cvs function)
    void drawCircle(const CoordI& center, int radius, const Pixel& pen);
    
    // Ray tracing functions (CPU reference implementation)
    Pixel rayTrace(const CoordI& origin, const CoordI& direction) const;
    uint32_t samplePixel(const CoordI& coord) const;
    
    // Texture synchronization
    void updateTexture(Texture& texture) const;
    void updateTextureRegion(Texture& texture, int x, int y, int width, int height) const;
    
    // Update both color and light textures
    void updateTextures(Texture& colorTexture, Texture& lightTexture);
    
    // Dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Data access (for GPU upload)
    const float* getData() const { return reinterpret_cast<const float*>(m_pixels.data()); }
    size_t getDataSize() const { return m_pixels.size() * sizeof(Pixel); }
    
    // Mark pixels as needing ray tracing update
    void markDirty(int x, int y);
    void markDirtyRegion(int x1, int y1, int x2, int y2);
    bool isDirty(int x, int y) const;
    void clearDirty();
    
    // Get bounding box of dirty region (returns true if any dirty pixels)
    bool getDirtyBounds(int& x1, int& y1, int& x2, int& y2) const;
    
private:
    int m_width;
    int m_height;
    std::vector<Pixel> m_pixels;
    std::vector<bool> m_dirty; // Marks pixels that need ray tracing update
    
    // Helper functions
    bool inBounds(int x, int y) const;
    int index(int x, int y) const { return y * m_width + x; }
    
    // Light properties (matching original)
    Pixel m_wallColor = Pixel(0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.2f, 0.2f);
    CoordD m_sunDirection = CoordD(0.70710678118, 0.70710678118); // sqrt(0.5)
    float m_sunScattering = 5e-3f;
    Pixel m_sunColor = Pixel(0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f, 10.0f);
};