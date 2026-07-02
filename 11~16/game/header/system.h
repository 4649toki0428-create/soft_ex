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
    double prev_acc_z;
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