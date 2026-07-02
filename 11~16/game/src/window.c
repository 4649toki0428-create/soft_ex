#include "system.h"
#include "window.h"
#include "player.h"

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

void DrawGame(GameContext* ctx)
{
    //画面を黒でクリア
    SDL_SetRenderDrawColor(ctx->render, 0, 0, 0, 255);
    SDL_RenderClear(ctx->render);

    //プレイヤーの描画
    SDL_Rect player_rect = { (int)ctx->player.x, (int)ctx->player.y, 50, 50 };
    SDL_SetRenderDrawColor(ctx->render, 0, 100, 255, 255);
    SDL_RenderFillRect(ctx->render, &player_rect);

    //ゲージの描画
    SDL_Rect hpBarRect = {20, 20, ctx->player.hp * 2, 20};
    SDL_SetRenderDrawColor(ctx->render, 0, 200, 255, 255);
    SDL_RenderFillRect(ctx->render, &hpBarRect);

    //レンダリングの更新
    SDL_RenderPresent(ctx->render);
}