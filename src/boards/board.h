#pragma once
/**********************************
 🍃 SELECCIÓN DE PLACA

 La composición de la interfaz se diseñó sobre 240x135. En vez de mantener
 un segundo layout para cada pantalla nueva, cada coordenada literal pasa
 por SX()/SY() y la placa grande reproduce el mismo diseño escalado.

 Los dos ejes escalan distinto (320/240 = 1,33 frente a 170/135 = 1,26),
 así que se escalan por separado y no con un único factor.
**********************************/

#if defined(SEEDER_BOARD_TDISPLAY_S3)
  #include "tdisplay_s3.h"
#else
  #include "tdisplay.h"
#endif

#define UI_REF_W  240        // lienzo de referencia del diseño
#define UI_REF_H  135

#define SX(v)  ((int)(((long)(v) * UI_W) / UI_REF_W))
#define SY(v)  ((int)(((long)(v) * UI_H) / UI_REF_H))
