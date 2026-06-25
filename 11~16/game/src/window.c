#include "system.h"
#include "window.h"

int InitSystem(GameContext* ctx)
{
    //SDLの初期化(ビデオとジョイスティック)
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
    {
        printf("SDLの初期化に失敗しました: %s\n", SDL_GetError());
        return -1;
    }

    //ウィンドウの作成
    ctx->window = SDL_CreateWindow("そ〜だっしゅ！",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   640, 480, 0);
    
    if(ctx->window == NULL)
    {
        printf("ウィンドウ作成エラー: %s\n", SDL_GetError());
        return -1;
    }

    //レンダラーの作成
    ctx->render = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if(ctx->render == NULL)
    {
        printf("レンダラー作成エラー: %s\n", SDL_GetError());
        return -1;
    }

    //Joy-Conの接続
    joycon_err err = joycon_open(&ctx->jc, JOYCON_R);
    if(err != JOYCON_ERR_NONE)
    {
        printf("Joy-Conオープンエラー: %d\n", err);
        return -1;
    }
    //実行フラグを立てて終了
    ctx->state = 1;
    return 0;
}