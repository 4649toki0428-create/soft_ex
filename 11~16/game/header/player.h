#ifndef PLAYER_H
#define PLAYER_H

#include <joyconlib.h>
#include "system.h"

//プレイヤーの物理・性能設定
#define PLAYER_BASE_SPEED 5.0     // 通常時の移動スピード
#define PLAYER_BURST_SPEED 15.0   // バースト時の移動スピード
#define PLAYER_GRAVITY 0.8        // 重力の強さ
#define PLAYER_JUMP_FORCE -15.0   // ジャンプ力
#define GROUND_Y (480.0 - 50.0)   // 地面のY座標

void InitPlayer(Player *p);
int IsJumpButtonPressed(joyconlib_t *jc);
int IsBoostButtonPressed(joyconlib_t *jc);
void UpdatePlayer(Player *p, joyconlib_t *jc, GameContext *ctx);


#endif