/*
 * ファイル名	: window.c
 * 機能		: ユーザーインターフェース処理
 */
#include "system.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>

/* フォントパス */
static char gFontFile[] = "/usr/share/fonts/truetype/freefont/FreeSerifBoldItalic.ttf";
/* メッセージ */
static char *gMsgStr[MSG_NUM] = { " ", "GameOver", "Clear!!" };
/* 色 */
static const SDL_Color gWhite = { 255, 255, 255, 255 };
static const SDL_Color gBlue  = { 0, 0, 255, 255 };

/* 関数 */
static int MakeMessage(void);
static void SetDispPoint(void);
static void SetDirPattern(CharaInfo *ch);

/* メインウインドウの表示，設定
 *
 * 引数
 *   bg_file: 背景画像ファイルのパス
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int InitWindow(const char *bg_file)
{
    /* SDL_image初期化 */
    if (IMG_INIT_PNG != IMG_Init(IMG_INIT_PNG)) {
        return PrintError("failed to initialize SDL_image");
    }

    /** メインのウインドウ(表示画面)とレンダラーの作成 **/
    gGame.window = SDL_CreateWindow("catch", 80, 50, WD_WIDTH, WD_HEIGHT, 0);
    if (gGame.window == NULL)
        return PrintError(SDL_GetError());

    gGame.render = SDL_CreateRenderer(gGame.window, -1, 0);
    if (gGame.render == NULL)
        return PrintError(SDL_GetError());

    /** キャラ画像の読み込み **/
    // 画像は，
    //  横方向にアニメーションパターン
    //  縦方向に向きパターン
    //  があると想定
    for (int i = 0; i < CHARATYPE_NUM; i++) {
        SDL_Surface *s = IMG_Load(gCharaType[i].path);
        if (NULL == s) {
            return PrintError("failed to open character image.");
        }
        gCharaType[i].aninum.x = s->w / gCharaType[i].w;
        gCharaType[i].aninum.y = s->h / gCharaType[i].h;
        gCharaType[i].img      = SDL_CreateTextureFromSurface(gGame.render, s);
        // サーフェイス解放(テクスチャに転送後はゲーム中に使わないので)
        SDL_FreeSurface(s);
        if (gCharaType[i].img == NULL) {
            return PrintError(SDL_GetError());
        }
    }

    /** 背景画像の作成 **/
    SDL_Surface *s = IMG_Load(bg_file);
    if (s == NULL) {
        return PrintError("failed to open background image.");
    }
    gGame.bg = SDL_CreateTextureFromSurface(gGame.render, s);
    SDL_FreeSurface(s);
    if (gGame.bg == NULL) {
        return PrintError(SDL_GetError());
    }

    /** メッセージ作成 **/
    if (MakeMessage())
        return -1;

    /** ウインドウへの描画 **/
    RenderWindow();

    /* image利用終了(テクスチャに転送後はゲーム中に使わないので) */
    IMG_Quit();

    /* デバッグ用 */
#ifdef DEBUG_SHOW
    DebugShowInit();
#endif

    return 0;
}

/* ウインドウの終了処理 */
void DestroyWindow(void)
{
    /* テクスチャなど */
    for (int i = 0; i < CHARATYPE_NUM; i++)
        SDL_DestroyTexture(gCharaType[i].img);
    for (int i = 0; i < MSG_NUM; i++)
        SDL_DestroyTexture(gGame.msgTexts[i]);
    SDL_DestroyTexture(gGame.bg);
    SDL_DestroyRenderer(gGame.render);
    SDL_DestroyWindow(gGame.window);

    /* デバッグ用 */
#ifdef DEBUG_SHOW
    DebugDestroy();
#endif
}

/* メッセージ作成
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int MakeMessage(void)
{
    int ret = 0;
    /* フォントからメッセージテクスチャ作成 */
    /* 初期化 */
    if (TTF_Init() < 0) {
        return PrintError(TTF_GetError());
    }
    /* フォントを開く */
    TTF_Font *ttf = TTF_OpenFont(gFontFile, 90);
    if (NULL == ttf) {
        ret = PrintError(TTF_GetError());
    }
    /* メッセージ作成 */
    SDL_Color cols[MSG_NUM] = { gBlue, gBlue, gWhite };
    for (int i = 0; i < MSG_NUM && ttf; i++) {
        SDL_Surface *sf;
        /* フォントと文字列，色からサーフェイス作成 */
        sf = TTF_RenderUTF8_Blended(ttf, gMsgStr[i], cols[i]);
        if (NULL == sf) {
            ret = PrintError(TTF_GetError());
        } else {
            /* テクスチャへ */
            gGame.msgTexts[i] = SDL_CreateTextureFromSurface(gGame.render, sf);
            if (NULL == gGame.msgTexts[i]) {
                ret = PrintError(SDL_GetError());
            }
            /* サーフェイス解放(テクスチャに転送後はゲーム中に使わないので) */
            SDL_FreeSurface(sf);
        }
    }

    /* フォントを閉じる */
    TTF_CloseFont(ttf);
    /* フォント利用終了(テクスチャに転送後はゲーム中に使わないので) */
    TTF_Quit();

    return ret;
}

/* ウインドウ表示位置設定
 *  メインウインドウに表示するマップの位置座標を設定する
 */
void SetDispPoint(void)
{
    SDL_Point dp;
    /* 基本はプレイヤーが中心となるように */
    dp.x = gGame.player->point.x - (WD_WIDTH / 2);
    dp.y = gGame.player->point.y - (WD_HEIGHT / 2);
    /* マップ端は超えないように */
    if (dp.x < 0)
        dp.x = 0;
    if (dp.y < 0)
        dp.y = 0;
    if (dp.x + WD_WIDTH > MAP_WIDTH)
        dp.x = MAP_WIDTH - WD_WIDTH;
    if (dp.y + WD_HEIGHT > MAP_HEIGHT)
        dp.y = MAP_HEIGHT - WD_HEIGHT;
    gGame.dp = dp; //dpがこの関数内だけの変数だったため, 計算した結果をgGame.dpに格納
}

/* 向きパターン設定
 *  キャラの方向から向きのアニメーションパターンを設定する
 *
 * 引数
 *   ch: キャラ情報
 */
void SetDirPattern(CharaInfo *ch)
{
    // 止まっているときは変更しない
    if (ch->dir.x == 0.0 && ch->dir.y == 0.0)
        return;

    // キャラの向き角度[-pi, pi]
    float arc = atan2f(ch->dir.y, ch->dir.x);
    // 画像の向きの境界角度算出（向きが4種類の時は90度，8種類の時は45度）
    float d = 2.0 * M_PI / ch->entity->aninum.y;
    // パターンの算出（画像は↑から時計回りを想定）
    // ←(-pi)から調査するが，
    // 画像は↑(-pi/2)が0のため，初期値を←に調整する
    int pat = ch->entity->aninum.y * 3 / 4;              // 画像の初期値が↑からだったため, ←殻に変更
    for (float f = -M_PI; f < M_PI + d; f += d, pat++) { // 誤差許容のためM_PI+αまで
        // f ± d/2 の範囲に arc があればその向きを設定
        if (f - d / 2 <= arc && arc < f + d / 2) {
            ch->ani.y = pat % ch->entity->aninum.y;
            break;
        }
    }
}

/* ウインドウ描画
 *  メインウインドウに背景，キャラ，メッセージなどを転送する
 */
void RenderWindow(void)
{
    /* マップ */
    SetDispPoint();
    SDL_Rect src = { gGame.dp.x, gGame.dp.y, WD_WIDTH, WD_HEIGHT };
    SDL_Rect dst = { 0, 0, WD_WIDTH, WD_HEIGHT };
    if (0 > SDL_RenderCopy(gGame.render, gGame.bg, &src, &dst)) {
        PrintError(SDL_GetError());
    }

    /* キャラ */
    for (int i = 0; i < gCharaNum; i++) {
        CharaInfo *ch = &(gChara[i]);
        // Disableは描画しない
        if (ch->stts == CS_Disable)
            continue;

        // 向き設定
        SetDirPattern(ch);

        /* 転送元設定
           キャラ画像1つ分の内，imgsrcに設定された領域のみを
           転送元にする
         */
        ch->imgsrc.y = ch->ani.y * ch->imgsrc.h;
        /* 転送先設定 */
        SDL_Rect dst = { ch->point.x - gGame.dp.x, ch->point.y - gGame.dp.y, ch->imgsrc.w, ch->imgsrc.h }; //カメラがプレイヤーに追尾して動くように修正

        /* 子のサイズを3/4にする
           （始めから小さい画像データを使っていればこの処理は不要）
           （今回はプレイヤーと同じ画像データを使っているため）
         */
        if (ch->type == CT_Child) {
            dst.w = dst.w * 3 / 4;
            dst.h = dst.h * 3 / 4;
        }

        // 転送
        if (0 > SDL_RenderCopy(gGame.render, ch->entity->img, &(ch->imgsrc), &dst)) {
            PrintError(SDL_GetError());
        }
    }

    /* メッセージ（ウインドウの中心に） */
    if (gGame.msg != MSG_None) {
        SDL_Rect src = { 0 };
        if (0 > SDL_QueryTexture(gGame.msgTexts[gGame.msg], NULL, NULL, &src.w, &src.h)) {
            PrintError(SDL_GetError());
        }
        SDL_Rect dst;
        dst.x = (WD_WIDTH - src.w) / 2;
        dst.w = src.w;
        dst.y = (WD_HEIGHT - src.h) / 2;
        dst.h = src.h;

        if (0 > SDL_RenderCopy(gGame.render, gGame.msgTexts[gGame.msg], &src, &dst)) {
            PrintError(SDL_GetError());
        }
    }

    /* レンダラーをウインドウに反映
       この関数の呼び出し間隔を1フレームとしている
     */
    SDL_RenderPresent(gGame.render);

/* デバッグ用 */
#ifdef DEBUG_SHOW
    DebugShow();
#endif
}

/* end of window.c */
