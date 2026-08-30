#include <Arduino.h>
#include <TFT_eSPI.h>
#include "theme.h"
#include "ui.h"
#include "../Lib/images.h"
#include "../Lib/Free_Fonts.h"
#include "../qrcoded.h"
#include "brand.h"

extern TFT_eSPI tft;

namespace ui {

/*==============================================================
  PIEZAS
==============================================================*/

/* Fuente 5x7 integrada. Se dibuja carácter a carácter para poder
   espaciarla: las versalitas necesitan aire o se leen como un bloque. */
void tiny(const char *s, int x, int y, uint16_t col, char datum, int sp, uint8_t size){
  const int n  = strlen(s);
  const int adv = UI_TINY_W * size + sp;
  const int w  = n * adv - sp;
  int px = x;
  if(datum == 'C')      px = x - w/2;
  else if(datum == 'R') px = x - w;
  tft.setTextFont(1);
  tft.setTextSize(size);
  tft.setTextColor(col, UI_BG);
  for(int i=0; i<n; i++){ tft.drawChar(s[i], px, y, 1); px += adv; }
  tft.setTextSize(1);
}
static void tiny(const String &s, int x, int y, uint16_t col, char datum='L', int sp=0, uint8_t size=1){
  tiny(s.c_str(), x, y, col, datum, sp, size);
}

/* Cuerpo grande: lo que el usuario tiene que copiar a mano */
static void bigLine(const String &s, int y, uint16_t col){
  tiny(s, UI_M, y, col, 'L', 0, UI_BIG_BODY);
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

  /* 0.22 / 0.50 / 0.78 del lado, no 1/6 / 3/6 / 5/6: los de las esquinas
     se iban al borde y ensuciaban la cara */
  const uint16_t face = FACE[value-1];
  for(int i=0; i<9; i++){
    if(!(face & (1 << i))) continue;
    tft.fillCircle(x + (s*(22 + 28*(i%3)))/100, y + (s*(22 + 28*(i/3)))/100, pip, col);
  }
}

/* Moneda de canto: cara elíptica a la izquierda y el canto a la derecha con
   sus estrías. Dibujada entera, así se tiñe igual que el dado y no depende
   de reducir un bitmap, que a este tamaño quedaba embarrado. */
void coin(int x, int y, int s, uint16_t col){
  const int rx = s/4, ry = s/2 - 2, dep = s/3;
  const int cx = x + rx + 1, cy = y + s/2;

  tft.drawEllipse(cx, cy, rx, ry, col);              //cara

  int prev = -1;                                     //borde exterior del canto
  for(int j = -ry; j <= ry; j++){
    const int w = (int)(rx * sqrtf(fmaxf(0.0f, 1.0f - (float)(j*j)/(float)(ry*ry))) + 0.5f);
    if(prev >= 0 && abs(w - prev) > 1){              //cerrar el escalón
      for(int q = min(w,prev); q <= max(w,prev); q++) tft.drawPixel(cx+dep+q, cy+j, col);
    }else tft.drawPixel(cx+dep+w, cy+j, col);
    prev = w;
  }
  tft.drawFastHLine(cx, cy-ry, dep+1, col);          //tapas
  tft.drawFastHLine(cx, cy+ry, dep+1, col);

  for(int k = -3; k <= 3; k++){                      //estrías
    const int j = (k*ry)/4;
    const int w = (int)(rx * sqrtf(fmaxf(0.0f, 1.0f - (float)(j*j)/(float)(ry*ry))) + 0.5f);
    tft.drawFastHLine(cx+w, cy+j, dep, col);
  }
}

/* El triángulo del bitmap original trae una fila punteada encima. Dibujado
   con primitivas no hay puntos sueltos y se puede teñir a cualquier nivel. */
void caret(int cx, int y, int w, int h, uint16_t col){
  tft.fillTriangle(cx - w/2, y, cx + w/2, y, cx, y + h, col);
}

void bar(int x, int y, int w, int h, float frac){
  if(frac < 0) frac = 0;
  if(frac > 1) frac = 1;
  tft.fillRect(x, y, w, h, UI_TRACK);
  tft.fillRect(x, y, (int)(w * frac + 0.5f), h, UI_ACCENT);
}

/* Raíl derecho: MOVE arriba, OK abajo, donde están los botones físicos */
/* Cuando la acción ya dice qué botón es (cara/cruz) el nombre de la tecla
   sobra: la posición en el raíl ya señala el botón físico. */
static void rail(const char *topAct, const char *botAct, bool showKeys){
  tft.drawFastVLine(UI_RAIL_X, 0, UI_H, UI_TRACK);
  caret(UI_RAIL_CX, SY(12), SX(9), SY(6), UI_DIM);
  if(showKeys){
    tiny("MOVE", UI_RAIL_CX, UI_RAIL_TOP_Y,    UI_ACCENT, 'C', 1);
    tiny(topAct, UI_RAIL_CX, UI_RAIL_TOP_Y+SY(10), UI_DIM,    'C', 0);
    tiny("OK",   UI_RAIL_CX, UI_RAIL_BOT_Y,    UI_ACCENT, 'C', 1);
    tiny(botAct, UI_RAIL_CX, UI_RAIL_BOT_Y+SY(10), UI_DIM,    'C', 0);
  }else{
    tiny(topAct, UI_RAIL_CX, UI_RAIL_TOP_Y+SY(4), UI_ACCENT, 'C', 1);
    tiny(botAct, UI_RAIL_CX, UI_RAIL_BOT_Y+SY(4), UI_ACCENT, 'C', 1);
  }
  caret(UI_RAIL_CX, SY(124), SX(9), SY(-6), UI_DIM);
}

static void eyebrow(const char *s){ tiny(s, UI_M, SY(10), UI_ACCENT, 'L', 1); }

/* Número grande: el único elemento a voz alta de la pantalla */
static int bigNumber(int n, int baselineY){
  char buf[8]; snprintf(buf, sizeof(buf), "%d", n);
  tft.fillRect(0, baselineY-36, SX(100), 40, UI_BG);   //alto por la fuente, ancho por la placa
  tft.setFreeFont(FMB24);
  tft.setTextColor(UI_TEXT);
  tft.setCursor(UI_M, baselineY);
  tft.print(buf);
  return tft.getCursorX();
}

/* Cabecera de las pantallas de semilla: etiqueta, paso y una regla fina.
   Sustituye a la franja verde maciza, que era lo que más envejecía la v1. */
static void head(const char *title, uint8_t step, uint8_t total){
  tft.fillScreen(UI_BG);
  tiny(title, UI_M, SY(10), UI_ACCENT, 'L', 1);
  if(total){
    char b[8]; snprintf(b, sizeof(b), "%u/%u", step, total);
    tiny(b, UI_W-UI_M, SY(10), UI_DIM, 'R', 0);
  }
  tft.drawFastHLine(UI_M, SY(20), UI_W - 2*UI_M, UI_TRACK);
}

/* Cuerpo de texto. Una sola tipografía por pantalla: o toda pequeña o toda
   FreeMono. UI_BODY_TINY decide cuál, para poder compararlo en la placa. */
#if UI_BODY_TINY
  #define BODY_LH   SY(14)
  #define BODY_CPL  UI_TINY_CPL
  static void bodyLine(const String &s, int y, uint16_t col){ tiny(s, UI_M, y, col, 'L', 0); }
#else
  #define BODY_LH   SY(18)
  #define BODY_CPL  UI_MONO_CPL
  static void bodyLine(const String &s, int y, uint16_t col){
    tft.setFreeFont(FM9); tft.setTextColor(col, UI_BG);
    tft.setCursor(UI_M, y); tft.print(s);
  }
#endif

/*==============================================================
  PANTALLAS
==============================================================*/

void splash(void){
  tft.fillScreen(UI_BG);
  tft.pushImage((UI_W-logoWidth)/2, (UI_H-logoHeight)/2 - SY(8), logoWidth, logoHeight, seeder_logo);
  tiny("V" SEEDER_VERSION "  " SEEDER_COMMIT, UI_W/2, UI_H - SY(15), UI_DIM, 'C', 1);
  delay(2000);
  tft.fillScreen(UI_BG);
  tft.pushImage((UI_W-logouBTCWidth)/2, (UI_H-logouBTCHeight)/2 - SY(12), logouBTCWidth, logouBTCHeight, uBitcoinLogo);
  tft.pushImage((UI_W-poweredWidth)/2, UI_H - poweredHeight - SY(8), poweredWidth, poweredHeight, powered_logo);
  delay(2000);
  tft.fillScreen(UI_BG);
}

/* Cabecera de marca: verde macizo con tinta negra. Es lo que separa el menú
   de las pantallas de trabajo, y por eso vuelve. Sin la palabra MENU: ya se
   ve. El logotipo y la hoja son recortes del arte original (ver brand.h). */
static void greenHead(void){
  tft.fillRect(0, 0, UI_W, UI_HEAD_H, UI_ACCENT);
  /* Los dos son bitmaps: no escalan, se colocan. Uno pegado al margen
     izquierdo y el otro al derecho, centrados en la franja. */
  tft.pushImage(SX(10), (UI_HEAD_H-seederMarkHeight)/2,
                seederMarkWidth, seederMarkHeight, seederMark);
  tft.pushImage(UI_W - leafMarkWidth - SX(10), (UI_HEAD_H-leafMarkHeight)/2,
                leafMarkWidth, leafMarkHeight, leafMark);
}

/* Raíl estrecho, sólo símbolos: arriba dos puntas opuestas para
   desplazarse, abajo un check para confirmar. */
static void thinRail(void){
  tft.drawFastVLine(UI_MRAIL_X, UI_HEAD_H, UI_H-UI_HEAD_H, UI_TRACK);
  caret(UI_MRAIL_CX, SY(62), SX(13), SY(-8), UI_DIM);
  caret(UI_MRAIL_CX, SY(68), SX(13), SY( 8), UI_DIM);
  for(int i=0; i<2; i++){                       //grosor 2, no hay drawWideLine
    tft.drawLine(SX(219), SY(108)+i, SX(225), SY(114)+i, UI_ACCENT);
    tft.drawLine(SX(225), SY(114)+i, SX(236), SY(101)+i, UI_ACCENT);
  }
}

/* Fila de opción: barra de acento a la izquierda cuando está elegida, y
   debajo lo que cuesta esa opción. Admite una tercera el día que haga falta. */
static void listRow(int y, bool sel, uint16_t col, const char *lab, const char *sub, int tx){
  if(sel) tft.fillRect(SX(4), y, SX(3), SY(40), UI_ACCENT);
  tiny(lab, tx, y+SY(4),  col, 'L', 1, UI_BIG_BODY);
  tiny(sub, tx, y+SY(26), sel ? UI_DIM : UI_TRACK, 'L', 1);
}

void menu(bool diceSelected){
  tft.fillScreen(UI_BG);
  greenHead();
  thinRail();

  const uint16_t dc = diceSelected ? UI_ACCENT : UI_DIM;
  const uint16_t cc = diceSelected ? UI_DIM    : UI_ACCENT;

  die(SX(14), SY(48), SY(32), 5, dc);
  listRow(SY(44), diceSelected, dc, "DICE SEED", "50 OR 99 ROLLS", SX(58));

  coin(SX(14), SY(92), SY(32), cc);
  listRow(SY(88), !diceSelected, cc, "COIN SEED", "128 OR 256 FLIPS", SX(58));
}

void words(uint8_t nWords){
  tft.fillScreen(UI_BG);
  greenHead();
  thinRail();

  const bool w12 = (nWords == 12);
  const uint16_t c1 = w12 ? UI_ACCENT : UI_DIM;
  const uint16_t c2 = w12 ? UI_DIM    : UI_ACCENT;

  /* FMB18, no FMB24: a 28px por dígito el numeral pesaba demasiado en la
     fila. Centrados en x=36, que es el centro de la columna del icono en la
     pantalla anterior, para que las dos casen. */
  tft.setFreeFont(FMB18);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(c1); tft.drawString("12", SX(36), SY(74),  GFXFF);
  tft.setTextColor(c2); tft.drawString("24", SX(36), SY(118), GFXFF);
  tft.setTextDatum(TL_DATUM);

  listRow(SY(44), w12,  c1, "WORDS", "128 BITS OF ENTROPY", SX(72));
  listRow(SY(88), !w12, c2, "WORDS", "256 BITS OF ENTROPY", SX(72));
}

/*----------------- captura de moneda -----------------*/
void coinEnter(uint16_t totalBits){
  tft.fillScreen(UI_BG);
  eyebrow("FLIP COIN");
  rail("HEADS", "TAILS", false);   //arriba cara, abajo cruz: no hace falta más
}

void coinUpdate(uint16_t done, uint16_t totalBits, const uint8_t *entropy){
  /* La etiqueta va pegada al número y se corre con él. El rastro que dejaba
     al pasar de 100 a 99 era que el rectángulo de borrado cubría y 40..50
     mientras el texto ocupa 46..53: sobrevivían las filas de abajo. */
  const int endX = bigNumber(totalBits - done, SY(56));
  tft.fillRect(endX, SY(44), UI_RAIL_X - endX - SX(4), SY(14), UI_BG);
  tiny("BITS LEFT", endX + SX(10), SY(46), UI_DIM, 'L', 1);

  /* Los últimos 16 lanzamientos: lleno = cara, hueco = cruz */
  const int from = (done > 16) ? done - 16 : 0;
  const int cell = SY(8), pitch = SX(9);
  tft.fillRect(UI_M, SY(72), UI_RAIL_X - UI_M - SX(4), cell, UI_BG);
  for(int i=0; i<16; i++){
    const int idx = from + i, x = UI_M + i*pitch;
    if(idx >= done) break;
    const uint8_t bit = (entropy[idx/8] >> (7 - idx%8)) & 1;
    if(bit) tft.fillRect(x, SY(72), cell, cell, UI_ACCENT);
    else    tft.drawRect(x, SY(72), cell, cell, UI_DIM);
  }

  /* La entropía en hexadecimal, por bytes y alternando el color: es lo que
     el usuario coteja contra su papel mientras lanza. */
  tft.fillRect(UI_M, SY(90), UI_RAIL_X - UI_M - SX(4), SY(24), UI_BG);
  const int bytes = done / 8;
  const int first = (bytes > 26) ? bytes - 26 : 0;
  for(int i=first; i<bytes; i++){
    char b[3]; snprintf(b, sizeof(b), "%02X", entropy[i]);
    const int k = i - first;
    tiny(b, UI_M + (k % 13) * SX(13), SY(90) + (k / 13) * SY(11),
         (i % 2) ? UI_TEXT : UI_ACCENT, 'L', 0);
  }

  bar(UI_M, SY(118), UI_RAIL_X - 2*UI_M, SY(4), (float)done / totalBits);
}

/*----------------- captura de dado -----------------*/
void diceEnter(uint8_t totalRolls){
  tft.fillScreen(UI_BG);
  eyebrow("ROLL DICE");
  tiny("ROLLS LEFT", UI_M, SY(64), UI_DIM, 'L', 1);
  rail("1-6", "ACCEPT", true);
}

/* Las tres últimas tiradas, de más antigua a más reciente. Ver el trío
   completo es lo que te deja comprobar que entró lo que lanzaste. */
void diceHistory(const uint8_t *hist){
  tft.fillRect(UI_M, SY(78), SX(100), SY(26), UI_BG);
  static const uint16_t shade[3] = { UI_TRACK, UI_DIM, UI_TEXT };
  for(int i=0; i<3; i++)
    if(hist[i]) die(UI_M + i*SX(30), SY(78), SY(26), hist[i], shade[i]);
}

void diceUpdate(uint8_t done, uint8_t totalRolls, uint8_t value, const uint8_t *hist){
  bigNumber(totalRolls - done, SY(56));

  const int size = SY(64), x = UI_RAIL_X - size - SX(11);
  tft.fillRect(x, SY(16), size, size, UI_BG);
  die(x, SY(16), size, value, UI_ACCENT);

  diceHistory(hist);
  bar(UI_M, SY(118), UI_RAIL_X - 2*UI_M, SY(4), (float)done / totalRolls);
}

void generating(void){
  tft.fillScreen(UI_BG);
  tiny("GENERATING SEED", UI_W/2, UI_H/2 - SY(4), UI_ACCENT, 'C', 1);
}

/*----------------- pantallas de la semilla -----------------*/
void mnemonic(const String &mn, uint8_t nWords, uint8_t from, uint8_t step, uint8_t total){
  head(nWords == 12 ? "MNEMONIC WORDS" : (from ? "MNEMONIC 13-24" : "MNEMONIC 1-12"), step, total);

  /* Rejilla fija de 6 filas x 2 columnas. En línea corrida, doce palabras
     largas ocupaban seis líneas y la última se salía de la pantalla; así
     caben siempre, sin depender de lo que midan. El número delante evita
     tener que contarlas al copiarlas. */
  const int COL[2] = { SX(6), SX(122) };
  int idx = 0, shown = 0, start = 0;
  while(start < (int)mn.length() && shown < 12){
    const int sp = mn.indexOf(' ', start);
    const String w = (sp < 0) ? mn.substring(start) : mn.substring(start, sp);
    start = (sp < 0) ? mn.length() : sp + 1;
    if(idx++ < from) continue;

    const int x = COL[shown / 6], y = SY(30) + (shown % 6) * SY(17);
    char num[4]; snprintf(num, sizeof(num), "%d", from + shown + 1);
    tiny(num, x + SX(14), y + SY(4), UI_DIM,  'R', 0);
    tiny(w,   x + SX(18), y,         UI_TEXT, 'L', 0, UI_BIG_BODY);
    shown++;
  }
}

void seedAddress(const String &addr, uint8_t step, uint8_t total){
  head("FIRST ADDRESS", step, total);
  bodyLine("m/84'/0'/0'/0/0", SY(30), UI_ACCENT);
  int y = SY(48);
  for(int i=0; i<(int)addr.length(); i += UI_BIG_CPL){
    bigLine(addr.substring(i, min((int)addr.length(), i + UI_BIG_CPL)), y, UI_TEXT);
    y += UI_BIG_LH;
  }
}

void seedZpub(const String &zpub, uint8_t step, uint8_t total){
  head("ACCOUNT ZPUB", step, total);

  /* Un zpub son 111 caracteres de base58: no se copia a mano, se escanea
     para montar el monedero de sólo lectura. Al lado, principio y final
     para poder identificarlo de un vistazo. */
  tiny(zpub.substring(0, 8),                    UI_M, SY(32), UI_TEXT, 'L', 0, UI_BIG_BODY);
  tiny("..." + zpub.substring(zpub.length()-5), UI_M, SY(54), UI_TEXT, 'L', 0, UI_BIG_BODY);
  tiny("SCAN TO IMPORT",                        UI_M, SY(84), UI_DIM,  'L', 1);
  tiny("WATCH-ONLY",                            UI_M, SY(96), UI_DIM,  'L', 1);

  const int version = 6, px = 2;
  QRCode qr;
  uint8_t buf[qrcode_getBufferSize(version)];
  if(qrcode_initText(&qr, buf, version, 0, zpub.c_str()) < 0) return;
  /* Centrado en el hueco que queda bajo la cabecera, no pegado a ella */
  const int x0 = UI_W - qr.size*px - SX(8);
  const int y0 = SY(22) + (UI_H - SY(22) - qr.size*px) / 2;
  for(uint8_t y=0; y<qr.size; y++)
    for(uint8_t x=0; x<qr.size; x++)
      tft.fillRect(x0 + x*px, y0 + y*px, px, px,
                   qrcode_getModule(&qr, x, y) ? UI_QR_LIGHT : UI_BG);
}

void seedEntropy(const String &hex, uint8_t step, uint8_t total){
  head("ENTROPY (HEX)", step, total);
  /* A tamaño 1 y todo seguido no había quien lo leyera. Ocho bytes por fila,
     a doble tamaño y alternando el color: se puede cantar en voz alta. */
  const int bytes = hex.length() / 2;
  for(int i=0; i<bytes; i++)
    tiny(hex.substring(i*2, i*2+2), SX(8) + (i % 8) * SX(29), SY(32) + (i / 8) * SY(26),
         (i % 2) ? UI_TEXT : UI_ACCENT, 'L', 0, UI_BIG_BODY);
  if(bytes <= 16) tiny("CHECK IT OFFLINE", UI_M, SY(98), UI_DIM, 'L', 1);
}

void seedQr(const String &data){
  const int version = 11, px = 2;
  QRCode qrcode;
  uint8_t buf[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, buf, version, 0, data.c_str());

  tft.fillScreen(UI_BG);
  tiny("EXPORT", UI_M, SY(10), UI_ACCENT, 'L', 1);
  tiny("SCAN WITH AN",  UI_M, SY(34), UI_DIM, 'L', 0);
  tiny("OFFLINE WALLET",UI_M, SY(44), UI_DIM, 'L', 0);
  tiny("NEVER A PHONE", UI_M, SY(62), UI_ACCENT, 'L', 0);

  /* Posición calculada en vez del desplazamiento fijo de la v1, que dejaba
     el código a 2px del borde derecho: un QR necesita zona de silencio para
     que una cámara lo enganche. */
  const int qw = qrcode.size * px;
  const int qx = UI_W - qw - SX(6);
  const int qy = (UI_H - qw) / 2;
  for(uint8_t y=0; y<qrcode.size; y++)
    for(uint8_t x=0; x<qrcode.size; x++)
      tft.fillRect(qx + x*px, qy + y*px, px, px,
                   qrcode_getModule(&qrcode, x, y) ? UI_QR_LIGHT : UI_BG);
}

void seedExit(void){
  tft.fillScreen(UI_BG);

  /* Cabecera distinta a propósito: una regla con la etiqueta incrustada.
     Esta pantalla no es una más de la semilla, y debe notarse. */
  tft.drawFastHLine(UI_M, SY(22), UI_W - 2*UI_M, UI_TRACK);
  tft.fillRect(UI_M + SX(6), SY(16), 46, 13, UI_BG);   //hueco del tamaño del texto
  tiny("EXIT", UI_M + SX(12), SY(18), UI_ACCENT, 'L', 2);

  /* Lo único que el usuario tiene que hacer, a doble tamaño */
  tiny("WRITE IT DOWN", SX(6), SY(36), UI_TEXT, 'L', 0, UI_BIG_BODY);

  /* Y por qué: lo que este aparato no ha hecho con tu semilla */
  tiny("GENERATED OFFLINE",      UI_M, SY(62), UI_DIM, 'L', 1);
  tiny("NEVER WRITTEN TO FLASH", UI_M, SY(74), UI_DIM, 'L', 1);
  tiny("IT ONLY LIVES IN RAM",   UI_M, SY(86), UI_DIM, 'L', 1);

  /* El recuadro va abajo a la derecha, a la altura del botón OK físico,
     y la punta apunta hacia él. */
  tiny("TO WIPE & LEAVE", UI_M, SY(112), UI_DIM, 'L', 1);
  const int bx = SX(126), by = SY(100), bw = SX(106), bh = SY(30);
  tft.drawRoundRect(bx,   by,   bw,   bh,   6, UI_ACCENT);
  tft.drawRoundRect(bx+1, by+1, bw-2, bh-2, 5, UI_ACCENT);
  tiny("HOLD OK", bx + bw/2, by + (bh-8*UI_BIG_BODY)/2, UI_ACCENT, 'C', 1, UI_BIG_BODY);
  /* la punta señala al botón OK físico, en el borde derecho */
  tft.fillTriangle(UI_W-SX(6), by+SY(9), UI_W-SX(6), by+SY(21), UI_W-SX(1), by+SY(15), UI_ACCENT);
}

}  // namespace ui
