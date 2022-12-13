#include "Lib/images.h"
#include "Lib/Free_Fonts.h"


/***************🍃 EEPROM ***************/
void EEPROMsetup(void);

/**************🍃 TFT functions ********************/
void Init_TFT(void);
void clrWorkArea(void);

/**************🍃 BUTTON ********************/
enum { None, SingleClick, DoubleClick, LongClick, ForcedClick };

class sButton
   {  private:
         byte antState;           //Button previous pin state
         byte pin;                //Button physical pin number
         unsigned long msecLst;   //Button last time was pressed
 
      public:
         uint8_t clickState;      //Button click [None, SingleClick, DoubleClick, LongClick]
         sButton(byte bPin);      // Constructor
         void init(void);         // Init button pin
         void check(void) ;        // Declaracion de funcion externa
         int click(void) ;         // Declaracion de funcion externa
         void forceClick(void);    //Generate a click
   } ;



