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

void UpdateGimmick(Player *p, Gimmick *g, int is_boosting, Gimmick *all_gimmicks, int gimmick_count)
{
    // 敵（スライム）の更新処理
    if (g->type == 5) {
        // 1. 物理演算（重力）の適用
        g->vy += 0.8f; 
        g->y += g->vy;
        
        // 地面との当たり判定
        if (g->y >= GROUND_Y) {
            g->y = GROUND_Y;
            g->vy = 0.0f;
        }

        // 2. 追跡処理
        float diff_x = p->x - g->x;
        float diff_y = p->y - g->y;
        if (diff_x < 0) diff_x = -diff_x;
        if (diff_y < 0) diff_y = -diff_y;
        
        if (diff_x + diff_y < 400.0f) {
            if (g->x < p->x) g->x += 2.0f; 
            if (g->x > p->x) g->x -= 2.0f;
        }

        // 3. ★追加：他の壁（ブロック）との当たり判定と押し出し処理
        for (int i = 0; i < gimmick_count; i++) {
            Gimmick *other = &all_gimmicks[i];
            
            // 自分自身、壊れて消えたもの、ブロック(type:3)以外は判定しない
            if (other == g || !other->is_active || other->type != 3) continue;

            // X軸とY軸の重なりチェック
            int col_x = (g->x < other->x + other->w) && (g->x + g->w > other->x);
            int col_y = (g->y < other->y + other->h) && (g->y + g->h > other->y);

            // 重なっている（壁にめり込んでいる）場合
            if (col_x && col_y) {
                // プレイヤーと同じロジックで上下左右の「めり込み量」を計算
                float overlap_left = (g->x + g->w) - other->x;
                float overlap_right = (other->x + other->w) - g->x;
                float overlap_top = (g->y + g->h) - other->y;
                float overlap_bottom = (other->y + other->h) - g->y;
                
                // 一番浅いめり込み方向を探す
                float min_overlap = overlap_left;
                if (overlap_right < min_overlap) min_overlap = overlap_right;
                if (overlap_top < min_overlap) min_overlap = overlap_top;
                if (overlap_bottom < min_overlap) min_overlap = overlap_bottom;
                
                // めり込みが一番浅い方向へ敵を押し出す
                if (min_overlap == overlap_left) {
                    g->x = other->x - g->w; // 左へ押し出す
                } else if (min_overlap == overlap_right) {
                    g->x = other->x + other->w; // 右へ押し出す
                } else if (min_overlap == overlap_top) {
                    g->y = other->y - g->h; // 上へ押し出す（ブロックに乗る）
                    g->vy = 0.0f; // 落下を止める
                } else if (min_overlap == overlap_bottom) {
                    g->y = other->y + other->h; // 下へ押し出す（頭をぶつける）
                    g->vy = 0.0f;
                }
            }
        }
    }

    // ★以下は既存のプレイヤーとの衝突判定処理（変更なし）
    if(CheckCollision(p, g))
    {
        if(g->type == 1) // トゲ
        {
            if (is_boosting) {
                // 何もしない
            } else if (p->invincible_timer <= 0) { 
                p->hp -= 10; 
                if(p->hp < 0) p->hp = 0; 
                
                p->vy = -12.0; 
                if (p->x < g->x) p->x -= 50.0; 
                else p->x += 50.0; 
                
                p->invincible_timer = 60; 
            }
        }
        else if(g->type == 2) // アイテム
        {
            p->hp += 20; 
            if(p->hp > 100) p->hp = 100; 
            p->score += 100; 
            g->is_active = 0; 
        }
        else if(g->type == 3) // 壊せるブロック
        {
            if (is_boosting) {
                g->is_active = 0; 
            } else {
                float overlap_left = (p->x + 50.0f) - g->x;
                float overlap_right = (g->x + g->w) - p->x;
                float overlap_top = (p->y + 50.0f) - g->y;
                float overlap_bottom = (g->y + g->h) - p->y;
                
                float min_overlap = overlap_left;
                if (overlap_right < min_overlap) min_overlap = overlap_right;
                if (overlap_top < min_overlap) min_overlap = overlap_top;
                if (overlap_bottom < min_overlap) min_overlap = overlap_bottom;
                
                if (min_overlap == overlap_left) {
                    p->x = g->x - 50.0f;
                } else if (min_overlap == overlap_right) {
                    p->x = g->x + g->w;
                } else if (min_overlap == overlap_top) {
                    p->y = g->y - 50.0f;
                    p->vy = 0.0;
                } else if (min_overlap == overlap_bottom) {
                    p->y = g->y + g->h;
                    p->vy = 0.0;
                }
            }
        }
        else if(g->type == 4) // ゴール
        {
            p->is_goal = 1; 
            p->score += 1000; 
        }
        else if(g->type == 5) // 敵
        {
            if (is_boosting) {
                g->is_active = 0; 
                p->score += 300; 
            } else if (p->invincible_timer <= 0) {
                p->hp -= 15; 
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
    if(*count < 300)
    {
        Gimmick *g  = &gimmicks[*count];
        g->x = x;
        g->y = y;
        g->w = w;
        g->h = h;
        g->type = type;
        g->is_active = 1;
        g->vy = 0.0f; // 初期の落下速度は0に設定
        (*count)++;
    }
}

// ステージの配置データを作成
void LoadStage(Gimmick *gimmicks, int *count) {
    *count = 0; 

    // 【エリア1：ブーストの洗礼】
    // ジャンプからの空中ブーストで壁を破壊して抜ける基本テクニック。
    AddGimmick(gimmicks, count, 600.0f, GROUND_Y, 50.0f, 50.0f, 1);
    AddGimmick(gimmicks, count, 600.0f, GROUND_Y - 50.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 600.0f, GROUND_Y - 100.0f, 50.0f, 50.0f, 3);

    // 【エリア2：絶望のトゲ海と命綱】
    // 途中のアイテムを空中で拾ってゲージを回復させないと絶対に越えられない距離のトゲ。
    for (int i = 0; i < 15; i++) { 
        AddGimmick(gimmicks, count, 1000.0f + (i * 50.0f), GROUND_Y, 50.0f, 50.0f, 1);
    }
    AddGimmick(gimmicks, count, 1200.0f, 500.0f, 40.0f, 40.0f, 2);
    AddGimmick(gimmicks, count, 1500.0f, 100.0f, 40.0f, 40.0f, 2);

    // 【エリア3：空挺部隊スライム】
    // 空中に浮かぶブロックの上にスライムが待ち構え、近づくと物理演算で落下してくる。
    for (int i = 0; i < 3; i++) { 
        AddGimmick(gimmicks, count, 2200.0f + (i * 50.0f), GROUND_Y - 100.0f, 50.f, 50.0f, 3);
    }
    AddGimmick(gimmicks, count, 2200.0f, GROUND_Y - 150.0f, 50.0f, 50.0f, 5); 
    for (int i = 0; i < 3; i++) { 
        AddGimmick(gimmicks, count, 2200.0f + (i * 50.0f), GROUND_Y - 200.0f, 50.f, 50.0f, 3);
    }
    AddGimmick(gimmicks, count, 2400.0f, GROUND_Y - 250.0f, 50.0f, 50.0f, 5);

    // 【エリア4：空中回廊】
    // 空中に足場（ブロック）が並び、下はトゲ。途中にスライムが徘徊している。
    for (int i = 0; i < 8; i++) {
        AddGimmick(gimmicks, count, 2800.0f + (i * 50.0f), GROUND_Y, 50.0f, 50.0f, 1);
        AddGimmick(gimmicks, count, 2800.0f + (i * 50.0f), GROUND_Y - 150.0f, 50.0f, 50.0f, 3);
    }
    AddGimmick(gimmicks, count, 2950.0f, GROUND_Y - 200.0f, 50.0f, 50.0f, 5);

    // 【エリア5：分厚い壁の連続】
    // 連続する壁をブーストで豪快にぶち抜く爽快エリア。抜け切った先に回復アイテム。
    for(int i=0; i<3; i++){
        AddGimmick(gimmicks, count, 3500.0f, GROUND_Y - (i*50.0f), 50.0f, 50.0f, 3);
        AddGimmick(gimmicks, count, 3600.0f, GROUND_Y - (i*50.0f), 50.0f, 50.0f, 3);
        AddGimmick(gimmicks, count, 3700.0f, GROUND_Y - (i*50.0f), 50.0f, 50.0f, 3);
    }
    AddGimmick(gimmicks, count, 3800.0f, GROUND_Y, 40.0f, 40.0f, 2);

    // 【エリア6：スライムの巣窟】
    // スライムが大量に密集している地帯。ブースト体当たりで一掃して高スコアを狙える。
    AddGimmick(gimmicks, count, 4100.0f, GROUND_Y, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 4200.0f, GROUND_Y, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 4300.0f, GROUND_Y, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 4400.0f, GROUND_Y, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 4250.0f, 200.0f, 40.0f, 40.0f, 2);

    // 【エリア7：大ジャンプと急降下】
    // 巨大なブロックの山を大きく飛び越え、着地狩りのトゲを避ける。
    for(int i=0; i<6; i++){
        AddGimmick(gimmicks, count, 4800.0f, GROUND_Y - (i*50.0f), 50.0f, 50.0f, 3);
    }
    AddGimmick(gimmicks, count, 5000.0f, 100.0f, 40.0f, 40.0f, 2); 
    for(int i=0; i<8; i++){
        AddGimmick(gimmicks, count, 5100.0f + (i*50.0f), GROUND_Y, 50.0f, 50.0f, 1);
    }

    // 【エリア8：迷路状のブロック】
    // 破壊できるブロックが上下に分かれて道を塞ぎ、各ルートにスライムが潜む。
    for(int i=0; i<5; i++){
        AddGimmick(gimmicks, count, 5600.0f + (i*50.0f), GROUND_Y - 100.0f, 50.0f, 50.0f, 3);
    }
    AddGimmick(gimmicks, count, 5700.0f, GROUND_Y, 50.0f, 50.0f, 5); 
    AddGimmick(gimmicks, count, 5800.0f, GROUND_Y - 150.0f, 50.0f, 50.0f, 5); 
    
    // 【エリア9：スライムの雨】
    // 高い位置のブロックから、時間差でスライムが次々と降ってくるパニックエリア。
    AddGimmick(gimmicks, count, 6300.0f, GROUND_Y - 300.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 6300.0f, GROUND_Y - 350.0f, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 6450.0f, GROUND_Y - 250.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 6450.0f, GROUND_Y - 300.0f, 50.0f, 50.0f, 5);
    AddGimmick(gimmicks, count, 6600.0f, GROUND_Y - 200.0f, 50.0f, 50.0f, 3);
    AddGimmick(gimmicks, count, 6600.0f, GROUND_Y - 250.0f, 50.0f, 50.0f, 5);

    // 【エリア10：ファイナル・デスロード】
    // トゲ、壁、スライムの全要素が組み合わさった最終関門。
    for(int i=0; i<10; i++){
        AddGimmick(gimmicks, count, 7000.0f + (i*50.0f), GROUND_Y, 50.0f, 50.0f, 1);
    }
    AddGimmick(gimmicks, count, 7150.0f, 250.0f, 40.0f, 40.0f, 2); 
    
    // ゴール直前の要塞と門番
    for(int i=0; i<4; i++){
        for(int j=0; j<2; j++){
            AddGimmick(gimmicks, count, 7500.0f + (j*50.0f), GROUND_Y - (i*50.0f), 50.0f, 50.0f, 3);
        }
    }
    AddGimmick(gimmicks, count, 7450.0f, GROUND_Y - 50.0f, 50.0f, 50.0f, 5);

    // 【ゴール】 (X座標を8000まで大幅に延長)
    AddGimmick(gimmicks, count, 8000.0f, GROUND_Y - 50.0f, 150.0f, 100.0f, 4);
}