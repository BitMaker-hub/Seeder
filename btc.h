#include <Arduino.h>

/**********🍃 BTC FUNCTIONS ****************/
void initWallet(void);
void createSeed(int nWords);
void createSeed(int nWords, uint8_t entropy[]);
void saveMnemonic(String mnemonic);

