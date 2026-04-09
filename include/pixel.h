#pragma once

#include <cstdint>

// Constants matching original code
constexpr int CANVAS_WIDTH = 128;
constexpr int CANVAS_HEIGHT = 128;
constexpr int SCREEN_WIDTH = 640 * 2;
constexpr int SCREEN_HEIGHT = 640 * 2;
constexpr double EPS = 1e-6;
constexpr double EPS_LIGHT = 1e-3;
constexpr int MAX_RADIUS = (CANVAS_WIDTH > CANVAS_HEIGHT) ? CANVAS_WIDTH : CANVAS_HEIGHT;
constexpr int PIXELS_PER_SECOND = 4 * MAX_RADIUS;

// Pixel structure matching original code
struct Pixel {
    float r, g, b, a;     // Color and alpha
    float lr, lg, lb;     // Light properties
    
    Pixel(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f,
          float lr = 0.0f, float lg = 0.0f, float lb = 0.0f)
        : r(r), g(g), b(b), a(a), lr(lr), lg(lg), lb(lb) {}
};

// Integer coordinate
struct CoordI {
    int x, y;
    
    CoordI(int x = 0, int y = 0) : x(x), y(y) {}
    
    CoordI operator+(const CoordI& other) const {
        return CoordI(x + other.x, y + other.y);
    }
    
    CoordI operator-(const CoordI& other) const {
        return CoordI(x - other.x, y - other.y);
    }
    
    bool operator==(const CoordI& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const CoordI& other) const {
        return !(*this == other);
    }
};

// Double coordinate
struct CoordD {
    double x, y;
    
    CoordD(double x = 0.0, double y = 0.0) : x(x), y(y) {}
    
    CoordD operator+(const CoordD& other) const {
        return CoordD(x + other.x, y + other.y);
    }
    
    CoordD operator-(const CoordD& other) const {
        return CoordD(x - other.x, y - other.y);
    }
    
    CoordD operator*(double scalar) const {
        return CoordD(x * scalar, y * scalar);
    }
    
    CoordD operator/(double scalar) const {
        return CoordD(x / scalar, y / scalar);
    }
};

// Camera structure matching original
struct Camera {
    double x, y, scale;
    
    Camera(double x = 0.5, double y = 0.5, double scale = 1.0)
        : x(x), y(y), scale(scale) {}
};

// Convert pixel to RGB (matching original pix2RGB function)
inline uint32_t pixelToRGB(const Pixel& p) {
    int r = static_cast<int>(p.r * 255.0f);
    int g = static_cast<int>(p.g * 255.0f);
    int b = static_cast<int>(p.b * 255.0f);
    
    // Clamp to 0-255
    r = (r < 0) ? 0 : (r > 255) ? 255 : r;
    g = (g < 0) ? 0 : (g > 255) ? 255 : g;
    b = (b < 0) ? 0 : (b > 255) ? 255 : b;
    
    return (r << 16) | (g << 8) | b;
}

// Check if coordinate is within canvas bounds
inline bool withinCanvas(const CoordI& p) {
    return p.x >= 0 && p.x < CANVAS_WIDTH && p.y >= 0 && p.y < CANVAS_HEIGHT;
}

// Coordinate conversions (matching original functions)
inline CoordD memToAbs(const CoordD& p) {
    return CoordD(p.x / CANVAS_WIDTH, p.y / CANVAS_HEIGHT);
}

inline CoordD absToMem(const CoordD& p) {
    return CoordD(p.x * CANVAS_WIDTH, p.y * CANVAS_HEIGHT);
}

inline CoordD absToScreen(const CoordD& p, const Camera& cam) {
    double screenX = SCREEN_WIDTH * ((p.x - cam.x) / cam.scale + 0.5);
    double screenY = SCREEN_HEIGHT * ((p.y - cam.y) / cam.scale + 0.5);
    return CoordD(screenX, screenY);
}

inline CoordD screenToAbs(const CoordD& p, const Camera& cam) {
    double absX = cam.scale * (p.x / SCREEN_WIDTH - 0.5) + cam.x;
    double absY = cam.scale * (p.y / SCREEN_HEIGHT - 0.5) + cam.y;
    return CoordD(absX, absY);
}

inline CoordI memToScreen(const CoordI& p, const Camera& cam) {
    int screenX = static_cast<int>(SCREEN_WIDTH * ((p.x / static_cast<double>(CANVAS_WIDTH) - cam.x) / cam.scale + 0.5));
    int screenY = static_cast<int>(SCREEN_HEIGHT * ((p.y / static_cast<double>(CANVAS_HEIGHT) - cam.y) / cam.scale + 0.5));
    
    // Clamp to screen bounds
    screenX = (screenX < 0) ? 0 : (screenX >= SCREEN_WIDTH) ? SCREEN_WIDTH - 1 : screenX;
    screenY = (screenY < 0) ? 0 : (screenY >= SCREEN_HEIGHT) ? SCREEN_HEIGHT - 1 : screenY;
    
    return CoordI(screenX, screenY);
}

inline CoordI screenToMem(const CoordI& p, const Camera& cam) {
    int memX = static_cast<int>((cam.scale * (p.x / static_cast<double>(SCREEN_WIDTH) - 0.5) + cam.x) * CANVAS_WIDTH);
    int memY = static_cast<int>((cam.scale * (p.y / static_cast<double>(SCREEN_HEIGHT) - 0.5) + cam.y) * CANVAS_HEIGHT);
    
    // Clamp to canvas bounds
    memX = (memX < 0) ? 0 : (memX >= CANVAS_WIDTH) ? CANVAS_WIDTH - 1 : memX;
    memY = (memY < 0) ? 0 : (memY >= CANVAS_HEIGHT) ? CANVAS_HEIGHT - 1 : memY;
    
    return CoordI(memX, memY);
}