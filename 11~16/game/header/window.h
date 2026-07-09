#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <joyconlib.h>
#include "system.h"
#include "gimmick.h"

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* render;
    joyconlib_t jc;
    int state;
    Player player;
    Gimmick stage_gimmicks[100];
    int gimmick_count;
    float camera_x;
    SDL_Texture* bgTexture;
    int has_joycon;
    //画像データ
    SDL_Texture* tex_player;
    SDL_Texture* tex_spike;
    SDL_Texture* tex_item;
    SDL_Texture* tex_block;
    SDL_Texture* tex_goal;
    SDL_Texture* tex_enemy;
    Uint32 start_time;
    int time_limit;
    TTF_Font* font;
} GameContext;

int InitSystem(GameContext* ctx);

void DrawGame(GameContext* ctx);

void camera_update(GameContext* ctx);

#endif