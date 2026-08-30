#include "Lib/images.h"
#include "Lib/Free_Fonts.h"


/**************🍃 TFT functions ********************/
void Init_TFT(void);
void clrWorkArea(void);

/**************🍃 BUTTON ********************/
enum { None, SingleClick, LongClick, ForcedClick };

class sButton
   {  private:
         byte antState;           //Button previous pin state
         byte pin;                //Button physical pin number
         bool longFired;          //LongClick already reported for this press
         unsigned long msecLst;   //When the current press started
         unsigned long msecEdge;  //Last accepted edge, for debouncing
 
      public:
         uint8_t clickState;      //Button click [None, SingleClick, DoubleClick, LongClick]
         sButton(byte bPin);      // Constructor
         void init(void);         // Init button pin
         void check(void) ;        // Declaracion de funcion externa
         int click(void) ;         // Declaracion de funcion externa
         void forceClick(void);    //Generate a click
   } ;



