/*
 * ファイル名： ui3kadai.c
 * 機能　　　： メインルーチン
 */
#include "ui3system.h"
#include <stdio.h>

/* 関数 */
static Uint32 FrameTimer(Uint32 interval, void* param);
static SDL_bool InputEvent(void);

/* main */
int main(int argc, char* argv[])
{
    /** 初期化処理 **/
    /* SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        return PrintError(SDL_GetError());
    }
    /** ゲームシステム初期化 **/
    if (InitSystem("chara.data", "map.data") < 0) {
        PrintError("failed to initialize System");
        goto DESTROYSYSTEM;
    }
    /** UI初期化 **/
    if (InitWindow() < 0) {
        PrintError("failed to initialize Windows");
        goto DESTROYALL;
    }
    /* 簡易フレームカウンタ
     *  メイン処理のループ回数をカウントし，
     *  タイマー処理にて1フレーム時間を簡易計算する
     */
    int framecnt = 0;
    /** タイマー起動 **/
    SDL_TimerID ftimer = SDL_AddTimer(100, FrameTimer, &framecnt);
    if (ftimer == 0) {
        PrintError(SDL_GetError());
        goto DESTROYALL;
    }

    /** メイン処理 **/
    /* メインループ */
    SDL_bool loopflg = SDL_TRUE;
    while (loopflg) {
        /** 入力読取 **/
        loopflg = InputEvent();
        /** ゲーム制御 **/
        /* ゲーム中のとき */
        if (gGame.stts == GS_Playing) {
            /** 動作更新 **/
            for (int i = 0; i < gCharaNum; i++) {
                /** キャラクタの移動 **/
                MoveChara(&(gChara[i]), 1);
                /** キャラクタの状態更新 **/
                UpdateCharaInfo(&(gChara[i]));
            }
            /** 当たり判定 **/
            for (int i = 0; i < gCharaNum; i++) {
                for (int j = i + 1; j < gCharaNum; j++)
                    Collision(&(gChara[i]), &(gChara[j]));
            }
        }

        /** 描画処理 **/
        RenderWindow();

        /* 少し待つ
         *  PC環境によって遅く(速く)なる時に調整してください
         *  消さない方がよいと思います
         */
        SDL_Delay(10);

        /* フレームカウント */
        framecnt++;
    }

    /** 終了処理 **/
    SDL_RemoveTimer(ftimer);
DESTROYALL:
    DestroyWindow();
DESTROYSYSTEM:
    DestroySystem();
    SDL_Quit();

    return 0;
}

/* エラーメッセージ表示
 *
 * 引数
 *   str: エラーメッセージ
 *
 * 返値: -1
 */
int PrintError(const char* str)
{
    fprintf(stderr, "%s\n", str);
    return -1;
}

/** タイマー処理(フレーム時間の簡易計算) **/
Uint32 FrameTimer(Uint32 interval, void* param)
{
    /* 時間増分の更新
     *  1フレームの時間を簡易計算する
     */
    if (*(int*)param > 0) {
        if (gGame.timeStep > 0.0)
            /* 値が正しくなったらゲーム開始 */
            gGame.stts = GS_Playing;
        /* 0.1秒ごとに呼び出されるので，
           フレーム数で割ることで1フレームの時間を簡易計算 */
        gGame.timeStep = 0.1 / *(int*)param;
        /* 秒単位のフレーム数を表示 */
        printf("FPS: %d\r", *(int*)param * 10);
        *(int*)param = 1;
    }

    return interval;
}

/* キー入力（イベント）読取
 *
 * 返値
 *   終了させる: SDL_FALSE
 *   継続する  : SDL_TRUE
 */
SDL_bool InputEvent(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            /** 終了指示 **/
            return SDL_FALSE;
        case SDL_KEYDOWN:
            if (event.key.repeat)
                break;
            /** キーが押された状態を保存 **/
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                /** 終了指示 **/
                return SDL_FALSE;
            case SDLK_UP:
                gGame.key.up = SDL_TRUE;
                break;
            case SDLK_DOWN:
                gGame.key.down = SDL_TRUE;
                break;
            case SDLK_LEFT:
                gGame.key.left = SDL_TRUE;
                break;
            case SDLK_RIGHT:
                gGame.key.right = SDL_TRUE;
            default:
                break;
            }
            break;
        case SDL_KEYUP:
            if (event.key.repeat)
                break;
            /* 離されたときは解除 */
            switch (event.key.keysym.sym) {
            case SDLK_UP:
                gGame.key.up = SDL_FALSE;
                break;
            case SDLK_DOWN:
                gGame.key.down = SDL_FALSE;
                break;
            case SDLK_LEFT:
                gGame.key.left = SDL_FALSE;
                break;
            case SDLK_RIGHT:
                gGame.key.right = SDL_FALSE;
            default:
                break;
            }
        default:
            break;
        }
    }
    return SDL_TRUE;
}

/* end of ui3kadai.c */
