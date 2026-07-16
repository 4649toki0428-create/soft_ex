#include "system.h"
#include "window.h"
#include "player.h"
#include "gimmick.h"

void    InitPlayer(Player *p)
{
    p->x = 30.0; //スタート地点のX座標
    p->y = GROUND_Y; //スタート地点のY座標
    p->vx = 0.0; //X方向の速度の初期化
    p->vy = 0.0; //Y方向の速度の初期化
    p->hp = 50; //初期ゲージ
    p->state = 0; //状態の初期化
    p->invincible_timer = 0;
    p->is_goal = 0; //ゴール判定の初期化
    p->is_boosting = 0; //ブースト状態の初期化
    p->score = 0; //スコアの初期化
    p->direction = 1; // 初期の向きは右
}

int IsJumpButtonPressed(joyconlib_t *jc)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    // Joy-ConのXボタン、またはキーボードのスペースキー
    return jc->button.btn.X || keys[SDL_SCANCODE_SPACE];
}

int IsBoostButtonPressed(joyconlib_t *jc)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    // Joy-ConのBボタン、またはキーボードの左Shiftキー
    return jc->button.btn.B || keys[SDL_SCANCODE_LSHIFT];
}

void UpdatePlayer(Player *p, joyconlib_t *jc, GameContext *ctx)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    float input_x = 0.0f;
    float input_y = 0.0f;
    
    // ★最初は基本スピード(5.0)を入れておく
    double current_speed = PLAYER_BASE_SPEED;

    // 毎フレームリセット
    p->is_boosting = 0;

    if(p->invincible_timer > 0) p->invincible_timer--;

    if (ctx->has_joycon && (jc->stick.y != 0 || jc->stick.x != 0)) {
        input_x = -jc->stick.y;
        input_y = jc->stick.x;
    } else {
        if (keys[SDL_SCANCODE_RIGHT]) input_x += 1.0f;
        if (keys[SDL_SCANCODE_LEFT])  input_x -= 1.0f;
        if (keys[SDL_SCANCODE_DOWN])  input_y += 1.0f;
        if (keys[SDL_SCANCODE_UP])    input_y -= 1.0f;
    }

    if(p->y >= GROUND_Y)
    {
        p->y = GROUND_Y;
        p->vy = 0.0;
    }

    if(p->y == GROUND_Y && IsJumpButtonPressed(jc))
    {
        p->vy = PLAYER_JUMP_FORCE;
    }

    // --- ブースト判定とゲージ消費の分離 ---
    int is_boost_pressed = IsBoostButtonPressed(jc);
    int current_w_key = keys[SDL_SCANCODE_W];

    if (is_boost_pressed) {
        // 1. ボタンを押している間はHPが減る
        p->hp -= 2;
        if (p->hp <= 0) p->hp = 1;

        // 2. 実際にブースト状態になるのは、HPが10以上の時だけ
        if (p->hp >= 5) {
            p->is_boosting = 1;
            // ★ここでスピードをバーストスピード(15.0)に引き上げる！
            current_speed = PLAYER_BURST_SPEED;
        }
    } else {
        // 3. ボタンを離している時だけ、ゲージを溜められる
        float acc_z = ctx->has_joycon ? jc->axis[0].acc_z : 0.0f;
        float threshold = 20.0f;
        if (acc_z > threshold || acc_z < -threshold) p->hp += 2;
        if (current_w_key == 1 && p->state == 0) p->hp += 2;
        if (p->hp > 100) p->hp = 100;
    }
    
    p->state = current_w_key;

    // --- 移動処理 ---
    // ★古いコードの「double speed = 5.0;」は消滅し、ここで引き上げたスピードを適用！
    p->x += input_x * current_speed;

    if(p->is_boosting)
    {
        p->y += input_y * current_speed;
        p->vy = 0.0; // ブースト中は重力無視で飛び回れる
    }
    else
    {
        p->vy += PLAYER_GRAVITY;
        p->y += p->vy;
    }

    // ★追加：移動方向に応じてキャラクターの向きを更新する
    if (input_x > 0.0f) {
        p->direction = 1; // 右移動なら右向き
    } else if (input_x < 0.0f) {
        p->direction = -1; // 左移動なら左向き
    }

    // 左画面外に出ないための壁
    if(p->x < ctx->camera_x) p->x = ctx->camera_x;
}
