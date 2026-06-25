#ifndef PLAYER_H
#define PLAYER_H

#include <joyconlib.h>
#include "system.h"

void InitPlayer(Player *p);
int IsJumpButtonPressed(joyconlib_t *jc);
int IsBoostButtonPressed(joyconlib_t *jc);

#endif