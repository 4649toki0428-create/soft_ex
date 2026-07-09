#include "gimmick.h"
#include "player.h"
#include "window.h"
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
    if (g->type == 5) {
        // プレイヤーと敵の距離を簡易計算（絶対値）
        float diff_x = p->x - g->x;
        float diff_y = p->y - g->y;
        if (diff_x < 0) diff_x = -diff_x;
        if (diff_y < 0) diff_y = -diff_y;
        
        // 距離が400ピクセル以内なら気付いて追いかけてくる
        if (diff_x + diff_y < 400.0f) {
            if (g->x < p->x) g->x += 2.0f; // 敵の歩くスピード
            if (g->x > p->x) g->x -= 2.0f;
        }
    }

    //衝突判定
    if(CheckCollision(p, g))
    {
        if(g->type == 1) // トゲ
        {
            if (is_boosting) {
                g->is_active = 0; 
            } else if (p->invincible_timer <= 0) { // ★無敵じゃない時だけダメージ
                p->hp -= 10; 
                if(p->hp < 0) p->hp = 0; 
                
                p->vy = -12.0; 
                // ★左右どちらからぶつかったかで弾く方向を変える
                if (p->x < g->x) {
                    p->x -= 50.0; // 左から当てたら左へ
                } else {
                    p->x += 50.0; // 右から当てたら右へ
                }
                
                p->invincible_timer = 60; // 約1秒間（60フレーム）無敵にする
            }
        }
        else if(g->type == 2) // アイテム
        {
            p->hp += 20; 
            if(p->hp > 100) p->hp = 100; 
            p->score += 100; // アイテムを取ったらスコアを加算
            g->is_active = 0; 
        }
        else if(g->type == 3) // 壊せるブロック
        {
            if (is_boosting) {
                g->is_active = 0; // ブースト中なら破壊
            } else {
                // ブーストしていない時は壁として機能
                p->x = g->x - 50.0f; 
            }
        }
        else if(g->type == 4) // ゴール
        {
            p->is_goal = 1; // ゴールフラグを立てる
            p->score += 1000; // ゴールしたらスコアを加算
        }
        else if(g->type == 5) 
        {
            if (is_boosting) {
                // ブースト中は体当たりで敵を吹っ飛ばして倒せる！
                g->is_active = 0; 
                p->score += 300; // 敵を倒したらスコアを加算
            } else if (p->invincible_timer <= 0) {
                p->hp -= 15; // トゲより少しダメージが大きい
                if(p->hp < 0) p->hp = 0; 
                p->vy = -12.0; 
                if (p->x < g->x) p->x -= 50.0;
                else p->x += 50.0;
                p->invincible_timer = 60; 
            }
        }
    }
}

//ギミックを配列に追加する関数
void AddGimmick(Gimmick *gimmicks, int *count, float x, float y, float w,  float h, int type)
{
    if(*count < 100)
    {
        Gimmick *g  = &gimmicks[*count];
        g->x = x;
        g->y = y;
        g->w = w;
        g->h = h;
        g->type = type;
        g->is_active = 1;
        (*count)++;
    }
}

// ステージの配置データを作成
void LoadStage(Gimmick *gimmicks, int *count) {
    *count = 0; 

    // 【エリア1：フェイント】
    // 最初のトゲをジャンプで越えると、直後にスライムが迫ってきます。
    AddGimmick(gimmicks, count, 600.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 800.0f, GROUND_Y, 50.0f, 50.0f, 5); 

    // 【エリア2：トゲの海と壁のコンボ】
    // 連続トゲの直後にジャンプ不可の壁。さらに壁を壊した直後にスライムが隠れています。
    // 壁を壊した瞬間にブーストをやめると被弾するため、長めのブースト維持が必要です。
    AddGimmick(gimmicks, count, 1100.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 1150.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 1200.0f, GROUND_Y, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 1200.0f, GROUND_Y - 50.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 1200.0f, GROUND_Y - 100.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 1250.0f, GROUND_Y, 50.0f, 50.0f, 5); // 壁の裏の刺客

    // 【エリア3：決死の空中回復】
    // アイテムの真下にトゲが敷き詰められています。
    // HPを回復したい場合は、精度の高いジャンプが要求されるハイリスク・ハイリターンな配置です。
    AddGimmick(gimmicks, count, 1600.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 1650.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 1700.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 1650.0f, 250.0f, 40.0f, 40.0f, 2);

    // 【エリア4：スライムのラッシュと要塞】
    // 3体のスライムが連続で押し寄せてくる奥に、分厚い「2列」の要塞があります。
    // 敵を倒すために無駄にブーストを消費すると、要塞を壊しきれずに詰んでしまいます。
    AddGimmick(gimmicks, count, 2100.0f, GROUND_Y, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 2150.0f, GROUND_Y, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 2200.0f, GROUND_Y, 50.0f, 50.0f, 5);
    
    // 分厚い要塞（2列×3段）
    AddGimmick(gimmicks, count, 2400.0f, GROUND_Y, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 2400.0f, GROUND_Y - 50.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 2400.0f, GROUND_Y - 100.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 2450.0f, GROUND_Y, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 2450.0f, GROUND_Y - 50.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 2450.0f, GROUND_Y - 100.0f, 50.0f, 50.0f, 3);

    // 要塞の中に回復アイテムを隠しておく（壊した人へのご褒美）
    AddGimmick(gimmicks, count, 2500.0f, GROUND_Y, 40.0f, 40.0f, 2);

    // 【エリア5：最後の罠】
    // ゴール直前の油断を狙うトゲとスライムの挟み撃ち
    AddGimmick(gimmicks, count, 2800.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 2900.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 3000.0f, GROUND_Y, 50.0f, 50.0f, 5);

    // ゴール
    AddGimmick(gimmicks, count, 3300.0f, GROUND_Y - 50.0f, 150.0f, 100.0f, 4);
}