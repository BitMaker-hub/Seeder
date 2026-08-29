#include <Arduino.h>
#include "qrcoded.h"
#include "gpio.h"
#include "btc.h"
#include "GlobalVARS.h"
#include "workflow.h"
#include <TFT_eSPI.h> // Graphics and font library for TTGO T-DISPLAY driver chip

/**********************🍃 GLOBAL Vars *******************************/
extern TFT_eSPI tft;  // Invoke library, pins defined in User_Setup.h
extern sWallet myWallet;
extern sButton btnMove;
extern sButton btnSelect;
int menuSeed = SHOW_SEED1;
int CursorX, CursorY;
uint8_t entropy[32];   //Raw entropy captured from the user (128 or 256 bits)
char diceRolls[DICE_MAX_ROLLS+1];  //ASCII digits of the rolls, hashed to get the entropy
uint8_t diceValue = 1;             //Value the user is currently dialling in

void displayMnemonic(int initWord);
void displayHeader(String headerText, bool printStep);
void displayGenerateSeed(void);
void displayGenerateSeed(uint8_t * entropy);
void displayGenerateSeed(bool isRGN, uint8_t * entropy);
void printEntropyBytes(void);
void drawDiceCapture(void);

/**************🍃 ENTROPY HELPERS *****************/
//Bytes of entropy a mnemonic of nWords needs: 16 for 12 words, 32 for 24
uint8_t entropyBytes(void){ return myWallet.nWords * 4 / 3; }

//Rolls needed to cover the entropy: 50 -> 129.2 bits, 99 -> 255.9 bits
uint8_t diceRollsNeeded(void){ return (myWallet.nWords == 12) ? DICE_ROLLS_12 : DICE_ROLLS_24; }

//Step counter in the header. With 12 words there is no page 2 of the
//mnemonic, so the remaining pages shift down by one instead of skipping a number.
int seedSteps(void){ return (myWallet.nWords == 12) ? SHOW_EXPORTQR-1 : SHOW_EXPORTQR; }
int seedStep(void) { return (myWallet.nWords == 12 && menuSeed > SHOW_SEED2) ? menuSeed-1 : menuSeed; }

//Always call before capturing entropy: entropy[] is global and would otherwise
//carry bits over from a previous seed, making the result impossible to verify.
void resetEntropy(void){
  memset(entropy, 0, sizeof(entropy));
  myWallet.nBCoinEntropy = 0;
}

/**************🍃 WORKSATES *********************
  🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃
 ***********************************************/

/**************🍃 INITIAL MENU *****************/

//No XBM for the die: drawn with primitives so it can be tinted like the icons
void drawDice(int x, int y, uint16_t color){
  const int s = 58, r = 10, pip = 4;
  tft.fillRoundRect(x, y, s, s, r, TFT_BLACK);
  tft.drawRoundRect(x, y, s, s, r, color);
  tft.drawRoundRect(x+1, y+1, s-2, s-2, r-1, color);
  const int a = x + s/4, b = x + s/2, c = x + (3*s)/4;
  const int d = y + s/4, e = y + s/2, f = y + (3*s)/4;
  const int pips[5][2] = {{a,d},{c,d},{b,e},{a,f},{c,f}};   //face 5
  for(int i=0; i<5; i++) tft.fillCircle(pips[i][0], pips[i][1], pip, color);
}

void drawInitMenu(void){
  bool coin = (myWallet.entropySrc == coinEntropy);
  drawDice(41, 49, coin ? SEEDER_GREY : SEEDER_GREEN);
  tft.drawXBitmap(135, 57, iconMoneda, iconMonedaWidth, iconMonedaHeight,
                  coin ? SEEDER_GREEN : SEEDER_GREY, TFT_BLACK);
  tft.drawXBitmap(65,  HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight,
                  coin ? TFT_BLACK : SEEDER_GREEN, TFT_BLACK);
  tft.drawXBitmap(155, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight,
                  coin ? SEEDER_GREEN : TFT_BLACK, TFT_BLACK);
}

void drawWordsMenu(void){
  bool w12 = (myWallet.nWords == 12);
  tft.drawXBitmap(41,  57, iconWords12, iconWords12Width, iconWords12Height,
                  w12 ? SEEDER_GREEN : SEEDER_GREY, TFT_BLACK);
  tft.drawXBitmap(135, 57, iconWords24, iconWords24Width, iconWords24Height,
                  w12 ? SEEDER_GREY : SEEDER_GREEN, TFT_BLACK);
  tft.drawXBitmap(65,  HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight,
                  w12 ? SEEDER_GREEN : TFT_BLACK, TFT_BLACK);
  tft.drawXBitmap(155, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight,
                  w12 ? TFT_BLACK : SEEDER_GREEN, TFT_BLACK);
}

void doInitMenu(){
  if(btnMove.click()){
    myWallet.entropySrc = (myWallet.entropySrc == coinEntropy) ? diceEntropy : coinEntropy;
    drawInitMenu();
  }
  if(btnSelect.click()){
    clrWorkArea();
    myWallet.nWords = 12;
    myWallet.State = STATE_WORDS;
    drawWordsMenu();
  }
}

/**************🍃 SELECT WORDS *****************/
void startCoinCapture(void){
  myWallet.State = STATE_COINSEED;
  resetEntropy();
  displayHeader("Flip coin - " + String(entropyBytes()*8), false);
  tft.setCursor(0, 38);
  tft.setFreeFont(FM9);
  tft.setTextWrap(true);
  tft.setTextColor(SEEDER_GREEN);
  tft.println("SEL[Heads] - OK[Tails]");
}

void startDiceCapture(void){
  myWallet.State = STATE_DICESEED;
  resetEntropy();
  myWallet.nRolls = 0;
  diceValue = 1;
  memset(diceRolls, 0, sizeof(diceRolls));
  tft.setCursor(0, 34);
  tft.setFreeFont(FM9);
  tft.setTextWrap(false);
  tft.setTextColor(SEEDER_GREEN);
  tft.println("SEL[1-6] - OK[accept]");
  drawDiceCapture();
}

void doMenuWords(){
  if(btnMove.click()){
    myWallet.nWords = (myWallet.nWords == 12) ? 24 : 12;
    drawWordsMenu();
  }
  if(btnSelect.click()){
    tft.fillScreen(TFT_BLACK);
    if(myWallet.entropySrc == coinEntropy) startCoinCapture();
    else                                   startDiceCapture();
  }
}

/*********🍃 UTILS DISPLAY DATA *********/
int qrVersionFromStringLength(int stringLength) {
  if (stringLength <= 17) return 1;
  if (stringLength <= 32) return 2;
  if (stringLength <= 53) return 3;
  if (stringLength <= 134) return 6;
  if (stringLength <= 367) return 11;
  return 28;
}

int squareSizeFromStringLength(int stringLength) {
  if (stringLength <= 53) return 4;
  return 3;
}

void showQRCode(String s) {
  int version = 11;//qrVersionFromStringLength(s.length());
  int px = 2;//squareSizeFromStringLength(s.length());
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, 0, s.c_str());

  tft.fillRect(107,0,D_ANCHO,D_ALTO, TFT_BLACK);
  //tft.fillScreen(TFT_BLACK);
  int Offset = 56; //Mostramos el QR a la derecha no a la izquierda

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      bool full = qrcode_getModule(&qrcode, x, y);

      int color = full ? TFT_WHITE : TFT_BLACK;
      tft.fillRect((Offset + x + 2) * px, (y + 3) * px, px, px, color);
    }
  }
}

/**************🍃 MENU SHOWING SEED *****************/
void displayGenerateSeed(){
  uint8_t trash[1];
  displayGenerateSeed(true, trash); //Call function to generate random seed
}
void displayGenerateSeed(uint8_t * entropy) { displayGenerateSeed(false, entropy);} //Call function to use entropy
void displayGenerateSeed(bool isRGN, uint8_t * entropy){
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF1);
  tft.setCursor (40, D2_ALTO+10);
  tft.setTextColor(SEEDER_GREY);
  tft.println("Generating seed");
  if(isRGN) createSeed(myWallet.nWords); // ---> Create Seed
  else createSeed(myWallet.nWords, entropy); // ---> Create Seed

  displayMnemonic(0);
  menuSeed = SHOW_SEED1;
}

void displayHeader(String headerText, bool printStep){
  tft.fillRect(0,0,D_ANCHO,18, SEEDER_GREEN);
  tft.setFreeFont(FMB9);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(0);
  tft.setCursor (4, 14);
  tft.print(headerText); 
  String step = String(seedStep()) + "/" + String(seedSteps());
  if(printStep){
    if(headerText.length()<19)
      for(int i=0; i<(18-headerText.length()); i++) tft.print(" ");
  }else step = "";
  tft.println(step);
}

void displayMnemonic(int initWord){

  tft.fillScreen(TFT_BLACK);
  if(myWallet.nWords==12) displayHeader("Mnemonic words", true);
  else if(initWord==0)    displayHeader("Mnemonic PART1/2", true);
  else if(initWord==12)   displayHeader("Mnemonic PART2/2", true);

  tft.setCursor (4, 38);
  tft.setFreeFont(FM9);
  tft.setTextWrap(true);
  tft.setTextColor(SEEDER_GREY);
  /***** Split mnemonic words to fit them on screen ****/
  char *words[myWallet.nWords]; // an array of pointers to the pieces of the above array after strtok()
  char *ptr = NULL;
  int str_len = myWallet.mnemonic.length() + 1; 
  char mnemonic_array[str_len];
  myWallet.mnemonic.toCharArray(mnemonic_array, str_len);
  ptr = strtok(mnemonic_array, " ");  // delimiter
  int index=0;
  while (ptr != NULL)
  {
     words[index] = ptr;
     index++;
     ptr = strtok(NULL, " ");
  }
  int currentLen = 0;
  int i=0;
  int countWords = 0;
  /***** Print 12 words per page ****/
  if(myWallet.nWords == 24) i = initWord;
  for(i; i<myWallet.nWords; i++){
    if((currentLen + strlen(words[i]) + 1) > 22){ tft.println(); currentLen = 0; tft.setCursor (4, tft.getCursorY());}
    tft.print(words[i]);tft.print(" ");
    currentLen = currentLen + strlen(words[i]) + 1;
    countWords = countWords + 1;
    if(countWords == 12) break;
  }
}

void displaySeedData1(void){

  tft.fillScreen(TFT_BLACK);
  displayHeader("Seed Data", true);

  tft.setCursor (0, 38);
  tft.setFreeFont(FM9);
  tft.setTextWrap(true);
  tft.setTextColor(SEEDER_GREEN);tft.print("Path: ");
  tft.setTextColor(SEEDER_GREY);tft.println("m/84'/0'/0'/0/0");
  tft.setTextColor(SEEDER_GREEN);tft.println("First address: ");
  tft.setTextColor(SEEDER_GREY);tft.println(myWallet.firstAddress);

}

void displaySeedData2(void){

  tft.fillScreen(TFT_BLACK);
  displayHeader("Seed Data", true);

  tft.setCursor (0, 38);
  tft.setFreeFont(FM9);
  tft.setTextWrap(true);
  tft.setTextColor(SEEDER_GREEN);tft.print("zpub: ");
  tft.setTextColor(SEEDER_GREY);tft.println(myWallet.xpub);

}

//The whole point of the device: you can take this hex to any offline BIP39
//tool and get the same words back, without trusting the SEEDER's maths.
void displaySeedEntropy(void){

  tft.fillScreen(TFT_BLACK);
  displayHeader("Entropy (hex)", true);

  tft.setCursor (0, 38);
  tft.setFreeFont(FM9);
  tft.setTextWrap(true);
  tft.setTextColor(SEEDER_GREY);
  tft.println(myWallet.entropyHex);
}

void displaySeedQR(void){

  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0,5,D2_ANCHO,D_ALTO-5, SEEDER_BLUE);
  tft.drawXBitmap(28, 49, iconCamera, iconCameraWidth, iconCameraHeight, TFT_BLACK, SEEDER_BLUE); //SEEDER_BLUE);

  showQRCode(myWallet.mnemonic);
}

void doShowSeed(void){
  
  if(btnMove.click()){
    menuSeed++; 
    if(menuSeed > SHOW_EXPORTQR) menuSeed = SHOW_SEED1;
    if((myWallet.nWords==12)&&(menuSeed == SHOW_SEED2)) menuSeed++;
    
    switch(menuSeed){
      case SHOW_SEED1:      displayMnemonic(0); break;
      case SHOW_SEED2:      displayMnemonic(12); break;
      case SHOW_DATA1:      displaySeedData1(); break;
      case SHOW_DATA2:      displaySeedData2(); break;
      case SHOW_ENTROPY:    displaySeedEntropy(); break;
      case SHOW_EXPORTQR:   displaySeedQR(); break;
    }
  }
  if(btnSelect.click()){
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(0, 0, menuHeaderWidth, menuHeaderHeight, menu_header);
    myWallet.State = STATE_INITMENU;
    drawInitMenu();
  }
}


/**************🍃 COIN ENTROPY *****************/

//Print every full byte of entropy captured so far, alternating colours
void printEntropyBytes(void){
  int x = tft.getCursorX();
  int y = tft.getCursorY();
  tft.setCursor(0,74);
  for(uint16_t i=0; i<myWallet.nBCoinEntropy/8; i++){
    tft.setTextColor((i%2 == 0) ? SEEDER_BLUE : SEEDER_GREEN);
    String b = String(entropy[i], HEX);
    if(b.length() == 1) b = "0" + b;
    b.toUpperCase();
    tft.print(b);
  }
  tft.setCursor(x,y);
}

void doCoinSeed(void){

  if(!(btnMove.click() || btnSelect.click())) return;

  uint16_t maxBits = entropyBytes() * 8;

  //Screen holds 22 chars per line, wipe it when a new line starts
  if(myWallet.nBCoinEntropy%22 == 0){
    tft.fillRect(0,44,D_ANCHO,15, TFT_BLACK);
    tft.setCursor(0,56);
  }
  CursorX = tft.getCursorX();
  CursorY = tft.getCursorY();

  uint8_t coin = btnMove.click() ? 1 : 0;

  //Store the bit. OR, never ADD: adding would carry into neighbouring bits
  entropy[myWallet.nBCoinEntropy/8] |= (coin << (7 - myWallet.nBCoinEntropy%8));
  myWallet.nBCoinEntropy++;

  displayHeader("Flip coin - " + String(maxBits - myWallet.nBCoinEntropy), false);
  tft.setFreeFont(FM9);
  tft.setCursor(CursorX, CursorY);
  tft.setTextColor(SEEDER_GREY);
  tft.print(coin);

  //Print entropy every 8 bits
  if(myWallet.nBCoinEntropy%8 == 0) printEntropyBytes();

  //Every bit is in -> derive the seed. Checked AFTER the increment so the
  //counter reaching 0 on screen means "done", not "one more flip please"
  if(myWallet.nBCoinEntropy >= maxBits){
    displayGenerateSeed(entropy);
    myWallet.State = STATE_SEED;
    menuSeed = SHOW_SEED1;
  }
}

/**************🍃 DICE ENTROPY *****************/

void drawDiceCapture(void){

  displayHeader("Roll dice - " + String(diceRollsNeeded() - myWallet.nRolls), false);

  //Value being dialled in, big enough to read at arm's length
  tft.fillRect(0, 48, D_ANCHO, 40, TFT_BLACK);
  tft.setFreeFont(FMB24);
  tft.setTextColor(SEEDER_GREEN);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(String(diceValue), D2_ANCHO, 68, GFXFF);
  tft.setTextDatum(TL_DATUM);

  //Tail of the roll string, so a wrong roll is spotted immediately
  tft.fillRect(0, 92, D_ANCHO, 22, TFT_BLACK);
  tft.setFreeFont(FM9);
  tft.setTextColor(SEEDER_GREY);
  tft.setCursor(0, 108);
  int from = (myWallet.nRolls > 21) ? myWallet.nRolls - 21 : 0;
  for(int i=from; i<myWallet.nRolls; i++) tft.print(diceRolls[i]);
}

void doDiceSeed(void){

  if(btnMove.click()){
    diceValue = (diceValue % 6) + 1;   //1..6, wraps
    drawDiceCapture();
  }

  if(btnSelect.click()){
    diceRolls[myWallet.nRolls++] = '0' + diceValue;

    if(myWallet.nRolls >= diceRollsNeeded()){
      entropyFromDice(diceRolls, myWallet.nRolls, entropy);
      displayGenerateSeed(entropy);
      myWallet.State = STATE_SEED;
      menuSeed = SHOW_SEED1;
      return;
    }
    diceValue = 1;
    drawDiceCapture();
  }
}
