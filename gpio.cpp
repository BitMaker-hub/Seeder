#include "GlobalVARS.h"
#include <EEPROM.h>
#include "gpio.h"
#include "Wire.h"
#include "workflow.h"
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

/**********************🍃 GLOBAL Vars *******************************/
extern TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
extern sWallet myWallet;
extern sButton btnMove;
extern sButton btnSelect;

/***********************🍃 EEPROM DATA ******************************/

void EEPROMsetup(){
  
  EEPROM.begin(EEPROM_SIZE);
   
  /********🍃 READ MNMONIC ************/
  String mnemonic="";
  if(EEPROM.read(1) == STX){
    for(uint16_t i=2; i<400; i++){
      uint8_t nm = EEPROM.read(i);
      if(nm == ETX) break;
      mnemonic = mnemonic + char(nm);
    }
  }
  myWallet.mnemonic = mnemonic;
  Serial.print("Current mnemonic: "); Serial.println(mnemonic);
  myWallet.preState = rgnSeed;
  
}

/*****************🍃 TFT WORK *********************/

void Init_TFT(void){

  /******** INIT DISPLAY ************/
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);// Swap the colour byte order when rendering
    
  /******** PRINT INIT SCREEN *****/
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(20, 23, logoWidth, logoHeight, seeder_logo);
  
  delay(2000);

  tft.fillScreen(TFT_BLACK);
  tft.pushImage(60, 34, logouBTCWidth, logouBTCHeight, uBitcoinLogo);
  tft.pushImage(95, 110, poweredWidth, poweredHeight, powered_logo);
  
  delay(2000);

  tft.fillScreen(TFT_BLACK);
  tft.pushImage(0, 0, menuHeaderWidth, menuHeaderHeight, menu_header);
  //tft.fillRect(0,0,D2_ANCHO,HEADER_HEIGHT, SEEDER_GREEN); //Borramos Texto header
  //tft.setTextColor(TFT_BLACK);
  //tft.setTextDatum(ML_DATUM); //MIDDLE CENTER - MC_DATUM / TOP CENTER - TC_DATUM
  //tft.setTextSize(1);
  //tft.setFreeFont(FF22);
  //tft.drawString("RGN SEED", 8 , 15, GFXFF); 
  
  myWallet.preState = coinSeed;
  btnMove.forceClick(); //Forzamos click para que pinte el menu
  
}

void clrWorkArea(void){
              //x1,y1,x2,y2, color 
  tft.fillRect(0,HEADER_HEIGHT,D_ANCHO,D_ALTO, TFT_BLACK);
}
/*****************🍃 BUTTON DETECTION *********************/

sButton::sButton(byte bPin){ pin = bPin; pinMode(pin, INPUT); } //Constructor
void sButton::init(void){  pinMode(pin, INPUT); }     // Init pushbutton pin
int sButton::click(void){  return clickState; }
void sButton::forceClick(void){ clickState = ForcedClick;} //Generates one click loop

void sButton::check(void)
{
    const  unsigned long ButTimeout    = 250;
    const  unsigned long ButLongClick  = 5000;
           unsigned long msec = millis();

    byte but = digitalRead (pin);
    if (antState != but)  {
        antState = but;
        delay (10);           // **** debounce

        if (LOW == but)  {   // press
            if (msecLst)  { // 2nd press
                msecLst = 0; 
                clickState = DoubleClick;
                Serial.println("DoubleClick --> ");
                return;
            }
            else
                msecLst = 0 == msec ? 1 : msec;
        }
    }

    int elapsed = msec - msecLst;
    if (msecLst && (elapsed > ButTimeout) && (elapsed < ButLongClick))  {
        if(but != LOW) {
          msecLst = 0;
          clickState = SingleClick;
          Serial.println("SingleClick --> ");
          return;
        }
    }

    //LongClick 
    if(msecLst && (but == antState) && (but == LOW)) {
        if(elapsed > ButLongClick) {
          msecLst = 0; 
          clickState = LongClick;
          Serial.println("LongClick --> ");
          return;
        }
    }

    //ForcedClick
    if(clickState == ForcedClick){ 
      clickState = SingleClick;
      return;
    }
    clickState = None;
}





