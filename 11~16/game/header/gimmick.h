#ifndef GIMMICK_H
#define GIMMICK_H

#include "system.h"

#define MAX_GIMMICKS 100

Gimmick stage_gimmicks[MAX_GIMMICKS];

int CheckCollision(Player *p, Gimmick *g);
void UpdateGimmick(Player *p, Gimmick *g, int is_boosting);

#endif