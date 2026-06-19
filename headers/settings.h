#ifndef PLAYER_H
#define PLAYER_H

#define WIDTH 1280
#define HEIGHT 680
#define TILE_SIZE 50

#define SPEED 3.0f
#define ROT_SPEED 0.008

#define FOV (3.14 / 3)
#define HALF_FOV (FOV / 2)
#define NUM_RAYS (WIDTH/2)
#define DELTA_ANGLE (FOV/NUM_RAYS)
#define SCREEN_DIST ((WIDTH/2) / tan(HALF_FOV))
#define SCALE (WIDTH/NUM_RAYS)
#define MAX_DEPTH 20

#define TEXTURE_SIZE 232

#include <SDL2/SDL.h>
// #include <C:/msys64/mingw64/include/SDL2/SDL.h>

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius);

#endif