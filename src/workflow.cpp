#include <Arduino.h>
#include "gpio.h"
#include "btc.h"
#include "GlobalVARS.h"
#include "workflow.h"
#include "ui/ui.h"

/**********************🍃 GLOBAL Vars *******************************/
extern sWallet myWallet;
extern sButton btnMove;
extern sButton btnSelect;

int menuSeed = SHOW_SEED1;
uint8_t entropy[32];               //Entropía cruda que aporta el usuario
char diceRolls[DICE_MAX_ROLLS+1];  //Dígitos ASCII de las tiradas
uint8_t diceValue = 1;             //Valor que se está marcando ahora
uint8_t rollHist[3] = {0,0,0};     //Tres últimas tiradas, la más reciente al final

/**************🍃 HELPERS *****************/
uint8_t entropyBytes(void){ return myWallet.nWords * 4 / 3; }
uint8_t diceRollsNeeded(void){ return (myWallet.nWords == 12) ? DICE_ROLLS_12 : DICE_ROLLS_24; }

//Con 12 palabras no hay segunda página de mnemónico, así que las siguientes
//bajan un número en vez de saltárselo
static uint8_t seedSteps(void){ return (myWallet.nWords == 12) ? SHOW_EXPORTQR-1 : SHOW_EXPORTQR; }
static uint8_t seedStep(void) { return (myWallet.nWords == 12 && menuSeed > SHOW_SEED2) ? menuSeed-1 : menuSeed; }

//Asignar "" sólo pone la longitud a cero: el texto se queda en el buffer
static void wipeString(String &s){
  for(unsigned int i = 0; i < s.length(); i++) s[i] = '\0';
  s = "";
}

//Todo lo que permitiría reconstruir la semilla. uBitcoin ya hace memzero de
//la clave privada y del chain code al destruirlas.
void wipeSeed(void){
  wipeString(myWallet.mnemonic);
  wipeString(myWallet.entropyHex);
  wipeString(myWallet.xpub);
  wipeString(myWallet.firstAddress);
  memset(entropy,   0, sizeof(entropy));
  memset(diceRolls, 0, sizeof(diceRolls));
  myWallet.nBCoinEntropy = 0;
  myWallet.nRolls = 0;
  diceValue = 1; memset(rollHist, 0, sizeof(rollHist));
}

//Siempre antes de capturar: entropy[] es global y si no arrastraría bits de
//una semilla anterior, y el resultado dejaría de ser verificable.
void resetEntropy(void){
  memset(entropy, 0, sizeof(entropy));
  myWallet.nBCoinEntropy = 0;
}

void drawInitMenu(void) { ui::menu(myWallet.entropySrc == diceEntropy); }
void drawWordsMenu(void){ ui::words(myWallet.nWords); }

static void drawSeedPage(void){
  const uint8_t st = seedStep(), tot = seedSteps();
  switch(menuSeed){
    case SHOW_SEED1:    ui::mnemonic(myWallet.mnemonic, myWallet.nWords, 0,  st, tot); break;
    case SHOW_SEED2:    ui::mnemonic(myWallet.mnemonic, myWallet.nWords, 12, st, tot); break;
    case SHOW_DATA1:    ui::seedAddress(myWallet.firstAddress, st, tot); break;
    case SHOW_DATA2:    ui::seedZpub(myWallet.xpub, st, tot); break;
    case SHOW_ENTROPY:  ui::seedEntropy(myWallet.entropyHex, st, tot); break;
    case SHOW_EXPORTQR: ui::seedQr(myWallet.mnemonic); break;
    case SHOW_EXIT:     ui::seedExit(); break;
  }
}

static void generateSeed(void){
  ui::generating();
  createSeed(myWallet.nWords, entropy);
  myWallet.State = STATE_SEED;
  menuSeed = SHOW_SEED1;
  drawSeedPage();
}

/**************🍃 WORKSATES *********************
  🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃🍃
 ***********************************************/

/**************🍃 INITIAL MENU *****************/
void doInitMenu(void){
  if(btnMove.click()){
    myWallet.entropySrc = (myWallet.entropySrc == coinEntropy) ? diceEntropy : coinEntropy;
    drawInitMenu();
  }
  if(btnSelect.click()){
    myWallet.nWords = 12;
    myWallet.State  = STATE_WORDS;
    drawWordsMenu();
  }
}

/**************🍃 SELECT WORDS *****************/
void doMenuWords(void){
  if(btnMove.click()){
    myWallet.nWords = (myWallet.nWords == 12) ? 24 : 12;
    drawWordsMenu();
  }
  if(btnSelect.click()){
    resetEntropy();
    if(myWallet.entropySrc == coinEntropy){
      myWallet.State = STATE_COINSEED;
      ui::coinEnter(entropyBytes()*8);
      ui::coinUpdate(0, entropyBytes()*8, entropy);
    }else{
      myWallet.State  = STATE_DICESEED;
      myWallet.nRolls = 0;
      diceValue = 1;
      memset(diceRolls, 0, sizeof(diceRolls));
      memset(rollHist,  0, sizeof(rollHist));
      ui::diceEnter(diceRollsNeeded());
      ui::diceUpdate(0, diceRollsNeeded(), diceValue, rollHist);
    }
  }
}

/**************🍃 COIN ENTROPY *****************/
void doCoinSeed(void){
  if(!(btnMove.click() || btnSelect.click())) return;

  const uint16_t maxBits = entropyBytes() * 8;
  const uint8_t  coin    = btnMove.click() ? 1 : 0;

  //OR, nunca sumar: sumar arrastraría acarreos a los bits vecinos
  entropy[myWallet.nBCoinEntropy/8] |= (coin << (7 - myWallet.nBCoinEntropy%8));
  myWallet.nBCoinEntropy++;

  ui::coinUpdate(myWallet.nBCoinEntropy, maxBits, entropy);

  //Comprobado DESPUÉS de incrementar: llegar a 0 en pantalla significa
  //"ya está", no "una tirada más"
  if(myWallet.nBCoinEntropy >= maxBits) generateSeed();
}

/**************🍃 DICE ENTROPY *****************/
void doDiceSeed(void){
  const uint8_t total = diceRollsNeeded();

  if(btnMove.click()){
    diceValue = (diceValue % 6) + 1;   //1..6, da la vuelta
    ui::diceUpdate(myWallet.nRolls, total, diceValue, rollHist);
  }

  if(btnSelect.click()){
    diceRolls[myWallet.nRolls++] = '0' + diceValue;
    rollHist[0] = rollHist[1]; rollHist[1] = rollHist[2]; rollHist[2] = diceValue;
    diceValue = 1;

    if(myWallet.nRolls >= total){
      entropyFromDice(diceRolls, myWallet.nRolls, entropy);
      generateSeed();
      return;
    }
    ui::diceUpdate(myWallet.nRolls, total, diceValue, rollHist);
  }
}

/**************🍃 SHOW SEED *****************/
//MOVE avanza, OK retrocede, ambos dan la vuelta. Salir es deliberado: sólo
//desde la última página y sólo manteniendo pulsado, para que nadie se salga
//de la semilla por accidente antes de haberla apuntado.
static void seedPageStep(int dir){
  do{
    menuSeed += dir;
    if(menuSeed > SHOW_EXIT)  menuSeed = SHOW_SEED1;
    if(menuSeed < SHOW_SEED1) menuSeed = SHOW_EXIT;
  }while(myWallet.nWords == 12 && menuSeed == SHOW_SEED2);
  drawSeedPage();
}

void doShowSeed(void){
  if(btnMove.click()) seedPageStep(+1);

  const int sel = btnSelect.click();
  if(sel == LongClick && menuSeed == SHOW_EXIT){
    wipeSeed();
    myWallet.State = STATE_INITMENU;
    drawInitMenu();
  }
  else if(sel == SingleClick) seedPageStep(-1);
}
