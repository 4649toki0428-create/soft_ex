#include <stdio.h>
#include "system.h"
#include "window.h"
#include "player.h"
#include "gimmick.h"

int main()
{
    GameContext ctx = {0};
    // 初期化
    if(InitSystem(&ctx) != 0){
        return -1;
    }

    SDL_Event event;

    // ★変更：起動時はタイトル画面からスタートする
    ctx.state = STATE_TITLE;

    // メインループ（QUITになるまで回り続ける）
    while(ctx.state != STATE_QUIT)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                ctx.state = STATE_QUIT;
            }
        }

        // 入力状態の取得
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(ctx.has_joycon)
        {
            joycon_get_state(&ctx.jc);
        }

        // ==========================================
        // 状態ごとの処理分岐
        // ==========================================
        if(ctx.state == STATE_TITLE)
        {
            // 【1. タイトル画面】
            // スペースキーまたはJoy-ConのXボタンが押されたらゲーム開始
            if(keys[SDL_SCANCODE_SPACE] || (ctx.has_joycon && ctx.jc.button.btn.X))
            {
                // ゲーム本編の初期化処理をここで行う
                InitPlayer(&ctx.player);
                LoadStage(ctx.stage_gimmicks, &ctx.gimmick_count);
                ctx.camera_x = 0.0f;
                ctx.start_time = SDL_GetTicks();
                ctx.time_limit = 200;
                ctx.state = STATE_PLAYING; // 状態を「プレイ中」に切り替え
            }
            
            DrawTitle(&ctx); // タイトル画面の描画
        }
        else if(ctx.state == STATE_PLAYING)
        {
            // 【2. ゲーム本編】
            UpdatePlayer(&ctx.player, &ctx.jc, &ctx);

            for(int i=0; i < ctx.gimmick_count; i++)
            {
                if(ctx.stage_gimmicks[i].is_active)
                {
                    UpdateGimmick(&ctx.player, &ctx.stage_gimmicks[i], ctx.player.is_boosting, ctx.stage_gimmicks, ctx.gimmick_count);
                }
            }
            camera_update(&ctx);

            // クリア判定
            if(ctx.player.is_goal)
            {
                ctx.player.score += (ctx.time_limit - (SDL_GetTicks() - ctx.start_time) / 1000) * 100;
                ctx.state = STATE_CLEAR;
            }
            // ゲームオーバー判定
            else if(ctx.player.hp <= 0)
            {
                ctx.state = STATE_GAMEOVER;
            }

            DrawGame(&ctx); // ゲーム本編の描画
        }
        else if(ctx.state == STATE_GAMEOVER || ctx.state == STATE_CLEAR)
        {
            // 【3. リザルト画面（ゲームオーバー／クリア）】
            // スペースキーまたはJoy-ConのXボタンでタイトル画面に戻る
            if(keys[SDL_SCANCODE_SPACE] || (ctx.has_joycon && ctx.jc.button.btn.X))
            {
                ctx.state = STATE_TITLE;
                SDL_Delay(200); // 連続入力を防ぐための少しの待機時間
            }

            DrawResult(&ctx); // リザルト画面の描画
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
    if (ctx.font) TTF_CloseFont(ctx.font);
    TTF_Quit();

    //  レンダラー、ウィンドウの順に破棄
    if(ctx.render) SDL_DestroyRenderer(ctx.render);
    if(ctx.window) SDL_DestroyWindow(ctx.window);
    
    // システム全体を終了
    SDL_Quit();
    
    return 0;
}