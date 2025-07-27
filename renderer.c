#include "renderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <math.h>

int WindowInit(SDL_Window **window, SDL_Renderer **rend, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    *window = SDL_CreateWindow("SDL3 C Project", width, height, 0);
    if (!*window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *rend = SDL_CreateRenderer(*window, NULL);
    if (!*rend) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    TTF_Init();

    return 0;
}

void DrawTriangle(SDL_Renderer *ren, Triangle tri, 
        Mat4 mvp, int screen_width, int screen_height, Vec4 colour, float *zbuffer, uint32_t *pixelBuffer) {
    Vec4 p0 = mat4_mul_vec4(mvp, vec4_from_vec3(tri.v0.pos, 1.0f));
    Vec4 p1 = mat4_mul_vec4(mvp, vec4_from_vec3(tri.v1.pos, 1.0f));
    Vec4 p2 = mat4_mul_vec4(mvp, vec4_from_vec3(tri.v2.pos, 1.0f));

    // Cull tris behind the camera
    if (p0.w >= 0.0f || p1.w >= 0.0f || p2.w >= 0.0f) return;

    p0 = vec4_scale(p0, 1.0f / p0.w);
    p1 = vec4_scale(p1, 1.0f / p1.w);
    p2 = vec4_scale(p2, 1.0f / p2.w);

    Vec2 s0 = { (p0.x + 1.0f) * 0.5f * screen_width, (1.0f - p0.y) * 0.5f * screen_height };
    Vec2 s1 = { (p1.x + 1.0f) * 0.5f * screen_width, (1.0f - p1.y) * 0.5f * screen_height };
    Vec2 s2 = { (p2.x + 1.0f) * 0.5f * screen_width, (1.0f - p2.y) * 0.5f * screen_height };

    int min_x = (int)fmaxf(0.0f, floorf(fminf(fminf(s0.x, s1.x), s2.x)));
    int max_x = (int)fminf(screen_width - 1, ceilf(fmaxf(fmaxf(s0.x, s1.x), s2.x)));
    int min_y = (int)fmaxf(0.0f, floorf(fminf(fminf(s0.y, s1.y), s2.y)));
    int max_y = (int)fminf(screen_height - 1, ceilf(fmaxf(fmaxf(s0.y, s1.y), s2.y)));

    float area = (s1.x - s0.x) * (s2.y - s0.y) - (s1.y - s0.y) * (s2.x - s0.x);
    if (area == 0.0f) return;

    // Precompute per-vertex depth
    float depth0 = (p0.z + 1.0f) * 0.5f;
    float depth1 = (p1.z + 1.0f) * 0.5f;
    float depth2 = (p2.z + 1.0f) * 0.5f;

    for (int y = min_y; y <= max_y; y++) {
        float *zrow = zbuffer + y * screen_width;
        for (int x = min_x; x <= max_x; x++) {
            int pixelIndex = y * screen_width + x;

            float px = (float)x + 0.5f;
            float py = (float)y + 0.5f;

            float w0 = ((s1.x - px)*(s2.y - py) - (s1.y - py)*(s2.x - px)) / area;
            float w1 = ((s2.x - px)*(s0.y - py) - (s2.y - py)*(s0.x - px)) / area;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                float depth = w0 * depth0 + w1 * depth1 + w2 * depth2;

                if (depth > zrow[x]) {
                    zrow[x] = depth;
                    pixelBuffer[pixelIndex] =
                        ((Uint8)(colour.w * 255.0f) << 24) |
                        ((Uint8)(colour.x * 255.0f) << 16) |
                        ((Uint8)(colour.y * 255.0f) << 8)  |
                        ((Uint8)(colour.z * 255.0f) << 0);

                } 
            }
        }
    }
}

SDL_Texture* DrawText(char *message, SDL_Color txtColour, SDL_Renderer *ren, TTF_Font *font) {
    SDL_Texture *texture = NULL;

    SDL_Surface *text = TTF_RenderText_Blended(font, message, 0, txtColour);

    if (text) {
        texture = SDL_CreateTextureFromSurface(ren, text);
        SDL_DestroySurface(text);
    }
    if (!text) {
        fprintf(stderr, "Failed to create text surface: %s\n", SDL_GetError());
        return NULL;
    }

    return texture;
}

void renderLoop(SDL_Renderer *ren, int window_height, int window_width, float *zbuffer, int triangleCount, 
        Mat4 view, Mat4 model, Triangle *tris, Camera cam, Mat4 mvp, Vec4 *triangleColours, 
        uint32_t *pixelBuffer, SDL_Texture *texture, TTF_Font *font) {
    
    memset(pixelBuffer, 0, sizeof(uint32_t) * window_width * window_height);

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    int totalPixels = window_width * window_height;
    for (int i = 0; i < totalPixels; i++) {
        zbuffer[i] = -INFINITY;
    }

    Mat4 modelView = mat4_mul(view, model);

    for (int i = 0; i < triangleCount; i++) {
        Triangle *tri = &tris[i];

        Vec3 v0w = mat4_mul_vec3(model, tri->v0.pos);
        Vec3 v1w = mat4_mul_vec3(model, tri->v1.pos);
        Vec3 v2w = mat4_mul_vec3(model, tri->v2.pos);

        Vec3 edge1 = vec3_sub(v1w, v0w);
        Vec3 edge2 = vec3_sub(v2w, v0w);
        Vec3 normal = vec3_normalize(vec3_cross(edge1, edge2));

        Vec3 centroid = vec3_scale(vec3_add(vec3_add(v0w, v1w), v2w), 1.0f / 3.0f);
        Vec3 toCamera = vec3_sub(cam.position, centroid);

        if (vec3_dot(normal, toCamera) < 0.0f) continue;

        DrawTriangle(ren, *tri, mvp, window_width, window_height, triangleColours[i], zbuffer, pixelBuffer);
    }

    SDL_UpdateTexture(texture, NULL, pixelBuffer, window_width * sizeof(uint32_t));

    SDL_RenderTexture(ren, texture, NULL, NULL);

    SDL_Texture *textTexture = DrawText("Hello, World!", (SDL_Color){255, 255, 255, 255}, ren, font);
    if (textTexture) {
        float textW, textH;
        SDL_GetTextureSize(textTexture, &textW, &textH);

        SDL_FRect dstRect = {20, 20, (float)textW, (float)textH};
        SDL_RenderTexture(ren, textTexture, NULL, &dstRect);
        SDL_DestroyTexture(textTexture);
    }

    SDL_RenderPresent(ren);
}
