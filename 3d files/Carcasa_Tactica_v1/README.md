# SEEDER — carcasa táctica v1

Carcasa para **LilyGO TTGO T-Display (ESP32)** con hueco para batería, cierre por
clips y anilla para colgar. Todas las cotas de la placa están sacadas del STEP
oficial `T-Display.step`, no estimadas.

**Exterior: 30.6 × 66.0 × 17.4 mm** · ~16 g de PLA el juego completo.

## Piezas

| Fichero | Qué es | Cantidad |
|---|---|---|
| `Seeder_CUERPO.stl` | Cuerpo frontal (pantalla, botones, USB, anilla) | 1 |
| `Seeder_TAPA.stl` | Tapa trasera con clips y bolsillo de batería | 1 |
| `Seeder_BOTONES_x2.stl` | Los dos pulsadores (`+` y check) | 1 juego |
| `Seeder_BOTONES_x2_giro_opuesto.stl` | Los mismos con los iconos girados 180° | alternativa |

### Por qué hay dos juegos de botones

El firmware arranca la UI en **landscape** (`setRotation(1)`, 240×135), o sea que
el aparato se lee girado 90° respecto a la carcasa. Los iconos van grabados en esa
orientación, pero según hacia qué lado gires el aparato al usarlo, el check queda
derecho o del revés. Imprime el juego que corresponda; el `+` da igual.

## Impresión (Bambu Lab P1S / P2S / H2S)

| Ajuste | Valor |
|---|---|
| Material | PLA (o PETG si lo quieres más resistente al calor) |
| Boquilla / capa | 0.4 mm / 0.2 mm |
| Perímetros | 3 |
| Relleno | 20 % giroide |
| Soportes | **ninguno** |
| Balsa/brim | no hace falta |

**Orientación en la placa:**

- **Cuerpo**: cara frontal (la de la pantalla) **contra la cama**. Así el grabado
  del SEEDER y los nervios salen limpios, y las cavidades quedan hacia arriba.
  Si usas placa texturizada, la cara vista sale con acabado mate tipo equipo militar.
- **Tapa**: cara exterior contra la cama. Los clips y las paredes de la batería
  quedan hacia arriba, sin voladizos.
- **Botones**: cara vista (la del icono) contra la cama.

Los únicos puentes son el techo de la abertura del USB (12.4 mm) y el de las
cuatro ventanas de clip (7 mm). Ambos los hace cualquier Bambu sin problema.

## Montaje

1. Mete la placa en el cuerpo **por detrás**, con la pantalla mirando a la ventana.
   Apoya sobre la repisa perimetral de 2 mm.
2. Coloca los dos botones por delante, desde fuera. Encajan a presión: la pestaña
   queda atrapada entre la placa frontal y la PCB, así que **no se caen una vez
   cerrada la tapa**.
3. Conecta la batería al JST y colócala en el bolsillo de la tapa. El cable sale
   por el hueco de 11.2 mm del lado del USB.
4. Cierra la tapa presionando hasta oír los cuatro clics.

**Para abrir**: hay una muesca de palanca en el canto del USB. Una uña o una púa
de guitarra bastan.

## Batería

Cabe hasta **21 × 40 × 6.6 mm**. Una LiPo 602040 (500 mAh) entra holgada. Sujétala
con un trozo de cinta de doble cara al fondo del bolsillo para que no baile.

## Comprobado

- Las tres piezas son **watertight** (malla cerrada, lista para laminar).
- Encaje verificado contra los 655 sólidos del STEP de la placa: interferencia
  máxima **0.010 mm**, y sólo en el plano donde la PCB apoya sobre su repisa.
  Se descuenta automáticamente la envolvente de cada componente frontal, así que
  ni el pulsador de reset ni los SMD del borde rozan.
- Los botones llevan 0.05 mm de precarga contra el pulsador SMD: hacen contacto
  sin dejarlo pulsado.

## ⚠️ Verificar antes de imprimir la serie

El STEP de LilyGO **no incluye ni el conector USB ni el módulo ESP32**. Por eso:

- La abertura del USB va deliberadamente holgada: **12.4 mm de ancho × 4.4 de alto**
  (z 4.5–8.9), que vale tanto para micro-USB como para USB-C. **Mide la tuya** y
  ajusta `USB_W`, `USB_Z0` y `USB_Z1` en `seeder_case.py` si quieres que ajuste fino.
- El hueco para componentes traseros es de 3.8 mm, suficiente para el ESP32-WROOM
  (3.1 mm). Si tu placa lleva algo más alto por detrás, sube `BACK_CLR`.

Imprime **una** unidad y prueba el encaje antes de tirar de serie.

## Regenerar / modificar

Todo es paramétrico. Los parámetros están arriba del todo de `seeder_case.py`.

```bash
py -m pip install build123d trimesh manifold3d shapely numpy rtree
```

```bash
py seeder_case.py
```

- `extract_clearance.py` regenera `components.json` desde el STEP (sólo hace falta
  si cambias de placa).
- `fitcheck.py` vuelve a comprobar que la placa no choca con nada.
