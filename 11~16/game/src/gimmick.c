#include "gimmick.h"
#include <stdio.h>

int CheckCollision(Player *p, Gimmick *g)
{
    // プレイヤーのサイズ
    float player_w = 50.0f;
    float player_h = 50.0f;
    
    //X軸の重なりチェック
    int collision_x = (p->x < g->x + g->w) && (p->x + player_w > g->x);
    
    //Y軸の重なりチェック
    int collision_y = (p->y < g->y + g->h) && (p->y + player_h > g->y);
    
    //X軸とY軸の両方で重なっている場合、衝突が発生している
    if(collision_x && collision_y)
    {
        return 1; // 衝突あり
    }
    else
    {
        return 0; // 衝突なし
    }
}

void UpdateGimmick(Player *p, Gimmick *g, int is_boosting)
{
    //衝突判定
    if(CheckCollision(p, g))
    {
        if(g->type == 1) // トゲ
        {
            p->hp -= 10; // プレイヤーのHPを減らす
            if(p->hp < 0) p->hp = 0; // HPが0未満にならないようにする
        }
        else if(g->type == 2) // アイテム
        {
            p->hp += 20; // プレイヤーのHPを増やす
            if(p->hp > 100) p->hp = 100; // HPが100を超えないようにする
            g->is_active = 0; // アイテムを非アクティブにする
        }
        else if(g->type == 3) // 壊せるブロック
        {
            g->is_active = 0; // ブロックを非アクティブにする
        }
    }
}