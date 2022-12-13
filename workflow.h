#include <Arduino.h>


/**********🍃 WORKFLOW FUNCTIONS ****************/
void doInitMenu(void);
void doMenuRGN(void);
void doShowSeed(void);
  
/**********🍃 WORKFLOW MENU ****************/
#define maxMenu     5 //Number of options in menu starting from 0
static String menuList[]={"Create new Seed",
                          "Import Seed",
                          "Show Seed",
                          "Display Address",
                          "Export ZPUB",
                          "Sign Transaction"};
