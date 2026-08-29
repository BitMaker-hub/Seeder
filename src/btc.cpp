#include <Arduino.h>
#include "btc.h"
#include "Bitcoin.h"
#include "Hash.h"
#include "Conversion.h"
#include "GlobalVARS.h"

extern sWallet myWallet;

String password="";

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

