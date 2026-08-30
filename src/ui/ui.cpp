#include <Arduino.h>
#include <TFT_eSPI.h>
#include "theme.h"
#include "ui.h"
#include "../Lib/images.h"
#include "../Lib/Free_Fonts.h"
#include "../qrcoded.h"

extern TFT_eSPI tft;

namespace ui {

/*==============================================================
  PIEZAS
==============================================================*/

/* Fuente 5x7 integrada. Se dibuja carácter a carácter para poder
   espaciarla: las versalitas necesitan aire o se leen como un bloque. */
void tiny(const char *s, int x, int y, uint16_t col, char datum, int sp){
  int n = strlen(s);
  int w = n * (UI_TINY_W + sp) - sp;
  int px = x;
  if(datum == 'C')      px = x - w/2;
  else if(datum == 'R') px = x - w;
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setTextColor(col, UI_BG);
  for(int i=0; i<n; i++){ tft.drawChar(s[i], px, y, 1); px += UI_TINY_W + sp; }
}
static void tiny(const String &s, int x, int y, uint16_t col, char datum='L', int sp=0){
  tiny(s.c_str(), x, y, col, datum, sp);
}

/* Cara del dado por su valor: los puntos salen de una máscara sobre la
   rejilla 3x3, así una sola función sirve para el menú y para la captura. */
void die(int x, int y, int s, uint8_t value, uint16_t col){
  static const uint16_t FACE[6] = { 0x010, 0x101, 0x111, 0x145, 0x155, 0x16D };
  if(value < 1 || value > 6) return;

  /* radio de esquina /8, no /6: a radios grandes el arco de TFT_eSPI escalona */
  const int r = max(2, s/8), pip = max(1, s/10);
  tft.fillRoundRect(x, y, s, s, r, UI_BG);
  tft.drawRoundRect(x, y, s, s, r, col);
  if(s >= 32) tft.drawRoundRect(x+1, y+1, s-2, s-2, r-1, col);

  const uint16_t face = FACE[value-1];
  for(int i=0; i<9; i++){
    if(!(face & (1 << i))) continue;
    tft.fillCircle(x + (s * (1 + 2*(i%3)))/6, y + (s * (1 + 2*(i/3)))/6, pip, col);
  }
}

/* El triángulo del bitmap original trae una fila punteada encima. Dibujado
   con primitivas no hay puntos sueltos y se puede teñir a cualquier nivel. */
void caret(int cx, int y, int w, int h, uint16_t col){
  tft.fillTriangle(cx - w/2, y, cx + w/2, y, cx, y + h, col);
}

void bar(int x, int y, int w, int h, float frac){
  if(frac < 0) frac = 0; if(frac > 1) frac = 1;
  tft.fillRect(x, y, w, h, UI_TRACK);
  tft.fillRect(x, y, (int)(w * frac + 0.5f), h, UI_ACCENT);
}

/* Raíl derecho: MOVE arriba, OK abajo, donde están los botones físicos */
static void rail(const char *topAct, const char *botAct){
  tft.drawFastVLine(UI_RAIL_X, 0, UI_H, UI_TRACK);
  caret(UI_RAIL_CX, 12, 9, 6, UI_DIM);
  tiny("MOVE", UI_RAIL_CX, UI_RAIL_TOP_Y,    UI_ACCENT, 'C', 1);
  tiny(topAct, UI_RAIL_CX, UI_RAIL_TOP_Y+10, UI_DIM,    'C', 0);
  tiny("OK",   UI_RAIL_CX, UI_RAIL_BOT_Y,    UI_ACCENT, 'C', 1);
  tiny(botAct, UI_RAIL_CX, UI_RAIL_BOT_Y+10, UI_DIM,    'C', 0);
  caret(UI_RAIL_CX, 124, 9, -6, UI_DIM);
}

static void eyebrow(const char *s){ tiny(s, UI_M, 10, UI_ACCENT, 'L', 1); }

/* Número grande: el único elemento a voz alta de la pantalla */
static void bigNumber(int n, int baselineY){
  char buf[8]; snprintf(buf, sizeof(buf), "%d", n);
  tft.fillRect(0, baselineY-36, 100, 40, UI_BG);   //hasta el dado, no más
  tft.setFreeFont(FMB24);
  tft.setTextColor(UI_TEXT);
  tft.setCursor(UI_M, baselineY);
  tft.print(buf);
}

/* Cabecera de las pantallas de semilla: etiqueta, paso y una regla fina.
   Sustituye a la franja verde maciza, que era lo que más envejecía la v1. */
static void head(const char *title, uint8_t step, uint8_t total){
  tft.fillScreen(UI_BG);
  tiny(title, UI_M, 10, UI_ACCENT, 'L', 1);
  if(total){
    char b[8]; snprintf(b, sizeof(b), "%u/%u", step, total);
    tiny(b, UI_W-UI_M, 10, UI_DIM, 'R', 0);
  }
  tft.drawFastHLine(UI_M, 20, UI_W - 2*UI_M, UI_TRACK);
}

/* Cuerpo de texto. Una sola tipografía por pantalla: o toda pequeña o toda
   FreeMono. UI_BODY_TINY decide cuál, para poder compararlo en la placa. */
#if UI_BODY_TINY
  #define BODY_LH   14
  #define BODY_CPL  UI_TINY_CPL
  static void bodyLine(const String &s, int y, uint16_t col){ tiny(s, UI_M, y, col, 'L', 0); }
#else
  #define BODY_LH   18
  #define BODY_CPL  UI_MONO_CPL
  static void bodyLine(const String &s, int y, uint16_t col){
    tft.setFreeFont(FM9); tft.setTextColor(col, UI_BG);
    tft.setCursor(UI_M, y); tft.print(s);
  }
#endif

/* Parte una cadena larga en líneas del ancho disponible */
static void bodyWrap(const String &s, int y, uint16_t col){
  int i = 0;
  while(i < (int)s.length()){
    bodyLine(s.substring(i, min((int)s.length(), i + BODY_CPL)), y, col);
    i += BODY_CPL; y += BODY_LH;
  }
}

/*==============================================================
  PANTALLAS
==============================================================*/

void splash(void){
  tft.fillScreen(UI_BG);
  tft.pushImage(20, 23, logoWidth, logoHeight, seeder_logo);
  delay(2000);
  tft.fillScreen(UI_BG);
  tft.pushImage(60, 34, logouBTCWidth, logouBTCHeight, uBitcoinLogo);
  tft.pushImage(95, 110, poweredWidth, poweredHeight, powered_logo);
  delay(2000);
  tft.fillScreen(UI_BG);
}

void menu(bool diceSelected){
  tft.pushImage(0, 0, menuHeaderWidth, menuHeaderHeight, menu_header);
  tft.fillRect(0, menuHeaderHeight, UI_W, UI_H - menuHeaderHeight, UI_BG);

  const uint16_t dc = diceSelected ? UI_ACCENT : UI_DIM;
  const uint16_t cc = diceSelected ? UI_DIM    : UI_ACCENT;

  die(49, 57, 46, 5, dc);
  tiny("DICE SEED", 72, 109, dc, 'C', 0);
  tft.drawXBitmap(135, 57, iconMoneda, iconMonedaWidth, iconMonedaHeight, cc, UI_BG);

  caret(72,  40, 13, 7, diceSelected ? UI_ACCENT : UI_TRACK);
  caret(162, 40, 13, 7, diceSelected ? UI_TRACK  : UI_ACCENT);
}

/* Tarjeta con el número en una fuente de verdad: los dígitos del bitmap
   original iban en manuscrita y a esta resolución no se leían. */
static void wordCard(int x, int y, int w, int h, uint8_t n, uint16_t col, uint16_t sub){
  tft.drawRoundRect(x, y, w, h, 7, col);
  tft.drawRoundRect(x+1, y+1, w-2, h-2, 6, col);
  char b[4]; snprintf(b, sizeof(b), "%u", n);
  tft.setFreeFont(FMB24);
  tft.setTextColor(col);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(b, x + w/2, y + h - 16, GFXFF);
  tft.setTextDatum(TL_DATUM);
  tiny("WORDS", x + w/2, y + h - 13, sub, 'C', 1);
}

void words(uint8_t nWords){
  tft.pushImage(0, 0, menuHeaderWidth, menuHeaderHeight, menu_header);
  tft.fillRect(0, menuHeaderHeight, UI_W, UI_H - menuHeaderHeight, UI_BG);

  const bool w12 = (nWords == 12);
  wordCard(45, 56, 54, 56, 12, w12 ? UI_ACCENT : UI_DIM, w12 ? UI_DIM  : UI_TRACK);
  wordCard(135,56, 54, 56, 24, w12 ? UI_DIM    : UI_ACCENT, w12 ? UI_TRACK : UI_DIM);

  caret(72,  40, 13, 7, w12 ? UI_ACCENT : UI_TRACK);
  caret(162, 40, 13, 7, w12 ? UI_TRACK  : UI_ACCENT);
}

/*----------------- captura de moneda -----------------*/
void coinEnter(uint16_t totalBits){
  tft.fillScreen(UI_BG);
  eyebrow("FLIP COIN");
  tiny("BITS LEFT", UI_M, 70, UI_DIM, 'L', 1);
  rail("HEADS", "TAILS");
}

void coinUpdate(uint16_t done, uint16_t totalBits, const uint8_t *entropy){
  bigNumber(totalBits - done, 62);

  /* Los últimos 16 lanzamientos: lleno = cara, hueco = cruz */
  const int from = (done > 16) ? done - 16 : 0;
  tft.fillRect(UI_M, 80, UI_RAIL_X - UI_M - 4, 8, UI_BG);
  for(int i=0; i<16; i++){
    const int idx = from + i, x = UI_M + i*9;
    if(idx >= done) break;
    const uint8_t bit = (entropy[idx/8] >> (7 - idx%8)) & 1;
    if(bit) tft.fillRect(x, 80, 8, 8, UI_ACCENT);
    else    tft.drawRect(x, 80, 8, 8, UI_DIM);
  }

  /* La entropía en hexadecimal, por bytes y alternando el color: es lo que
     el usuario coteja contra su papel mientras lanza. */
  tft.fillRect(UI_M, 96, UI_RAIL_X - UI_M - 4, 20, UI_BG);
  const int bytes = done / 8;
  const int first = (bytes > 26) ? bytes - 26 : 0;
  for(int i=first; i<bytes; i++){
    char b[3]; snprintf(b, sizeof(b), "%02X", entropy[i]);
    const int k = i - first;
    tiny(b, UI_M + (k % 13) * 13, 96 + (k / 13) * 10,
         (i % 2) ? UI_TEXT : UI_ACCENT, 'L', 0);
  }

  bar(UI_M, 120, UI_RAIL_X - 2*UI_M, 4, (float)done / totalBits);
}

/*----------------- captura de dado -----------------*/
void diceEnter(uint8_t totalRolls){
  tft.fillScreen(UI_BG);
  eyebrow("ROLL DICE");
  tiny("ROLLS LEFT", UI_M, 70, UI_DIM, 'L', 1);
  rail("1-6", "ACCEPT");
}

void diceUpdate(uint8_t done, uint8_t totalRolls, uint8_t value, uint8_t last){
  bigNumber(totalRolls - done, 62);

  const int size = 64, x = UI_RAIL_X - size - 11;
  tft.fillRect(x, 22, size, size, UI_BG);
  die(x, 22, size, value, UI_ACCENT);

  /* La tirada anterior, apagada: confirma que entró lo que querías */
  tft.fillRect(UI_M, 86, 80, 30, UI_BG);
  if(last){
    die(UI_M, 86, 30, last, UI_DIM);
    tiny("LAST", UI_M + 36, 98, UI_TRACK, 'L', 1);
  }

  bar(UI_M, 120, UI_RAIL_X - 2*UI_M, 4, (float)done / totalRolls);
}

void generating(void){
  tft.fillScreen(UI_BG);
  tiny("GENERATING SEED", UI_W/2, 64, UI_ACCENT, 'C', 1);
}

/*----------------- pantallas de la semilla -----------------*/
void mnemonic(const String &mn, uint8_t nWords, uint8_t from, uint8_t step, uint8_t total){
  head(nWords == 12 ? "MNEMONIC WORDS" : (from ? "MNEMONIC 2/2" : "MNEMONIC 1/2"), step, total);

  /* Doce palabras por página, cortando por espacios, nunca por la mitad */
  int idx = 0, shown = 0, y = 34, used = 0;
  String line = "";
  int start = 0;
  while(start < (int)mn.length() && shown < 12){
    int sp = mn.indexOf(' ', start);
    String w = (sp < 0) ? mn.substring(start) : mn.substring(start, sp);
    start = (sp < 0) ? mn.length() : sp + 1;
    if(idx++ < from){ continue; }
    if(used + (int)w.length() + 1 > BODY_CPL){
      bodyLine(line, y, UI_TEXT); y += BODY_LH; line = ""; used = 0;
    }
    line += w; line += ' '; used += w.length() + 1;
    shown++;
  }
  if(line.length()) bodyLine(line, y, UI_TEXT);
}

void seedAddress(const String &addr, uint8_t step, uint8_t total){
  head("FIRST ADDRESS", step, total);
  bodyLine("m/84'/0'/0'/0/0", 34, UI_ACCENT);
  bodyWrap(addr, 34 + 2*BODY_LH, UI_TEXT);
}

void seedZpub(const String &zpub, uint8_t step, uint8_t total){
  head("ACCOUNT ZPUB", step, total);
  bodyWrap(zpub, 34, UI_TEXT);
}

void seedEntropy(const String &hex, uint8_t step, uint8_t total){
  head("ENTROPY (HEX)", step, total);
  /* Por bytes y alternando color: así se canta en voz alta sin perderse */
  int y = 34;
  for(int i=0; i*2 < (int)hex.length(); i++){
    const int k = i % 17;
    if(i && k == 0) y += 12;
    tiny(hex.substring(i*2, i*2+2), UI_M + k*13, y, (i%2) ? UI_TEXT : UI_ACCENT, 'L', 0);
  }
  tiny("CHECK IT OFFLINE", UI_M, y + 24, UI_DIM, 'L', 1);
}

void seedQr(const String &data){
  const int version = 11, px = 2;
  QRCode qrcode;
  uint8_t buf[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, buf, version, 0, data.c_str());

  tft.fillScreen(UI_BG);
  tiny("EXPORT", UI_M, 10, UI_ACCENT, 'L', 1);
  tiny("SCAN WITH AN",  UI_M, 34, UI_DIM, 'L', 0);
  tiny("OFFLINE WALLET",UI_M, 44, UI_DIM, 'L', 0);
  tiny("NEVER A PHONE", UI_M, 62, UI_ACCENT, 'L', 0);

  const int off = 56;
  for(uint8_t y=0; y<qrcode.size; y++)
    for(uint8_t x=0; x<qrcode.size; x++)
      tft.fillRect((off + x + 2)*px, (y + 3)*px, px, px,
                   qrcode_getModule(&qrcode, x, y) ? UI_QR_LIGHT : UI_BG);
}

void seedExit(void){
  head("EXIT", 0, 0);
  bodyLine("Seed is only in RAM.",  36, UI_TEXT);
  bodyLine("Write it down first.",  36 + BODY_LH, UI_TEXT);
  bodyLine("Hold OK: wipe & exit",  36 + 3*BODY_LH, UI_ACCENT);
}

}  // namespace ui
