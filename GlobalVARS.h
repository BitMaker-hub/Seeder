#include <stdint.h>
#include <Arduino.h>

#define byte uint8_t

/**********************************
 🍃 DEFINITIONS
**********************************/
enum { rgnSeed, coinSeed };
/**********************************
 🍃 DEBUG
 The T-Display is powered over USB, so ANYTHING printed here ends up on the
 host PC. The mnemonic and the raw entropy are never printed, at any level.
 This flag only enables harmless UI traces (button events).
**********************************/
#define SEEDER_DEBUG      0

#if SEEDER_DEBUG
  #define DBGLN(x)        Serial.println(x)
#else
  #define DBGLN(x)        do{}while(0)
#endif

/**********************************
 🍃 GENERAL
**********************************/
#define SERIAL_BAUD       115200   // baudrate debug

// The seed is NEVER written to flash: it only ever lives in RAM and a power
// cycle wipes it. That is deliberate - do not add persistence here.

/***************🍃 BUTTONS ***************/
#define PIN_MOVE          35
#define PIN_SELECT        0

/***************🍃 SCREEN ***************/
#define D_ANCHO           240     // screen width
#define D_ALTO            128     // screen height
#define D2_ANCHO          125     // half screen width
#define D2_ALTO           64     // half screen height
#define HEADER_HEIGHT     37
#define SEEDER_GREEN      0x86F3  //Green color used in seeder
#define SEEDER_GREY       0xA514
#define SEEDER_BLUE       0x0619

/********** 🍃 STATES ************************/
#define STATE_INITMENU    1
#define STATE_WORDS       2
#define STATE_SEED        3
#define STATE_COINSEED    4
/********** 🍃 SHOW SEED PARAMS ************************/
#define SHOW_SEED1        1
#define SHOW_SEED2        2
#define SHOW_DATA1        3
#define SHOW_DATA2        4
#define SHOW_EXPORTQR     5
/**********************************/

//🍃 Global vars 
typedef struct {
   uint8_t State;           //SEEDER current menu position
   uint8_t preState;        //Current menu position before selecting it
   uint8_t nWords;          //Current number of Words selected on wallet
   uint16_t nBCoinEntropy;  //Bits of entropy captured so far (24 words needs 256, does not fit in uint8_t)
   String xpub;             //Current xpub address
   String firstAddress;     //Current xpub address
   String mnemonic;         //Current words
}sWallet;


/*********************************************************************************
**                            End Of File
*********************************************************************************/


