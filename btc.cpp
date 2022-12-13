#include <Arduino.h>
#include <EEPROM.h>
#include <bootloader_random.h>
#include "btc.h"
#include "Bitcoin.h"
#include "GlobalVARS.h"

extern sWallet myWallet;

String password="";

/**************🍃 INIT WALLET *******************/
void initWallet(void){

  if(myWallet.mnemonic.length() > 0){
    HDPrivateKey hd(myWallet.mnemonic, password);
    HDPrivateKey account = hd.derive("m/84'/0'/0'/");
    
    myWallet.xpub= account.xpub();
    myWallet.firstAddress= account.address();
  }

}

void random_buffer_esp(uint8_t *buf, size_t len)
{
  uint32_t r = 0;
  for (size_t i = 0; i < len; i++) {
    if (i % 4 == 0) {
      r = esp_random();
    }
    buf[i] = (r >> ((i % 4) * 8)) & 0xFF;
  }
}

void createSeed(int nWords){

  // entropy bytes to mnemonic
  bootloader_random_enable();
  delay(1000);
  
  uint8_t arr[512] = {0};
  random_buffer_esp(arr, 512);
  String mn = generateMnemonic(nWords, arr, sizeof(arr));
  Serial.println(mn);

  // Extract xpub from primary address
  HDPrivateKey hd(mn, password);
  HDPrivateKey account = hd.derive("m/84'/0'/0'/");
    
  myWallet.xpub= account.xpub();
  myWallet.mnemonic = mn;
  myWallet.firstAddress= account.address();

  bootloader_random_disable();
}


