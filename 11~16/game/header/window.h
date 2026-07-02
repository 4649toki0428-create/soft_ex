#include <SDL2/SDL.h>
#include <joyconlib.h>

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* render;
    joyconlib_t jc;
    int state;
    Player player;
    Gimmick spike;
    float camera_x;
} GameContext;

int InitSystem(GameContext* ctx);

void DrawGame(GameContext* ctx);

void camera_update(GameContext* ctx);