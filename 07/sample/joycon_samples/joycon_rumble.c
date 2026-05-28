/*!
 @file  joycon_rumble.c
 @brief 振動サンプル
 @author    K. Morita
 @date  2023/5/3
 @note  コンパイルオプション：-lSDL2 -ljoyconlib -lhidapi-hidraw -lm
 */
#include <SDL2/SDL.h>
#include <joyconlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

joyconlib_t gJC;

// joycon_play_rumbleのコールバック関数
int playback_ctl(void *data)
{
    // 引数の値が0なら演奏終了指示
    return *(int *)data;
}

// 演奏スレッド
int playback_thread(void *data)
{
    // 演奏
    char mml[] = "v70l8e4>b<cd4c>ba4a<ce4dc>b4b<cl4dec>aarl8r<d4fa4gfe4rce4dc>b4b<cl4dec>aa2";
    puts("Playing...");
    joycon_play_rumble(&gJC, mml, strlen(mml), playback_ctl, data);
    puts("play end.");
    joycon_rumble(&gJC, 0);
    return 0;
}

int main(void)
{
    joycon_err err = joycon_open(&gJC, JOYCON_R);
    if (JOYCON_ERR_NONE != err) {
        printf("joycon open failed:%d\n", err);
        return -1;
    }

    joycon_btn btn = { 0 };
    int flg        = 1;
    int thread_flg = 1;
    while (flg) {
        btn = gJC.button;
        joycon_get_state(&gJC);

        // ボタンが押されている間振動
        if (gJC.button.btn.X) {
            puts("Rumble:50");
            joycon_rumble(&gJC, 50);
        }
        if (gJC.button.btn.Y) {
            puts("Rumble:(140, 50, 30, 30)");
            joycon_rumble_raw(&gJC, 140, 50, 30, 30);
        }

        // ボタンを離したら振動を止める
        if (btn.btn.Y != gJC.button.btn.Y && !gJC.button.btn.Y) {
            joycon_rumble(&gJC, 0);
        }
        if (btn.btn.X != gJC.button.btn.X && !gJC.button.btn.X) {
            joycon_rumble(&gJC, 0);
        }

        if (btn.btn.ZR != gJC.button.btn.ZR) {
            // ボタンが押されたら演奏開始
            if (gJC.button.btn.ZR) {
                thread_flg         = 1;
                SDL_Thread *thread = SDL_CreateThread(playback_thread, "play", &thread_flg);
                if (!thread) {
                    puts(SDL_GetError());
                }
                SDL_DetachThread(thread);
            } else {
                // 離されたら演奏終了
                thread_flg = 0;
            }
        }

        if (gJC.button.btn.Home) {
            flg = 0;
        }
    }

    printf("\n");
    joycon_close(&gJC);

    return 0;
}