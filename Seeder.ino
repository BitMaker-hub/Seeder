#include "gpio.h"
#include "btc.h"
#include "workflow.h"
#include "GlobalVARS.h"

sWallet myWallet;
sButton btnMove(PIN_MOVE);
sButton btnSelect(PIN_SELECT);
  
void setup() {

#if SEEDER_DEBUG
  Serial.begin(SERIAL_BAUD);                  // UART only exists in debug builds
#endif
  Init_TFT();                                   // Init TFT wallet
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
      case STATE_DICESEED:    doDiceSeed(); break;
    }
    delay(10);
  }
}
