#include "../include/renderer.h"
#include <iostream>

int main() {
    std::cout << "GPU-Accelerated 2D Pixel Renderer" << std::endl;
    std::cout << "=================================" << std::endl;
    
    Renderer renderer;
    
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        return 1;
    }
    
    std::cout << "Renderer initialized successfully." << std::endl;
    std::cout << "Canvas: " << renderer.getCanvasWidth() << "x" << renderer.getCanvasHeight() << std::endl;
    std::cout << "Screen: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    std::cout << "Press ESC to exit." << std::endl;
    
    renderer.run();
    renderer.shutdown();
    
    return 0;
}