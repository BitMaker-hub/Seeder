#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qrcoded.h"

int main(int argc, char **argv) {
    if (argc != 4 || strlen(argv[1]) != 42) return 2;
    const int scale = atoi(argv[2]);
    if (scale != 3 && scale != 4) return 3;
    char address[43];
    for (int i = 0; i < 42; ++i) address[i] = (char)toupper((unsigned char)argv[1][i]);
    address[42] = '\0';

    QRCode qr;
    uint8_t modules[qrcode_getBufferSize(2)];
    if (qrcode_initText(&qr, modules, 2, ECC_LOW, address) < 0) return 4;
    printf("%u %u %u %u\n", qr.version, qr.size, qr.ecc, qr.mode);

    // The two supported screens use 3px and 4px modules, respectively.
    const int quiet = 4;
    const int side = (qr.size + 2*quiet) * scale;
    FILE *image = fopen(argv[3], "wb");
    if (!image) return 5;
    fprintf(image, "P5\n%d %d\n255\n", side, side);
    for (int y = 0; y < side; ++y)
        for (int x = 0; x < side; ++x) {
            const int mx = x/scale - quiet, my = y/scale - quiet;
            const int dark = mx >= 0 && my >= 0 && mx < qr.size && my < qr.size
                && qrcode_getModule(&qr, (uint8_t)mx, (uint8_t)my);
            if (fputc(dark ? 0 : 255, image) == EOF) return 6;
        }
    return fclose(image) ? 7 : 0;
}
