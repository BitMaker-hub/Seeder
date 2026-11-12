#include "GlobalVARS.h"
#include "gpio.h"
#include "workflow.h"
#include "ui/ui.h"
#include "display.h"

/**********************🍃 GLOBAL Vars *******************************/
#if defined(SEEDER_BOARD_STICKS3)
SeederDisplay &tft = M5.Display;
#else
SeederDisplay tft;  // LilyGO pins remain defined in TFT_eSPI/User_Setup.h.
#endif
extern sWallet myWallet;
extern sButton btnMove;
extern sButton btnSelect;

/*****************🍃 TFT WORK *********************/

void Init_TFT(void){

#if defined(SEEDER_BOARD_STICKS3)
  // Never initialize serial, radios, audio, sensors, or external accessories.
  // Silence library diagnostics before initialization; seeds never enter logs.
  esp_log_level_set("*", ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_callback, ESP_LOG_NONE);
  auto cfg = M5.config();
  cfg.serial_baudrate = 0;
  cfg.clear_display = true;
  cfg.output_power = false;
  cfg.internal_imu = false;
  cfg.internal_rtc = false;
  cfg.internal_mic = false;
  cfg.internal_spk = false;
  cfg.external_imu = false;
  cfg.external_rtc = false;
  cfg.external_speaker_value = 0;
  cfg.external_display_value = 0;
  cfg.fallback_board = m5::board_t::board_M5StickS3;
  M5.begin(cfg);
  M5.Power.setExtOutput(false);
  tft.setBrightness(128);
#else
#ifdef PIN_POWER_ON
  /* La T-Display-S3 alimenta sus periféricos desde este pin. Si no se pone
     alto ANTES de arrancar el panel, la pantalla no enciende y la placa
     parece muerta: es el fallo clásico de esa placa. */
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  delay(20);
#endif

  tft.init();
#endif
  tft.setRotation(1);
  tft.setSwapBytes(true);   // orden de bytes al volcar imágenes

  ui::splash();

  myWallet.entropySrc = coinEntropy;
  drawInitMenu();
}

void Update_Board(void){
#if defined(SEEDER_BOARD_STICKS3)
  M5.update();
#endif
}

/*****************🍃 BUTTON DETECTION *********************/

sButton::sButton(byte bPin){        //Constructor
    pin = bPin; pinMode(pin, INPUT);
    antState = HIGH; longFired = holdFired = false;
    msecLst = msecEdge = 0; clickState = None;
}
void sButton::init(void){  pinMode(pin, INPUT); }     // Init pushbutton pin
int sButton::click(void){  return clickState; }
void sButton::forceClick(void){ clickState = ForcedClick;} //Generates one click loop

//Cuanto lleva pulsado ahora mismo. Cero significa suelto, asi que sirve
//igual para dibujar el progreso de un mantenido y para saber si sigue ahi.
unsigned long sButton::heldMs(void){
    if(!msecLst) return 0;
    const unsigned long h = millis() - msecLst;
    return h ? h : 1;               //0 esta reservado para "suelto"
}

void sButton::check(void)
{
    const unsigned long ButDebounce  = 25;
          unsigned long msec = millis();

    if(clickState == ForcedClick){ clickState = SingleClick; return; }
    clickState = None;

    byte but = digitalRead(pin);

    if(but != antState){
        if(msec - msecEdge < ButDebounce) return;   // bounce, ignore the edge
        msecEdge = msec;
        antState = but;

        if(but == LOW){                             // pressed
            msecLst = msec ? msec : 1;
            longFired = holdFired = false;
        }else{                                      // released
            //Click reported on release, so holding the button never turns two
            //presses into one and no input is ever swallowed
            if(msecLst && !longFired){ clickState = SingleClick; DBGLN("SingleClick"); }
            msecLst = 0;
        }
        return;
    }

    //Still held: LongClick fires once and suppresses the click on release.
    //HoldClick comes later on that same press, so a screen armed by LongClick
    //has something to fill up to and letting go still backs out of it.
    if(but != LOW || !msecLst) return;
    const unsigned long held = msec - msecLst;

    if(!longFired && held > BTN_LONG_MS){
        longFired = true;
        clickState = LongClick;
        DBGLN("LongClick");
    }else if(longFired && !holdFired && held > BTN_HOLD_MS){
        holdFired = true;
        clickState = HoldClick;
        DBGLN("HoldClick");
    }
}





