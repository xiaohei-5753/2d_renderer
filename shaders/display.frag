#version 330 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D computedTexture; // Texture from compute shader (128x128)
uniform vec2 cameraPos;            // Camera position in absolute coords (0..1)
uniform float cameraScale;         // Camera zoom scale

// Convert texture coordinate to canvas pixel coordinate with camera transformation
vec2 screenTexToCanvasPixel(vec2 screenTexCoord) {
    // screenTexCoord is normalized screen coordinates (0 to 1)
    // Convert to absolute coordinates (0..1) using camera transformation
    float absX = (screenTexCoord.x - 0.5) * cameraScale + cameraPos.x;
    float absY = (0.5 - screenTexCoord.y) * cameraScale + cameraPos.y;
    
    // Convert absolute coordinates (0..1) to canvas texture coordinates
    return vec2(absX, absY);
}

void main() {
    // Convert screen coordinate to canvas coordinate
    vec2 canvasCoord = screenTexToCanvasPixel(vTexCoord);
    
    // Clamp to valid range (0..1)
    canvasCoord = clamp(canvasCoord, 0.0, 1.0);
    
    // Sample computed texture (which is at canvas resolution)
    fragColor = texture(computedTexture, canvasCoord);
}