#pragma once
/**********************************
 🍃 LilyGO TTGO T-Display  ·  ESP32  ·  240x135  ·  ST7789 por SPI
**********************************/
#define BOARD_NAME        "T-Display"

#define UI_W              240
#define UI_H              135

#define PIN_MOVE          35     // botón de arriba
#define PIN_SELECT        0      // botón de abajo (BOOT)
/* Sin pin de alimentación de periféricos: la pantalla cuelga del 3V3 */
