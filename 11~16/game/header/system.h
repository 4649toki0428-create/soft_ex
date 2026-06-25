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

#endif