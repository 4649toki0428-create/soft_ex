#include "system.h"
#include "window.h"
#include "player.h"

void    InitPlayer(Player *p)
{
    p->x = 50.0; //スタート地点のX座標
    p->y = 300.0; //スタート地点のY座標
    p->vx = 0.0; //X方向の速度の初期化
    p->vy = 0.0; //Y方向の速度の初期化
    p->hp = 50; //初期ゲージ
    p->state = 0; //状態の初期化
}

int IsJumpButtonPressed(joyconlib_t *jc)
{
    return jc->button.btn.X;
}

int IsBoostButtonPressed(joyconlib_t *jc)
{
    return jc->button.btn.B;
}