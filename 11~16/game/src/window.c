#include "system.h"
#include "window.h"
#include "player.h"


//画像を安全に読み込む関数
SDL_Texture* LoadImage(SDL_Renderer* render, const char* file)
{
    SDL_Surface* surf = SDL_LoadBMP(file);
    if (!surf) {
        printf("画像エラー [%s]: %s\n", file, SDL_GetError());
        return NULL;
    }

    SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 255, 0, 255)); // マゼンタを透過色に設定

    SDL_Texture* tex = SDL_CreateTextureFromSurface(render, surf);
    SDL_FreeSurface(surf);
    return tex;
}

int InitSystem(GameContext* ctx)
{
    //SDLの初期化(ビデオとジョイスティック)
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
    {
        printf("SDLの初期化に失敗しました: %s\n", SDL_GetError());
        return -1;
    }

    // TTFの初期化
    if (TTF_Init() == -1) {
        printf("TTFの初期化に失敗しました: %s\n", TTF_GetError());
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

    //画像の読み込み
    ctx->bgTexture = LoadImage(ctx->render, "assets/bg.bmp");
    ctx->tex_player = LoadImage(ctx->render, "assets/player.bmp");
    ctx->tex_spike = LoadImage(ctx->render, "assets/spike.bmp");
    ctx->tex_item = LoadImage(ctx->render, "assets/item.bmp");
    ctx->tex_block = LoadImage(ctx->render, "assets/block.bmp");
    ctx->tex_goal = LoadImage(ctx->render, "assets/goal.bmp");
    ctx->tex_enemy = LoadImage(ctx->render, "assets/enemy.bmp");

    //Joy-Conの接続
    joycon_err err = joycon_open(&ctx->jc, JOYCON_R);
    if(err != JOYCON_ERR_NONE)
    {
        // エラーが出ても強制終了(return -1)させず、フラグを0にするだけ！
        printf("Joy-Conオープンエラー: %d\n", err);
        ctx->has_joycon = 0; 
    }
    else
    {
        printf("Joy-Conの接続に成功しました。\n");
        ctx->has_joycon = 1;
    }

    // 実行フラグを立てて終了（ここは失敗しても必ず通るようにする）
    ctx->state = 1;
    return 0;
}

void DrawGame(GameContext* ctx)
{
    //画面を黒でクリア
    SDL_SetRenderDrawColor(ctx->render, 0, 0, 0, 255);
    SDL_RenderClear(ctx->render);

    //背景の描画
    if (ctx->bgTexture != NULL) 
    {
        int bg_w = 2000; // 背景画像の幅
        int bg_h = 480;  // ウィンドウの高さに合わせる

        // カメラのX座標を画像の幅で割った「余り」を出すことで、どれだけ進んでもループする
        int scroll_x = (int)ctx->camera_x % bg_w;

        // 1枚目を描画
        SDL_Rect bgRect1 = {
            -scroll_x, 
            0, 
            bg_w, 
            bg_h
        };
        SDL_RenderCopy(ctx->render, ctx->bgTexture, NULL, &bgRect1);

        // 1枚目の右端が画面の中に入ってきて「隙間」ができる場合、2枚目を右にぴったり繋げて描画する
        if (-scroll_x + bg_w < 640) // 640はウィンドウの横幅
        {
            SDL_Rect bgRect2 = {
                -scroll_x + bg_w, 
                0, 
                bg_w, 
                bg_h
            };
            SDL_RenderCopy(ctx->render, ctx->bgTexture, NULL, &bgRect2);
        }
    }

    // ギミックの描画（ここから書き換え）
    for (int i = 0; i < ctx->gimmick_count; i++) {
        Gimmick *g = &ctx->stage_gimmicks[i];
        
        if (g->is_active) {
            SDL_Rect g_rect = { (int)g->x - (int)ctx->camera_x, (int)g->y, (int)g->w, (int)g->h };
            SDL_Texture* draw_tex = NULL;

            // どの画像を使うか判定
            if (g->type == 1) draw_tex = ctx->tex_spike;
            else if (g->type == 2) draw_tex = ctx->tex_item;
            else if (g->type == 3) draw_tex = ctx->tex_block;
            else if (g->type == 4) draw_tex = ctx->tex_goal;
            else if (g->type == 5) draw_tex = ctx->tex_enemy;

            if (draw_tex != NULL) {
                // 画像がある場合は画像を貼り付ける！
                SDL_RenderCopy(ctx->render, draw_tex, NULL, &g_rect);
            } else {
                // 画像が無い場合のフォールバック（四角形）
                if (g->type == 1) SDL_SetRenderDrawColor(ctx->render, 255, 0, 0, 255);
                else if (g->type == 2) SDL_SetRenderDrawColor(ctx->render, 0, 255, 0, 255);
                else if (g->type == 3) SDL_SetRenderDrawColor(ctx->render, 139, 69, 19, 255);
                else if (g->type == 4) SDL_SetRenderDrawColor(ctx->render, 255, 255, 0, 255);
                else if (g->type == 5) SDL_SetRenderDrawColor(ctx->render, 128, 0, 128, 255);

                SDL_RenderFillRect(ctx->render, &g_rect);
            }
        }
    }

    //プレイヤーの描画
    if (ctx->player.invincible_timer == 0 || (ctx->player.invincible_timer / 4) % 2 == 0) {
        SDL_Rect player_rect = { (int)ctx->player.x - (int)ctx->camera_x, (int)ctx->player.y, 50, 50 };
        
        if (ctx->tex_player != NULL) {
            // 画像がある場合
            if (ctx->player.invincible_timer > 0) {
                // ダメージ中は画像全体を赤っぽく染める
                SDL_SetTextureColorMod(ctx->tex_player, 255, 50, 50);
            } else {
                // 通常時は元の色に戻す
                SDL_SetTextureColorMod(ctx->tex_player, 255, 255, 255);
            }
            // 画像を貼り付ける！
            SDL_RenderCopy(ctx->render, ctx->tex_player, NULL, &player_rect);
        } else {
            // 画像が無い場合（四角形）
            if (ctx->player.invincible_timer > 0) {
                SDL_SetRenderDrawColor(ctx->render, 255, 50, 50, 255);
            } else {
                SDL_SetRenderDrawColor(ctx->render, 0, 100, 255, 255);
            }
            SDL_RenderFillRect(ctx->render, &player_rect);
        }
    }

    //ゲージの描画
    SDL_Rect hpBarRect = {20, 20, ctx->player.hp * 2, 20};
    SDL_SetRenderDrawColor(ctx->render, 0, 200, 255, 255);
    SDL_RenderFillRect(ctx->render, &hpBarRect);

    ctx->font = TTF_OpenFont("assets/font.ttf", 24);
    if(!ctx->font)
    {
        printf("フォントの読み込みエラー（font.ttfがありません）: %s\n", TTF_GetError());
    }
    
    // スコアとタイマーの描画
    if (ctx->font != NULL) {
        char text_buf[128];
        int elapsed_time = (SDL_GetTicks() - ctx->start_time) / 1000;
        int remaining_time = ctx->time_limit - elapsed_time;
        if (remaining_time < 0) remaining_time = 0;

        // 描画する文字列を作成
        snprintf(text_buf, sizeof(text_buf), "SCORE: %d   TIME: %d", ctx->player.score, remaining_time);

        // 白色でテキストサーフェスを作成
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(ctx->font, text_buf, textColor);
        
        if (textSurface != NULL) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(ctx->render, textSurface);
            
            // 描画位置（HPゲージの下あたり）
            SDL_Rect textRect = {20, 50, textSurface->w, textSurface->h};
            
            SDL_RenderCopy(ctx->render, textTexture, NULL, &textRect);
            
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }
    }

    //レンダリングの更新
    SDL_RenderPresent(ctx->render);
}

void    camera_update(GameContext* ctx)
{
    if(ctx->player.x > 400.0f)
    {
        if(ctx->player.x - 400.0f > ctx->camera_x)
        {
            ctx->camera_x = ctx->player.x - 400.0f;
        }
    }

    if (ctx->player.x < ctx->camera_x)
    {
        ctx->player.x = ctx->camera_x;
    }
}

