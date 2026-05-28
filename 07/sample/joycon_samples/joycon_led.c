/*!
 @file  joycon_state.c
 @brief LEDサンプル
 @author    K. Morita
 @date  2023/5/4
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

    // LED
    joycon_set_led(&jc, JOYCON_LED_1_BLINK | JOYCON_LED_4_ON);
    puts("led 1 blink, led 4 on\npush enter key.");
    getchar();

    for (int i = 8; i; i--) {
        joycon_set_led(&jc, i);
        printf("%d:push enter key.", i);
        getchar();
    }

    // home LED
    joycon_set_homeled(&jc, JOYCON_HOMELED_ON);
    puts("home led on\npush enter key.");
    getchar();

    joycon_homeled hl       = { 0 };
    hl.mc_duration          = 7;  // 基準時間
    hl.mc_num               = 4;  // mc[0].mc1,mc[0].mc2,mc[1].mc1,mc[1].mc2の順なので 4
    hl.fc_num               = 10; // mc[0].mc1〜mc[1].mc2を10回繰り返す
    hl.intensity            = 2;  // 初期明るさ
    hl.mc[0].mc1_intensity  = 8;  // 明るさ
    hl.mc[0].mc1_duration   = 5;  // 今の明るさ(8)になってからの点灯時間（5*基準時間(7)）
    hl.mc[0].mc1_transition = 10; // 前の明るさ(2)から今の明るさ(8)へ10段階（10*基準時間(7)）でフェードイン
    hl.mc[0].mc2_intensity  = 4;
    hl.mc[0].mc2_duration   = 0;
    hl.mc[0].mc2_transition = 10;
    hl.mc[1].mc1_intensity  = 12;
    hl.mc[1].mc1_duration   = 0;
    hl.mc[1].mc1_transition = 10;
    hl.mc[1].mc2_intensity  = 2;
    hl.mc[1].mc2_duration   = 5;
    hl.mc[1].mc2_transition = 10;
    joycon_set_homeled(&jc, &hl);

    puts("home led pattern\npush enter key.");
    getchar();

    printf("\n");
    joycon_close(&jc);

    return 0;
}