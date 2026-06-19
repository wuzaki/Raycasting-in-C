#ifndef PLAYER_H
#define PLAYER_H

// #include <SDL2/SDL.h>
#include <C:/msys64/mingw64/include/SDL2/SDL.h>

typedef struct {
    float x;
    float y;
    float angle; 
    SDL_Rect rect;
} Player;

// fonctions du player
void init_player(Player *player, float x, float y);
void move_player(int map[11][12], Player *player, float dt);
void update_player(int map[11][12], Player *player, float dt);
void draw_player(SDL_Renderer* renderer, Player *player);
// void player_move(Player *p, int dx, int dy);
// void player_draw(Player *p, SDL_Renderer *renderer);
#endif