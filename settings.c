#include <stdio.h>
// #include <SDL2/SDL.h>
#include <C:/msys64/mingw64/include/SDL2/SDL.h>

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