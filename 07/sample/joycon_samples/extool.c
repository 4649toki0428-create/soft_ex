/*!
 @file  extool.c
 @brief GUIサンプルツール
 @author    K. Morita
 @date  2023/5/10
 @note  コンパイルオプション：-lSDL2 -lSDL2_ttf -ljoyconlib -lhidapi-hidraw -lm
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <joyconlib.h>
#include <math.h>
#include <string.h>

static char gFontFile[]       = "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf";
static const SDL_Color gBlack = { 0, 0, 0, 255 };

// エラーメッセージ表示
int PrintError(const char* str)
{
    fprintf(stderr, "%s\n", str);
    return -2;
}

// 文字列をテクスチャーへ
SDL_Texture* GetTextTexture(SDL_Renderer* render, TTF_Font* ttf, const char* txt, SDL_Color col)
{
    SDL_Texture* ret;
    SDL_Surface* sf;
    if (strstr(txt, "\n"))
        sf = TTF_RenderUTF8_Blended_Wrapped(ttf, txt, col, 300);
    else
        sf = TTF_RenderUTF8_Blended(ttf, txt, col);
    if (NULL == sf) {
        return NULL;
    } else {
        /* テクスチャへ */
        ret = SDL_CreateTextureFromSurface(render, sf);
        SDL_FreeSurface(sf);
    }
    return ret;
}

// 文字列の描画
void RenderText(SDL_Renderer* render, TTF_Font* ttf, const char* txt, SDL_Rect* sr)
{
    SDL_Rect sr2        = { 0 };
    SDL_Texture* txture = GetTextTexture(render, ttf, txt, gBlack);
    SDL_QueryTexture(txture, NULL, NULL, &sr2.w, &sr2.h);
    sr2.x = (sr->x + sr->w / 2) - sr2.w / 2;
    sr2.y = (sr->y + sr->h / 2) - sr2.h / 2;
    SDL_RenderCopy(render, txture, NULL, &sr2);
    SDL_DestroyTexture(txture);
}

/* main */
int main(int argc, char* argv[])
{
    /* SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return PrintError(SDL_GetError());
    }

    SDL_Window* window;   // ウィンドウデータを格納する構造体
    SDL_Renderer* render; // レンダリングコンテキスト（描画設定）を格納する構造体
                          // ウィンドウ生成・表示
    if ((window = SDL_CreateWindow("Press [Esc] to exit", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 820, 590, 0)) == NULL) {
        printf("failed to initialize videomode.\n");
        exit(-1);
    }
    render = SDL_CreateRenderer(window, -1, 0);
    if (render == NULL)
        return PrintError(SDL_GetError());

    /* フォント */
    if (TTF_Init() < 0) {
        return PrintError(TTF_GetError());
    }
    TTF_Font* ttf = TTF_OpenFont(gFontFile, 14);
    if (NULL == ttf) {
        return PrintError(TTF_GetError());
    }
    /* ラベル */
    char* btnTxt[11] = { "Y", "X", "B", "A", "SR", "SL", "R", "ZR", "Plus", "Stick", "Home" };
    SDL_Texture* txtA[11];
    for (int i = 0; i < 11 && ttf; i++) {
        txtA[i] = GetTextTexture(render, ttf, btnTxt[i], gBlack);
        if (NULL == txtA[i]) {
            return PrintError(SDL_GetError());
        }
    }
    char* labelTxt[15]    = { " ", "joyconlib", "Acc", "Gyro", "X", "Y", "Z", "led", "home led", "IR camera", "NFC", "Resolution:", "Exposure:", "Click to capture and stop", "Touch on Joy-Con(R) stick\nand Click to scan" };
    SDL_Point txtLpos[15] = { { 100, 10 }, { 400, 10 }, { 240, 240 }, { 240, 400 }, { 150, 280 }, { 150, 320 }, { 150, 360 }, { 650, 40 }, { 650, 100 }, { 650, 160 }, { 650, 460 }, { 550, 180 }, { 550, 200 }, { 500, 220 }, { 500, 480 } };
    SDL_Texture* txtL[15];
    for (int i = 0; i < 15 && ttf; i++) {
        txtL[i] = GetTextTexture(render, ttf, labelTxt[i], gBlack);
        if (NULL == txtL[i]) {
            return PrintError(SDL_GetError());
        }
    }
    char* irTxt[4]               = { "40x30", "80x60", "160x120", "320x240" };
    int irtxtP                   = 0;
    joycon_ir_resolution irresol = JOYCON_IR40X30;

    // ジョイコンオープン
    joyconlib_t jc;
    joycon_err err = joycon_open(&jc, JOYCON_R);
    if (JOYCON_ERR_NONE != err) {
        printf("joycon open failed:%d\n", err);
        return -1;
    }

    // イベント処理（無限ループ内でイベントを取得し，対応する処理を行う）
    int flg                  = 1;
    int mflg                 = 0;
    int axis[8]              = { 0 };
    int btns[11]             = { 0 };
    u8 leds                  = jc.led_bk;
    SDL_Rect ledsr[4]        = { { 600, 60, 32, 32 }, { 640, 60, 32, 32 }, { 680, 60, 32, 32 }, { 720, 60, 32, 32 } };
    SDL_Rect homeledsr       = { 600, 120, 20, 20 };
    u8 homeledcol            = 255;
    int homeledflg           = 0;
    int irflg                = 0;
    int ircapflg             = 0;
    SDL_Rect irsr            = { 770, 200, 20, 18 }; // 650-770
    SDL_Rect irressr         = { 650, 180, 100, 18 };
    SDL_Rect ircapsr         = { 490, 220, 320, 240 };
    u8 irdata[320 * 240 * 3] = { 0 };
    u16 old_exposure         = 0;
    SDL_Texture* irture      = NULL;
    SDL_Rect nfcsr           = { 490, 480, 320, 100 };
    joycon_nfc_data nfcdata  = { 0 };
    int nfcflg               = 0;
    int nfcresflg            = 0;
    char str[210];
    SDL_Event event;
    while (flg) {
        joycon_get_state(&jc);

        // イベントを取得したなら
        while (SDL_PollEvent(&event)) {
            // イベントの種類ごとに処理
            switch (event.type) {
            case SDL_KEYDOWN: // キーボードのキーが押された時
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    flg = 0;
                }
                break;
            case SDL_MOUSEBUTTONDOWN: // マウスのボタンが押された時
                joycon_rumble(&jc, 50);
                mflg         = 1;
                SDL_Point mp = { event.button.x, event.button.y };
                // led
                int bflg = 0;
                for (int i = 0; i < 4; i++) {
                    if (SDL_PointInRect(&mp, &ledsr[i])) {
                        bflg = 1;
                        switch (i) {
                        case 0:
                            if (leds & JOYCON_LED_4_ON) {
                                leds &= ~JOYCON_LED_4_ON;
                                leds |= JOYCON_LED_4_BLINK;
                            } else if (leds & JOYCON_LED_4_BLINK) {
                                leds &= ~JOYCON_LED_4_BLINK;
                            } else {
                                leds |= JOYCON_LED_4_ON;
                            }
                            break;
                        case 1:
                            if (leds & JOYCON_LED_3_ON) {
                                leds &= ~JOYCON_LED_3_ON;
                                leds |= JOYCON_LED_3_BLINK;
                            } else if (leds & JOYCON_LED_3_BLINK) {
                                leds &= ~JOYCON_LED_3_BLINK;
                            } else {
                                leds |= JOYCON_LED_3_ON;
                            }
                            break;
                        case 2:
                            if (leds & JOYCON_LED_2_ON) {
                                leds &= ~JOYCON_LED_2_ON;
                                leds |= JOYCON_LED_2_BLINK;
                            } else if (leds & JOYCON_LED_2_BLINK) {
                                leds &= ~JOYCON_LED_2_BLINK;
                            } else {
                                leds |= JOYCON_LED_2_ON;
                            }
                            break;
                        case 3:
                            if (leds & JOYCON_LED_1_ON) {
                                leds &= ~JOYCON_LED_1_ON;
                                leds |= JOYCON_LED_1_BLINK;
                            } else if (leds & JOYCON_LED_1_BLINK) {
                                leds &= ~JOYCON_LED_1_BLINK;
                            } else {
                                leds |= JOYCON_LED_1_ON;
                            }
                        default:
                            break;
                        }
                    }
                }
                if (bflg)
                    joycon_set_led(&jc, leds);
                // home led
                if (SDL_PointInRect(&mp, &homeledsr)) {
                    homeledflg = 1;
                }
                // ir
                if (SDL_PointInRect(&mp, &ircapsr)) {
                    ircapflg = 1;
                }
                if (SDL_PointInRect(&mp, &irsr)) {
                    irflg = 1;
                }
                if (SDL_PointInRect(&mp, &irressr)) {
                    irtxtP = (irtxtP + 1) & 3;
                }
                // nfc
                if (SDL_PointInRect(&mp, &nfcsr)) {
                    nfcflg = 1;
                }
                break;
            case SDL_MOUSEBUTTONUP: // マウスのボタンが離された時
                joycon_rumble(&jc, 0);
                mflg       = 0;
                homeledflg = 0;
                irflg      = 0;
                break;
            case SDL_MOUSEMOTION: // マウスが移動した時
                if (mflg) {
                    if (homeledflg) {
                        homeledsr.x += event.motion.xrel;
                    }
                    if (irflg) {
                        irsr.x += event.motion.xrel;
                    }
                }
                break;
            // SDLの利用を終了する時（イベントキューにSDL_QUITイベントが追加された時）
            case SDL_QUIT:
                flg = 0;
                break;
            }
        }

        // 描画
        SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
        SDL_RenderClear(render);
        // ラベル
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        for (int i = 1; i < 13; i++) {
            SDL_Rect sr = { txtLpos[i].x, txtLpos[i].y, 0, 0 };
            SDL_QueryTexture(txtL[i], NULL, NULL, &sr.w, &sr.h);
            SDL_RenderCopy(render, txtL[i], NULL, &sr);
        }
        for (int i = 4; i < 7; i++) {
            SDL_Rect sr = { txtLpos[i].x, txtLpos[i].y + 160, 0, 0 };
            SDL_QueryTexture(txtL[i], NULL, NULL, &sr.w, &sr.h);
            SDL_RenderCopy(render, txtL[i], NULL, &sr);
        }
        sprintf(str, "Battery: %u", jc.battery);
        SDL_Rect sr = { 750, 20, 0, 0 };
        RenderText(render, ttf, str, &sr);

        // スティック
        SDL_SetRenderDrawColor(render, 128, 128, 128, 255);
        sr = (SDL_Rect) { 40, 40, 160, 160 };
        SDL_RenderDrawRect(render, &sr);
        SDL_RenderDrawLine(render, 120, 40, 120, 200);
        SDL_RenderDrawLine(render, 40, 120, 200, 120);
        int x, y;
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        x = (80 * jc.stick.x);
        y = (80 * jc.stick.y);
        SDL_RenderDrawLine(render, 118 + x, 120 + y, 122 + x, 120 + y);
        SDL_RenderDrawLine(render, 120 + x, 118 + y, 120 + x, 122 + y);
        {
            sprintf(str, "(%f, %f)", jc.stick.x, jc.stick.y);
            sr = (SDL_Rect) { 120, 210, 0, 0 };
            RenderText(render, ttf, str, &sr);
        }

        // 6軸
        SDL_SetRenderDrawColor(render, 255, 0, 255, 255);
        for (int i = 2; i >= 0; i--) {
            SDL_SetRenderDrawColor(render, 85 * (3 - i), 0, 85 * (3 - i), 255);
            sr = (SDL_Rect) { 250, 280 + i * 10, 0, 10 };
            if (jc.axis[i].acc_x < 0)
                sr.x += (80 * jc.axis[i].acc_x / 10);
            sr.w = fabsf(80 * jc.axis[i].acc_x / 10);
            SDL_RenderFillRect(render, &sr);

            sr.y += 40;
            sr.x = 250;
            if (jc.axis[i].acc_y < 0)
                sr.x += (80 * jc.axis[i].acc_y / 10);
            sr.w = fabsf(80 * jc.axis[i].acc_y / 10);
            SDL_RenderFillRect(render, &sr);

            sr.y += 40;
            sr.x = 250;
            if (jc.axis[i].acc_z < 0)
                sr.x += (80 * jc.axis[i].acc_z / 10);
            sr.w = fabsf(80 * jc.axis[i].acc_z / 10);
            SDL_RenderFillRect(render, &sr);

            sr.y += 80;
            sr.x = 250;
            if (jc.axis[i].gyro_x < 0)
                sr.x += (80 * jc.axis[i].gyro_x / 10);
            sr.w = fabsf(80 * jc.axis[i].gyro_x / 10);
            SDL_RenderFillRect(render, &sr);

            sr.y += 40;
            sr.x = 250;
            if (jc.axis[i].gyro_y < 0)
                sr.x += (80 * jc.axis[i].gyro_y / 10);
            sr.w = fabsf(80 * jc.axis[i].gyro_y / 10);
            SDL_RenderFillRect(render, &sr);

            sr.y += 40;
            sr.x = 250;
            if (jc.axis[i].gyro_z < 0)
                sr.x += (80 * jc.axis[i].gyro_z / 10);
            sr.w = fabsf(80 * jc.axis[i].gyro_z / 10);
            SDL_RenderFillRect(render, &sr);
        }
        {
            sr = (SDL_Rect) { 250, 290, 0, 0 };
            sprintf(str, "%f", jc.axis[0].acc_x);
            RenderText(render, ttf, str, &sr);

            sr.y += 40;
            sprintf(str, "%f", jc.axis[0].acc_y);
            RenderText(render, ttf, str, &sr);

            sr.y += 40;
            sprintf(str, "%f", jc.axis[0].acc_z);
            RenderText(render, ttf, str, &sr);

            sr.y += 80;
            sprintf(str, "%f", jc.axis[0].gyro_x);
            RenderText(render, ttf, str, &sr);

            sr.y += 40;
            sprintf(str, "%f", jc.axis[0].gyro_y);
            RenderText(render, ttf, str, &sr);

            sr.y += 40;
            sprintf(str, "%f", jc.axis[0].gyro_z);
            RenderText(render, ttf, str, &sr);
        }

        // ボタン
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        joycon_elapsed ela;
        joycon_get_button_elapsed(&jc, &ela);
        sr = (SDL_Rect) { 240, 0, 32, 20 };
        for (int i = 0; i < 11; i++) {
            if (i == 4) {
                sr.x += 70;
                sr.y = 0;
            }
            if (i == 8) {
                sr.x += 70;
                sr.y = 0;
                sr.w = 64;
            }
            sr.y += 40;
            SDL_RenderDrawRect(render, &sr);
            int on = 0;
            u16 tm = 0;
            switch (i) {
            case 0:
                if (jc.button.btn.Y)
                    on = 1;
                break;
            case 1:
                if (jc.button.btn.X)
                    on = 1;
                break;
            case 2:
                if (jc.button.btn.B)
                    on = 1;
                break;
            case 3:
                if (jc.button.btn.A)
                    on = 1;
                break;
            case 4:
                tm = ela.SR;
                if (jc.button.btn.SR_r)
                    on = 1;
                break;
            case 5:
                tm = ela.SL;
                if (jc.button.btn.SL_r)
                    on = 1;
                break;
            case 6:
                tm = ela.R;
                if (jc.button.btn.R)
                    on = 1;
                break;
            case 7:
                tm = ela.ZR;
                if (jc.button.btn.ZR)
                    on = 1;
                break;
            case 8:
                if (jc.button.btn.Plus)
                    on = 1;
                break;
            case 9:
                if (jc.button.btn.RStick)
                    on = 1;
                break;
            case 10:
                tm = ela.Home;
                if (jc.button.btn.Home)
                    on = 1;
            default:
                break;
            }
            if (on)
                SDL_RenderFillRect(render, &sr);
            SDL_Rect sr2 = sr;
            SDL_QueryTexture(txtA[i], NULL, NULL, &sr2.w, &sr2.h);
            sr2.x = sr.x + (i < 8 ? 16 : 32) - sr2.w / 2;
            sr2.y = sr.y + 10 - sr2.h / 2;
            SDL_RenderCopy(render, txtA[i], NULL, &sr2);
            if (tm) {
                sprintf(str, "%u", tm * 10);
                sr2.y += sr.h;
                RenderText(render, ttf, str, &sr2);
            }
        }

        // led
        sr = (SDL_Rect) { 600, 60, 32, 32 };
        if (leds & JOYCON_LED_4_ON) {
            SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
            SDL_RenderFillRect(render, &sr);
        } else {
            if (leds & JOYCON_LED_4_BLINK) {
                SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
                sr.h = 16;
                SDL_RenderFillRect(render, &sr);
                sr.h = 32;
            }
        }
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawRect(render, &sr);
        sr.x += 40;
        if (leds & JOYCON_LED_3_ON) {
            SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
            SDL_RenderFillRect(render, &sr);
        } else {
            if (leds & JOYCON_LED_3_BLINK) {
                SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
                sr.h = 16;
                SDL_RenderFillRect(render, &sr);
                sr.h = 32;
            }
        }
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawRect(render, &sr);
        sr.x += 40;
        if (leds & JOYCON_LED_2_ON) {
            SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
            SDL_RenderFillRect(render, &sr);
        } else {
            if (leds & JOYCON_LED_2_BLINK) {
                SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
                sr.h = 16;
                SDL_RenderFillRect(render, &sr);
                sr.h = 32;
            }
        }
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawRect(render, &sr);
        sr.x += 40;
        if (leds & JOYCON_LED_1_ON) {
            SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
            SDL_RenderFillRect(render, &sr);
        } else {
            if (leds & JOYCON_LED_1_BLINK) {
                SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
                sr.h = 16;
                SDL_RenderFillRect(render, &sr);
                sr.h = 32;
            }
        }
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawRect(render, &sr);

        // home led
        if (homeledflg) {
            if (homeledsr.x < 600)
                homeledsr.x = 600;
            if (homeledsr.x > 760)
                homeledsr.x = 760;
            int c = 256 - ((homeledsr.x - 600) / 10) * 16;
            if (c > 255)
                c = 255;
            if (c < 0)
                c = 0;
            homeledcol                      = c;
            joycon_homeled homeleddata      = { 1, 1, 0 };
            homeleddata.intensity           = (~homeledcol) >> 4;
            homeleddata.mc[0].mc1_intensity = (~homeledcol) >> 4;
            homeleddata.mc[0].mc1_duration  = 15;
            joycon_set_homeled(&jc, &homeleddata);
        }
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawLine(render, 600, 130, 780, 130);
        SDL_SetRenderDrawColor(render, homeledcol, homeledcol, 255, 255);
        SDL_RenderFillRect(render, &homeledsr);
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawRect(render, &homeledsr);

        // ir{ 500, 220 }, { 500, 480 }
        if (irflg) {
            if (irsr.x < 650)
                irsr.x = 650;
            if (irsr.x > 770)
                irsr.x = 770;
        }
        switch (irtxtP) {
        case 0:
            irresol = JOYCON_IR40X30;
            break;
        case 1:
            irresol = JOYCON_IR80X60;
            break;
        case 2:
            irresol = JOYCON_IR160X120;
            break;
        case 3:
            irresol = JOYCON_IR320X240;
        default:
            break;
        }
        if (ircapflg) {
            if (jc.ir_enable) {
                joycon_disable_ir(&jc);
            } else {
                old_exposure = (irsr.x - 650) * 5;
                joycon_enable_ir(&jc, irresol, old_exposure);
            }
            ircapflg = 0;
        }
        if (jc.ir_enable) {
            joycon_ir_result irret;
            joycon_read_ir(&jc, irdata, sizeof(irdata), old_exposure, &irret);
            printf("Avg Int: %u, White: %u, Noise: %u\n", irret.avg_intensity, irret.white_pixels, irret.noise_pixels);
            for (int i = 320 * 240; i > 0; i--) {
                irdata[i * 3 - 1] = irdata[i - 1];
                irdata[i * 3 - 2] = irdata[i - 1];
                irdata[i * 3 - 3] = irdata[i - 1];
            }
            int w            = irresol;
            int h            = w * 3 / 4;
            SDL_Surface* sur = SDL_CreateRGBSurfaceWithFormatFrom(irdata, w, h, 24, 3 * w, SDL_PIXELFORMAT_RGB24);
            SDL_DestroyTexture(irture);
            irture = SDL_CreateTextureFromSurface(render, sur);
            SDL_FreeSurface(sur);

            int white_pixels_percent = irret.white_pixels * 100 / (jc.ir_max_frag_no * 300);
            if (white_pixels_percent == 0) {
                old_exposure = jc.ir_exposure + 10;
                if (old_exposure > 600)
                    old_exposure = 600;
            } else if (white_pixels_percent > 5) {
                old_exposure = jc.ir_exposure - white_pixels_percent * 10;
                if (old_exposure > 600)
                    old_exposure = 0;
            }
        }
        SDL_Rect sr2 = ircapsr;
        SDL_QueryTexture(txtL[13], NULL, NULL, &sr2.w, &sr2.h);
        sr2.x = ircapsr.x + 160 - sr2.w / 2;
        sr2.y = ircapsr.y + 120 - sr2.h / 2;
        SDL_RenderCopy(render, txtL[13], NULL, &sr2);
        if (irture) {
            SDL_RenderCopy(render, irture, NULL, &ircapsr);
        }
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawRect(render, &ircapsr);

        SDL_RenderDrawRect(render, &irressr);
        RenderText(render, ttf, irTxt[irtxtP], &irressr);

        SDL_RenderDrawLine(render, 650, 209, 790, 209);
        SDL_RenderDrawRect(render, &irsr);

        // nfc
        sr = (SDL_Rect) { 490, 480, 320, 100 };
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderDrawRect(render, &sr);
        if (nfcflg) {
            joycon_read_nfc(&jc, &nfcdata);
            nfcresflg = 1;
            nfcflg    = 0;
        }
        if (nfcresflg) {
            if (nfcdata.tag_type) {
                sprintf(str, "uid: %02x", nfcdata.tag_uid[0]);
                for (int i = 1; i < nfcdata.tag_uid_size; i++) {
                    char substr[10];
                    sprintf(substr, ":%02x", nfcdata.tag_uid[i]);
                    strncat(str, substr, 200);
                }
                strncat(str, "\nntag: ", 200);
                for (int i = 0; i < ((nfcdata.ntag_data_size < 40) ? nfcdata.ntag_data_size : 40); i++) {
                    if (i == 12 || i == 26)
                        strncat(str, "\n", 200);
                    char substr[10];
                    sprintf(substr, ":%02x", nfcdata.ntag_data[i]);
                    strncat(str, substr, 200);
                }
                RenderText(render, ttf, str, &sr);
            } else
                RenderText(render, ttf, "Scan Error.", &sr);
        } else {
            sr2 = sr;
            SDL_QueryTexture(txtL[14], NULL, NULL, &sr2.w, &sr2.h);
            sr2.x = sr.x + 160 - sr2.w / 2;
            sr2.y = sr.y + 50 - sr2.h / 2;
            SDL_RenderCopy(render, txtL[14], NULL, &sr2);
        }

        SDL_RenderPresent(render);
        // SDL_Delay(20);
    }

    joycon_close(&jc);

    TTF_CloseFont(ttf);
    TTF_Quit();

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}