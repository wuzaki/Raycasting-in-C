#include <stdio.h>
// #include <SDL2/SDL.h>
#include <C:/msys64/mingw64/include/SDL2/SDL.h>

#define WIDTH 1280
#define HEIGHT 680
#define TILE_SIZE 50

#define FOV (3.14 / 3)
#define HALF_FOV (FOV / 2)
#define NUM_RAYS (WIDTH/2)
#define DELTA_ANGLE (FOV/NUM_RAYS)
#define SCREEN_DIST ((WIDTH/2) / tan(HALF_FOV))
#define SCALE (WIDTH/NUM_RAYS)
#define MAX_DEPTH 20

#define TEXTURE_SIZE 232

void draw_circle(SDL_Renderer* renderer, int cx, int cy, int radius)
{
    for(int y = -radius; y <= radius; y++)
    {
        for(int x = -radius; x <= radius; x++)
        {
            if(x*x + y*y <= radius*radius)
            {
                SDL_RenderDrawPoint(renderer, cx+x, cy+y);
            }
        }
    }
}