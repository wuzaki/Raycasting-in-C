#include <stdio.h>
#define SDL_MAIN_HANDLED
// #include <SDL2/SDL.h>
#include <C:/msys64/mingw64/include/SDL2/SDL.h>

#include "headers/player.h"
#include "headers/raycasting.h"

// === Settings ===
#define WIDTH 1280
#define HEIGHT 680
#define ROWS 11
#define COLS 12
#define TILE_SIZE 50

#define MOUSE_SENSITIVITY 1
#define MOUSE_MAX_REL 40
#define MOUSE_BORDER_LEFT 100
#define MOUSE_BORDER_RIGHT (WIDTH - MOUSE_BORDER_LEFT)

// === Process ===
void process(SDL_Renderer* renderer, int map[ROWS][COLS], Player *player, Raycasting *raycasting, float dt){
    update_player(map, player, dt);
    update_raycasting(renderer, raycasting, map, player);
}

void mouse_control(SDL_Window* screen, Player *player, float dt)
{
    int mouse_x, mouse_y;

    SDL_GetMouseState(&mouse_x, &mouse_y);

    if(mouse_x < MOUSE_BORDER_LEFT || mouse_x > MOUSE_BORDER_RIGHT)
    {
        SDL_WarpMouseInWindow(
            screen,
            WIDTH / 2,
            HEIGHT / 2
        );
    }

    int mouse_dx, mouse_dy;

    SDL_GetRelativeMouseState(&mouse_dx, &mouse_dy);

    // clamp du déplacement souris
    if(mouse_dx > MOUSE_MAX_REL)
        mouse_dx = MOUSE_MAX_REL;

    if(mouse_dx < -MOUSE_MAX_REL)
        mouse_dx = -MOUSE_MAX_REL;

    player->angle += mouse_dx * MOUSE_SENSITIVITY * dt;
}

void draw_map(SDL_Renderer* renderer, int map[ROWS][COLS]){
    for(int y = 0; y < ROWS; y++){
        for(int x = 0; x < COLS; x++){
            // printf("%i", map[y][x]);
            SDL_Rect tile = {
                x * TILE_SIZE,
                y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };

            if(map[y][x] == 1){
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &tile);
            }
            else{
                SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
                SDL_RenderFillRect(renderer, &tile);

                SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
                tile.x -= 1;
                tile.y -= 1;
                tile.w -= 1;
                tile.h -= 1;
                SDL_RenderFillRect(renderer, &tile);
            }
        }
    }
}

// === Draw ===
void draw(SDL_Renderer* renderer, int map[ROWS][COLS], Raycasting *raycasting, Player *player){
    // draw_map(renderer, map);
    draw_wall_3D(renderer, raycasting);
    // draw_player(renderer, player);
}

// === Main ===
int main(){
    // Initialisation
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *screen = SDL_CreateWindow(
        "Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );

    int map[11][12] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };

    // === FPS ===
    Uint32 last_time = SDL_GetTicks();
    Uint32 fps_timer = SDL_GetTicks();
    int frame_count = 0;
    float fps = 0;
    float dt = 1.0f;

    // Gérer les rendus (carré, cercle ect)
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);

    // Classes
    Player player;
    init_player(&player, 1.5, 1.5);
    int speed = 1;

    Raycasting *raycasting = malloc(sizeof(Raycasting));
    memset(raycasting, 0, sizeof(Raycasting));

    int running = 1;
    SDL_Event event;

    while(running){
        // Events
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    running = 0;
                }
            }
        }

        Uint32 now = SDL_GetTicks();

        dt = (now - last_time) / 1000.0f;
        last_time = now;

        // FPS
        frame_count++;

        if(now - fps_timer >= 1000)
        {
            fps = frame_count * 1000.0f / (now - fps_timer);

            frame_count = 0;
            fps_timer = now;

            char title[64];
            snprintf(title, sizeof(title), "Raycasting Test | FPS: %.1f", fps);
            SDL_SetWindowTitle(screen, title);
        }

        // Fond noir
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        process(renderer, map, &player, raycasting, dt);
        draw(renderer, map, raycasting, &player);
        mouse_control(screen, &player, dt);

        SDL_RenderPresent(renderer);
    }
    free(raycasting);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);
    SDL_Quit();
    return 0;
}
