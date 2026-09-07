#include <Arduino.h>

/**************🍃 TFT functions ********************/
void Init_TFT(void);

/**************🍃 BUTTON ********************/
enum { None, SingleClick, LongClick, ForcedClick, HoldClick };

/* Dos umbrales sobre la misma pulsacion. LongClick es el que ya suprime el
   click de la suelta, asi que cualquier aviso en pantalla se arma a partir
   de el: soltar a medias no puede colar una pulsacion que el usuario ya no
   queria. HoldClick llega despues y es el que confirma la accion larga. */
#define BTN_LONG_MS   1200
#define BTN_HOLD_MS   3000

class sButton
   {  private:
         byte antState;           //Button previous pin state
         byte pin;                //Button physical pin number
         bool longFired;          //LongClick already reported for this press
         bool holdFired;          //HoldClick already reported for this press
         unsigned long msecLst;   //When the current press started
         unsigned long msecEdge;  //Last accepted edge, for debouncing
 
      public:
         uint8_t clickState;      //Button click [None, SingleClick, LongClick, HoldClick]
         sButton(byte bPin);      // Constructor
         void init(void);         // Init button pin
         void check(void) ;        // Declaracion de funcion externa
         int click(void) ;         // Declaracion de funcion externa
         void forceClick(void);    //Generate a click
         unsigned long heldMs(void); //How long the button has been held, 0 if released
   } ;



