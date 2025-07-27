#include "renderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int WindowInit(SDL_Window **window, SDL_Renderer **rend, int width, int height) {
    // Initialize SDL subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window
    *window = SDL_CreateWindow("SDL3 C Project", width, height, 0);
    if (!*window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create SDL renderer
    *rend = SDL_CreateRenderer(*window, NULL);
    if (!*rend) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize true type font subsystem
    TTF_Init();

    return 0;
}

// Rasterizes a triangle on screen with depth buffering and colour
void DrawTriangle(SDL_Renderer *ren, Triangle tri, 
        Mat4 mvp, int screen_width, int screen_height, Vec4 colour, float *zbuffer, uint32_t *pixelBuffer) {
    // Transform vertices to clip space
    Vec4 p0 = mat4_mul_vec4(mvp, vec4_from_vec3(tri.v0.pos, 1.0f));
    Vec4 p1 = mat4_mul_vec4(mvp, vec4_from_vec3(tri.v1.pos, 1.0f));
    Vec4 p2 = mat4_mul_vec4(mvp, vec4_from_vec3(tri.v2.pos, 1.0f));

    // Perform backface culling: skip any triangle if the vertex is behind the camera
    if (p0.w >= 0.0f || p1.w >= 0.0f || p2.w >= 0.0f) return;

    // Perspective divide to get normalized device coordinates
    p0 = vec4_scale(p0, 1.0f / p0.w);
    p1 = vec4_scale(p1, 1.0f / p1.w);
    p2 = vec4_scale(p2, 1.0f / p2.w);

    // Convert normalized device coords to screen space
    Vec2 s0 = { (p0.x + 1.0f) * 0.5f * screen_width, (1.0f - p0.y) * 0.5f * screen_height };
    Vec2 s1 = { (p1.x + 1.0f) * 0.5f * screen_width, (1.0f - p1.y) * 0.5f * screen_height };
    Vec2 s2 = { (p2.x + 1.0f) * 0.5f * screen_width, (1.0f - p2.y) * 0.5f * screen_height };

    // Compute bounding box for triangle in screen space
    int min_x = (int)fmaxf(0.0f, floorf(fminf(fminf(s0.x, s1.x), s2.x)));
    int max_x = (int)fminf(screen_width - 1, ceilf(fmaxf(fmaxf(s0.x, s1.x), s2.x)));
    int min_y = (int)fmaxf(0.0f, floorf(fminf(fminf(s0.y, s1.y), s2.y)));
    int max_y = (int)fminf(screen_height - 1, ceilf(fmaxf(fmaxf(s0.y, s1.y), s2.y)));
    
    // Calculate twice the area of the triangle for barycentric coords calculation
    float area = (s1.x - s0.x) * (s2.y - s0.y) - (s1.y - s0.y) * (s2.x - s0.x);
    if (area == 0.0f) return;

    // Precompute depth values (in [0, 1])
    float depth0 = (p0.z + 1.0f) * 0.5f;
    float depth1 = (p1.z + 1.0f) * 0.5f;
    float depth2 = (p2.z + 1.0f) * 0.5f;
    
    // Loop over each pixel in the bounding box to rasterize the triangle
    for (int y = min_y; y <= max_y; y++) {
        float *zrow = zbuffer + y * screen_width; // Row pointer for zbuffer optimization
        for (int x = min_x; x <= max_x; x++) {
            int pixelIndex = y * screen_width + x;

            // Pixel center coordinates for barycentric calculation
            float px = (float)x + 0.5f;
            float py = (float)y + 0.5f;

            // Compute barycentric coordinates for the pixel relative to the triangle
            float w0 = ((s1.x - px)*(s2.y - py) - (s1.y - py)*(s2.x - px)) / area;
            float w1 = ((s2.x - px)*(s0.y - py) - (s2.y - py)*(s0.x - px)) / area;
            float w2 = 1.0f - w0 - w1;

            // If the pixel lies inside the triangle
            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                // Interpolate depth at pixel using barycentric weights
                float depth = w0 * depth0 + w1 * depth1 + w2 * depth2;

                // Depth test update only if closer than current z value
                if (depth > zrow[x]) {
                    zrow[x] = depth; // Update our zbuffer with our depth

                    // Pack ARGB colour into 32 bit integer and pixel update buffer
                    pixelBuffer[pixelIndex] =
                        ((Uint8)(colour.w * 255.0f) << 24) | // Alpha
                        ((Uint8)(colour.x * 255.0f) << 16) | // Red
                        ((Uint8)(colour.y * 255.0f) << 8)  | // Green
                        ((Uint8)(colour.z * 255.0f) << 0);   // Blue

                } 
            }
        }
    }
}

// Create an SDL_Texture containing rendered multiline text
SDL_Texture* DrawText(char *message, SDL_Color txtColour, SDL_Renderer *ren, TTF_Font *font) {
    if (!message || !font || !ren) return NULL;

    // Duplicate message for tokenizing without modifying original
    char *msgCopy = strdup(message);
    if (!msgCopy) {
        fprintf(stderr, "Failed to allocate memory for message copy\n");
        return NULL;
    }

    // Count lines by counting \n
    int linesCount = 1;
    for (const char *p = message; *p; p++) {
        if (*p == '\n') linesCount++;
    }

    // Allocate array to store pointers to each line string
    char **lines = (char **)malloc(linesCount * sizeof(char *));
    if (!lines) {
        free(lines);
        free(msgCopy);
        fprintf(stderr, "Failed to allocate memory for lines array\n");
        return NULL;
    }

    // Tokenize message copy into individual lines separated by newline
    int idx = 0;
    char *line = strtok(msgCopy, "\n");
    while (line && idx < linesCount) {
        lines[idx++] = line;
        line = strtok(NULL, "\n");
    }

    // Allocate array for SDL_Surface pointers, one for each line
    SDL_Surface **surfaces = (SDL_Surface **)malloc(linesCount * sizeof(SDL_Surface *));
    if (!surfaces) {
        free(lines);
        free(msgCopy);
        fprintf(stderr, "Failed to allocate memory for surfaces\n");
        return NULL;
    }

    // Render each line into a surface and compute total size needed
    int totalHeight = 0;
    int maxWidth = 0;
    for (int i = 0; i < linesCount; i++) {
        surfaces[i] = TTF_RenderText_Blended(font, lines[i], 0, txtColour);
        if (!surfaces[i]) {
            fprintf(stderr, "Failed to render line %d: %s\n", i, SDL_GetError());
            // Cleanup on failure
            for (int j = 0; j < i; j++) SDL_DestroySurface(surfaces[j]);
            free(surfaces);
            free(lines);
            free(msgCopy);
            return NULL;
        }
        if (surfaces[i]->w > maxWidth) maxWidth = surfaces[i]->w; // Track max width
        totalHeight += surfaces[i]->h; // Sum heights
    }

    // Create a combined surface large enough to hold all lines vertically stacked
    SDL_Surface *combined = SDL_CreateSurface(maxWidth, totalHeight, SDL_PIXELFORMAT_RGBA32);
    if (!combined) {
        fprintf(stderr, "Failed to create combined surface: %s\n", SDL_GetError());
        for (int i = 0; i < linesCount; i++) SDL_DestroySurface(surfaces[i]);
        free(surfaces);
        free(lines);
        free(msgCopy);
        return NULL;
    }

    // Copy (blit) each line surface onto the combined surface one below the other
    int yOffset = 0;
    for (int i = 0; i < linesCount; i++) {
        SDL_Rect dstRect = {0, yOffset, surfaces[i]->w, surfaces[i]->h};
        SDL_BlitSurface(surfaces[i], NULL, combined, &dstRect);
        yOffset += surfaces[i]->h; // Increment vertical offset
        SDL_DestroySurface(surfaces[i]); // Free individual line surface after blit
    }

    // Cleanup: free individual surfaces and lines array
    free(surfaces);
    free(lines);
    free(msgCopy);

    // Create texture from combined surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, combined);
    SDL_DestroySurface(combined);

    if (!texture) {
        fprintf(stderr, "Failed to create texture from combined surface: %s\n", SDL_GetError());
        return NULL;
    }

    return texture;
}

// Main rendering loop that handles drawing triangles and text
void renderLoop(SDL_Renderer *ren, int window_height, int window_width, float *zbuffer, int triangleCount, 
        Mat4 view, Mat4 model, Triangle *tris, Camera cam, Mat4 mvp, Vec4 *triangleColours, 
        uint32_t *pixelBuffer, SDL_Texture *texture, TTF_Font *font, char *message) {
    
    // Clear pixel buffer to 0 (black)
    memset(pixelBuffer, 0, sizeof(uint32_t) * window_width * window_height);

    // Set renderer clear colour to black and clear the renderer
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    // Initialize zbuffer with infinity
    int totalPixels = window_width * window_height;
    for (int i = 0; i < totalPixels; i++) {
        zbuffer[i] = -INFINITY;
    }

    // Calculate model-view matrix (view * model)
    Mat4 modelView = mat4_mul(view, model);

    // Loop over all triangles to draw
    for (int i = 0; i < triangleCount; i++) {
        Triangle *tri = &tris[i];

        // Transform triangle vertices to world space
        Vec3 v0w = mat4_mul_vec3(model, tri->v0.pos);
        Vec3 v1w = mat4_mul_vec3(model, tri->v1.pos);
        Vec3 v2w = mat4_mul_vec3(model, tri->v2.pos);

        // Calculate edges and face normal of the triangle
        Vec3 edge1 = vec3_sub(v1w, v0w);
        Vec3 edge2 = vec3_sub(v2w, v0w);
        Vec3 normal = vec3_normalize(vec3_cross(edge1, edge2));

        // Calculate centroid of the triangle for backface culling
        Vec3 centroid = vec3_scale(vec3_add(vec3_add(v0w, v1w), v2w), 1.0f / 3.0f);

        // Vector from triangle centroid to camera position
        Vec3 toCamera = vec3_sub(cam.position, centroid);

        // Backface culling: skip triangle if normal points away from camera
        if (vec3_dot(normal, toCamera) < 0.0f) continue;

        // Draw the triangle using the transformed vertices and model-view-projection matrix
        DrawTriangle(ren, *tri, mvp, window_width, window_height, triangleColours[i], zbuffer, pixelBuffer);
    }

    // Update the texture with the pixel buffer
    SDL_UpdateTexture(texture, NULL, pixelBuffer, window_width * sizeof(uint32_t));

    // Render the updated texture to the renderer (fullscreen)
    SDL_RenderTexture(ren, texture, NULL, NULL);

    // Render the message text as texture overlay
    SDL_Texture *textTexture = DrawText(message, (SDL_Color){255, 255, 255, 255}, ren, font);
    if (textTexture) {
        float textW, textH;
        SDL_GetTextureSize(textTexture, &textW, &textH);

        SDL_FRect dstRect = {20, 20, (float)textW, (float)textH};
        SDL_RenderTexture(ren, textTexture, NULL, &dstRect);
        SDL_DestroyTexture(textTexture);
    }

    // Present the rendered frame to the window
    SDL_RenderPresent(ren);
}
