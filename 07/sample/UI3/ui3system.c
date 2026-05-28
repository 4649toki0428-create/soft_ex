/*
 *  ファイル名	：ui3system.c
 *  機能	：ゲームシステム処理
 */
#include "ui3system.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINEBUF 256

/* 変数実体 */
GameInfo gGame;
CharaInfo* gChara;
int gCharaNum;

/* 関数 */
static SDL_bool AdjustPoint(CharaInfo* ch, SDL_Rect* overlap);
static SDL_bool ReflectDir(CharaInfo* ch, SDL_Rect* overlap);
static int GetWallsOnChara(CharaInfo* ch, SDL_Rect* chips);
static SDL_bool AdjustOverlapMap(CharaInfo* ch, SDL_Rect* mt, int mtcnt, SDL_bool chdir);
static void UpdatePlayerDir(void);
static SDL_FPoint GetRandomDir(void);
static void InitCharaInfo(CharaInfo* chara);

/* ゲームシステム初期化
 *
 * 引数
 *   chara_data_file: キャラクタデータファイルのパス
 *   map_data_file  : マップデータファイルのパス
 *
 * 返値
 *   正常終了: 0
 *   エラー  : 負数
 */
int InitSystem(const char* chara_data_file, const char* map_data_file)
{
    int ret = 0;
    /* 乱数初期化 */
    srand(time(NULL));

    /** マップ情報読込 **/
    /* ファイルオープン */
    FILE* fp = fopen(map_data_file, "r");
    if (fp == NULL) {
        return PrintError("failed to open map data file.");
    }
    /* 1行読込 */
    int mapy = 0;
    char linebuf[MAX_LINEBUF];
    while (fgets(linebuf, MAX_LINEBUF, fp)) {
        /* 先頭が#の行はコメントとして飛ばす */
        if (linebuf[0] == '#')
            continue;
        /* マップ配置読込 */
        else if (mapy < MAP_Height) {
            char* lp = linebuf;
            int ln   = 0;
            for (int x = 0; x < MAP_Width; x++) {
                /* 値の読込(意味はMapTypeに準ずる) */
                if (1 != sscanf(lp, "%u%n", &(gGame.map[x][mapy]), &ln)) {
                    ret = PrintError("failed to load map data.");
                    goto CLOSEFILE;
                }
                lp += ln;
            }
            mapy++;
        }
    }
    fclose(fp);

    /** キャラクター情報読込 **/
    /* ファイルオープン */
    fp = fopen(chara_data_file, "r");
    if (fp == NULL) {
        return PrintError("failed to open chara data file.");
    }
    /* 1行読込 */
    int charano = -1;
    while (fgets(linebuf, MAX_LINEBUF, fp)) {
        /* 先頭が#の行はコメントとして飛ばす */
        if (linebuf[0] == '#')
            continue;
        /* キャラ総数読込 */
        else if (charano < 0) {
            if (1 != sscanf(linebuf, "%d", &gCharaNum)) {
                ret = PrintError("failed to read the number of chara data.");
                goto CLOSEFILE;
            }
            /* キャラ情報確保 */
            gChara = (CharaInfo*)malloc(sizeof(CharaInfo) * gCharaNum);
            if (!gChara) {
                ret = PrintError("failed to allocate the chara data.");
                goto CLOSEFILE;
            }
            charano = 0;
        }
        /* キャラ情報読込 */
        else if (charano < gCharaNum) {
            if (5 != sscanf(linebuf, "%u%f%f%d%d", &(gChara[charano].type), &(gChara[charano].mass), &(gChara[charano].basevel), &(gChara[charano].w), &(gChara[charano].h))) {
                ret = PrintError("failed to read the chara data.");
                break;
            }
            if (gChara[charano].type == CT_Player) {
                /* プレイヤー番号のセット */
                gGame.player = charano;
            }
            /** キャラ情報設定 **/
            InitCharaInfo(&gChara[charano]);
            charano++;
        }
    }
CLOSEFILE:
    fclose(fp);
    return ret;
}

/* システム終了処理 */
void DestroySystem(void)
{
    free(gChara);
    return;
}

/* キャラ状態の初期設定
 *
 * 引数
 *   chara: キャラ
 */
void InitCharaInfo(CharaInfo* chara)
{
    /* 初期値設定 */
    chara->rest     = 0;
    chara->stts     = CS_Normal;
    chara->velocity = 0.0;
    chara->dir      = GetRandomDir();
    /* 床の位置にランダムに配置 */
    SDL_Point p;
    MapType t;
    do {
        p.x = rand() % MAP_Width;
        p.y = rand() % MAP_Height;
        t   = gGame.map[p.x][p.y];
    } while (t != MT_Floor);
    chara->point.x = p.x * MAP_ChipSize;
    chara->point.y = p.y * MAP_ChipSize;
}

/* 重なりを補正する
 *  対象キャラの座標をoverlapと重ならない位置まで補正する
 *
 * 引数
 *   ch  : 対象キャラ
 *   overlap: 重なり部分
 *
 * 返値: 補正したらTRUE
 */
SDL_bool AdjustPoint(CharaInfo* ch, SDL_Rect* overlap)
{
    SDL_bool ret = SDL_FALSE;
    SDL_Point p  = { ch->point.x, ch->point.y };

    if (overlap->w && overlap->h) {
        ret = SDL_TRUE;
        if ((ch->w > overlap->w)
            && (overlap->w < overlap->h)) {
            /* x方向の重なりの補正
                overlapはキャラ矩形の4辺のいずれかに接するので
                接する辺の逆方向に座標を補正する
             */
            if (overlap->x > p.x) {
                ch->point.x -= overlap->w;
            } else {
                ch->point.x += overlap->w;
            }
        } else {
            /* y方向の重なりの補正 */
            if (overlap->y > p.y) {
                ch->point.y -= overlap->h;
            } else {
                ch->point.y += overlap->h;
            }
        }
    }
    return ret;
}

/* 向きを変える
 *  対象キャラの向きをoverlapの反対方向へ変える
 *
 * 引数
 *   ch  : 対象キャラ
 *   overlap: 重なり部分
 *
 * 返値: 補正したらTRUE
 */
SDL_bool ReflectDir(CharaInfo* ch, SDL_Rect* overlap)
{
    SDL_bool ret = SDL_FALSE;
    if (overlap->w && overlap->h) {
        ret = SDL_TRUE;
        if ((ch->w > overlap->w)
            && (overlap->w < overlap->h)) {
            /* x方向の向きを変える */
            ch->dir.x = -ch->dir.x;
        } else {
            /* y方向の向きを変える */
            ch->dir.y = -ch->dir.y;
        }
    }
    return ret;
}

/* キャラ位置のマップ矩形取得
 *  キャラ位置のマップが壁のとき，その矩形を取得する
 *  キャラの矩形サイズ < MAP_ChipSize であることを想定
 *  このため，キャラ位置のマップ矩形は最大でも4となる
 *  (実際は結合しているので2まで)
 *
 * 引数
 *  ch : キャラ情報
 *  chips : 取得する矩形
 *
 * 返値
 *  矩形の数
 */
int GetWallsOnChara(CharaInfo* ch, SDL_Rect* chips)
{
    /* キャラ位置からマップ配置位置を算出
       [x][y], [x2][y],
       [x][y2],[x2][y2]が対象 */
    int x  = ch->point.x;
    int x2 = (x + ch->w) / MAP_ChipSize;
    x /= MAP_ChipSize;
    int y  = ch->point.y;
    int y2 = (y + ch->h) / MAP_ChipSize;
    y /= MAP_ChipSize;

    int ret = 0;
    int flg = 0; /* 取得したチップの位置番号，左上が1，右上が2，左下が3，上二つが12，左二つが13 */
    if (gGame.map[x][y] == MT_Wall) {
        /* [x][y]が壁のとき */
        chips[ret] = (SDL_Rect) { x * MAP_ChipSize, y * MAP_ChipSize, MAP_ChipSize, MAP_ChipSize };
        ret++;
        flg = 1;
    }
    if (x < x2) {
        /* x==x2のときは[x][y],[x][y2]のみ*/
        if (gGame.map[x2][y] == MT_Wall) {
            if (flg == 0) {
                /* [x2][y]のみ壁のとき */
                chips[ret] = (SDL_Rect) { x2 * MAP_ChipSize, y * MAP_ChipSize, MAP_ChipSize, MAP_ChipSize };
                ret++;
                flg = 2;
            } else {
                /* [x][y],[x2][y]が壁のとき結合 */
                chips[ret - 1].w += MAP_ChipSize;
                flg = 12;
            }
        }
    }
    if (y < y2) {
        /* y==y2のときは[x][y],[x2][y]のみ */
        if (gGame.map[x][y2] == MT_Wall) {
            if (flg == 1) {
                /* [x][y],[x][y2]が壁のとき結合 */
                chips[ret - 1].h += MAP_ChipSize;
                flg = 13;
            } else {
                /* [x][y2]のみか，[x2][y],[x][y2]が壁 */
                chips[ret] = (SDL_Rect) { x * MAP_ChipSize, y2 * MAP_ChipSize, MAP_ChipSize, MAP_ChipSize };
                ret++;
                flg = 3;
            }
        }
        if (x < x2) {
            if (gGame.map[x2][y2] == MT_Wall) {
                if (flg == 2) {
                    /* [x2][y],[x2][y2]を結合 */
                    chips[ret - 1].h += MAP_ChipSize;
                } else if (flg == 3) {
                    /* [x][y2],[x2][y2]を結合 */
                    chips[ret - 1].w += MAP_ChipSize;
                } else {
                    /* [x2][y2]のみか，[x][y],[x2][y2]が壁 */
                    chips[ret] = (SDL_Rect) { x2 * MAP_ChipSize, y2 * MAP_ChipSize, MAP_ChipSize, MAP_ChipSize };
                    ret++;
                }
            }
        }
    }
    return ret;
}

/* マップとの重なりを補正する
 *  対象キャラが壁に接したときの座標を補正する
 *
 * 引数
 *   ch   : キャラ情報
 *   mt : 補正対象の壁矩形
 *   mtcnt : mtの数
 *   chdir : キャラの向きも変える（反射させる）ときは true
 *
 * 返値: 補正したときSDL_TRUE
 */
SDL_bool AdjustOverlapMap(CharaInfo* ch, SDL_Rect* mt, int mtcnt, SDL_bool chdir)
{
    SDL_bool ret = SDL_FALSE;
    SDL_Rect chrect;
    chrect.x = ch->point.x;
    chrect.y = ch->point.y;
    chrect.w = ch->w;
    chrect.h = ch->h;

    /* マップの矩形一覧とキャラ矩形との重なりを調べ，
     * 重なっていれば補正する
     */
    for (int i = 0; i < mtcnt; i++) {
        SDL_Rect r;
        if (SDL_IntersectRect(&chrect, &(mt[i]), &r)) {
            if (SDL_RectEquals(&chrect, &r)) {
                /* マップ矩形内に完全に埋もれたときは
                 * 少し巻き戻して補正
                 */
                MoveChara(ch, -0.3);
                return AdjustOverlapMap(ch, mt, mtcnt, chdir);
            }
            if (AdjustPoint(ch, &r)) {
                if (chdir) {
                    ReflectDir(ch, &r);
                }
                ret = SDL_TRUE;
            }
        }
    }

    return ret;
}

/* 入力状態からプレイヤーの方向を更新 */
void UpdatePlayerDir(void)
{
    SDL_FPoint dir = { 0.0, 0.0 };
    if (gGame.key.up && !gGame.key.down) {
        dir.y = -0.7;
    }
    if (gGame.key.down && !gGame.key.up) {
        dir.y = 0.7;
    }
    if (gGame.key.left && !gGame.key.right) {
        dir.x = -0.7;
    }
    if (gGame.key.right && !gGame.key.left) {
        dir.x = 0.7;
    }
    /* 斜め方向の時，縦のみ，横のみと同じ
       （ベクトルの）大きさ(0.7)になるように補正 */
    if (dir.x != 0.0 && dir.y != 0.0) {
        dir.x *= cosf(M_PI / 4.0);
        dir.y *= sinf(M_PI / 4.0);
    }
    gChara[gGame.player].dir = dir;
}

/* キャラの情報更新
 *  対象キャラの情報（状態，行動など）を更新する
 *
 * 引数
 *   ch: 対象キャラ
 */
void UpdateCharaInfo(CharaInfo* ch)
{
    SDL_bool reflect_flg = SDL_TRUE;

    switch (ch->stts) {
    case CS_Normal:
        if (ch->type == CT_Player) {
            reflect_flg = SDL_FALSE;
            UpdatePlayerDir();
        }
        /* 速度を基準値に戻す */
        ch->velocity = ch->basevel;
        break;
    case CS_Kickback:
        ch->rest--;
        /* 終わったら通常状態に戻る */
        if (ch->rest <= 0) {
            ch->rest = 0;
            ch->stts = CS_Normal;
        }
    default:
        break;
    }

    SDL_Rect chips[4];
    int chipcnt = GetWallsOnChara(ch, chips);
    AdjustOverlapMap(ch, chips, chipcnt, reflect_flg);
}

/* キャラの移動
 *  対象キャラの座標を（指定フレーム分）更新する
 *
 * 引数
 *   ch: 対象キャラ
 *   frame: フレーム数，巻き戻すときは負数を指定する
 */
void MoveChara(CharaInfo* ch, float frame)
{
    SDL_FPoint newpoint = ch->point;

    /* 向きに応じた移動(等速運動 p=vt) */
    newpoint.x += ch->dir.x * ch->velocity * gGame.timeStep * frame;
    newpoint.y += ch->dir.y * ch->velocity * gGame.timeStep * frame;

    /* マップ外となったときはマップ内まで戻す */
    if (newpoint.x < 0)
        newpoint.x = 0.0;
    else if (newpoint.x + ch->w >= WD_Width)
        newpoint.x = WD_Width - ch->w;
    if (newpoint.y < 0)
        newpoint.y = 0.0;
    else if (newpoint.y + ch->h >= WD_Height)
        newpoint.y = WD_Height - ch->h;

    /* 座標更新 */
    ch->point = newpoint;
}

/* 当たり判定
 *  対象キャラ同士が重なったか調べ，状態更新などをする
 *
 * 引数
 *   ci: 対象キャラ
 *   cj: 対象キャラ
 */
void Collision(CharaInfo* ci, CharaInfo* cj)
{
    /* 当たり矩形をマップ座標に合わせる */
    SDL_Rect mi;
    mi.x = ci->point.x;
    mi.y = ci->point.y;
    mi.w = ci->w;
    mi.h = ci->h;
    SDL_Rect mj;
    mj.x = cj->point.x;
    mj.y = cj->point.y;
    mj.w = cj->w;
    mj.h = cj->h;
    /* 当たり（重なり）判定 */
    SDL_Rect r;
    if (SDL_IntersectRect(&mi, &mj, &r)) {
        /* 重ならない位置へずらす */
        AdjustPoint(ci, &r);

        /* 反発係数
         * e==-(cj_v1 - ci_v1)/(cj_v0 - ci_v0)
         * 運動量保存の法則
         * ci->mass*ci_v0+cj->mass*cj_v0==ci->mass*ci_v1+cj->mass*cj_v1
         * を変形して衝突後の速度ci_v1,cj_v1を算出
         */
        float e          = 1.0;
        SDL_FPoint ci_v0 = { ci->dir.x * ci->velocity, ci->dir.y * ci->velocity };
        SDL_FPoint cj_v0 = { cj->dir.x * cj->velocity, cj->dir.y * cj->velocity };
        SDL_FPoint ci_v1, cj_v1;
        ci_v1.x = (ci->mass * ci_v0.x + cj->mass * cj_v0.x + cj_v0.x * e * cj->mass - ci_v0.x * e * cj->mass) / (ci->mass + cj->mass);
        cj_v1.x = -e * (cj_v0.x - ci_v0.x) + ci_v1.x;

        ci_v1.y = (ci->mass * ci_v0.y + cj->mass * cj_v0.y + cj_v0.y * e * cj->mass - ci_v0.y * e * cj->mass) / (ci->mass + cj->mass);
        cj_v1.y = -e * (cj_v0.y - ci_v0.y) + ci_v1.y;

        /* 方向と大きさに分解 */
        ci->velocity = sqrtf(ci_v1.x * ci_v1.x + ci_v1.y * ci_v1.y);
        ci->dir.x    = (ci->velocity == 0.0) ? 0.0 : ci_v1.x / ci->velocity;
        ci->dir.y    = (ci->velocity == 0.0) ? 0.0 : ci_v1.y / ci->velocity;

        cj->velocity = sqrtf(cj_v1.x * cj_v1.x + cj_v1.y * cj_v1.y);
        cj->dir.x    = (cj->velocity == 0.0) ? 0.0 : cj_v1.x / cj->velocity;
        cj->dir.y    = (cj->velocity == 0.0) ? 0.0 : cj_v1.y / cj->velocity;

        /* 残りフレームと状態更新 */
        ci->rest = cj->rest = KICKBACK_TIME / gGame.timeStep;
        ci->stts = cj->stts = CS_Kickback;
    }
}

/* ランダムな向きを得る
 *
 * 返値: 向き
 */
SDL_FPoint GetRandomDir(void)
{
    SDL_FPoint r = { (float)rand() / RAND_MAX, 0.0 };
    r.y          = sqrtf(1 - r.x * r.x);
    return r;
}

/* end of ui3system.c */
