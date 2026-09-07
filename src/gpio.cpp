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

#ifdef PIN_POWER_ON
  /* La T-Display-S3 alimenta sus periféricos desde este pin. Si no se pone
     alto ANTES de arrancar el panel, la pantalla no enciende y la placa
     parece muerta: es el fallo clásico de esa placa. */
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  delay(20);
#endif

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);   // orden de bytes al volcar imágenes

  ui::splash();

  myWallet.entropySrc = coinEntropy;
  drawInitMenu();
}

/*****************🍃 BUTTON DETECTION *********************/

sButton::sButton(byte bPin){        //Constructor
    pin = bPin; pinMode(pin, INPUT);
    antState = HIGH; longFired = holdFired = false;
    msecLst = msecEdge = 0; clickState = None;
}
void sButton::init(void){  pinMode(pin, INPUT); }     // Init pushbutton pin
int sButton::click(void){  return clickState; }
void sButton::forceClick(void){ clickState = ForcedClick;} //Generates one click loop

//Cuanto lleva pulsado ahora mismo. Cero significa suelto, asi que sirve
//igual para dibujar el progreso de un mantenido y para saber si sigue ahi.
unsigned long sButton::heldMs(void){
    if(!msecLst) return 0;
    const unsigned long h = millis() - msecLst;
    return h ? h : 1;               //0 esta reservado para "suelto"
}

void sButton::check(void)
{
    const unsigned long ButDebounce  = 25;
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
            longFired = holdFired = false;
        }else{                                      // released
            //Click reported on release, so holding the button never turns two
            //presses into one and no input is ever swallowed
            if(msecLst && !longFired){ clickState = SingleClick; DBGLN("SingleClick"); }
            msecLst = 0;
        }
        return;
    }

    //Still held: LongClick fires once and suppresses the click on release.
    //HoldClick comes later on that same press, so a screen armed by LongClick
    //has something to fill up to and letting go still backs out of it.
    if(but != LOW || !msecLst) return;
    const unsigned long held = msec - msecLst;

    if(!longFired && held > BTN_LONG_MS){
        longFired = true;
        clickState = LongClick;
        DBGLN("LongClick");
    }else if(longFired && !holdFired && held > BTN_HOLD_MS){
        holdFired = true;
        clickState = HoldClick;
        DBGLN("HoldClick");
    }
}





