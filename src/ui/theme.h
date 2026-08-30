#pragma once

/**********************************
 🍃 TEMA
 Todo lo que define el aspecto vive aquí. Cambiar de estética, o de placa,
 es cambiar este fichero: ni la máquina de estados ni la lógica de entropía
 saben nada de colores ni de coordenadas.
**********************************/

/*** Colores (RGB565) *************************************************
 Tres niveles de brillo, no dos. Sin el nivel apagado todo grita a la vez
 y nada guía la mirada: era el motivo de que la v1 se viera dura.        */
#define UI_ACCENT     0x86F3   // verde SEEDER: lo que importa ahora
#define UI_TEXT       0xD6BA   // texto principal
#define UI_DIM        0x4A49   // texto secundario y pistas
#define UI_TRACK      0x2945   // reglas y fondo de barras
#define UI_BG         0x0000
#define UI_QR_LIGHT   0xFFFF

/*** Métricas *********************************************************/
#define UI_W          240
#define UI_H          135
#define UI_M          10      // margen izquierdo del contenido
#define UI_RAIL_X     183     // el raíl de botones empieza aquí
#define UI_RAIL_CX    ((UI_RAIL_X + UI_W) / 2)

/* Los dos botones del T-Display están en el borde derecho, uno arriba y
   otro abajo, así que la pista de cada uno se pinta a su altura.        */
#define UI_RAIL_TOP_Y   22    // MOVE
#define UI_RAIL_BOT_Y   96    // OK

/*** Tipografía *******************************************************
 La fuente 5x7 integrada avanza 6px por carácter: 40 caben por línea.
 UI_BODY_TINY la usa también para el cuerpo de las pantallas de semilla.
 Ponlo a 0 para volver a FreeMono 9pt si en pantalla no se lee bien.     */
#define UI_TINY_W     6
#define UI_TINY_H     8
#define UI_BODY_TINY  1

/* La misma fuente a doble tamaño para lo que hay que copiar a mano: las
   palabras y la dirección. Un solo tipo de letra, dos tamaños, que es
   jerarquía y no mezcla. 12px de avance -> 18 caracteres por línea.        */
#define UI_BIG_BODY   2
#define UI_BIG_CPL    ((UI_W - 2*UI_M) / (UI_TINY_W * UI_BIG_BODY))
#define UI_BIG_LH     20

/*** Ancho de línea de las fuentes monoespaciadas **********************/
#define UI_MONO_W     11      // FreeMono9pt7b avanza 11px
#define UI_MONO_CPL   (UI_W / UI_MONO_W)          // 21
#define UI_TINY_CPL   ((UI_W - 2*UI_M) / UI_TINY_W)  // 36 con márgenes
