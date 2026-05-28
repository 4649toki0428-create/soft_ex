/*!
 @file  joycon_axis.c
 @brief 6軸センサーサンプル
 @author    K. Morita
 @date  2023/5/10
 @note  コンパイルオプション：-ljoyconlib -lhidapi-hidraw -lm
 */
#include <joyconlib.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
    joyconlib_t jc;

    joycon_err err = joycon_open(&jc, JOYCON_R);
    if (JOYCON_ERR_NONE != err) {
        printf("joycon open failed:%d\n", err);
        return -1;
    }

    int flg  = 1;
    float tx = 0, ty = 0, tz = 0;
    while (flg) {
        joycon_get_state(&jc);

        // センサーの値を表示
        printf("Sensor value is (%f,%f,%f)\n", jc.axis[0].acc_x, jc.axis[0].acc_y, jc.axis[0].acc_z);
        printf("                (%f,%f,%f)\n", jc.axis[0].gyro_x, jc.axis[0].gyro_y, jc.axis[0].gyro_z);

        // 傾きの推定
        // 参考文献
        // https://garchiving.com/calculate-angle-of-3axis-rotation-in-6axis-sensor/
        float dx = 0, dy = 0, dz = 0;
        for (int i = 2; i >= 0; i--) {
            float x = dx, y = dy, z = dz;
            dx = jc.axis[i].gyro_x + z * jc.axis[i].gyro_y - y * jc.axis[i].gyro_z;
            dy = -z * jc.axis[i].gyro_x + jc.axis[i].gyro_y + x * jc.axis[i].gyro_z;
            dz = y * jc.axis[i].gyro_x - x * jc.axis[i].gyro_y + jc.axis[i].gyro_z;

            // 積分
            tx += dx * 0.005;
            ty += dy * 0.005;
            tz += dz * 0.005;
        }
        // 加算され続けるので，±2πを超えたら修正
        if (tx < -2 * M_PI) tx += 2 * M_PI;
        if (tx > 2 * M_PI) tx -= 2 * M_PI;
        if (ty < -2 * M_PI) ty += 2 * M_PI;
        if (ty > 2 * M_PI) ty -= 2 * M_PI;
        if (tz < -2 * M_PI) tz += 2 * M_PI;
        if (tz > 2 * M_PI) tz -= 2 * M_PI;

        printf("estimated angle:(%f,%f,%f)\n", tx * 180 / M_PI, ty * 180 / M_PI, tz * 180 / M_PI);

        if (jc.button.btn.Home) { // homeが押されたら
            flg = 0;
        }
    }

    printf("\n");
    joycon_close(&jc);

    return 0;
}