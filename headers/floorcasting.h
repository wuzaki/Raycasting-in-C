#ifndef FLOORCASTING_H
#define FLOORCASTING_H

// #include <SDL2/SDL.h>
#include <C:/msys64/mingw64/include/SDL2/SDL.h>

typedef struct {
    SDL_Texture *floor_texture;
    SDL_Texture *ceil_texture;
} FloorCasting;

SDL_Texture* load_floor_texture(SDL_Renderer *renderer, char *path);
void draw_floor(SDL_Renderer *renderer, FloorCasting *floorcasting, Player *player);

#endif