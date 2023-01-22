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
  
  //Test last word generation - uncoment to test your coin data
  //uint8_t data[16]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  //createSeed(12, data);
  
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

//Calculate RGN seed
void createSeed(int nWords){

  size_t len = nWords*4/3;
  if (len % 4 || len < 16 || len > 32) {
    return;
  }
  
  // enable 
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

//Get MnemonicWords from coin data and calculate last word
void createSeed(int nWords, uint8_t * entropy){

  // Using Generate Mnemonic
  delay(1000);

  size_t len = nWords*4/3;
  if (len % 4 || len < 16 || len > 32) {
    return;
  }
  String mn = mnemonicFromEntropy(entropy, len);
  Serial.print("Hex entropy: ");
  for(int i=0; i<32; i++){
   if (entropy[i] < 16) { Serial.print("0"); }
   Serial.print(entropy[i], HEX);
   Serial.print(" ");
  }
  Serial.println();
  Serial.println(mn);

  // Extract xpub from primary address
  HDPrivateKey hd(mn, password);
  HDPrivateKey account = hd.derive("m/84'/0'/0'/");
    
  myWallet.xpub= account.xpub();
  myWallet.mnemonic = mn;
  myWallet.firstAddress= account.address();
}

