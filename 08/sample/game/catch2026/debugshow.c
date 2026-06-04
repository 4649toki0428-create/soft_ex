/*
 * ファイル名： debugshow.c
 * 機能　　　： デバッグ用の情報表示
 */
#include "system.h"
#include <math.h>

#ifdef DEBUG_SHOW

/* このファイルに記載のコードはデバッグで活用するためのもので
    デバッグ対象ではありません．デバッグ過程で，
    このファイルのコードが原因で調子が悪くなったら
    簡易マップ表示をオフにしてください
    簡易マップはゲーム画面を正確に反映していない場合があります
 */

static SDL_Window *window;
static SDL_Renderer *render;

static void DrawCircle(float sx, float sy, float r)
{
    /* x を基準に点を描画 */
    for (int x = -r * 0.7; x <= r * 0.7; x++) {
        float y = sqrt(r * r - x * x) + 0.5;
        SDL_RenderDrawPointF(render, sx + x, sy - y);
        SDL_RenderDrawPointF(render, sx + x, sy + y);
    }
    /* y を基準に点を描画 */
    for (int y = -r * 0.7; y <= r * 0.7; y++) {
        float x = sqrt(r * r - y * y) + 0.5;
        SDL_RenderDrawPointF(render, sx + x, sy - y);
        SDL_RenderDrawPointF(render, sx - x, sy - y);
    }
}

void DebugShowInit(void)
{
    window = SDL_CreateWindow("debug", 80, 500, MAP_WIDTH / 2.0, MAP_HEIGHT / 2.0, 0);

    render = SDL_CreateRenderer(window, -1, 0);
    if (render == NULL)
        PrintError(SDL_GetError());

    DebugShow();
}

void DebugDestroy(void)
{
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
}

void DebugShow(void)
{

    SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
    SDL_RenderClear(render);

    SDL_SetRenderDrawColor(render, 0, 200, 200, 255);
    SDL_Rect rect = { gGame.dp.x / 2, gGame.dp.y / 2, WD_WIDTH / 2, WD_HEIGHT / 2 };
    SDL_RenderFillRect(render, &(rect));

    for (int i = 0; i < gCharaNum; i++) {
        CharaInfo *p = &(gChara[i]);
        switch (p->type) {
        case CT_Player:
            SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
            break;
        case CT_Wall:
            SDL_SetRenderDrawColor(render, 128, 128, 128, 255);
            break;
        case CT_Child:
            switch (p->stts) {
            case CS_Normal:
                SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
                break;
            case CS_Rescue:
                SDL_SetRenderDrawColor(render, 0, 0, 200, 255);
                break;
            case CS_Held:
                SDL_SetRenderDrawColor(render, 200, 0, 200, 255);
                break;
            case CS_RunAway:
                SDL_SetRenderDrawColor(render, 200, 200, 0, 255);
                break;
            default:
                break;
            }
        default:
            break;
        }
        if (p->stts != CS_Disable) {
            SDL_Rect r = { p->rect.x / 2, p->rect.y / 2, p->rect.w / 2, p->rect.h / 2 };
            SDL_RenderFillRect(render, &r);

            r.x -= 3;
            r.w = 2;
            r.h = i + 1;
            SDL_RenderFillRect(render, &r);

            r.x = p->point.x / 2;
            r.y = p->point.y / 2;
            r.w = p->imgsrc.w / 2;
            r.h = p->imgsrc.h / 2;
            if (p->type == CT_Child) {
                r.w = r.w * 3 / 4;
                r.h = r.h * 3 / 4;
            }
            SDL_RenderDrawRect(render, &r);

            if (p->type == CT_Player || p->stts == CS_Rescue) {
                SDL_FPoint f = { p->point.x + p->entity->baserect.x + p->rect.w / 2.0, p->point.y + p->entity->baserect.y + p->rect.h / 2.0 };

                DrawCircle(f.x / 2, f.y / 2, SEARCH_RADIUS / 2);
            }
        }
    }
    SDL_RenderPresent(render);
}

#endif
/* end of debugshow.c */
