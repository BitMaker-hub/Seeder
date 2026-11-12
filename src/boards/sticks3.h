#pragma once
/* M5Stack StickS3 (K150), ESP32-S3-PICO-1-N8R8.
   This is not the ESP32-based StickC Plus2.

   M5GFX configures LCD power through M5PM1, SPI pins, panel offsets and
   backlight. Rotation 1 provides the upstream 240x135 reference canvas. */
#define BOARD_NAME        "M5StickS3"
#define UI_W              240
#define UI_H              135

#define PIN_MOVE          12     // side button B / KEY2
#define PIN_SELECT        11     // front button A / KEY1

/* The separate power key remains under M5PM1 hardware control:
   single click = power on / reset; double click = power off. */
