#include "GlobalVARS.h"
#include "gpio.h"
#include "workflow.h"
#include "ui/ui.h"
#include <TFT_eSPI.h> // Graphics and font library for ST7789 driver chip

/**********************🍃 GLOBAL Vars *******************************/
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
extern sWallet myWallet;
extern sButton btnMove;
extern sButton btnSelect;

/*****************🍃 TFT WORK *********************/

void Init_TFT(void){

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);   // orden de bytes al volcar imágenes

  ui::splash();

  myWallet.entropySrc = coinEntropy;
  drawInitMenu();
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





