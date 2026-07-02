#include <SDL2/SDL.h>
#include <joyconlib.h>

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* render;
    joyconlib_t jc;
    int state;
    Player player;
} GameContext;

int InitSystem(GameContext* ctx);

void DrawGame(GameContext* ctx);
