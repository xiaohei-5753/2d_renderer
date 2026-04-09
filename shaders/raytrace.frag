#version 330 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D pixelTexture;    // Color texture (RGBA)
uniform sampler2D lightTexture;    // Light properties texture (RGB)
uniform vec2 canvasSize;           // Canvas dimensions (128x128)
uniform vec2 sunDirection;         // Sunlight direction (normalized)
uniform float sunScattering;       // Sunlight scattering degree (5e-3)
uniform vec4 sunColor;             // Sunlight color (light properties * 10)
uniform vec4 wallColor;            // Wall color (light properties 0.2)
uniform vec2 cameraPos;            // Camera position in absolute coords (0..1)
uniform float cameraScale;         // Camera zoom scale

// Constants matching original code
const float EPS = 1e-6;
const float EPS_LIGHT = 1e-3;

// Performance optimization constants
const int MAX_RADIUS = 128;  // max(canvasSize.x, canvasSize.y)
const int MAX_RAYS = 8;      // Number of symmetric directions (8-fold symmetry)

// Quality adjustment based on pixel density
// When many screen pixels map to same canvas pixel, we can reduce quality
int getQualityAdjustedRadius(vec2 pixelCoord) {
    // Compute how much pixelCoord changes between adjacent fragments
    float coordChange = fwidth(pixelCoord.x) + fwidth(pixelCoord.y);
    
    // If coordChange is small, many screen pixels map to same canvas pixel
    if (coordChange < 0.3) {
        return 64;  // Lower quality for dense mapping
    } else if (coordChange < 0.7) {
        return 96;  // Medium quality
    } else {
        return MAX_RADIUS;  // Full quality
    }
}

int getQualityAdjustedRayCount(vec2 pixelCoord) {
    // Compute how much pixelCoord changes between adjacent fragments
    float coordChange = fwidth(pixelCoord.x) + fwidth(pixelCoord.y);
    
    // If coordChange is small, many screen pixels map to same canvas pixel
    if (coordChange < 0.3) {
        return 4;  // 4 directions instead of 8
    } else {
        return MAX_RAYS;  // Full 8 directions
    }
}

// Convert texture coordinate to pixel coordinate with camera transformation
vec2 screenTexToCanvasPixel(vec2 screenTexCoord) {
    // screenTexCoord is normalized screen coordinates (0 to 1)
    // Convert to absolute coordinates (0..1) using camera transformation
    float absX = (screenTexCoord.x - 0.5) * cameraScale + cameraPos.x;
    float absY = (0.5 - screenTexCoord.y) * cameraScale + cameraPos.y;
    
    // Convert absolute coordinates (0..1) to canvas pixel coordinates
    return vec2(absX * canvasSize.x, absY * canvasSize.y);
}

// Sample pixel color with nearest filtering
vec4 sampleColor(vec2 pixelCoord) {
    // Convert to integer pixel coordinates (nearest pixel)
    vec2 pixelCoordInt = floor(pixelCoord + 0.5);
    // Clamp to canvas bounds
    pixelCoordInt = clamp(pixelCoordInt, vec2(0.0), canvasSize - vec2(1.0));
    // Convert to texture coordinates with pixel-center alignment
    vec2 texCoord = (pixelCoordInt + 0.5) / canvasSize;
    return texture(pixelTexture, texCoord);
}

// Sample light properties with nearest filtering
vec3 sampleLight(vec2 pixelCoord) {
    // Convert to integer pixel coordinates (nearest pixel)
    vec2 pixelCoordInt = floor(pixelCoord + 0.5);
    // Clamp to canvas bounds
    pixelCoordInt = clamp(pixelCoordInt, vec2(0.0), canvasSize - vec2(1.0));
    // Convert to texture coordinates with pixel-center alignment
    vec2 texCoord = (pixelCoordInt + 0.5) / canvasSize;
    return texture(lightTexture, texCoord).rgb;
}

// Ray tracing function (matching original ray() function)
// Returns light color (lr, lg, lb) that reaches origin from given direction
vec3 traceRay(vec2 origin, ivec2 direction) {
    // Normalize direction (matching original: l = R, dr = {a.x/l, a.y/l})
    float l = float(MAX_RADIUS);
    vec2 dr = vec2(direction) / l;
    
    // Handle all quadrants using octant transformation
    ivec2 a = direction;
    int transform = 0;
    
    if (a.y < 0) {
        a.y = -a.y;
        transform |= 1; // bit 0: flip y
    }
    if (a.x < 0) {
        a.x = -a.x;
        transform |= 2; // bit 1: flip x
    }
    if (a.y > a.x) {
        int temp = a.x;
        a.x = a.y;
        a.y = temp;
        transform |= 4; // bit 2: swap x and y
    }
    
    // Bresenham's line algorithm for first octant
    int m = 0;
    int y = 0;
    
    // Initial light (white)
    vec3 light = vec3(1.0, 1.0, 1.0);
    
    for (int x = 0; x < MAX_RADIUS * 2; x++) { // Safety limit
        // Apply inverse transformation to get actual pixel
        ivec2 p = ivec2(x, y);
        
        if ((transform & 4) != 0) {  // bit 2: swap x and y
            int temp = p.x;
            p.x = p.y;
            p.y = temp;
        }
        if ((transform & 2) != 0) {  // bit 1: flip x
            p.x = -p.x;
        }
        if ((transform & 1) != 0) {  // bit 0: flip y
            p.y = -p.y;
        }
        
        // Add origin offset
        p = p + ivec2(origin);
        
        // Check bounds
        if (p.x < 0 || p.x >= int(canvasSize.x) || p.y < 0 || p.y >= int(canvasSize.y)) {
            // Hit wall or sky
            float dotp = 1.0 - (sunDirection.x * dr.x + sunDirection.y * dr.y);
            if (dotp <= sunScattering) {
                // Sunlight
                light *= sunColor.rgb;
            } else {
                // Wall light
                light *= wallColor.rgb;
            }
            return light;
        }
        
        // Check if light is extinguished
        if (light.r < EPS_LIGHT && light.g < EPS_LIGHT && light.b < EPS_LIGHT) {
            return light;
        }
        
        // Sample pixel at this position
        vec4 color = sampleColor(vec2(p));
        vec3 pixelLight = sampleLight(vec2(p));
        
        // Check if pixel is opaque (alpha close to 1)
        if (1.0 - color.a < EPS_LIGHT) {
            // Opaque pixel - absorb light according to pixel's light properties
            light *= pixelLight;
            return light;
        }
        
        // Semi-transparent pixel - light passes through (continue)
        // In original: light is attenuated by (1-alpha)*color, but commented out
        // For now, just continue
        
        // Bresenham step
        m += a.y;
        if (m >= a.x) {
            y++;
            m -= a.x;
        }
        
        // Safety break
        if (x > MAX_RADIUS * 2) break;
    }
    
    // Should not reach here
    return vec3(0.0, 0.0, 0.0);
}

// Sample pixel with ray tracing (matching original sample_pix function)
vec3 samplePixelWithRays(vec2 pixelCoord) {
    // Convert to integer canvas coordinates
    ivec2 p = ivec2(floor(pixelCoord + 0.5));
    
    // Clamp to canvas bounds
    p.x = int(clamp(float(p.x), 0.0, canvasSize.x - 1.0));
    p.y = int(clamp(float(p.y), 0.0, canvasSize.y - 1.0));
    
    // Sample pixel color and alpha
    vec4 color = sampleColor(vec2(p));
    
    // If pixel is almost opaque, return its color directly
    if (1.0 - color.a < EPS_LIGHT) {
        return color.rgb;
    }
    
    // Otherwise, trace rays in all directions (8-fold symmetry)
    vec3 result = vec3(0.0, 0.0, 0.0);
    int radius = MAX_RADIUS;
    
    // Bresenham's circle algorithm for first octant
    int m = 1 - radius;
    ivec2 i = ivec2(0, radius);
    int steps = 0;
    
    // Loop over all points in first octant
    // Using fixed loop limit for GLSL compatibility
    for (int loop = 0; loop < MAX_RADIUS * 2; loop++) {
        if (i.x >= i.y) break;
        
        // 8 symmetric directions for this point
        ivec2 dirs[8];
        dirs[0] = ivec2(i.x, i.y);
        dirs[1] = ivec2(-i.x, i.y);
        dirs[2] = ivec2(i.x, -i.y);
        dirs[3] = ivec2(-i.x, -i.y);
        dirs[4] = ivec2(i.y, i.x);
        dirs[5] = ivec2(-i.y, i.x);
        dirs[6] = ivec2(i.y, -i.x);
        dirs[7] = ivec2(-i.y, -i.x);
        
        // Trace each direction
        for (int d = 0; d < 8; d++) {
            result += traceRay(p, dirs[d]);
        }
        
        // Bresenham step (matching original algorithm)
        i.x++;
        if (m < 0) {
            m += 2 * i.x + 1;
        } else {
            i.y--;
            m += 2 * (i.x - i.y) + 1;
        }
        
        steps++;
    }
    
    // Average the results (8 rays per step)
    if (steps > 0) {
        result /= float(8 * steps);
    }
    
    // Clamp to non-negative
    result = max(result, vec3(0.0));
    
    // Normalize if any component exceeds 1.0
    float maxComponent = max(max(result.r, result.g), result.b);
    if (maxComponent > 1.0) {
        result /= maxComponent;
    }
    
    return result;
}

void main() {
    // Get canvas pixel coordinate for this screen fragment
    vec2 pixelCoord = screenTexToCanvasPixel(vTexCoord);
    
    // Perform ray traced sampling
    vec3 color = samplePixelWithRays(pixelCoord);
    
    // Ensure uniforms are not optimized out by using them in a negligible way
    // This adds an extremely small value that won't affect visual output
    float uniformGuard = 0.0;
    uniformGuard += sunDirection.x * 0.0000001;
    uniformGuard += sunDirection.y * 0.0000001;
    uniformGuard += sunScattering * 0.0000001;
    uniformGuard += sunColor.r * 0.0000001;
    uniformGuard += sunColor.g * 0.0000001;
    uniformGuard += sunColor.b * 0.0000001;
    uniformGuard += sunColor.a * 0.0000001;
    uniformGuard += wallColor.r * 0.0000001;
    uniformGuard += wallColor.g * 0.0000001;
    uniformGuard += wallColor.b * 0.0000001;
    uniformGuard += wallColor.a * 0.0000001;
    
    // Add the negligible guard value to each color channel
    color += vec3(uniformGuard);
    
    // Output final color
    fragColor = vec4(color, 1.0);
}