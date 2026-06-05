/*
 *  ファイル名	: system.h
 *  機能	: 共通変数，外部関数の定義
 */
#pragma once

/* 下のコメントを外すと
    簡易マップを表示してくれます
    デバッグで活用してください
 */
#define DEBUG_SHOW

#include <SDL2/SDL.h>

/* マップサイズ(ピクセル) */
#define MAP_WIDTH 720
#define MAP_HEIGHT 720

/* ウインドウサイズ(ピクセル) */
#define WD_WIDTH 360
#define WD_HEIGHT 360

/* 定義 */
#define DELAY_BASE 15     // つかまっているときの遅延基準値
#define SEARCH_RADIUS 150 // 逃げる，助けるときの探索範囲
#define MOVABLE_DIR 15    // ランダム行動での方向の範囲

/* キャラクタータイプ */
typedef enum {
    CT_Player = 0, // プレイヤー
    CT_Child  = 1, // 子
    CT_Wall   = 2, // 壁
} CharaType;
#define CHARATYPE_NUM 3 // キャラタイプ総数

/* キャラクタータイプ別情報 */
typedef struct {
    int w;             // キャラ(画像1つ)の幅
    int h;             // 　　　　　　　 高さ
    SDL_Rect baserect; // 当たり判定用矩形（画像の相対座標）
    char* path;        // 画像ファイル名
    SDL_Point aninum;  // アニメーションパターン数（縦[y]に向き，横[x]にアニメーション）
    SDL_Texture* img;  // 画像のテクスチャー
} CharaTypeInfo;

/* キャラクターの状態 */
typedef enum {
    CS_Disable = 0, // 非表示
    CS_Normal  = 1, // 通常
    CS_Held    = 2, // つかまっている
    CS_RunAway = 3, // 逃げる
    CS_Rescue  = 4  // 助ける
} CharaStts;

/* キャラクターの情報 */
typedef struct CharaInfo_t {
    CharaStts stts;           // 現在の状態
    CharaType type;           // キャラクタータイプ
    CharaTypeInfo* entity;    // タイプ別情報の実体
    SDL_Rect rect;            // 当たり判定用矩形（マップ上の座標）
    SDL_Rect imgsrc;          // 画像転送元の領域（画像1つから実際に転送する部分）
    SDL_FPoint dir;           // 現在の向き（大きさ最大1となる成分）
    float vel;                // 現在の速度
    SDL_FPoint point;         // 現在の座標（画像の左上，マップにおける計算上の座標）
    SDL_Point ani;            // 現在のアニメーションパターン
    struct CharaInfo_t* next; // 連結リストの次の要素
} CharaInfo;

/* 座標キュー */
#define QUEUE_MAX 200
typedef struct PointQueue_t {
    SDL_FPoint queue[QUEUE_MAX];
    int head;
    int tail;
} PointQueue;

/* 補正の方向 */
typedef enum {
    AD_NONE, // 補正なし
    AD_LR,   // 左右
    AD_UD    // 上下
} AdjustDir;

/* メッセージ */
typedef enum {
    MSG_None     = 0, // 表示なし
    MSG_GameOver = 1,
    MSG_Clear    = 2
} Msg;
#define MSG_NUM 3 // メッセージの数

/* キー入力の状態 */
typedef struct {
    SDL_bool right;
    SDL_bool left;
    SDL_bool up;
    SDL_bool down;
} KeyStts;

/* ゲームの状態 */
typedef enum {
    GS_End     = 0, // 終了
    GS_Playing = 1, // 通常
    GS_Ready   = 2  // 開始前
} GameStts;

/* ゲームの情報 */
typedef struct {
    GameStts stts;
    KeyStts input;
    float timeDelta;                // 時間の増分(1フレームの時間,s)
    float TimeLimit;                // 時間制限
    float MaxTime;                  // 初期時間
    CharaInfo* player;              // プレイヤー
    CharaInfo* wall;                // 壁のリスト
    Msg msg;                        // 表示中のメッセージ番号
    SDL_Point dp;                   // ウインドウ表示位置
    SDL_Window* window;             // ウインドウ
    SDL_Renderer* render;           // レンダラー
    SDL_Texture* msgTexts[MSG_NUM]; // メッセージ画像
    SDL_Texture* bg;                // 背景画像
} GameInfo;

/* 変数 */
extern GameInfo gGame;
extern CharaTypeInfo gCharaType[CHARATYPE_NUM];
extern CharaInfo* gChara;
extern int gCharaNum; // キャラ総数

/* 関数 */
// catch.c
extern int PrintError(const char* str);
// system.c
extern int InitSystem(const char* chara_data_file, const char* map_data_file);
extern void DestroySystem(void);
extern void UpdateCharaInfo(CharaInfo* ch);
extern SDL_bool Collision(CharaInfo* ci, CharaInfo* cj);
extern void EnqueuePoint(SDL_FPoint fp);
extern SDL_FPoint PeekQueuePoint(int delay);
// window.c
extern int InitWindow(const char* bg_file);
extern void DestroyWindow(void);
extern void RenderWindow(void);
// debugshow.c
#ifdef DEBUG_SHOW
extern void DebugShowInit(void);
extern void DebugShow(void);
extern void DebugDestroy(void);
#endif

/* end of system.h */
