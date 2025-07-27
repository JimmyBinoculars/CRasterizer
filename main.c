#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <SDL3/SDL.h>

#include "renderer.h"
#include "calcs.h"
#include "ImportObj.h"
#include "eventMgr.h"

int main(int argc, char* argv[]) {
    printf("TinyRasta by JimmyBinoculars\n");

    srand((unsigned int)time(NULL));

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }

    const int WIN_WIDTH = 640;
    const int WIN_HEIGHT = 480;
    const float PITCH_LIMIT = 1.55f;
    const float MOUSE_SENSITIVITY = 0.001f;

    SDL_Window* win = NULL;
    SDL_Renderer* ren = NULL;

    int initCode = WindowInit(&win, &ren, WIN_WIDTH, WIN_HEIGHT);
    if (initCode != 0) return 1;

    // Set our mouse mode
    SDL_SetWindowRelativeMouseMode(win, true);
    
    FILE* test = fopen("../models/suzanne.obj", "r");
    if (!test) {
        fprintf(stderr, "Failed to open OBJ file!\n");
        return 1;
    }
    fclose(test);

    int triangleCount = 0;
    Triangle* tris = LoadObjTriangles("../models/suzanne.obj", &triangleCount);

    if (!tris || triangleCount == 0) {
        fprintf(stderr, "OBJ loading failed or returned 0 triangles!\n");
        return 1;
    }

    Vec4* triangleColours = malloc(sizeof(Vec4) * triangleCount);
    if (!triangleColours) {
        fprintf(stderr, "Failed to allocate vertex colours!\n");
        return 1;
    }
    
    for (int i = 0; i < triangleCount; i++) {
        triangleColours[i] = (Vec4){
            (float)(rand() % 256) / 255.0f,
            (float)(rand() % 256) / 255.0f,
            (float)(rand() % 256) / 255.0f,
            1.0
        };
    }

    printf("Loaded %d triangles\n", triangleCount);

    Mat4 model = mat4_identity();
    Camera cam = {
        .position = {0, 0, 2},
        .yaw = 0.0f,
        .pitch = 0.0f
    };

    Mat4 proj = mat4_perspective(70.0f * (3.14159f / 180.0f), (float)WIN_WIDTH / WIN_HEIGHT, 0.1f, 100.0f);
    if (!tris) return 1;
    
    float* zbuffer = malloc(sizeof(float) * WIN_WIDTH * WIN_HEIGHT);
    if (!zbuffer) {
        fprintf(stderr, "Failed to allocate zbuffer");
        return 1;
    }

    uint32_t *pixelBuffer = malloc(sizeof(uint32_t) * WIN_WIDTH * WIN_HEIGHT);
    if (!pixelBuffer) {
        fprintf(stderr, "Failed to allocate pixelBuffer");
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(
        ren,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIN_WIDTH, WIN_HEIGHT
    );

    bool running = true;
    SDL_Event event;

    const float moveSpeed = 1.0f;
    const float rotSpeed = 0.02f;

    uint64_t lastTime = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();
    int frames = 0;
    double fpsTimer = 0.0;

    while (running) {
        uint64_t currentTime = SDL_GetPerformanceCounter();
        double deltaTime = (currentTime - lastTime) / freq;
        lastTime = currentTime;

        fpsTimer += deltaTime;
        frames++;

        if (fpsTimer >= 1.0) {
            printf("FPS: %d\n", frames);
            fpsTimer = 0.0;
            frames = 0;
        }

        HandleEvents(&running, &cam, rotSpeed, moveSpeed, PITCH_LIMIT, deltaTime);

        float mouse_delta_x;
        float mouse_delta_y;
        SDL_GetRelativeMouseState(&mouse_delta_x, &mouse_delta_y);
        
        cam.yaw -= mouse_delta_x * MOUSE_SENSITIVITY;
        cam.pitch -= mouse_delta_y * MOUSE_SENSITIVITY;

        Vec3 cam_forward = get_camera_forward(cam);
        Vec3 cam_target  = vec3_add(cam.position, cam_forward);
        Vec3 cam_up      = {0, 1, 0};
        Mat4 view        = mat4_look_at(cam.position, cam_target, cam_up);
        Mat4 mvp         = mat4_mul(proj, mat4_mul(view, model));
        
        renderLoop(ren, WIN_HEIGHT, WIN_WIDTH, zbuffer, triangleCount, view, model,
                tris, cam, mvp, triangleColours, pixelBuffer, texture);

        // SDL_Delay(16);
    }
    
    // Reset our mouse
    SDL_SetWindowRelativeMouseMode(win, false);
    SDL_WarpMouseInWindow(win, WIN_WIDTH/2, WIN_HEIGHT/2);

    // Destroy and free up memory
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_DestroyTexture(texture);
    SDL_Quit();
    free(tris);
    free(triangleColours);
    free(zbuffer);
    free(pixelBuffer);
    return 0;
}
