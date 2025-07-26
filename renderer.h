#include <SDL3/SDL.h>
#include "calcs.h"

#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

int WindowInit(SDL_Window **window, SDL_Renderer **rend, int width, int height);

void DrawTriangle(SDL_Renderer *ren, Triangle tri, Mat4 mvp, int screen_width, 
        int screen_height, Vec4 colours, float *zbuffer);

void renderLoop(SDL_Renderer *ren, int window_height, int window_width, float *zbuffer, int triangleCount, 
        Mat4 view, Mat4 model, Triangle *tris, Camera cam, Mat4 mvp, Vec4 *triangleColours);
#endif
