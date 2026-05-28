/*!
 @file  joycon_state.c
 @brief 状態取得サンプル
 @author    K. Morita
 @date  2023/5/10
 @note  コンパイルオプション：-ljoyconlib -lhidapi-hidraw -lm
 */
#include <joyconlib.h>
#include <stdio.h>

int main(void)
{
    joyconlib_t jc;

    joycon_err err = joycon_open(&jc, JOYCON_R);
    if (JOYCON_ERR_NONE != err) {
        printf("joycon open failed:%d\n", err);
        return -1;
    }

    joycon_btn btn = { 0 };
    int flg        = 1;
    while (flg) {
        btn = jc.button;
        joycon_get_state(&jc);

        if (jc.button.btn.X) { // Xが押されているとき
            puts("X pressing...");
        }

        // スティックの状態（傾きの値：-1.0(下，左)〜1.0(上，右)）を表示
        printf("Stick value is (%f,%f)\n", jc.stick.x, jc.stick.y);

        // ボタンの状態が変わった時
        if (btn.btn.A != jc.button.btn.A) {
            if (jc.button.btn.A) {
                puts("Button A pressed.");
            } else {
                puts("button A released.");
            }
        }

        if (btn.btn.SL_r != jc.button.btn.SL_r) {
            if (jc.button.btn.SL_r) {
                puts("Start measuring SL.");
            }
        }

        if (jc.button.btn.Home) { // homeが押されたら
            joycon_elapsed el;
            joycon_get_button_elapsed(&jc, &el);
            printf("about %u ms passed from last pushed SL.\n", el.SL * 10);
            flg = 0;
        }
    }

    printf("\n");
    joycon_close(&jc);

    return 0;
}