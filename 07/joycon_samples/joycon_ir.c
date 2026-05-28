/*!
 @file  joycon_ir.c
 @brief IRカメラサンプル
 @author    K. Morita
 @date  2023/5/10
 @note  コンパイルオプション：-lSDL2 -lSDL2_image -ljoyconlib -lhidapi-hidraw -lm
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

    // SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        puts(SDL_GetError());
        return -1;
    }

    // IRカメラ起動
    joycon_enable_ir(&jc, JOYCON_IR160X120, 600);
    puts("Aim the IR camera and push enter key.");
    getchar();

    // 読み取り
    int w    = 160;
    int h    = 120;
    int size = w * h;
    u8 irdata[size * 3]; // 後にRGBに拡張するため3倍にしておく
    joycon_ir_result result;
    // 1回目はノイズが乗りやすいので，
    joycon_read_ir(&jc, irdata, size, 0, &result);
    // 2回目以降を採用する
    joycon_read_ir(&jc, irdata, size, 0, &result);

    printf("Avg Int: %u, White: %u, Noise: %u\n", result.avg_intensity, result.white_pixels, result.noise_pixels);
    // データ拡張(グレースケールの1ByteからRGBの3Byteへ)
    for (int i = size; i > 0; i--) {
        irdata[i * 3 - 1] = irdata[i - 1];
        irdata[i * 3 - 2] = irdata[i - 1];
        irdata[i * 3 - 3] = irdata[i - 1];
    }
    // サーフェイスに転写
    SDL_Surface* sur = SDL_CreateRGBSurfaceWithFormatFrom(irdata, w, h, 24, 3 * w, SDL_PIXELFORMAT_RGB24);
    // pngに書き出し
    if (IMG_SavePNG(sur, "irdata.png") < 0) {
        puts(SDL_GetError());
    } else
        puts("IR image saved.");
    // サーフェイス開放
    SDL_FreeSurface(sur);

    // IR終了
    joycon_disable_ir(&jc);

    printf("\n");
    SDL_Quit();
    joycon_close(&jc);

    return 0;
}