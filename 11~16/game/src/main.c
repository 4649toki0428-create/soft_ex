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

    //描画処理の開始
    SDL_SetRenderDrawColor(ctx.render, 0, 0, 0, 255);
    SDL_RenderClear(ctx.render);

    SDL_Rect playerRect = {(int)ctx.player.x, (int)ctx.player.y, 50, 50};
    SDL_SetRenderDrawColor(ctx.render, 0, 200, 255, 255);
    SDL_RenderFillRect(ctx.render, &playerRect);

    SDL_RenderPresent(ctx.render);

    while(ctx.state)
    {
        //Joy-Conの状態を取得
        joycon_get_state(&ctx.jc);

        //ボタン判定の処理
        //スティックで移動
        double speed = 5.0;
        ctx.player.x += ctx.jc.stick.x * speed;
        ctx.player.y += ctx.jc.stick.y * speed;

        //ウィンドウの境界チェック
        if(ctx.player.x < 0) ctx.player.x = 0;
        if(ctx.player.x > 640 - 50) ctx.player.x = 640 - 50;

        //ジャンプ
        if(IsJumpButtonPressed(&ctx.jc))
        {
            printf("Jump");
        }
        
    //ゲージ溜めの処理
        float acc_z = ctx.jc.axis[0].acc_z;

        float threshold = 0.5f;

        if(acc_z < -threshold)
        {
            ctx.player.hp += 2;

            if(ctx.player.hp > 100)
            {
                ctx.player.hp = 100;
            }
        }

        //60FPSにする
        SDL_Delay(16);
    }



    //終了処理
    joycon_close(&ctx.jc);
    return 0;
}