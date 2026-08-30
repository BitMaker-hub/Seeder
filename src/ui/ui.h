#pragma once
#include <Arduino.h>

/**********************************
 🍃 CAPA DE PINTADO
 workflow.cpp llama a estas funciones y no toca un solo píxel. Cada una
 recibe los datos que necesita y decide cómo se ven.
**********************************/

namespace ui {

/*** Piezas reutilizables, expuestas por si hacen falta fuera ***/
void tiny(const char *s, int x, int y, uint16_t col, char datum='L', int sp=0);
void die(int x, int y, int size, uint8_t value, uint16_t col);
void caret(int cx, int y, int w, int h, uint16_t col);
void bar(int x, int y, int w, int h, float frac);

/*** Pantallas ***/
void splash(void);
void menu(bool diceSelected);
void words(uint8_t nWords);

void coinEnter(uint16_t totalBits);
void coinUpdate(uint16_t done, uint16_t totalBits, const uint8_t *entropy);

void diceEnter(uint8_t totalRolls);
void diceUpdate(uint8_t done, uint8_t totalRolls, uint8_t value, uint8_t last);

void generating(void);

void mnemonic(const String &mn, uint8_t nWords, uint8_t from, uint8_t step, uint8_t total);
void seedAddress(const String &addr, uint8_t step, uint8_t total);
void seedZpub(const String &zpub, uint8_t step, uint8_t total);
void seedEntropy(const String &hex, uint8_t step, uint8_t total);
void seedQr(const String &data);
void seedExit(void);

}  // namespace ui
