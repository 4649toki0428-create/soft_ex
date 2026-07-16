#ifndef GIMMICK_H
#define GIMMICK_H

#include "system.h"

#define MAX_GIMMICKS 100

int CheckCollision(Player *p, Gimmick *g);
void UpdateGimmick(Player *p, Gimmick *g, int is_boosting, Gimmick *all_gimmicks, int gimmick_count);

void LoadStage(Gimmick *gimmicks, int *count);

#endif