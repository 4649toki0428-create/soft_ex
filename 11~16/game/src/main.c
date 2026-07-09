#include <stdio.h>
#include "system.h"
#include "window.h"
#include "player.h"
#include "gimmick.h"

int main()
{
    GameContext ctx = {0};
    //初期化
    if(InitSystem(&ctx) != 0){
        return -1;
    }

    SDL_Event event;

    InitPlayer(&ctx.player);

    //ステージギミックの読み込み
    LoadStage(ctx.stage_gimmicks, &ctx.gimmick_count);

    ctx.start_time = SDL_GetTicks(); // ゲーム開始時刻を記録
    ctx.time_limit = 200; // 制限時間を200秒に設定


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
        if(ctx.has_joycon)
        {
            joycon_get_state(&ctx.jc);
        }
        //プレイヤーの更新
        UpdatePlayer(&ctx.player, &ctx.jc, &ctx);

        //ギミックの更新
        for(int i=0; i < ctx.gimmick_count; i++)
        {
            if(ctx.stage_gimmicks[i].is_active)
            {
                UpdateGimmick(&ctx.player, &ctx.stage_gimmicks[i], ctx.player.is_boosting);
            }
        }
        //描画処理
        DrawGame(&ctx);
        camera_update(&ctx);
        int elapsed_time = (SDL_GetTicks() - ctx.start_time) / 1000;
        int remaining_time = ctx.time_limit - elapsed_time;
        if (remaining_time <= 0 && !ctx.player.is_goal) {
            printf("タイムアップ！ゲームオーバー！\n");
            ctx.state = 0;
        }

        //ゴール判定
        if(ctx.player.is_goal)
        {
            printf("ゴールしました！\n");
            ctx.state = 0;
        }

        else if(ctx.player.hp <= 0)
        {
            printf("ゲームオーバー！\n");
            ctx.state = 0;
        }
        
        SDL_Delay(12);
    }

    //終了処理
    // Joy-Conを閉じる
    if(ctx.has_joycon)
    {
        joycon_close(&ctx.jc);
    }

    //  テクスチャの破棄
    if (ctx.bgTexture) SDL_DestroyTexture(ctx.bgTexture);
    if (ctx.tex_player) SDL_DestroyTexture(ctx.tex_player);
    if (ctx.tex_spike) SDL_DestroyTexture(ctx.tex_spike);
    if (ctx.tex_item) SDL_DestroyTexture(ctx.tex_item);
    if (ctx.tex_block) SDL_DestroyTexture(ctx.tex_block);
    if (ctx.tex_goal) SDL_DestroyTexture(ctx.tex_goal);
    if (ctx.tex_enemy) SDL_DestroyTexture(ctx.tex_enemy);

    //  レンダラー、ウィンドウの順に破棄
    if(ctx.render) SDL_DestroyRenderer(ctx.render);
    if(ctx.window) SDL_DestroyWindow(ctx.window);
    if (ctx.font) TTF_CloseFont(ctx.font);

    // システム全体を終了
    SDL_Quit();
    
    return 0;
}