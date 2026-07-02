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

void    UpdatePlayer(Player *p, joyconlib_t *jc)
{
    //Joy-Conを横持ちで使う方法
    float   input_x = -jc->stick.y;
    float   input_y = jc->stick.x;
    int     is_boosting = 0;
    double current_speed = PLAYER_BASE_SPEED;

    //地面の当たり判定
    if(p->y >= GROUND_Y)
    {
        p->y = GROUND_Y;
        p->vy = 0.0;
    }

    //ジャンプの処理
    if(p->y == GROUND_Y && IsJumpButtonPressed(jc))
    {
        p->vy = PLAYER_JUMP_FORCE;
    }

    //ウィンドウの境界チェック
    if(p->x < 0) p->x = 0;
    if(p->x > 640 - 50) p->x = 640 - 50;

    //ブーストの設定
    // ゲージ溜めの処理
    float acc_z = jc->axis[0].acc_z;
    float threshold = 20.0f;

    if(acc_z > threshold || acc_z < -threshold)
    {
        p->hp += 2;

        if(p->hp > 100)
        {
            p->hp = 100;
        }
    }

    //ブーストの処理
    if(IsBoostButtonPressed(jc) && p->hp > 0)
    {
        current_speed = PLAYER_BURST_SPEED;
        p->hp -= 1; //ブースト中はゲージを消費
        p->x += input_x * current_speed; //ブースト中は移動速度を上げる
        is_boosting = 1;
    }

    //ボタン判定の処理
    //スティックで移動
    double speed = 5.0;
    p->x += input_x * speed;

    if(is_boosting)
    {
        //ブースト中の飛行処理
        p->y += input_y * current_speed;
        p->vy += 0.0;
    }
    else
    {
        //重力とy座標の更新
        p->vy += PLAYER_GRAVITY;
        p->y += p->vy;
    }

}

int CheckCollision(Player *p, Gimmick *g)
{
    // プレイヤーの矩形
    SDL_Rect playerRect = { (int)p->x, (int)p->y, 50, 50 };
    // ギミックの矩形
    SDL_Rect gimmickRect = { (int)g->x, (int)g->y, (int)g->w, (int)g->h };

    // 矩形の衝突判定
    if(SDL_HasIntersection(&playerRect, &gimmickRect))
    {
        return 1; // 衝突あり
    }
    return 0; // 衝突なし
}