
/*************************************************
sdl2_mutex.c
Programmed by X. Kang (2018,5)
Edited by X. Kang (2024,5)

コンパイルオプション：-ljoyconlib -lhidapi-hidraw -lSDL2 -lSDL2_gfx -lm -L/usr/lib
*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <joyconlib.h>

// Joy-Conライブラリの構造体
joyconlib_t jc;
joycon_btn prev_btn = { 0 }; // 前回のボタン状態

SDL_Event event; // SDLイベント構造体

SDL_Window * window; // ウィンドウ構造体
SDL_Renderer * renderer; // レンダラ構造体

// 描画用の構造体
SDL_Rect ground = { 0, 400, 512, 112 }; // 地面の描画範囲
SDL_Rect sky = { 10, 0, 492, 400 }; // 空の描画範囲
SDL_Rect wall1 = { 0, 0, 10, 400 }; // 左の壁の描画範囲
SDL_Rect wall2 = { 502, 0, 10, 400 }; // 右の壁の描画範囲
SDL_Rect mario_loc = { 100, 350, 20, 50 }; // マリオの初期位置とサイズ

int mario_step = 8; // マリオの移動ステップ

#define JOYCON_BUTTON_EVENT SDL_USEREVENT + 1 // Joy-Conボタンイベントの定義

// マリオを右に移動
void mario_right()
{
    // マリオが壁にぶつからないかチェックし、移動
    if(mario_loc.x + mario_loc.w + mario_step <= wall2.x)
    {
        mario_loc.x += mario_step; // 右に移動
    }
}

// マリオを左に移動
void mario_left()
{
    // マリオが壁にぶつからないかチェックし、移動
    if(mario_loc.x - mario_step >= wall1.w)
    {
        mario_loc.x -= mario_step; // 左に移動
    }
}

// マリオを広げる
void mario_wide()
{
    // マリオの幅を最大40ピクセルに広げる
    int max_w = wall2.x - mario_loc.x; // 最大幅を計算
    if(max_w >= 40)
    {
        mario_loc.w = 40; // 幅を40に設定
    }
    else
    {
        mario_loc.w = max_w; // 幅を最大幅に設定
    }
}

// マリオを元のサイズに戻す
void mario_narrow()
{
    mario_loc.w = 20; // 幅を20に戻す
}

// Joy-Conの状態を監視する関数
int joy_func(void *args)
{
    /////////////////////////////////////////
    // STEP. 7_1                           //
    // STEP_7_1_BEGINからSTEP_7_1_ENDまでの//
    // コードのコメンドを外す              //
    /////////////////////////////////////////

    // ***** STEP_7_1_BEGIN *****
    // SDL_mutex *mtx = (SDL_mutex *)args; // ミューテックスを取得
    // ***** STEP_7_1_END *****
    
    while(1)
    {
        /////////////////////////////////////////
        // STEP. 2_1                           //
        // STEP. 1_1で指示されたコードを       //
        // STEP_2_1_BEGINからSTEP_2_1_ENDまでに//
        // に移動する                          //
        /////////////////////////////////////////

        // ***** STEP_2_1_BEGIN (Joy-Conのイベント処理の初め) *****
        // ***** STEP_2_1_END (Joy-Conのイベント処理の終わり) *****
    }
    return 0;
}

// イベントを処理する関数
int event_func(void *args)
{
    /////////////////////////////////////////
    // STEP. 7_2                           //
    // STEP_7_2_BEGINからSTEP_7_2_ENDまでの//
    // コードのコメンドを外す              //
    /////////////////////////////////////////

    // ***** STEP_7_2_BEGIN *****
    // SDL_mutex *mtx = (SDL_mutex *)args; // ミューテックスを取得
    // ***** STEP_7_2_END *****

    while(1)
    {
        /////////////////////////////////////////
        // STEP. 2_2                           //
        // STEP. 1_2で指示されたコードを        //
        // STEP_2_2_BEGINからSTEP_2_2_ENDまでに//
        // に移動する                          //
        /////////////////////////////////////////

        // ***** STEP_2_2_BEGIN (イベント処理の初め) *****
        // ***** STEP_2_2_END (イベント処理の終わり) *****
    }
    return 0;
}

int min_flips = -1; // 1秒あたりの最小描画回数
// 時間間隔(flip_interval)あたりの最小描画回数を計算
Uint32 min_flips_callback(Uint32 flip_interval, void * param)
{
    int flips = * (int *)param;
    if(min_flips == -1 || flips < min_flips)
    {
        min_flips = flips;
    }
    // 描画回数を表示する
    fprintf(stderr, "Flips per sec: %d\n", flips);
    * (int *)param = 0;
    return flip_interval;
}

// メイン関数
int main(int argc, char *argv[])
{
    // SDLの初期化
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("failed to initialize SDL.\n"); // エラーメッセージを表示
        exit(-1); // プログラム終了
    }

    // ウィンドウの作成
    if ((window = SDL_CreateWindow("Multi-Thread Mario", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, 0)) == NULL)
    {
        printf("failed to create window.\n"); // エラーメッセージを表示
        exit(-1); // プログラム終了
    }

    // レンダラの作成
    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
    {
        printf("failed to create renderer.\n"); // エラーメッセージを表示
        exit(-1); // プログラム終了
    }

    // Joy-Conの接続
    joycon_err err = joycon_open(&jc, JOYCON_R);

    if (JOYCON_ERR_NONE != err)
    {
        printf("Joy-Conの接続に失敗しました: %d\n", err); // エラーメッセージを表示
        SDL_DestroyRenderer(renderer); // レンダラを破棄
        SDL_DestroyWindow(window); // ウィンドウを破棄
        SDL_Quit(); // SDLを終了
        return 1; // プログラム終了
    }
    else
    {
        printf("Joy-Con successfully connected.\n"); // 接続成功メッセージを表示
    }

    // 地面と壁の描画
    SDL_SetRenderDrawColor(renderer, 153, 76, 0, 255); // 描画色を設定
    SDL_RenderFillRect(renderer, &ground); // 地面を描画
    SDL_RenderFillRect(renderer, &wall1); // 左の壁を描画
    SDL_RenderFillRect(renderer, &wall2); // 右の壁を描画

    ///////////////////////////////////////
    // STEP. 3                           //
    // STEP_3_BEGINからSTEP_3_ENDまでの  //
    // コードのコメンドを外す            //
    ///////////////////////////////////////

    // ***** STEP_3_BEGIN *****
    // SDL_Thread * joy_thread; // joy_threadを用いる
    // SDL_Thread * event_thread; // keyboard_threadを用いる
    // ***** STEP_3_END *****

    ///////////////////////////////////////
    // STEP. 6                           //
    // STEP_6_BEGINからSTEP_6_ENDまでの  //
    // コードのコメンドを外す            //
    ///////////////////////////////////////

    // ミューテックスの作成
    // ***** STEP_6_BEGIN *****
    // SDL_mutex *mtx = SDL_CreateMutex();
    // ***** STEP_6_END *****

    ///////////////////////////////////////
    // STEP. 4                           //
    // STEP_4_BEGINからSTEP_4_ENDまでの  //
    // コードのコメンドを外す            //
    // 注意：引数はNULLからmtxに変わった //
    ///////////////////////////////////////

    // ***** STEP_4_BEGIN *****
    // joy_thread = SDL_CreateThread(joy_func, "joy_thread", mtx); // Joy-Conの状態を監視するスレッドの作成
    // event_thread = SDL_CreateThread(event_func, "event_thread", mtx); // イベントを処理するスレッドの作成
    // ***** STEP_4_END *****

    int flips = 0; // 1秒あたりの描画回数
    SDL_TimerID timer_id = SDL_AddTimer(1000, min_flips_callback, &flips); // 1秒あたりの最小描画回数を計算

    while (1)
    {
        /////////////////////////////////////////
        // STEP. 1_1                           //
        // STEP_1_1_BEGINからSTEP_1_1_ENDまでの//
        // コードをStep. 2_1で指示されたところ //
        // に移動する                          //
        /////////////////////////////////////////

        // ***** STEP_1_1_BEGIN (Joy-Conのイベント処理の初め) *****
        // Joy-Conの状態を取得
        joycon_get_state(&jc);

        /////////////////////////////////////////
        // STEP. 8_1                           //
        // STEP_8_1_BEGINからSTEP_8_1_ENDまでの//
        // コードのコメンドを外す              //
        /////////////////////////////////////////

        // ***** STEP_8_1_BEGIN *****
        // SDL_LockMutex(mtx); // ミューテックスをロック
        //***** STEP_8_1_END *****

        // ボタンの状態が前回と異なる場合はイベントを発生させる
        if (prev_btn.btn.B != jc.button.btn.Y ||
            prev_btn.btn.X != jc.button.btn.A ||
            prev_btn.btn.Y != jc.button.btn.X ||
            prev_btn.btn.Home != jc.button.btn.Home) {

            SDL_Event joycon_event; // Joy-Conイベントを作成
            joycon_event.type = JOYCON_BUTTON_EVENT; // イベントタイプを設定
            SDL_PushEvent(&joycon_event); // イベントをキューに追加

            prev_btn = jc.button; // 現在のボタン状態を保存
        }

        /////////////////////////////////////////
        // STEP. 9_1                           //
        // STEP_9_1_BEGINからSTEP_9_1_ENDまでの//
        // コードのコメンドを外す              //
        /////////////////////////////////////////

        // ***** STEP_9_1_BEGIN *****
        // SDL_UnlockMutex(mtx); // Mutexをアンロックし、他のスレッドが共有変数にアクセスできるようにする
        // ***** STEP_9_1_END *****

        // ***** STEP_1_1_END (Joy-Conのイベント処理の終わり) *****

        /////////////////////////////////////////
        // STEP. 1_2                           //
        // STEP. 1_2で指示されたコードを       //
        // STEP_2_2_BEGINからSTEP_2_2_ENDまでに//
        // に移動する                          //
        /////////////////////////////////////////

        // ***** STEP_1_2_BEGIN (イベント処理の初め) *****
        // イベントを待機
        if(SDL_WaitEventTimeout(&event, 1))
        {
            /////////////////////////////////////////
            // STEP. 8_2                           //
            // STEP_8_2_BEGINからSTEP_8_2_ENDまでの//
            // コードのコメンドを外す              //
            /////////////////////////////////////////

            // ***** STEP_8_2_BEGIN *****
            // SDL_LockMutex(mtx); // ミューテックスをロック
            //***** STEP_8_2_END *****

            // イベントタイプごとに処理を分岐
            switch (event.type)
            {
                case JOYCON_BUTTON_EVENT:
                    // Joy-Conのボタンに応じたマリオの動き
                    if (jc.button.btn.Y) {
                        mario_left(); // 左に移動
                    }
                    if (jc.button.btn.A) {
                        mario_right(); // 右に移動
                    }
                    if (jc.button.btn.X) {
                        mario_wide(); // 幅を広げる
                    } else {
                        mario_narrow(); // 幅を元に戻す
                    }
                    if (jc.button.btn.Home) {
                        SDL_Event quit_event; // 終了イベントを作成
                        quit_event.type = SDL_QUIT; // イベントタイプを設定
                        SDL_PushEvent(&quit_event); // イベントをキューに追加
                    }
                    break;
                case SDL_KEYDOWN:
                    // キーボードのキーに応じたマリオの動き
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_RIGHT:
                            mario_right(); // 右に移動
                            break;
                        case SDLK_LEFT:
                            mario_left(); // 左に移動
                            break;
                        case SDLK_SPACE:
                            mario_wide(); // 幅を広げる
                            break;
                        case SDLK_ESCAPE:
                            SDL_Quit(); // SDLを終了
                            exit(0); // プログラム終了
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    // キーボードのキーリリースに応じたマリオの動き
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_SPACE:
                            mario_narrow(); // 幅を元に戻す
                            break;
                    }
                    break;
                case SDL_QUIT:
                    SDL_Quit(); // SDLを終了
                    exit(0); // プログラム終了
                    break;
                default:
                    break;
            }

            /////////////////////////////////////////
            // STEP. 9_2                           //
            // STEP_9_2_BEGINからSTEP_9_2_ENDまでの//
            // コードのコメンドを外す              //
            /////////////////////////////////////////

            // ***** STEP_9_2_BEGIN *****
            // SDL_UnlockMutex(mtx); // ミューテックスをアンロック
            // ***** STEP_9_2_BEGIN *****
        }
        // ***** STEP_1_2_END (イベント処理の初め) *****

        // 描画処理
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // 描画色を設定（空）
        SDL_RenderFillRect(renderer, &sky); // 空を描画
        SDL_SetRenderDrawColor(renderer, 255, 150, 150, 255); // 描画色を設定（マリオ）
        SDL_RenderFillRect(renderer, &mario_loc); // マリオを描画
        SDL_SetRenderDrawColor(renderer, 153, 76, 0, 255); // 描画色を設定（地面と壁）
        SDL_RenderFillRect(renderer, &ground); // 地面を描画
        SDL_RenderFillRect(renderer, &wall1); // 左の壁を描画
        SDL_RenderFillRect(renderer, &wall2); // 右の壁を描画
        SDL_RenderPresent(renderer); // 描画結果を表示

        flips += 1; // 表示回数
    }

    // 終了処理
    ///////////////////////////////////////
    // STEP. 5                           //
    // STEP_5_BEGINからSTEP_5_ENDまでの  //
    // コードのコメンドを外す            //
    ///////////////////////////////////////

    // ***** STEP_5_BEGIN *****
    // SDL_WaitThread(joy_thread, NULL); // Joy-Con監視スレッドの終了を待機
    // SDL_WaitThread(event_thread, NULL); // イベント処理スレッドの終了を待機
    // ***** STEP_5_END *****

    ///////////////////////////////////////
    // STEP. 10                          //
    // STEP_10_BEGINからSTEP_10_ENDまでの//
    // コードのコメンドを外す            //
    ///////////////////////////////////////

    // ***** STEP_10_BEGIN *****
    // SDL_DestroyMutex(mtx); // ミューテックスを破棄
    // ***** STEP_10_END *****

    SDL_DestroyRenderer(renderer); // レンダラを破棄
    SDL_DestroyWindow(window); // ウィンドウを破棄
    SDL_Quit(); // SDLを終了

    return 0;
}
