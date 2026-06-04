/*
 * ファイル名： catch.c
 * 機能　　　： メインルーチン
 */
#include "system.h"
#include <stdio.h>

/* 関数 */
static Uint32 AniTimer(Uint32 interval, void* param);
static SDL_bool InputEvent(void);
static void UpdateGameInfo(void);
static void MoveChara(CharaInfo* ch);

/* main */
int main(int argc, char* argv[])
{
    /** 初期化処理 **/
    /* SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        return PrintError(SDL_GetError());
    }
    /** ゲームシステム初期化 **/
    if (InitSystem("chara.data", "position.data") < 0) {
        PrintError("failed to initialize System");
        goto DESTROYSYSTEM;
    }
    /** UI初期化 **/
    if (InitWindow("bg.png") < 0) {
        PrintError("failed to initialize Windows");
        goto DESTROYALL;
    }
    /* 簡易フレームカウンタ
     *  メイン処理のループ回数をカウントし，
     *  タイマー処理にて1フレーム時間を簡易計算する
     */
    int framecnt = 0;
    /** タイマー起動（0.1秒間隔） **/
    SDL_TimerID atimer = SDL_AddTimer(100, AniTimer, &framecnt);
    if (atimer == 0) {
        PrintError(SDL_GetError());
        goto DESTROYALL;
    }

    /** メイン処理 **/
    /* ゲーム開始 */
    gGame.stts = GS_Ready;
    /* メインループ */
    SDL_bool loopflg = SDL_TRUE;
    while (loopflg) {
        /** ゲーム制御 **/
        loopflg = InputEvent();
        /* ゲーム中のとき */
        if (gGame.stts == GS_Playing) {
            /** キャラの状態更新 **/
            for (int i = 0; i < gCharaNum; i++) {
                UpdateCharaInfo(&(gChara[i]));
            }
            /** キャラクタの移動 **/
            for (int i = 0; i < gCharaNum; i++) {
                MoveChara(&(gChara[i]));
            }
            /** 当たり判定 **/
            for (int i = 0; i < gCharaNum; i++) {
                for (int j = i + 1; j < gCharaNum; j++) {
                    Collision(&(gChara[i]), &(gChara[j]));
                }
            }

            /* ゲームの状態更新 */
            UpdateGameInfo();
        }

        /** 描画処理 **/
        RenderWindow();

        /* 少し待つ
         *  PC環境によって遅く(速く)なる時に調整してください
         *  削除してしまうと，デバッグ時に動作が重くなるので
         *  消さない方がよいと思います
         */
        SDL_Delay(10);

        /* フレームカウント */
        framecnt++;
    }

    /** 終了処理 **/
    SDL_RemoveTimer(atimer);
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

/** タイマー処理(アニメーションの更新) **/
Uint32 AniTimer(Uint32 interval, void* param)
{
    /* 時間増分の更新
     *  1フレームの時間を簡易計算する
     */
    if (*(int*)param > 0) {
        /* 時間の値が正しくなったらゲーム開始 */
        if (gGame.stts == GS_Ready && gGame.timeDelta > 0.0)
            gGame.stts = GS_Playing;
        gGame.timeDelta = 0.1 / *(int*)param;
        printf("FPS: %d\r", *(int*)param * 10);
        fflush(stdout);
        *(int*)param = 1;
    }

    /* アニメーションパターンの更新 */
    // プレイヤーの動作状態
    int stop = (gGame.player->dir.x == 0.0 && gGame.player->dir.y == 0.0); //プレイヤーのアニメーションが正しく動かなかったため修正
    // printf("stop:%d", stop);
    for (int i = 0; i < gCharaNum; i++) {
        CharaInfo* ch = &(gChara[i]);
        /* 静止中はアニメーションしない */
        if (!(ch == gGame.player && stop)) {
            ch->ani.x    = (ch->ani.x + 1) % (ch->entity->aninum.x);
            ch->imgsrc.x = ch->ani.x * ch->imgsrc.w;
        }
    }

    return interval;
}

/* ゲーム情報の更新 */
void UpdateGameInfo(void)
{
    /* 子が全てつかまるとゲームクリア */
    int num  = 0;
    int held = 0;
    for (int i = 0; i < gCharaNum; i++) {
        CharaInfo* ch = &(gChara[i]);
        if (ch->type == CT_Child) {
            num++;
            if (ch->stts == CS_Held) {
                held++;
            }
        }
    }
    if (num == held) {
        gGame.stts = GS_End;
        gGame.msg  = MSG_Clear;
    }
}

/* キャラの移動
 *  対象キャラの座標を更新する
 *
 * 引数
 *   ch: 対象キャラ
 */
void MoveChara(CharaInfo* ch)
{
    /* 計算が不要な状態を除外 */
    if (ch->stts == CS_Disable || ch->stts == CS_Held)
        return;

    // 新座標
    SDL_FPoint newpoint = ch->point;

    /* x,y方向の移動（等速運動 p=vt） */
    newpoint.x += ch->dir.x * ch->vel * gGame.timeDelta;
    newpoint.y += ch->dir.y * ch->vel * gGame.timeDelta;

    /* マップ外となったときはマップ内まで戻す */
    AdjustDir adj = AD_NONE;
    if (newpoint.x + ch->entity->baserect.x < 0.0) {
        newpoint.x = -ch->entity->baserect.x;
        adj        = AD_LR;
    } else if (newpoint.x + ch->entity->baserect.x + ch->rect.w >= MAP_WIDTH) {
        newpoint.x = MAP_WIDTH - ch->entity->baserect.x - ch->rect.w;
        adj        = AD_LR;
    }
    if (newpoint.y + ch->entity->baserect.y < 0.0) {
        newpoint.y = -ch->entity->baserect.y;
        adj        = AD_UD;
    } else if (newpoint.y + ch->entity->baserect.y + ch->rect.h >= MAP_HEIGHT) {
        newpoint.y = MAP_HEIGHT - ch->entity->baserect.y - ch->rect.h;
        adj        = AD_UD;
    }

    /* 座標更新 */
    ch->point  = newpoint;
    ch->rect.x = newpoint.x + ch->entity->baserect.x;
    ch->rect.y = newpoint.y + ch->entity->baserect.y;

    // プレイヤーのとき
    if (ch->type == CT_Player) {
        // 座標をキューに追加
        if (ch->dir.x != 0.0 || ch->dir.y != 0.0)
            EnqueuePoint(newpoint);
        // つかまえている子の座標をキューから設定
        int delay = 1;
        for (ch = ch->next; ch; ch = ch->next) {
            ch->point  = PeekQueuePoint(DELAY_BASE * delay);
            ch->rect.x = ch->point.x + ch->entity->baserect.x;
            ch->rect.y = ch->point.y + ch->entity->baserect.y;
            delay++;
        }
    }
    // 子のとき次のランダム行動で壁に当たらないよう向きを変える
    else if (adj != AD_NONE && ch->type == CT_Child) {
        if (adj == AD_LR)
            ch->dir.x = -ch->dir.x;
        else
            ch->dir.y = -ch->dir.y;
    }
}

/* キー入力（イベント）読取
 *
 * 返値
 *   終了させる: 0
 *   継続する  : 1
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
            /* キーリピートは無視 */
            if (event.key.repeat)
                break;
            /** キーが押された方向を設定 **/
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                /** 終了指示 **/
                return SDL_FALSE;
            case SDLK_UP:
                gGame.input.up = SDL_TRUE;
                break;
            case SDLK_DOWN:
                gGame.input.down = SDL_TRUE;
                break;
            case SDLK_RIGHT:
                gGame.input.right = SDL_TRUE;
                break;
            case SDLK_LEFT:
                gGame.input.left = SDL_TRUE;
                break;
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
                gGame.input.up = SDL_FALSE;
                break;
            case SDLK_DOWN:
                gGame.input.down = SDL_FALSE; //TRUEをFALSEに変更
                break;
            case SDLK_RIGHT:
                gGame.input.right = SDL_FALSE;
                break;
            case SDLK_LEFT:
                gGame.input.left = SDL_FALSE;
                break;
            default:
                break;
            }
        default:
            break;
        }
    }
    return SDL_TRUE;
}

/* end of catch.c */
