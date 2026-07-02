#include <stdio.h>
#include "system.h"
#include "window.h"
#include "player.h"

int main()
{
    GameContext ctx;
    //初期化
    if(InitSystem(&ctx) != 0){
        return -1;
    }

    SDL_Event event;

    InitPlayer(&ctx.player);


    while(ctx.state)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                ctx.state = 0;
            }
        }

        //Joy-Conの状態を取得
        joycon_get_state(&ctx.jc);

        //プレイヤーの更新
        UpdatePlayer(&ctx.player, &ctx.jc);

        //描画処理
        DrawGame(&ctx);
        
        //30FPSにする
        SDL_Delay(10);
    }



    //終了処理
    joycon_close(&ctx.jc);
    return 0;
}