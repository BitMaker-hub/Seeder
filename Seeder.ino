#include "gpio.h"
#include "btc.h"
#include "workflow.h"
#include "GlobalVARS.h"

sWallet myWallet;
sButton btnMove(PIN_MOVE);
sButton btnSelect(PIN_SELECT);
  
void setup() {

  Serial.begin(SERIAL_BAUD);                    // Init Serial port
  EEPROMsetup();                                // Init EEPROMdata
  Init_TFT();                                   // Init TFT wallet
  initWallet();                                 // Init wallet data
  myWallet.State = STATE_MENU;
}


void loop() {
  
  while(true){
    /***** Check button state ******/
    btnMove.check();
    btnSelect.check();

    /***** Print menu options ***********/
    switch(myWallet.State){
      case STATE_MENU:      doInitMenu(); break;
      case STATE_MENU_RGN:  doMenuRGN(); break;
      case STATE_SEED:      doShowSeed(); break;
    }
    delay(10);
  }
}
