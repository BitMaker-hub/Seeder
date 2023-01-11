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
  myWallet.State = STATE_INITMENU;
}


void loop() {
  
  while(true){
    /***** Check button state ******/
    btnMove.check();
    btnSelect.check();

    /***** Print menu options ***********/
    switch(myWallet.State){
      case STATE_INITMENU:      doInitMenu(); break;
      case STATE_WORDS:        doMenuWords(); break;
      case STATE_SEED:        doShowSeed(); break;
      case STATE_COINSEED:    doCoinSeed(); break;
    }
    delay(10);
  }
}
