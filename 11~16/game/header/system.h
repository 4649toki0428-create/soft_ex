#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <joyconlib.h>

typedef struct
{
    double x;
    double y;
    double vx;
    double vy;
    int hp;
    int state;
    double prev_acc_z; //Joy-Conの加速度センサー
    int invincible_timer; //無敵時間の管理
    int is_goal; //ゴール判定
    int is_boosting; //ブースト中かどうかのフラグ
    int score; //スコアの管理
} Player;

typedef struct
{
    float x;
    float y;
    float w; // 幅
    float h; // 高さ
    int type; // 1:トゲ, 2:アイテム, 3:壊せるブロック
    int is_active; // 1:アクティブ, 0:非アクティブ
} Gimmick;


#endif