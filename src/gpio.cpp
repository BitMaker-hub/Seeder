#include "GlobalVARS.h"
#include "gpio.h"
#include "Wire.h"
#include "workflow.h"
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip

/**********************🍃 GLOBAL Vars *******************************/
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
extern sWallet myWallet;
extern sButton btnMove;
extern sButton btnSelect;

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
  
  myWallet.entropySrc = coinEntropy;
  drawInitMenu();
  
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
    const unsigned long ButDebounce  = 25;
    const unsigned long ButLongClick = 1200;
          unsigned long msec = millis();

    if(clickState == ForcedClick){ clickState = SingleClick; return; }
    clickState = None;

    byte but = digitalRead(pin);

    if(but != antState){
        if(msec - msecEdge < ButDebounce) return;   // bounce, ignore the edge
        msecEdge = msec;
        antState = but;

        if(but == LOW){                             // pressed
            msecLst = msec ? msec : 1;
            longFired = false;
        }else{                                      // released
            //Click reported on release, so holding the button never turns two
            //presses into one and no input is ever swallowed
            if(msecLst && !longFired){ clickState = SingleClick; DBGLN("SingleClick"); }
            msecLst = 0;
        }
        return;
    }

    //Still held: LongClick fires once and suppresses the click on release
    if(but == LOW && msecLst && !longFired && (msec - msecLst) > ButLongClick){
        longFired = true;
        clickState = LongClick;
        DBGLN("LongClick");
    }
}





