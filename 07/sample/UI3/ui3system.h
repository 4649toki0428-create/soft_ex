/*
 *  ファイル名	: ui3system.h
 *  機能	: 共通変数，外部関数の定義
 */
#pragma once

#include <SDL2/SDL.h>
#include <joyconlib.h>

/* マップサイズ(チップ数) */
#define MAP_Width 16
#define MAP_Height 16
#define MAP_ChipSize 24 /* 1チップの大きさ */

/* ウインドウサイズ */
#define WD_Width (MAP_Width * MAP_ChipSize)
#define WD_Height (MAP_Height * MAP_ChipSize)

/* キャラクタータイプ */
typedef enum {
    CT_Player = 0, /* プレイヤー */
    CT_Enemy  = 1, /* 敵 */
} CharaType;

/* キャラクターの状態 */
typedef enum {
    CS_Normal   = 1, /* 通常 */
    CS_Kickback = 4  /* キックバック */
} CharaStts;

/* 時間,距離など */
#define KICKBACK_TIME 0.5 /* キックバックの時間(s) */

/* キャラクターの情報 */
typedef struct {
    CharaStts stts;
    CharaType type;
    float mass;       /* 質量 */
    float basevel;    /* 速度の大きさの基準値 */
    float velocity;   /* 現在の速度の大きさ */
    SDL_FPoint dir;   /* 現在の方向（大きさ最大1となる成分） */
    SDL_FPoint point; /* 現在の座標 */
    int rest;         /* 残り時間(特殊な動作をするのに必要な，フレーム数) */
    int w;            /* キャラの幅 */
    int h;            /* 　　　　高さ */
} CharaInfo;

/* マップの種類 */
typedef enum {
    MT_Floor = 1, /* 床 */
    MT_Wall  = 2  /* 壁 */
} MapType;

/* ゲームの状態 */
typedef enum {
    GS_End     = 0, /* 終了 */
    GS_Playing = 1  /* ゲーム中 */
} GameStts;

/* キー入力の状態 */
typedef struct {
    SDL_bool up;
    SDL_bool down;
    SDL_bool left;
    SDL_bool right;
} KeyStts;

/* ゲームの情報 */
typedef struct {
    GameStts stts;
    KeyStts key;
    int player;     /* プレイヤーの配列要素番号 */
    float timeStep; /* 時間の増分(1フレームの時間,s) */
    SDL_Window* window;
    SDL_Renderer* render;
    MapType map[MAP_Width][MAP_Height]; /* マップ配置 */
} GameInfo;

/* 変数 */
extern GameInfo gGame;
extern CharaInfo* gChara;
extern int gCharaNum; /* キャラ総数 */

/* 関数 */
/* ui3kadai.c */
extern int PrintError(const char* str);
/* ui3system.c */
extern int InitSystem(const char* chara_data_file, const char* map_data_file);
extern void DestroySystem(void);
extern void UpdateCharaInfo(CharaInfo* ch);
extern void MoveChara(CharaInfo* ch, float frame);
extern void Collision(CharaInfo* ci, CharaInfo* cj);
/* ui3window.c */
extern int InitWindow(void);
extern void DestroyWindow(void);
extern void RenderWindow(void);

/* end of ui3system.h */
