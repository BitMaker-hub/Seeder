#include <Arduino.h>
#include <bootloader_random.h>
#include "btc.h"
#include "Bitcoin.h"
#include "Hash.h"
#include "Conversion.h"
#include "GlobalVARS.h"

extern sWallet myWallet;

String password="";

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

  // Extract account zpub and the FIRST RECEIVE address
  HDPrivateKey hd(mn, password);
  HDPrivateKey account = hd.derive("m/84'/0'/0'/");

  myWallet.xpub= account.xpub();
  myWallet.mnemonic = mn;
  // m/84'/0'/0'/0/0 - account.address() would be the account key itself,
  // which no wallet ever shows and cannot be used to cross-check the seed
  myWallet.firstAddress= account.derive("0/0").address();

  bootloader_random_disable();
}

//Dice entropy, same scheme as Coldcard: SHA-256 over the ASCII digits of the
//rolls. Nothing here comes from the device, and the user can reproduce it with
//  printf '3141...' | sha256sum
void entropyFromDice(const char * rolls, size_t nRolls, uint8_t out[32]){
  sha256((const uint8_t *)rolls, nRolls, out);
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

  //Kept so the user can check the words against the entropy they produced
  myWallet.entropyHex = toHex(entropy, len);
  myWallet.entropyHex.toUpperCase();

  // Extract account zpub and the FIRST RECEIVE address
  HDPrivateKey hd(mn, password);
  HDPrivateKey account = hd.derive("m/84'/0'/0'/");

  myWallet.xpub= account.xpub();
  myWallet.mnemonic = mn;
  // m/84'/0'/0'/0/0 - account.address() would be the account key itself,
  // which no wallet ever shows and cannot be used to cross-check the seed
  myWallet.firstAddress= account.derive("0/0").address();
}

