#include "../include/renderer.h"
#include "../include/texture.h"
#include "../include/shader.h"
#include "../include/pixel_buffer.h"
#include "../include/pixel.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>

// Static callback functions
static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer && key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

Renderer::Renderer()
    : m_window(nullptr)
    , m_running(false)
    , m_forceRedraw(true)
    , m_renderEnabled(true)
    , m_renderCount(0)
    , m_pixelsDirty(false)
    , m_manualRenderMode(true)  // Enable manual render mode by default (like RENDER_MANUAL)
    , m_lastTime(0.0)
    , m_deltaTime(0.0)
    , m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
    , m_mouseX(0.0)
    , m_mouseY(0.0)
    , m_prevMouseX(0.0)
    , m_prevMouseY(0.0)
    , m_leftMousePressed(false)
    , m_rightMousePressed(false)
    , m_penExpression("~ x 1 y 1 x 1 y")  // Default expression from original code
    , m_expressionInputRequested(false) {
}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize() {
    // Initialize GLFW
    if (!initGLFW()) {
        return false;
    }
    
    // Initialize GLEW (OpenGL function loader)
    if (!initGLEW()) {
        return false;
    }
    
    // Initialize resources
    if (!initTextures() || !initShaders() || !initBuffers()) {
        return false;
    }
    
    // Set up initial state
    m_running = true;
    m_lastTime = glfwGetTime();
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    return true;
}

bool Renderer::initGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Configure GLFW - use compatibility profile for immediate mode OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    // Create window
    m_window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Pixel Renderer (GPU Accelerated)", NULL, NULL);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_window));
    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_window), this);
    glfwSetKeyCallback(static_cast<GLFWwindow*>(m_window), glfwKeyCallback);
    
    // Enable vsync
    glfwSwapInterval(1);
    
    return true;
}

bool Renderer::initGLEW() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    
    // Check for required OpenGL features
    if (!GLEW_VERSION_3_3) {
        std::cerr << "OpenGL 3.3 is required but not supported" << std::endl;
        return false;
    }
    
    return true;
}

bool Renderer::initTextures() {
    // Create pixel texture (RGBA32F for pixel data)
    m_pixelTexture = std::make_unique<Texture>();
    if (!m_pixelTexture->create(CANVAS_WIDTH, CANVAS_HEIGHT, Texture::Format::RGBA32F, Texture::Filter::Nearest)) {
        std::cerr << "Failed to create pixel texture" << std::endl;
        return false;
    }
    
    // Create light texture (RGBA32F for light properties)
    m_lightTexture = std::make_unique<Texture>();
    if (!m_lightTexture->create(CANVAS_WIDTH, CANVAS_HEIGHT, Texture::Format::RGBA32F, Texture::Filter::Nearest)) {
        std::cerr << "Failed to create light texture" << std::endl;
        return false;
    }
    
    // Create compute output texture (for ray tracing results at canvas resolution)
    m_computeOutputTexture = std::make_unique<Texture>();
    if (!m_computeOutputTexture->create(CANVAS_WIDTH, CANVAS_HEIGHT, Texture::Format::RGBA8, Texture::Filter::Nearest)) {
        std::cerr << "Failed to create compute output texture" << std::endl;
        return false;
    }
    
    // Create render texture (for display - can be same as compute output or separate)
    m_renderTexture = std::make_unique<Texture>();
    if (!m_renderTexture->create(SCREEN_WIDTH, SCREEN_HEIGHT, Texture::Format::RGBA8, Texture::Filter::Nearest)) {
        std::cerr << "Failed to create render texture" << std::endl;
        return false;
    }
    
    return true;
}

bool Renderer::initShaders() {
    // Load compute shader for ray tracing (runs at canvas resolution)
    m_computeShader = std::make_unique<Shader>();
    if (!m_computeShader->loadFromFiles("shaders/raytrace.comp")) {
        std::cerr << "Failed to load compute shader" << std::endl;
        return false;
    }
    
    // Load display shader (simple texture display)
    m_displayShader = std::make_unique<Shader>();
    if (!m_displayShader->loadFromFiles("shaders/display.vert", "shaders/display.frag")) {
        std::cerr << "Failed to load display shader" << std::endl;
        return false;
    }
    
    // Set OpenGL state for optimal performance
    // Disable depth test (we're doing 2D rendering)
    glDisable(GL_DEPTH_TEST);
    
    // Disable blending (we're using opaque rendering with alpha in shader)
    glDisable(GL_BLEND);
    
    // Enable backface culling (optional, but can improve performance)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Set clear color once (can be overridden in render() if needed)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    return true;
}

bool Renderer::initBuffers() {
    // Create pixel buffer
    m_pixelBuffer = std::make_unique<PixelBuffer>();
    m_pixelBuffer->initialize();
    

    
    // Update textures with initial pixel data
    m_pixelBuffer->updateTextures(*m_pixelTexture, *m_lightTexture);
    
    // Initialize camera
    m_camera = std::make_unique<Camera>(0.5, 0.5, 1.0);
    
    // Create fullscreen quad VAO
    float vertices[] = {
        // positions   // texture coords
        -1.0f,  1.0f,  0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,  0.0f, 0.0f,  // bottom-left
         1.0f, -1.0f,  1.0f, 0.0f,  // bottom-right
         1.0f,  1.0f,  1.0f, 1.0f   // top-right
    };
    
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    return true;
}

void Renderer::run() {
    // Frame rate tracking
    double lastFpsTime = glfwGetTime();
    int frameCount = 0;
    double fps = 0.0;
    
    while (m_running && !glfwWindowShouldClose(static_cast<GLFWwindow*>(m_window))) {
        // Update timing
        double currentTime = glfwGetTime();
        m_deltaTime = currentTime - m_lastTime;
        m_lastTime = currentTime;
        
        // Update frame rate calculation
        frameCount++;
        if (currentTime - lastFpsTime >= 1.0) {
            fps = frameCount / (currentTime - lastFpsTime);
            frameCount = 0;
            lastFpsTime = currentTime;
            
            // Update window title with FPS
            std::string title = "2D Pixel Renderer (GPU Accelerated) - FPS: " + 
                               std::to_string(static_cast<int>(fps)) + 
                               " - " + std::to_string(static_cast<int>(1000.0 / fps)) + "ms";
            glfwSetWindowTitle(static_cast<GLFWwindow*>(m_window), title.c_str());
            
            // Also output to console for debugging
            std::cout << "FPS: " << static_cast<int>(fps) << " (" << static_cast<int>(1000.0 / fps) << "ms)" << std::endl;
        }
        
        // Handle input
        handleInput();
        
        // Handle expression input if requested
        if (m_expressionInputRequested) {
            m_expressionInputRequested = false;
            
            // Switch to console for input
            std::cout << "\n=== Enter new pen expression ===" << std::endl;
            std::cout << "Current: " << m_penExpression << std::endl;
            std::cout << "Syntax: Use variables x, y. Example: ~ x 1 y 1 x 1 y" << std::endl;
            std::cout << "Enter expression: ";
            
            std::string newExpression;
            std::getline(std::cin, newExpression);
            
            // Trim whitespace
            size_t start = newExpression.find_first_not_of(" \t\n\r");
            size_t end = newExpression.find_last_not_of(" \t\n\r");
            if (start != std::string::npos && end != std::string::npos) {
                newExpression = newExpression.substr(start, end - start + 1);
            }
            
            if (!newExpression.empty()) {
                m_penExpression = newExpression;
                std::cout << "Expression updated to: " << m_penExpression << std::endl;
                
                // Mark pixels as dirty to force redraw with new expression
                m_pixelsDirty = true;
                m_forceRedraw = true;
            } else {
                std::cout << "Expression unchanged." << std::endl;
            }
        }
        
        // Update state
        update();
        
        // Render frame
        render();
        
        // Swap buffers and poll events
        glfwSwapBuffers(static_cast<GLFWwindow*>(m_window));
        glfwPollEvents();
    }
}

void Renderer::handleInput() {
    if (!m_window || !m_camera) return;
    
    GLFWwindow* window = static_cast<GLFWwindow*>(m_window);
    
    // Get window and framebuffer sizes for HiDPI scaling
    int windowWidth, windowHeight;
    int fbWidth, fbHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    float scaleX = (windowWidth > 0) ? static_cast<float>(fbWidth) / windowWidth : 1.0f;
    float scaleY = (windowHeight > 0) ? static_cast<float>(fbHeight) / windowHeight : 1.0f;
    
    // Update mouse state with scaling
    glfwGetCursorPos(window, &m_mouseX, &m_mouseY);
    float scaledMouseX = m_mouseX * scaleX;
    float scaledMouseY = m_mouseY * scaleY;
    
    // Map scaled coordinates to SCREEN_WIDTH/SCREEN_HEIGHT space for screenToMem
    float screenMouseX = (fbWidth > 0) ? scaledMouseX * SCREEN_WIDTH / fbWidth : scaledMouseX;
    float screenMouseY = (fbHeight > 0) ? scaledMouseY * SCREEN_HEIGHT / fbHeight : scaledMouseY;
    
    // Store scaled coordinates (in SCREEN_WIDTH/SCREEN_HEIGHT space)
    m_mouseX = screenMouseX;
    m_mouseY = screenMouseY;
    
    bool leftPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    
    // Handle mouse drawing
    if (leftPressed) {
        if (m_leftMousePressed) {
            // Left mouse held down, draw line from previous to current position
            CoordI prevCanvas = screenToMem(CoordI(static_cast<int>(m_prevMouseX + 0.5), static_cast<int>(m_prevMouseY + 0.5)), *m_camera);
            CoordI currCanvas = screenToMem(CoordI(static_cast<int>(m_mouseX + 0.5), static_cast<int>(m_mouseY + 0.5)), *m_camera);
            
            // Use expression-based pen color
            Pixel pen = evaluatePenAtCanvas(prevCanvas);
            m_pixelBuffer->drawLine(prevCanvas, currCanvas, pen);
            m_pixelsDirty = true;
            m_forceRedraw = true;
        } else {
            // First press, draw a single point
            CoordI canvasPos = screenToMem(CoordI(static_cast<int>(m_mouseX + 0.5), static_cast<int>(m_mouseY + 0.5)), *m_camera);
#if DEBUG_OUTPUT
            std::cout << "Left click at screen (" << m_mouseX << ", " << m_mouseY << ") -> canvas (" << canvasPos.x << ", " << canvasPos.y << ")" << std::endl;
#endif
            Pixel pen = evaluatePenAtCanvas(canvasPos);
            m_pixelBuffer->setPixel(canvasPos, pen);
            m_pixelsDirty = true;
            m_forceRedraw = true;
        }
    }
    
    if (rightPressed) {
        if (m_rightMousePressed) {
            // Right mouse held down, erase (set transparent)
            CoordI prevCanvas = screenToMem(CoordI(static_cast<int>(m_prevMouseX + 0.5), static_cast<int>(m_prevMouseY + 0.5)), *m_camera);
            CoordI currCanvas = screenToMem(CoordI(static_cast<int>(m_mouseX + 0.5), static_cast<int>(m_mouseY + 0.5)), *m_camera);
            
            Pixel erase(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            m_pixelBuffer->drawLine(prevCanvas, currCanvas, erase);
            m_pixelsDirty = true;
            m_forceRedraw = true;
        } else {
            // First press, erase a single point
            CoordI canvasPos = screenToMem(CoordI(static_cast<int>(m_mouseX + 0.5), static_cast<int>(m_mouseY + 0.5)), *m_camera);
#if DEBUG_OUTPUT
            std::cout << "Right click at screen (" << m_mouseX << ", " << m_mouseY << ") -> canvas (" << canvasPos.x << ", " << canvasPos.y << ")" << std::endl;
#endif
            Pixel erase(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            m_pixelBuffer->setPixel(canvasPos, erase);
            m_pixelsDirty = true;
            m_forceRedraw = true;
        }
    }
    
    // Update previous mouse state
    m_leftMousePressed = leftPressed;
    m_rightMousePressed = rightPressed;
    m_prevMouseX = m_mouseX;
    m_prevMouseY = m_mouseY;
    
    // Keyboard camera controls
    bool controlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                          glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
    
    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        m_camera->x -= 0.02 * m_camera->scale;
        m_forceRedraw = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        m_camera->x += 0.02 * m_camera->scale;
        m_forceRedraw = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (controlPressed) {
            // Zoom in
            m_camera->scale /= 1.03;
            m_forceRedraw = true;
        } else {
            m_camera->y -= 0.02 * m_camera->scale;
            m_forceRedraw = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (controlPressed) {
            // Zoom out
            m_camera->scale *= 1.03;
            m_forceRedraw = true;
        } else {
            m_camera->y += 0.02 * m_camera->scale;
            m_forceRedraw = true;
        }
    }
    
    // Reset camera
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && controlPressed) {
        m_camera->x = 0.5;
        m_camera->y = 0.5;
        m_camera->scale = 1.0;
        m_forceRedraw = true;
    }
    
    // Additional controls from original code
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        // Reset rendering state (force full redraw)
        m_forceRedraw = true;
#if DEBUG_OUTPUT
        std::cout << "Rendering state reset" << std::endl;
#endif
    }
    
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        // Clear canvas (reset to transparent)
        if (m_pixelBuffer) {
            m_pixelBuffer->clear();
            m_pixelsDirty = true;
            m_forceRedraw = true;
#if DEBUG_OUTPUT
            std::cout << "Canvas cleared" << std::endl;
#endif
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        // Save screenshot with timestamp
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << "screenshot_" 
           << std::put_time(std::localtime(&now_time_t), "%Y%m%d_%H%M%S")
           << "_" << std::setfill('0') << std::setw(3) << now_ms.count()
           << ".bmp";
        
        std::string filename = ss.str();
        
        if (saveScreenshot(filename)) {
#if DEBUG_OUTPUT
            std::cout << "Screenshot saved to: " << filename << std::endl;
#endif
        } else {
#if DEBUG_OUTPUT
            std::cerr << "Failed to save screenshot" << std::endl;
#endif
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        // Request expression input
        if (!m_expressionInputRequested) {
            m_expressionInputRequested = true;
#if DEBUG_OUTPUT
            std::cout << "Expression input requested. Please enter new expression in console." << std::endl;
            std::cout << "Current expression: " << m_penExpression << std::endl;
#endif
        }
    }
}

void Renderer::update() {
    // Update pixel texture if dirty (do this before rendering to avoid tearing)
    if (m_pixelsDirty && m_pixelBuffer && m_pixelTexture && m_lightTexture) {
        m_pixelBuffer->updateTextures(*m_pixelTexture, *m_lightTexture);
        m_pixelsDirty = false;
        
        // Mark that we need to redraw because pixels changed
        m_forceRedraw = true;
    }
    
    // No longer forcing redraw every frame
    // Only redraw when m_forceRedraw is set by input or pixel changes
}

void Renderer::render() {

    // Set viewport to match window size
    int width, height;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_window), &width, &height);
    glViewport(0, 0, width, height);
    
    // Clear screen (color already set in initShaders)
    // Temporarily disabled to test flickering issue
    // glClear(GL_COLOR_BUFFER_BIT);
    
    // Step 1: Run compute shader to perform ray tracing at canvas resolution
    if (m_forceRedraw && m_computeShader && m_computeOutputTexture && m_pixelTexture && m_lightTexture) {
        // Clear compute output texture to black before writing new results
        // This ensures no stale data from previous frames causes flickering
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glClearTexImage(m_computeOutputTexture->getId(), 0, GL_RGBA, GL_FLOAT, clearColor);
        
        m_computeShader->bind();
        
        // Set compute shader uniforms
        m_computeShader->setUniformVec2("canvasSize", static_cast<float>(CANVAS_WIDTH), static_cast<float>(CANVAS_HEIGHT));
        m_computeShader->setUniformVec2("sunDirection", 0.707f, 0.707f);
        m_computeShader->setUniformFloat("sunScattering", 5e-3f);
        m_computeShader->setUniformVec4("sunColor", 1.0f, 1.0f, 1.0f, 1.0f);  // Sunlight disabled for now
        m_computeShader->setUniformVec4("wallColor", 0.2f, 0.2f, 0.2f, 1.0f);
        
        // Note: Compute shader works in canvas pixel coordinates, not screen coordinates
        // Camera transformation is handled in the display shader, not the compute shader
        
        // Bind input textures to compute shader
        m_pixelTexture->bind(0);
        m_computeShader->setUniformTexture("pixelTexture", m_pixelTexture->getId(), 0);
        
        m_lightTexture->bind(1);
        m_computeShader->setUniformTexture("lightTexture", m_lightTexture->getId(), 1);
        
        // Bind compute output texture as image
        m_computeOutputTexture->bindAsImage(0, GL_WRITE_ONLY);
        
        // Dispatch compute shader (work groups of 16x16 threads)
        int workGroupsX = (CANVAS_WIDTH + 15) / 16;
        int workGroupsY = (CANVAS_HEIGHT + 15) / 16;
        glDispatchCompute(workGroupsX, workGroupsY, 1);
        
        // Memory barrier to ensure compute shader completes before fragment shader reads
        // Use all barriers to guarantee synchronization between compute and fragment stages
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        
        m_computeShader->unbind();
    }
    
    // Step 2: Display the computed texture
    // Always display to prevent flickering between frames
    // The compute output texture retains its content between frames
    if (m_displayShader && m_computeOutputTexture) {
        m_displayShader->bind();
        
        // Set camera uniforms for proper coordinate transformation
        if (m_camera) {
            m_displayShader->setUniformVec2("cameraPos", static_cast<float>(m_camera->x), static_cast<float>(m_camera->y));
            m_displayShader->setUniformFloat("cameraScale", static_cast<float>(m_camera->scale));
        }
        
        // Bind computed texture to display shader
        m_computeOutputTexture->bind(0);
        m_displayShader->setUniformTexture("computedTexture", m_computeOutputTexture->getId(), 0);
        
        // Render fullscreen quad using VAO
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        m_displayShader->unbind();
    }
    

    m_forceRedraw = false;
}

void Renderer::shutdown() {
    // Cleanup OpenGL objects
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
    
    // Cleanup resources
    m_pixelBuffer.reset();
    m_displayShader.reset();
    m_computeShader.reset();
    m_computeOutputTexture.reset();
    m_renderTexture.reset();
    m_pixelTexture.reset();
    m_lightTexture.reset();
    
    // Destroy window and terminate GLFW
    if (m_window) {
        glfwDestroyWindow(static_cast<GLFWwindow*>(m_window));
        m_window = nullptr;
    }
    glfwTerminate();
    
    m_running = false;
}

Pixel Renderer::evaluatePenAtCanvas(const CoordI& canvasCoord) const {
    // Convert canvas coordinates (0..CANVAS_WIDTH-1, 0..CANVAS_HEIGHT-1) to absolute coordinates (0..1)
    double absX = static_cast<double>(canvasCoord.x) / CANVAS_WIDTH;
    double absY = static_cast<double>(canvasCoord.y) / CANVAS_HEIGHT;
    
    Point2D point(absX, absY);
    PenResult result = evaluatePen(m_penExpression, point);
    
    // Convert to Pixel structure
    return Pixel(
        result.r, result.g, result.b, result.a,
        result.lr, result.lg, result.lb
    );
}

bool Renderer::saveScreenshot(const std::string& filename) const {
    if (!m_window) return false;
    
    GLFWwindow* window = static_cast<GLFWwindow*>(m_window);
    
    // Get framebuffer size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    if (width <= 0 || height <= 0) return false;
    
    // Allocate buffer for pixel data (RGBA)
    std::vector<unsigned char> pixels(width * height * 4);
    
    // Read pixels from framebuffer
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error reading pixels: " << error << std::endl;
        return false;
    }
    
    // Create BMP file
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    // BMP file header (14 bytes)
    unsigned int fileSize = 54 + width * height * 3; // 54 bytes header + 3 bytes per pixel (BGR)
    unsigned char fileHeader[14] = {
        'B', 'M',           // Signature
        static_cast<unsigned char>(fileSize), static_cast<unsigned char>(fileSize >> 8),
        static_cast<unsigned char>(fileSize >> 16), static_cast<unsigned char>(fileSize >> 24),
        0, 0,               // Reserved
        0, 0,               // Reserved
        54, 0, 0, 0         // Offset to pixel data
    };
    
    // BMP info header (40 bytes)
    unsigned int infoHeaderSize = 40;
    unsigned short planes = 1;
    unsigned short bitCount = 24; // 24-bit BGR
    unsigned int compression = 0; // No compression
    unsigned int imageSize = width * height * 3;
    
    unsigned char infoHeader[40] = {
        static_cast<unsigned char>(infoHeaderSize), 0, 0, 0, // Header size
        static_cast<unsigned char>(width), static_cast<unsigned char>(width >> 8),
        static_cast<unsigned char>(width >> 16), static_cast<unsigned char>(width >> 24),
        static_cast<unsigned char>(height), static_cast<unsigned char>(height >> 8),
        static_cast<unsigned char>(height >> 16), static_cast<unsigned char>(height >> 24),
        static_cast<unsigned char>(planes), 0, // Planes
        static_cast<unsigned char>(bitCount), 0, // Bits per pixel
        static_cast<unsigned char>(compression), 0, 0, 0, // Compression
        static_cast<unsigned char>(imageSize), static_cast<unsigned char>(imageSize >> 8),
        static_cast<unsigned char>(imageSize >> 16), static_cast<unsigned char>(imageSize >> 24),
        0, 0, 0, 0, // Horizontal resolution (pixels per meter)
        0, 0, 0, 0, // Vertical resolution
        0, 0, 0, 0, // Colors used
        0, 0, 0, 0  // Important colors
    };
    
    // Write headers
    file.write(reinterpret_cast<char*>(fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<char*>(infoHeader), sizeof(infoHeader));
    
    // Convert RGBA to BGR and write pixel data (BMP is bottom-to-top, same as OpenGL)
    // Also need to flip vertically because OpenGL origin is bottom-left
    std::vector<unsigned char> bgrData(width * 3);
    for (int y = 0; y < height; ++y) {
        int srcRow = (height - 1 - y) * width * 4; // Flip vertically
        
        for (int x = 0; x < width; ++x) {
            int srcPos = srcRow + x * 4;
            int dstPos = x * 3;
            
            // Convert RGBA to BGR (ignore alpha)
            bgrData[dstPos + 2] = pixels[srcPos];     // Red
            bgrData[dstPos + 1] = pixels[srcPos + 1]; // Green
            bgrData[dstPos] = pixels[srcPos + 2];     // Blue
        }
        
        // Write row (pad rows to multiple of 4 bytes)
        file.write(reinterpret_cast<char*>(bgrData.data()), width * 3);
        
        // Add padding if needed
        int padding = (4 - (width * 3) % 4) % 4;
        if (padding > 0) {
            unsigned char pad[3] = {0, 0, 0};
            file.write(reinterpret_cast<char*>(pad), padding);
        }
    }
    
    file.close();
    
    std::cout << "Screenshot saved to: " << filename << std::endl;
    return true;
}
