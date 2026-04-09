#pragma once

// Debug output control
// Set to 1 to enable debug console output, 0 to disable for performance
#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

#include "pixel.h"
#include "expression.h"
#include <memory>
#include <string>

// Forward declarations
struct GLFWwindow;
class Texture;
class Shader;
class PixelBuffer;
class Camera;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    // Initialize OpenGL context, window, and resources
    bool initialize();
    
    // Main rendering loop
    void run();
    
    // Cleanup resources
    void shutdown();
    
    // Getter methods
    bool isRunning() const { return m_running; }
    int getCanvasWidth() const { return CANVAS_WIDTH; }
    int getCanvasHeight() const { return CANVAS_HEIGHT; }
    
private:
    // Private methods
    bool initGLFW();
    bool initGLEW();
    bool initShaders();
    bool initTextures();
    bool initBuffers();
    
    void handleInput();
    void update();
    void render();
    
    // Pen evaluation
    Pixel evaluatePenAtCanvas(const CoordI& canvasCoord) const;
    
    // Screenshot functionality
    bool saveScreenshot(const std::string& filename) const;
    
    // Window and OpenGL state
    GLFWwindow* m_window;
    bool m_running;
    
    // Resources
    std::unique_ptr<Texture> m_pixelTexture;     // Color texture (RGBA)
    std::unique_ptr<Texture> m_lightTexture;     // Light properties texture (RGB)
    std::unique_ptr<Texture> m_renderTexture;
    std::unique_ptr<Texture> m_computeOutputTexture; // Texture for compute shader output
    std::unique_ptr<Shader> m_computeShader;     // Compute shader for ray tracing
    std::unique_ptr<Shader> m_displayShader;     // Simple display shader
    std::unique_ptr<PixelBuffer> m_pixelBuffer;
    std::unique_ptr<Camera> m_camera;
    
    // Rendering state
    bool m_forceRedraw;
    bool m_renderEnabled;
    int m_renderCount;
    bool m_pixelsDirty;
    bool m_manualRenderMode;  // Similar to RENDER_MANUAL in original code
    
    // OpenGL objects for fullscreen quad
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_ebo;
    
    // Mouse state
    double m_mouseX;
    double m_mouseY;
    double m_prevMouseX;
    double m_prevMouseY;
    bool m_leftMousePressed;
    bool m_rightMousePressed;
    
    // Pen expression
    std::string m_penExpression;
    bool m_expressionInputRequested;

    // Timing
    double m_lastTime;
    double m_deltaTime;
    
    // Prevent copying
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
};