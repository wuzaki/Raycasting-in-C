#ifndef RAYCASTING_H
#define RAYCASTING_H

#include <SDL2/SDL.h>
// #include <C:/msys64/mingw64/include/SDL2/SDL.h>
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

typedef struct {
    double (*results)[4]; // 4 infos pour chque rayon envoyé
    SDL_Texture *textures[10];
} RayCasting;

void draw_textured_walls(SDL_Renderer *renderer, RayCasting *raycasting);
void draw_wall_2D(SDL_Renderer* renderer, Player *player, double hit_x, double hit_y);
void draw_wall_3D(SDL_Renderer* renderer, RayCasting *raycasting);
void update_raycasting(SDL_Renderer* renderer, RayCasting *raycasting, int map[11][12], Player *player);
SDL_Texture* load_texture(SDL_Renderer *renderer, char *path);

#endif