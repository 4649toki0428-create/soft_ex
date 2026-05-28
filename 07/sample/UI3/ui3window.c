/*
 * ファイル名	: ui3window.c
 * 機能		: ユーザーインターフェース処理
 */
#include "ui3system.h"
#include <math.h>

/* メインウインドウの初期化（表示，設定）
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int InitWindow(void)
{
    /** メインのウインドウ(表示画面)とレンダラーの作成 **/
    gGame.window = SDL_CreateWindow("ui3kadai", 80, 50, WD_Width, WD_Height, 0);
    if (gGame.window == NULL)
        return PrintError(SDL_GetError());

    gGame.render = SDL_CreateRenderer(gGame.window, -1, 0);
    if (gGame.render == NULL)
        return PrintError(SDL_GetError());

    return 0;
}

/* ウインドウの終了処理 */
void DestroyWindow(void)
{
    SDL_DestroyRenderer(gGame.render);
    SDL_DestroyWindow(gGame.window);
}

/* ウインドウ描画
 *  メインウインドウに背景，キャラを描画する
 */
void RenderWindow(void)
{
    /* 一旦消す */
    if (0 > SDL_SetRenderDrawColor(gGame.render, 0, 0, 0, 255))
        PrintError(SDL_GetError());
    if (0 > SDL_RenderClear(gGame.render))
        PrintError(SDL_GetError());

    /* マップ描画 */
    for (int x = 0; x < MAP_Width; x++) {
        for (int y = 0; y < MAP_Height; y++) {
            switch (gGame.map[x][y]) {
            case MT_Wall:
                if (0 > SDL_SetRenderDrawColor(gGame.render, 255, 255, 255, 255))
                    PrintError(SDL_GetError());
                break;
            case MT_Floor:
                if (0 > SDL_SetRenderDrawColor(gGame.render, 0, 255, 0, 255))
                    PrintError(SDL_GetError());
                break;
            default:
                break;
            }
            SDL_Rect rect = { x * MAP_ChipSize, y * MAP_ChipSize, MAP_ChipSize, MAP_ChipSize };
            if (0 > SDL_RenderFillRect(gGame.render, &(rect)))
                PrintError(SDL_GetError());
        }
    }

    /* キャラ描画 */
    for (int i = 0; i < gCharaNum; i++) {
        switch (gChara[i].type) {
        case CT_Player: /* 赤いキャラ，キックバックでは黄色 */
            if (gChara[i].stts == CS_Kickback) {
                if (0 > SDL_SetRenderDrawColor(gGame.render, 255, 255, 0, 255))
                    PrintError(SDL_GetError());
            } else if (0 > SDL_SetRenderDrawColor(gGame.render, 255, 0, 0, 255))
                PrintError(SDL_GetError());
            break;
        case CT_Enemy: /* 青いキャラ，キックバックではマゼンタ */
            if (gChara[i].stts == CS_Kickback) {
                if (0 > SDL_SetRenderDrawColor(gGame.render, 255, 0, 255, 255))
                    PrintError(SDL_GetError());
            } else if (0 > SDL_SetRenderDrawColor(gGame.render, 0, 0, 255, 255))
                PrintError(SDL_GetError());
            break;
        default:
            break;
        }

        SDL_Rect rect;
        rect.x = gChara[i].point.x;
        rect.y = gChara[i].point.y;
        rect.w = gChara[i].w;
        rect.h = gChara[i].h;
        if (0 > SDL_RenderFillRect(gGame.render, &rect))
            PrintError(SDL_GetError());
    }

    /* レンダラーをウインドウに反映
     *  この関数の呼び出し間隔を1フレームとしている
     */
    SDL_RenderPresent(gGame.render);
}

/* end of ui3window.c */
