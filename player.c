#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <SDL2/SDL.h>
//#include <C:/msys64/mingw64/include/SDL2/SDL.h>

#include "headers/settings.h"

typedef struct {
    float x;
    float y;
    float angle;
    SDL_Rect rect;
} Player;

void init_player(Player *player, float x, float y){
    player->x = x;
    player->y  = y;
    player->angle = 0;
    player->rect.w = TILE_SIZE;
    player->rect.h = TILE_SIZE;
}

int is_wall(int map[11][12], int x, int y){
    // Hors de la map = mur
    if(x < 0 || x >= 12 || y < 0 || y >= 11){
        return 1;
    }
    return map[y][x] != 0;
}

int check_walls(int map[11][12], Player *player, float dx, float dy){

    int next_x = (int)((player->x + dx));
    int next_y = (int)(player->y);

    // collision horizontale
    if(!is_wall(map, next_x, next_y)){
        player->x += dx;
    }

    next_x = (int)(player->x);
    next_y = (int)((player->y + dy));

    // collision verticale
    if(!is_wall(map, next_x, next_y)){
        player->y += dy;
    }

    return 0;
}

void move_player(int map[11][12], Player *player, float dt){
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    float dx = 0;
    float dy = 0;

    // Translation
    // if (keys[SDL_SCANCODE_W]) dy -= SPEED * dt;
    // if (keys[SDL_SCANCODE_S]) dy += SPEED * dt;
    // if (keys[SDL_SCANCODE_A]) dx -= SPEED * dt;
    // if (keys[SDL_SCANCODE_D]) dx += SPEED * dt;

    if (keys[SDL_SCANCODE_W]){
        dx += SPEED * cos(player->angle) * dt;
        dy += SPEED * sin(player->angle) * dt;
    }
    if (keys[SDL_SCANCODE_S]){
        dx -= SPEED * cos(player->angle) * dt;
        dy -= SPEED * sin(player->angle) * dt;
    }

    check_walls(map, player, dx, dy);

    // Rotate Angle
    if (keys[SDL_SCANCODE_LEFT]) player->angle -= ROT_SPEED;
    if (keys[SDL_SCANCODE_RIGHT]) player->angle += ROT_SPEED;

    player->angle = fmod(player->angle, 2 * M_PI);
}

void update_player(int map[11][12], Player *player, float dt){
    move_player(map, player, dt);
    // ray_cast(map, player);
    player->rect.x = (int)(player->x * TILE_SIZE);
    player->rect.y = (int)(player->y * TILE_SIZE);
}

void draw_player(SDL_Renderer* renderer, Player *player){
    // Couleur du carré (rouge)
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // SDL_RenderFillRect(renderer, &p->rect);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    draw_circle(renderer, player->x * TILE_SIZE, player->y * TILE_SIZE, 9);
}