# La SEEDER con batería

La T-Display admite una LiPo por su conector JST 1.25, pero **no trae interruptor**:
la enchufas y está encendida para siempre. Este documento recoge lo que hay que
saber para montarla con batería, incluida una función de la placa que no está
documentada en ningún sitio.

---

## Sí se puede apagar: PWR_EN es GPIO14

La T-Display tiene un MOSFET que corta la batería, y el ESP32 puede accionarlo.
Sale del [esquemático oficial](https://github.com/Xinyuan-LilyGO/TTGO-T-Display/blob/master/schematic/ESP32-TFT(6-26).pdf),
donde la red `PWR_EN` está en el **pin 17 del módulo, `MTMS/IO14`**:

```
PWR_EN (GPIO14) ──D6──┐
                      ├── R25 10K ──► base Q3 (PMBT3904)     R27 100K a GND
VBUS ─────────────D1──┘
                            colector Q3 ── R26 1K ──► puerta de Q4 y Q6

BAT ──► Q4 (SI2301, canal P) ──► IN del AP2112K-3.3V ──► VDD3V3
              R28 100K entre la puerta y el lado del regulador
```

`Q4` es el interruptor de la batería. Su puerta la manda `Q3`, que conduce si hay
`VBUS` **o** si `PWR_EN` está alto.

En la práctica:

```cpp
pinMode(14, OUTPUT);
digitalWrite(14, HIGH);   // engancha la alimentación al arrancar
...
digitalWrite(14, LOW);    // corta la batería
```

### Dos advertencias

**Por USB no se puede apagar, y esto es seguro.** `D1` mete VBUS directamente en
la base de `Q3`, así que mientras haya cable, `Q3` conduce pase lo que pase con
GPIO14. El corte sólo tiene efecto con la placa alimentada sólo por batería.

**El enganche es flojo, y esto está sin verificar.** `R28` lleva la puerta de `Q4`
al lado del regulador, no al de la batería. Eso hace que la placa arranque sola al
conectar la LiPo, pero también que al soltar `PWR_EN` la puerta suba siguiendo al
propio raíl que está alimentando: puede cortar limpiamente o quedarse en una zona
intermedia y rearrancar. En el repo de LilyGO hay un
[hilo abierto](https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/6) de alguien
que reporta justo eso, sin respuesta.

**Si lo pruebas, cuéntanoslo.** Es la clase de dato que no está escrito en ninguna
parte.

---

## La alternativa segura: un interruptor

Para una unidad que va a estar guardada, lo que no falla es un **interruptor
deslizante en el positivo del JST**. Cero consumo, y en un generador de semillas
"apagado" de verdad vale más que "durmiendo". La carcasa de `3d files/` tiene sitio.

---

## Sueño profundo, y por qué aquí importa

Si no cortas la corriente, la otra vía es el *deep sleep* del ESP32, despertando con
un botón. El detalle que casi todo el mundo se salta: **si no apagas la pantalla se
siguen yendo unos 10 mA** aunque el chip duerma. Hay que apagarla a mano:

```cpp
digitalWrite(TFT_BL, LOW);                    // GPIO4, retroiluminación
tft.writecommand(ST7789_DISPOFF);             // apagar el panel
tft.writecommand(ST7789_SLPIN);               // dormir el controlador
esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // despertar con OK
esp_deep_sleep_start();
```

Con eso se llega a **~380 µA**, [medidos por la comunidad](https://www.paleotechnologist.net/?p=5041).
Con una LiPo de 500 mAh son unos **55 días** en reposo, frente a dos días si te
olvidas del panel. No es cero: para meses en un cajón, interruptor.

Los dos botones sirven para despertar. Los pines RTC del ESP32 son
[`0, 2, 4, 12-15, 25-27, 32-39`](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/sleep_modes.html),
y tanto `GPIO0` como `GPIO35` están dentro.

### Y para este aparato en concreto, dormir es borrar

La documentación de Espressif lo dice de las modalidades de sueño profundo: *las CPU,
la mayor parte de la RAM y todos los periféricos digitales se apagan*. Sólo sobrevive
la memoria RTC, y la SEEDER no guarda nada ahí.

O sea que **entrar en sueño profundo destruye la semilla por hardware**, sin depender
de que el `memzero` del firmware esté bien escrito. Para un aparato cuya premisa es
no fiarse del firmware, eso no es un detalle menor.

---

## Un fallo conocido

Hay un [hilo abierto](https://github.com/Xinyuan-LilyGO/TTGO-T-Display/issues/111)
donde, tras descargar la batería del todo, la placa "olvida" que podía despertar por
GPIO y sólo revive con un reset manual. Sin causa identificada. Tenlo presente si
montas una unidad para dejarla guardada mucho tiempo.

---

## Medir la batería

El divisor de tensión de la batería está en **GPIO34** (`ADC_IN` en el pinout de
LilyGO). No lo usamos, pero está ahí si alguien quiere un indicador de carga.
