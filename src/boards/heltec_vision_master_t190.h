#pragma once
/* Heltec Vision Master T190 · ESP32-S3R8 · ST7789 SPI · 170x320.
 * The Seeder UI is landscape, so TFT_eSPI rotation 1 exposes 320x170.
 */
#define BOARD_NAME          "Heltec Vision Master T190"
#define UI_W                320
#define UI_H                170

#define PIN_MOVE            21    // custom button
#define PIN_SELECT          0     // BOOT button
#define PIN_DISPLAY_POWER   7     // TFT supply enable, active LOW
#define PIN_DISPLAY_POWER_ACTIVE LOW
