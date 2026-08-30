//  Selección de configuración de pantalla para la SEEDER.
//
//  El fichero original de TFT_eSPI es una lista de #include comentados donde
//  descomentas el tuyo. Aquí lo elegimos según la placa, para que un solo
//  árbol de fuentes sirva a las dos sin editar la librería al cambiar.

#ifndef USER_SETUP_LOADED

  #if defined(SEEDER_BOARD_TDISPLAY_S3)
    #include <User_Setups/Setup206_LilyGo_T_Display_S3.h>   // 320x170, paralelo 8-bit
  #else
    #include <User_Setups/Setup25_TTGO_T_Display.h>         // 240x135, SPI
  #endif

#endif

#include <TFT_Drivers/ST7789_Defines.h>
#define  TFT_DRIVER 0x7789
