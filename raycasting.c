#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
// #include <SDL2/SDL.h>

#include "headers/player.h"

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
    double results[NUM_RAYS][4]; // 4 infos pour chque rayon envoyé
    double depth_list[NUM_RAYS];
    double wall_column_list[NUM_RAYS];
    double wall_pos_list[NUM_RAYS][2];
    SDL_Texture *textures[10];
} RayCasting;

SDL_Texture* load_texture(SDL_Renderer *renderer, char *path){
    SDL_Surface *original = SDL_LoadBMP(path);
    if (!original){
        printf("Erreur chargement %s : %s\n", path, SDL_GetError());
        return NULL;
    }

    // Créer une surface vide de la bonne taille
    SDL_Surface *scaled = SDL_CreateRGBSurface(
        0, TEXTURE_SIZE, TEXTURE_SIZE, 32,
        original->format->Rmask,
        original->format->Gmask,
        original->format->Bmask,
        original->format->Amask
    );

    if (!scaled){
        printf("Erreur création surface : %s\n", SDL_GetError());
        SDL_FreeSurface(original);
        return NULL;
    }

    // Redimensionner original → scaled
    SDL_Rect dst_rect = {0, 0, TEXTURE_SIZE, TEXTURE_SIZE};
    SDL_BlitScaled(original, NULL, scaled, &dst_rect);
    SDL_FreeSurface(original);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, scaled);
    SDL_FreeSurface(scaled);
    return texture;
}

void draw_textured_walls(SDL_Renderer *renderer, RayCasting *raycasting) {
    for (int ray = 0; ray < NUM_RAYS; ray++) {
        double depth = raycasting->results[ray][0];
        double proj_height = raycasting->results[ray][1];
        int texture_id = (int)raycasting->results[ray][2];
        double offset = raycasting->results[ray][3];

        SDL_Texture *texture = raycasting->textures[texture_id];
        if (!texture) continue;

        // Colonne source dans la texture (axe X)
        int src_x = (int)(offset * (TEXTURE_SIZE - SCALE));
        if (src_x < 0) src_x = 0;
        if (src_x > TEXTURE_SIZE - SCALE) src_x = TEXTURE_SIZE - SCALE;

        SDL_Rect src, dst;
        src.x = src_x;
        src.w = SCALE;

        dst.x = ray * SCALE;
        dst.w = SCALE;

        if (proj_height < HEIGHT) {
            // Cas normal : la colonne tient dans l'écran
            src.y = 0;
            src.h = TEXTURE_SIZE;

            dst.y = (int)(HEIGHT / 2 - proj_height / 2);
            dst.h = (int)proj_height;
        } else {
            // Cas clamp : mur très proche, on ne prend qu'une tranche de texture
            double texture_height = TEXTURE_SIZE * HEIGHT / proj_height;

            src.y = (int)(TEXTURE_SIZE / 2 - texture_height / 2);
            src.h = (int)texture_height;

            dst.y = 0;
            dst.h = HEIGHT;
        }
        SDL_RenderCopy(renderer, texture, &src, &dst);
    }
}

void draw_wall_2D(SDL_Renderer* renderer, Player *player, double hit_x, double hit_y){
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

    SDL_RenderDrawLine(
        renderer,
        player->x * TILE_SIZE,
        player->y * TILE_SIZE,
        hit_x * TILE_SIZE,
        hit_y * TILE_SIZE
    );
}

void draw_wall_3D(SDL_Renderer* renderer, RayCasting *raycasting){
    for(int ray = 0; ray<NUM_RAYS; ray++){
        // Couleur mur
        float color = 255 / (1 + pow(raycasting->results[ray][0], 5) * 0.00002);
        SDL_SetRenderDrawColor(renderer, color, color, color, 255);

        // Colonne 3D
        SDL_Rect rect = {
            ray * SCALE,
            HEIGHT/2 - raycasting->results[ray][1]/2,
            SCALE,
            raycasting->results[ray][1]
        };
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void ray_cast(RayCasting *raycasting, int map[11][12], Player *player){
    double ox = player->x;
    double oy = player->y;
    int texture_hor = 1;
    int texture_vert = 1;
    int x_map = (int)player->x;
    int y_map = (int)player->y;

    double ray_angle = player->angle - HALF_FOV + 0.0001;
    double sin_a, cos_a;
    double x_hor, y_hor, x_vert, y_vert;
    double dx, dy;
    double depth_hor, depth_vert;
    double depth, delta_depth;
    double offset;
    float proj_height;

    for(int ray = 0; ray < NUM_RAYS; ray++){
        sin_a = sin(ray_angle);
        cos_a = cos(ray_angle);

        // HORIZONTALS
        if (sin_a > 0) {y_hor = y_map + 1;dy = 1;} else{y_hor = y_map - 1e-6; dy = -1;}

        depth_hor = (y_hor - oy) / sin_a;
        x_hor = ox + depth_hor * cos_a;

        delta_depth = dy / sin_a;
        dx = delta_depth * cos_a;

        for(int i = 0; i < MAX_DEPTH; i++){
            int tile_hor_x = (int)x_hor;
            int tile_hor_y = (int)y_hor;
            if(tile_hor_x < 0 || tile_hor_x >= 12 || tile_hor_y < 0 || tile_hor_y >= 11){
                 break;
            }
            if(map[tile_hor_y][tile_hor_x]!=0){
                texture_hor = map[tile_hor_y][tile_hor_x];
                break;
            }
            x_hor += dx;
            y_hor += dy;
            depth_hor += delta_depth;
        }

        // VERTICALS
        if (cos_a > 0) {x_vert = x_map + 1;dx = 1;} else{x_vert = x_map - 1e-6; dx = -1;};
        depth_vert = (x_vert - ox) / cos_a;
        y_vert = oy + depth_vert * sin_a;

        delta_depth = dx / cos_a;
        dy = delta_depth * sin_a;

        for(int i = 0; i < MAX_DEPTH; i++){
            int tile_vert_x = (int)x_vert;
            int tile_vert_y = (int)y_vert;
            if(tile_vert_x < 0 || tile_vert_x >= 12 || tile_vert_y < 0 || tile_vert_y >= 11){
                break;
            }
            if(map[tile_vert_y][tile_vert_x]!=0){
                texture_vert = map[tile_vert_y][tile_vert_x];
                 break;
            }
            x_vert += dx;
            y_vert += dy;
            depth_vert += delta_depth;
        }

        // DEPTH
        int texture;

        if(depth_vert < depth_hor)
        {
            depth = depth_vert;
            texture = texture_vert;
            if(cos_a > 0){ offset = fmod(y_vert, 1.0); }
            else          { offset = 1.0 - fmod(y_vert, 1.0); }
        }
        else
        {
            depth = depth_hor;
            texture = texture_hor;
            if(sin_a > 0){ offset = 1.0 - fmod(x_hor, 1.0); }
            else          { offset = fmod(x_hor, 1.0); }
        }

        // Remove Fishbowl effect
        depth *= cos(player->angle - ray_angle);

        // Projection
        proj_height = SCREEN_DIST / (depth + 0.0001);

        int ray_result[4] = {depth, proj_height, texture, offset};
        raycasting->results[ray][0] = depth;
        raycasting->results[ray][1] = proj_height;
        raycasting->results[ray][2] = texture;
        raycasting->results[ray][3] = offset;

        // draw_wall_2D(renderer, player, hit_x, hit_y);
        // draw_wall_3D(renderer, ray, depth, (int)proj_height);

        ray_angle += DELTA_ANGLE;
    }
}

void update_raycasting(SDL_Renderer* renderer, RayCasting *raycasting, int map[11][12], Player *player){
    ray_cast(raycasting, map, player);
    // draw_textured_walls(renderer, raycasting);
}