/*!
 @file  joycon_ir.c
 @brief NFCサンプル
 @author    K. Morita
 @date  2023/5/5
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

    puts("Touch the NFC tag (e.g. Student card, IC card) on the stick, and push enter key.");
    getchar();

    // NFC読込
    joycon_nfc_data nfcdata;
    joycon_read_nfc(&jc, &nfcdata);

    if (nfcdata.tag_type) {
        printf("uid: %02x", nfcdata.tag_uid[0]);
        for (int i = 1; i < nfcdata.tag_uid_size; i++) {
            printf(":%02x", nfcdata.tag_uid[i]);
        }
        printf("\nntag: ");
        for (int i = 0; i < nfcdata.ntag_data_size; i++) {
            printf(":%02x", nfcdata.ntag_data[i]);
        }
        printf("\n");
    } else
        puts("Scan Error.");

    printf("\n");
    joycon_close(&jc);

    return 0;
}