#include <SDL3/SDL.h>

#include "eventMgr.h"
#include "calcs.h"

void HandleEvents(bool *running, Camera *cam, float rotSpeed, float moveSpeed, 
        float PITCH_LIMIT, float deltaTime) {
    // We define our event here for simplicity
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            *running = false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                *running = false;
            }
        }
    }

    const bool *keys = SDL_GetKeyboardState(NULL);

    // Adjust rotation (to be removed)
    if (keys[SDL_SCANCODE_RIGHT]) cam->yaw -= rotSpeed * deltaTime;
    if (keys[SDL_SCANCODE_LEFT]) cam->yaw += rotSpeed * deltaTime;
    if (keys[SDL_SCANCODE_UP]) cam->pitch += rotSpeed * deltaTime;
    if (keys[SDL_SCANCODE_DOWN]) cam->pitch -= rotSpeed * deltaTime;


    // Clamp pitch
    if (cam->pitch > PITCH_LIMIT) cam->pitch = PITCH_LIMIT;
    if (cam->pitch < -PITCH_LIMIT) cam->pitch = -PITCH_LIMIT;

    // Calculate directions
    Vec3 forward = get_camera_forward(*cam);
    Vec3 right = get_camera_right(*cam);

    // Adjust movement
    if (keys[SDL_SCANCODE_W]) {
        cam->position = vec3_sub(cam->position, vec3_scale(forward, moveSpeed * deltaTime));
    }
    if (keys[SDL_SCANCODE_S]) {
        cam->position = vec3_add(cam->position, vec3_scale(forward, moveSpeed * deltaTime));
    }
    if (keys[SDL_SCANCODE_A]) {
        cam->position = vec3_sub(cam->position, vec3_scale(right, moveSpeed * deltaTime));
    }
    if (keys[SDL_SCANCODE_D]) {
        cam->position = vec3_add(cam->position, vec3_scale(right, moveSpeed * deltaTime));
    }
}
