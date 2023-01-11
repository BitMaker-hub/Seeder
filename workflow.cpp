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
void displayHeader(String headerText, bool printStep);
void displayGenerateSeed(void);
void displayGenerateSeed(uint8_t * entropy);
void displayGenerateSeed(bool isRGN, uint8_t * entropy);

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
  if(btnSelect.click()){
    clrWorkArea(); //Borramos zona donde había los otros iconos
    myWallet.nWords = 24;
    myWallet.State = STATE_WORDS;
    btnMove.forceClick(); //Forzamos click para que pinte el menu
  }
  
}

/**************🍃 SELECT WORDS *****************/
void doMenuWords(){
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
    if(myWallet.preState == rgnSeed) {
      displayGenerateSeed();
      myWallet.State = STATE_SEED;
      menuSeed = SHOW_SEED1;
    }
    else{
      tft.fillScreen(TFT_BLACK);//Borramos pantalla entera
      myWallet.State = STATE_COINSEED;
      myWallet.nBCoinEntropy = 0; //Set to 0 number of bits gotten from coin seed
      int nBits = (myWallet.nWords * 4/3) * 8;
      String header = "Flip coin - " + String(nBits);
      displayHeader(header, false); //Forzamos click para que pinte el menu
      tft.setCursor (0, 38);
      tft.setFreeFont(FM9);
      tft.setTextWrap(true);
      tft.setTextColor(SEEDER_GREEN);tft.println("SEL[Heads] - OK[Tails]");
    }
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
#define MAX_MENUSEED "4"
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
  String step = String(menuSeed) + "/" + MAX_MENUSEED;
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
  tft.setTextColor(SEEDER_GREY);tft.println("m/84'/0'/0'/");
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
      case SHOW_EXPORTQR:   displaySeedQR(); break;
    }
  }
  if(btnSelect.click()){
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(0, 0, menuHeaderWidth, menuHeaderHeight, menu_header);
    myWallet.State = STATE_INITMENU;
    btnMove.forceClick(); //Forzamos click para que pinte el menu
  }
}


/**************🍃 SELECT WORDS *****************/
int CursorX, CursorY;
uint8_t entropy[32];

void doCoinSeed(void){
  
  if(btnMove.click() || btnSelect.click()){

    //Screen can holds 22chars per line clear line
    if(myWallet.nBCoinEntropy%22 == 0){
      tft.fillRect(0,44,D_ANCHO,15, TFT_BLACK);
      tft.setCursor(0,56);
    }

    CursorX = tft.getCursorX();
    CursorY = tft.getCursorY();
    int maxBitsNeeded = (myWallet.nWords * 4/3) * 8;
    String header = "Flip coin - " + String(maxBitsNeeded - myWallet.nBCoinEntropy);
    displayHeader(header, false); //Forzamos click para que pinte el menu
    tft.setFreeFont(FM9);
    tft.setCursor (CursorX, CursorY);
    tft.setTextColor(SEEDER_GREY);
    uint8_t coin;
    if(btnMove.click()) coin = 1;
    else                coin = 0;
    
    tft.print(coin);
    uint8_t cByte = myWallet.nBCoinEntropy/8;
    entropy[cByte] = entropy[cByte] + (coin << (7- myWallet.nBCoinEntropy%8));
    Serial.print("Byte: ");Serial.print(cByte);Serial.print(" - Entropy: ");Serial.print(String(entropy[cByte],BIN)); Serial.print(" - d: ");Serial.println((coin << (7- myWallet.nBCoinEntropy%8)),BIN);
    
    //Check all entropy gotten
    if(myWallet.nBCoinEntropy == maxBitsNeeded) {
      //myWallet.nWords = 24;
      displayGenerateSeed(entropy);
      myWallet.State = STATE_SEED;
      menuSeed = SHOW_SEED1;
    }
    
    //Increase number of bits gotten from coin seed
    myWallet.nBCoinEntropy++; 

    //Print entropy every 8bits
    if(myWallet.nBCoinEntropy%8 == 0){
      CursorX = tft.getCursorX();
      CursorY = tft.getCursorY();
      tft.setCursor(0,74);
      for(int x=0; x<(myWallet.nBCoinEntropy + 1)/8; x++){
        if(x%2 == 0) tft.setTextColor(SEEDER_BLUE);
        else tft.setTextColor(SEEDER_GREEN);
        String temp = String(entropy[x], HEX);
        if(temp.length() == 1) temp = "0" + temp;
        temp.toUpperCase();
        tft.print(temp);
      }
      tft.setCursor (CursorX, CursorY);
    }
  }
}
