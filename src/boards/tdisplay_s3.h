#pragma once
/**********************************
 🍃 LilyGO T-Display-S3  ·  ESP32-S3  ·  320x170  ·  ST7789 paralelo 8-bit

 GPIO15 alimenta los periféricos: si no se pone alto, la pantalla no
 enciende y parece una placa muerta. Es el fallo clásico de esta placa.
**********************************/
#define BOARD_NAME        "T-Display-S3"

#define UI_W              320
#define UI_H              170

#define PIN_MOVE          14     // botón de arriba
#define PIN_SELECT        0      // botón de abajo (BOOT)
#define PIN_POWER_ON      15     // alimentación de periféricos, alto = encendido
