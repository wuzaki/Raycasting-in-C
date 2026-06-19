#include <stdio.h>
#include <math.h>
#include <string.h>
#include <C:/msys64/mingw64/include/SDL2/SDL.h>

#include "headers/player.h"
#include "headers/raycasting.h"

typedef struct {
    SDL_Texture *floor_texture;
    SDL_Texture *ceil_texture;
} FloorCasting;

SDL_Texture* load_floor_texture(SDL_Renderer *renderer, char *path) {
    SDL_Surface *original = SDL_LoadBMP(path);
    if (!original) {
        printf("Erreur chargement %s : %s\n", path, SDL_GetError());
        return NULL;
    }

    SDL_Surface *scaled = SDL_CreateRGBSurface(
        0, TEXTURE_SIZE, TEXTURE_SIZE, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000
    );
    if (!scaled) {
        printf("Erreur création surface : %s\n", SDL_GetError());
        SDL_FreeSurface(original);
        return NULL;
    }

    SDL_Rect dst_rect = {0, 0, TEXTURE_SIZE, TEXTURE_SIZE};
    SDL_BlitScaled(original, NULL, scaled, &dst_rect);
    SDL_FreeSurface(original);

    SDL_Texture *texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, TEXTURE_SIZE, TEXTURE_SIZE
    );
    if (!texture) {
        printf("Erreur création texture : %s\n", SDL_GetError());
        SDL_FreeSurface(scaled);
        return NULL;
    }

    void *pixels;
    int tex_pitch;
    SDL_LockTexture(texture, NULL, &pixels, &tex_pitch);
    memcpy(pixels, scaled->pixels, TEXTURE_SIZE * tex_pitch);
    SDL_UnlockTexture(texture);

    SDL_FreeSurface(scaled);
    return texture;
}

void draw_floor(SDL_Renderer *renderer, FloorCasting *floorcasting, Player *player) {
    void *floor_pixels = NULL;
    void *ceil_pixels  = NULL;
    int floor_pitch, ceil_pitch;

    if (SDL_LockTexture(floorcasting->floor_texture, NULL, &floor_pixels, &floor_pitch) != 0) {
        printf("Erreur lock floor texture : %s\n", SDL_GetError());
        return;
    }
    if (SDL_LockTexture(floorcasting->ceil_texture, NULL, &ceil_pixels, &ceil_pitch) != 0) {
        printf("Erreur lock ceil texture : %s\n", SDL_GetError());
        SDL_UnlockTexture(floorcasting->floor_texture);
        return;
    }

    SDL_Texture *screen_tex = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT
    );
    if (!screen_tex) {
        printf("Erreur création screen_tex : %s\n", SDL_GetError());
        SDL_UnlockTexture(floorcasting->floor_texture);
        SDL_UnlockTexture(floorcasting->ceil_texture);
        return;
    }

    void *screen_pixels;
    int screen_pitch;
    SDL_LockTexture(screen_tex, NULL, &screen_pixels, &screen_pitch);

    Uint32 *floor_buf  = (Uint32 *)floor_pixels;
    Uint32 *ceil_buf   = (Uint32 *)ceil_pixels;
    Uint32 *screen_buf = (Uint32 *)screen_pixels;

    int half_h        = HEIGHT / 2;
    int floor_stride  = floor_pitch  / 4;
    int ceil_stride   = ceil_pitch   / 4;
    int screen_stride = screen_pitch / 4;

    /*
     * SCREEN_DIST = (WIDTH/2) / tan(HALF_FOV)
     *
     * Dans ray_cast, la profondeur réelle d'un rayon est corrigée du fishbowl :
     *   depth_real = depth_raw * cos(player->angle - ray_angle)
     * Cela revient à une projection perspective où la colonne x correspond
     * à un angle : ray_angle = player->angle + atan((x - WIDTH/2) / SCREEN_DIST)
     *
     * Pour synchroniser, le floorcasting doit calculer pixel par pixel
     * la direction 3D exacte de chaque rayon en espace caméra, puis
     * retrouver la coordonnée monde au sol.
     *
     * Pour la ligne y (y > half_h) :
     *   - La hauteur relative depuis le centre : dy_screen = y - half_h
     *   - Pour la colonne x : dx_screen = x - WIDTH/2
     *   - Le vecteur rayon normalisé en espace caméra : (dx_screen, SCREEN_DIST, dy_screen)
     *   - On veut l'intersection avec le plan z = 0 (le sol, joueur à z = 0.5)
     *     => t = 0.5 / dy_screen  (car le joueur est à mi-hauteur)
     *   - Coordonnée monde :
     *       world_x = player->x + cos(player->angle) * SCREEN_DIST * t
     *                           - sin(player->angle) * dx_screen * t
     *       world_y = player->y + sin(player->angle) * SCREEN_DIST * t
     *                           + cos(player->angle) * dx_screen * t
     *
     * Ce calcul est EXACTEMENT cohérent avec la correction fishbowl du raycasting.
     */
    double cos_a = cos(player->angle);
    double sin_a = sin(player->angle);
    double half_w = WIDTH / 2.0;

    for (int y = half_h + 1; y < HEIGHT; y++) {
        double dy_screen = (double)(y - half_h);

        /* t = facteur d'échelle : distance au plan du sol.
         * Joueur à z = 0.5, sol à z = 0 → t = 0.5 / (dy_screen / SCREEN_DIST)
         * Simplifié : t = SCREEN_DIST * 0.5 / dy_screen               */
        double t = SCREEN_DIST * 0.5 / dy_screen;

        for (int x = 0; x < WIDTH; x++) {
            double dx_screen = (double)(x - half_w);

            /* Coordonnée monde du pixel (x, y) */
            double world_x = player->x + (cos_a * SCREEN_DIST - sin_a * dx_screen) * t / SCREEN_DIST;
            double world_y = player->y + (sin_a * SCREEN_DIST + cos_a * dx_screen) * t / SCREEN_DIST;

            /* Coordonnée dans la texture */
            int tx = (int)(world_x * TEXTURE_SIZE) % TEXTURE_SIZE;
            int ty = (int)(world_y * TEXTURE_SIZE) % TEXTURE_SIZE;

            if (tx < 0) tx += TEXTURE_SIZE;
            if (ty < 0) ty += TEXTURE_SIZE;
            if (tx >= TEXTURE_SIZE) tx = TEXTURE_SIZE - 1;
            if (ty >= TEXTURE_SIZE) ty = TEXTURE_SIZE - 1;

            Uint32 floor_color = floor_buf[ty * floor_stride + tx];
            Uint32 ceil_color  = ceil_buf [ty * ceil_stride  + tx];

            screen_buf[y * screen_stride + x] = floor_color;
            screen_buf[(HEIGHT - y - 1) * screen_stride + x] = ceil_color;
        }
    }

    SDL_UnlockTexture(screen_tex);
    SDL_UnlockTexture(floorcasting->floor_texture);
    SDL_UnlockTexture(floorcasting->ceil_texture);

    SDL_RenderCopy(renderer, screen_tex, NULL, NULL);
    SDL_DestroyTexture(screen_tex);
}