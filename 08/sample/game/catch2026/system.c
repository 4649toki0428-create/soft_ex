/*
 *  ファイル名	：system.c
 *  機能	：ゲームシステム処理
 */
#include "system.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINEBUF 256 // 1行の最大文字数

/* 変数 */
static char gImgFilePath[CHARATYPE_NUM][MAX_LINEBUF]; // パス保存用
static PointQueue gQueue;                             // 座標キュー
GameInfo gGame;
CharaTypeInfo gCharaType[CHARATYPE_NUM];
CharaInfo* gChara;
int gCharaNum;

/* 関数 */
static void InitCharaInfo(CharaInfo* ch);
static AdjustDir AdjustPoint(CharaInfo* cadj, CharaInfo* cfix, SDL_Rect* rt);
static float GetDistance(const CharaInfo* ci, const CharaInfo* cj);
static void UpdateChildInfo(CharaInfo* ch);
static CharaInfo* SearchChara(const CharaInfo* ch);

/* 座標キューへのデータ追加
 *  キューが一杯のときは，古いデータを上書きして消す
 *
 * 引数
 *   fp: 追加するデータ
 */
void EnqueuePoint(SDL_FPoint fp)
{
    // データ追加
    gQueue.queue[gQueue.tail] = fp;
    // tailを進める
    gQueue.tail = (gQueue.tail + 1) % QUEUE_MAX;
    if (gQueue.tail == gQueue.head) {
        // 一杯のときは古いデータを消す（headを進める）
        gQueue.head = (gQueue.head + 1) % QUEUE_MAX;
    }
}

/* 座標キューからデータを読む（取り出さない）
 *  最後の追加から指定の数遡ったデータを返す
 *
 * 引数
 *   delay: 遡る数
 *
 * 返値: データ
 */
SDL_FPoint PeekQueuePoint(int delay)
{
    // キューのデータ数
    int size = gQueue.tail - gQueue.head + ((gQueue.head < gQueue.tail) ? 0 : QUEUE_MAX);
    // 遡った位置
    int p = (delay >= size) ? gQueue.head : (QUEUE_MAX + gQueue.tail - delay) % QUEUE_MAX;
    // 値を返す（取り出さない）
    return gQueue.queue[p];
}

/* ゲームシステム初期化
 *
 * 引数
 *   chara_data_file: キャラクタデータファイルのパス
 *   position_data_file  : 配置データファイルのパス
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int InitSystem(const char* chara_data_file, const char* position_data_file)
{
    int ret = 0;
    /* 乱数初期化 */
    srand(time(NULL));

    /** キャラクター情報読込 **/
    /* ファイルオープン */
    FILE* fp = fopen(chara_data_file, "r");
    if (fp == NULL) {
        return PrintError("failed to open chara data file.");
    }
    /* 1行読込 */
    int typeno = 0;
    char linebuf[MAX_LINEBUF];
    while (fgets(linebuf, MAX_LINEBUF, fp)) {
        /* 先頭が#の行はコメントとして飛ばす */
        if (linebuf[0] == '#')
            continue;
        /* キャラタイプ別情報読込 */
        if (typeno < CHARATYPE_NUM) {
            CharaTypeInfo* ct = &(gCharaType[typeno]);
            if (7 != sscanf(linebuf, "%d%d%d%d%d%d%s", &(ct->w), &(ct->h), &(ct->baserect.x), &(ct->baserect.y), &(ct->baserect.w), &(ct->baserect.h), gImgFilePath[typeno])) {
                ret = PrintError("failed to read the chara image data.");
                goto CLOSEFILE;
            }
            ct->path = gImgFilePath[typeno];
            typeno++;
        }
    }
    fclose(fp);

    /** キャラ配置情報読込 **/
    /* ファイルオープン */
    fp = fopen(position_data_file, "r");
    if (fp == NULL) {
        return PrintError("failed to open position data file.");
    }
    /* 読込 */
    // 1行読む
    int charano = 0;
    while (fgets(linebuf, MAX_LINEBUF, fp)) {
        /* 先頭が#の行はコメントとして飛ばす */
        if (linebuf[0] == '#')
            continue;
        // 総数の読み込み，キャラ情報確保
        if (gCharaNum == 0) {
            // 総数の読み込み
            if (1 != sscanf(linebuf, "%d", &gCharaNum)) {
                ret = PrintError("failed to read the number of position data.");
                goto CLOSEFILE;
            }
            // キャラ情報確保
            gChara = (CharaInfo*)malloc(sizeof(CharaInfo) * gCharaNum);
            if (!gChara) {
                ret = PrintError("failed to allocate the chara data.");
                goto CLOSEFILE;
            }
        }
        // 配置読み込み
        else if (charano < gCharaNum) {
            CharaInfo* ch = &(gChara[charano++]);
            if (4 != sscanf(linebuf, "%u%f%f%f", &(ch->type), &(ch->point.x), &(ch->point.y), &(ch->vel))) {
                ret = PrintError("failed to load position data.");
                goto CLOSEFILE;
            }

            /* キャラ情報設定 */
            InitCharaInfo(ch);
        }
    }

CLOSEFILE:
    fclose(fp);
    return ret;
}

/* システム終了処理 */
void DestroySystem(void)
{
    // 確保した要素を削除
    free(gChara);
    return;
}

/* キャラ情報の設定
 *  キャラクタの状態を初期設定する
 *
 * 引数
 *   ch: キャラ情報
 */
void InitCharaInfo(CharaInfo* ch)
{
    ch->stts   = CS_Normal;
    ch->entity = &(gCharaType[ch->type]);
    ch->dir.x  = 0.0;
    ch->dir.y  = 0.0;
    ch->ani.x  = 0;
    ch->ani.y  = 0;
    ch->next   = NULL;
    // 画像転送元の初期値設定
    ch->imgsrc.x = 0;
    ch->imgsrc.y = 0;
    ch->imgsrc.w = ch->entity->w;
    ch->imgsrc.h = ch->entity->h;
    // 当たり矩形を絶対座標（マップ上の座標）に
    ch->rect = ch->entity->baserect;
    ch->rect.x += ch->point.x;
    ch->rect.y += ch->point.y;
    // タイプ別の設定
    switch (ch->type) {
    case CT_Player:
        gGame.player = ch;
        break;
    case CT_Wall:
        ch->next   = gGame.wall;
        gGame.wall = ch;
        // 読み込んだ値を 横．縦 と想定し，幅と高さを設定
        float tmpi;
        float tmpf   = modff(ch->vel, &tmpi);
        ch->imgsrc.w = tmpi;
        ch->imgsrc.h = tmpf * 1000.0;

        //当たり判定を画像サイズと同じにする
        ch->rect.w = ch->imgsrc.w;
        ch->rect.h = ch->imgsrc.h;
        break;
    default:
        break;
    }
}

/* 距離を得る
 *  2つのキャラ間の距離を返す
 *  （当たり矩形の中心の距離）
 *
 * 引数
 *   ci, cj: 対象キャラ
 *
 * 返値: 距離
 */
float GetDistance(const CharaInfo* ci, const CharaInfo* cj)
{
    // 中心を計算
    SDL_FPoint fi = { ci->point.x + ci->entity->baserect.x + ci->rect.w / 2.0,
        ci->point.y + ci->entity->baserect.y + ci->rect.h / 2.0 };
    SDL_FPoint fj = { cj->point.x + cj->entity->baserect.x + cj->rect.w / 2.0,
        cj->point.y + cj->entity->baserect.y + cj->rect.h / 2.0 };

    // 距離を返す
    SDL_FPoint f = { fj.x - fi.x, fj.y - fi.y };
    return sqrtf((f.x * f.x) + (f.y * f.y));
}

/* キャラを探す
 *  探索範囲内で最も近いキャラ（プレイヤーorつかまっている子）を返す
 *
 * 引数
 *   ch   : キャラ情報
 *
 * 返値: キャラ情報，範囲内にいないときはNULL
 */
CharaInfo* SearchChara(const CharaInfo* ch)
{
    // 対象外を除く
    if (ch->stts == CS_Held)
        return NULL;

    CharaInfo* ret = NULL;
    float near     = 0.0;
    for (CharaInfo* p = gGame.player; p; p = p->next) {

        /* 間に壁があるときは除外（見えなかったことにする）
            2点を結ぶ直線と壁の矩形が重なるか，で判定
         */
        SDL_bool flg = SDL_FALSE;
        for (CharaInfo* w = gGame.wall; w; w = w->next) {

            SDL_Point chpt = { ch->point.x, ch->point.y };
            SDL_Point ppt  = { p->point.x, p->point.y };
            flg            = SDL_IntersectRectAndLine(&(w->rect), &(chpt.x), &(chpt.y), &(ppt.x), &(ppt.y));
            if (flg)
                break;
        }

        // 近いキャラを探す
        float dist = GetDistance(ch, p);
        if (!ret || near > dist) {
            ret  = p;
            near = dist;
        }
    }
    return ret;
}

/* 子キャラの状態更新
 *
 * 引数
 *   ch   : キャラ情報
 */
void UpdateChildInfo(CharaInfo* ch)
{
    // つかまっているときは更新なし
    if (ch->stts == CS_Held)
        return;

    // 探索範囲内のキャラを探す
    CharaInfo* near = SearchChara(ch);
    if (near) {
        // 相手によって状態を変える
        switch (near->type) {
        case CT_Player:
            ch->stts = CS_RunAway;
            break;
        case CT_Child:
            ch->stts = CS_Rescue;
            break;
        default:
            break;
        }
    } else {
        ch->stts = CS_Normal;
    }

    // 状態ごとに処理
    float rad = 0.0;
    switch (ch->stts) {
    case CS_Normal:
        // ランダム行動

        // 現在の向きの±MOVABLE_DIR度の方向をランダムで決める
        rad = atan2(ch->dir.y, ch->dir.x);
        rad += (rand() % (MOVABLE_DIR * 2) - MOVABLE_DIR) * M_PI / 180.0;
        ch->dir.x = cosf(rad);
        ch->dir.y = sinf(rad);

        break;
    case CS_RunAway:
        // プレイヤーから離れる方向に
        rad       = atan2(ch->point.y - near->point.y, ch->point.x - near->point.x);
        ch->dir.x = cosf(rad);
        ch->dir.y = sinf(rad);

        break;
    case CS_Rescue:
        // 子に近づく方向に
        rad       = atan2(near->point.y - ch->point.y, near->point.x - ch->point.x);
        ch->dir.x = cosf(rad);
        ch->dir.y = sinf(rad);

    default:
        break;
    }
}

/* キャラの状態更新
 *
 * 引数
 *   ch   : キャラ情報
 */
void UpdateCharaInfo(CharaInfo* ch)
{
    // タイプごとに設定
    switch (ch->type) {
    case CT_Player: {
        // キー入力状態を元に向きを計算
        SDL_FPoint dir = { 0.0, 0.0 };
        if (gGame.input.up && !gGame.input.down) {
            dir.y = -1.0;
        }
        if (gGame.input.down && !gGame.input.up) {
            dir.y = 1.0;
        }
        if (gGame.input.left && !gGame.input.right) {
            dir.x = -1.0;
        }
        if (gGame.input.right && !gGame.input.left) {
            dir.x = 1.0;
        }
        // 斜め方向の時は大きさ1となるように補正
        if (dir.x != 0.0 && dir.y != 0.0) {
            dir.x *= cosf(M_PI / 4.0);
            dir.y *= sinf(M_PI / 4.0);
        }
        ch->dir = dir; //ch->dirの数値が変わってもdirの数値が変わらなかったため, 対応するようにした
    } break;
    case CT_Child:
        UpdateChildInfo(ch);
        break;
    case CT_Wall:
    default:
        break;
    }
}

/* 重なりを補正する
 *  対象キャラの座標を重ならない（当たらない）位置まで補正する
 *
 * 引数
 *   cadj : 対象キャラ（補正する方）
 *   cfix : 対象キャラ（固定の方）
 *   rt  : 重なり矩形
 *
 * 返値: 補正した方向
 */
AdjustDir AdjustPoint(CharaInfo* cadj, CharaInfo* cfix, SDL_Rect* rt)
{
    AdjustDir ret = AD_NONE;
    /* 判定が不要な組み合わせを除外 */
    if (cadj->stts == CS_Disable || cfix->stts == CS_Disable)
        return ret;

    // 補正する方向を決める
    // 側面を全て含むときはその方
    if (rt->w == cadj->rect.w)
        ret = AD_UD;
    else if (rt->h == cadj->rect.h)
        ret = AD_LR;
    // 一部を含むときは短い方
    else if (rt->w > rt->h)
        ret = AD_UD;
    else if (rt->w < rt->h)
        ret = AD_LR;
    // 縦横同じときは向きの大きい方
    else if (fabsf(cadj->dir.x) > fabsf(cadj->dir.y))
        ret = AD_LR;
    else
        ret = AD_UD;

    /* 補正 */
    if (ret == AD_LR) { // x方向
        int dx = cadj->rect.x - (int)(cadj->point.x);
        cadj->point.x += (cadj->rect.x == rt->x) ? rt->w : -rt->w;
        cadj->rect.x = cadj->point.x + dx;
        // 次のランダム行動で壁に当たらないよう，向きを変える
        cadj->dir.x = -cadj->dir.x;

    } else { // y方向
        int dy = cadj->rect.y - (int)(cadj->point.y);
        cadj->point.y += (cadj->rect.y == rt->y) ? rt->h : -rt->h;
        cadj->rect.y = cadj->point.y + dy;
        // 次のランダム行動で壁に当たらないよう，向きを変える
        cadj->dir.y = -cadj->dir.y;
    }
    return ret;
}

/* 当たり判定
 *  対象キャラ同士が重なったか調べ，状態更新などをする
 *
 * 引数
 *   ci: 対象キャラ
 *   cj: 対象キャラ
 *
 * 返値: 当たった（重なった）ときTRUE
 */
SDL_bool Collision(CharaInfo* ci, CharaInfo* cj)
{
    /* 判定が不要な組み合わせを除外 */
    if (ci->stts == CS_Disable || cj->stts == CS_Disable)
        return SDL_FALSE;

    /* 通常の当たり（重なり）判定 */
    /* 当たり判定用矩形が重なっていると当たり*/
    SDL_Rect ir;
    if (SDL_IntersectRect(&(ci->rect), &(cj->rect), &ir)) {
        // 対象キャラの種類別処理

        // 壁と重なるときは補正
        if (ci->type == CT_Wall || cj->type == CT_Wall) {
            //子供が壁を押すと壁が動いたため（cj->typeのとき壁が固定されていなかっため）修正
            if(ci->type == CT_Wall)
            {
                AdjustPoint(cj, ci, &ir);
            }
            else
            {
                AdjustPoint(ci, cj, &ir);
            }
        }

        // プレイヤーと重なるときはつかまえる
        else if (ci->type == CT_Player || cj->type == CT_Player) {
            CharaInfo* cply = (ci->type == CT_Player) ? ci : cj;
            CharaInfo* coth = (ci->type == CT_Player) ? cj : ci;

            // つかまっている子は無視（当たっていないことにする）
            if (coth->stts == CS_Held) {
                return SDL_FALSE;
            }
            // つかまえる（リストに追加）
            else {
                coth->next = cply->next;
                cply->next = coth;
                // 状態更新
                coth->stts = CS_Held;
            }
        }

        // その他（子同士のとき）
        else {
            // 片方がつかまっているとき（助けたとき）
            if (ci->stts == CS_Held || cj->stts == CS_Held) {
                CharaInfo* chld = (ci->stts == CS_Held) ? ci : cj;
                CharaInfo* crsc = (ci->stts == CS_Held) ? cj : ci;

                // 他方もつかまっている，のときは無視（当たっていないことにする）
                if (crsc->stts == CS_Held) {
                    return SDL_FALSE;
                }
                // 助ける
                else {
                    // リストから外す
                    CharaInfo* prev = gGame.player;
                    for (CharaInfo* p = prev->next; p; prev = p, p = p->next) {
                        if (p == chld) {
                            prev->next = NULL;
                            break;
                        }
                    }
                    // 助けられた子の状態更新
                    for (; chld; chld = chld->next) {
                        chld->stts = CS_RunAway;
                    }
                    // 助けた方の状態更新
                    crsc->stts = CS_RunAway;
                }
            }
        }
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

/* end of system.c */
