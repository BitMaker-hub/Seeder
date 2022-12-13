#include <Arduino.h>
#include <EEPROM.h>
#include "qrcoded.h"
#include "gpio.h"
#include "btc.h"
#include "GlobalVARS.h"
#include "workflow.h"
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

/**********************🍃 GLOBAL Vars *******************************/
extern TFT_eSPI tft;  // Invoke library, pins defined in User_Setup.h
extern sWallet myWallet;
extern sButton btnMove;
extern sButton btnSelect;
int menuSeed = SHOW_SEED1;

void displayMnemonic(int initWord);

/**************🍃 WORKSATES *********************
  🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃
 ***********************************************/

/**************🍃 INITIAL MENU *****************/
void doInitMenu(){
  if(btnMove.click()) {
    if(myWallet.preState == rgnSeed){
      //Coin Seed
      myWallet.preState = coinSeed;
      tft.drawXBitmap(49, 49, RGN_icon, RGNiconWidth, RGNiconHeight, SEEDER_GREY, TFT_BLACK);
      tft.drawXBitmap(135, 57, iconMoneda, iconMonedaWidth, iconMonedaHeight, SEEDER_GREEN, TFT_BLACK);
      tft.drawXBitmap(65, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, TFT_BLACK, TFT_BLACK); //oldTriangle black
      tft.drawXBitmap(155, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, SEEDER_GREEN, TFT_BLACK); //newTriangle green      
    }else{
      //RGN Seed
      myWallet.preState = rgnSeed;
      tft.drawXBitmap(49, 49, RGN_icon, RGNiconWidth, RGNiconHeight, SEEDER_GREEN , TFT_BLACK);
      tft.drawXBitmap(135, 57, iconMoneda, iconMonedaWidth, iconMonedaHeight, SEEDER_GREY, TFT_BLACK);
      tft.drawXBitmap(155, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, TFT_BLACK, TFT_BLACK); //oldTriangle black
      tft.drawXBitmap(65, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, SEEDER_GREEN, TFT_BLACK); //newTriangle green
    }
  }
  if(btnSelect.click() && (myWallet.preState == rgnSeed)) {
    clrWorkArea(); //Borramos zona donde había los otros iconos
    myWallet.nWords = 24;
    myWallet.State = STATE_MENU_RGN;
    btnMove.forceClick(); //Forzamos click para que pinte el menu
  }
}

/**************🍃 SELECT WORDS *****************/
void doMenuRGN(){
  if(btnMove.click()) {
    if(myWallet.nWords == 12){
      myWallet.nWords = 24;
      tft.drawXBitmap(41, 57, iconWords12, iconWords12Width, iconWords12Height, SEEDER_GREY, TFT_BLACK);
      tft.drawXBitmap(135, 57, iconWords24, iconWords12Width, iconWords12Height, SEEDER_GREEN, TFT_BLACK);
      tft.drawXBitmap(65, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, TFT_BLACK, TFT_BLACK); //oldTriangle black
      tft.drawXBitmap(155, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, SEEDER_GREEN, TFT_BLACK); //newTriangle green      
    }else{
      myWallet.nWords = 12;
      tft.drawXBitmap(41, 57, iconWords12, iconWords12Width, iconWords12Height, SEEDER_GREEN, TFT_BLACK);
      tft.drawXBitmap(135, 57, iconWords24, iconWords12Width, iconWords12Height, SEEDER_GREY, TFT_BLACK);
      tft.drawXBitmap(155, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, TFT_BLACK, TFT_BLACK); //oldTriangle black
      tft.drawXBitmap(65, HEADER_HEIGHT, iconTriangle, iconTriangleWidth, iconTriangleHeight, SEEDER_GREEN, TFT_BLACK); //newTriangle green
    }
  }
  if(btnSelect.click()){
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(FF1);
    tft.setCursor (40, D2_ALTO+10);
    tft.setTextColor(SEEDER_GREY);
    tft.println("Generating seed");
    createSeed(myWallet.nWords); // ---> Create Seed

    displayMnemonic(0);

    myWallet.State = STATE_SEED;
    menuSeed = SHOW_SEED1;
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

  tft.fillRect(100,0,D_ANCHO,D_ALTO, TFT_BLACK);
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
#define MAX_MENUSEED "4"

void displayHeader(String headerText){
  tft.fillRect(0,0,D_ANCHO,18, SEEDER_GREEN);
  tft.setFreeFont(FMB9);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(0);
  tft.setCursor (4, 14);
  tft.print(headerText); 
  if(headerText.length()<19)
    for(int i=0; i<(18-headerText.length()); i++) tft.print(" ");
  String step = String(menuSeed) + "/" + MAX_MENUSEED;
  tft.println(step);
}

void displayMnemonic(int initWord){

  tft.fillScreen(TFT_BLACK);
  if(myWallet.nWords==12) displayHeader("Mnemonic words");
  else if(initWord==0)    displayHeader("Mnemonic PART1/2");
  else if(initWord==12)   displayHeader("Mnemonic PART2/2");

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
  displayHeader("Seed Data");

  tft.setCursor (0, 38);
  tft.setFreeFont(FM9);
  tft.setTextWrap(true);
  tft.setTextColor(SEEDER_GREEN);tft.print("Path: ");
  tft.setTextColor(SEEDER_GREY);tft.println("m/84'/0'/0'/");
  tft.setTextColor(SEEDER_GREEN);tft.println("First address: ");
  tft.setTextColor(SEEDER_GREY);tft.println(myWallet.firstAddress);

}

void displaySeedData2(void){

  tft.fillScreen(TFT_BLACK);
  displayHeader("Seed Data");

  tft.setCursor (0, 38);
  tft.setFreeFont(FM9);
  tft.setTextWrap(true);
  tft.setTextColor(SEEDER_GREEN);tft.print("zpub: ");
  tft.setTextColor(SEEDER_GREY);tft.println(myWallet.xpub);

}

void displaySeedQR(void){

  tft.fillScreen(TFT_BLACK);
  displayHeader("Export");

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
      case SHOW_EXPORTQR:   displaySeedQR(); break;
    }
  }
  if(btnSelect.click()){
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(0, 0, menuHeaderWidth, menuHeaderHeight, menu_header);
    myWallet.State = STATE_MENU;
    btnMove.forceClick(); //Forzamos click para que pinte el menu
  }
}
