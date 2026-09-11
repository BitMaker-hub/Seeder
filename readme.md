# SEEDER - Sovereign seed generator

Un dispositivo DIY que te permite crear tus propias semillas de Bitcoin en cuestión
de segundos, sin depender de nadie ni de nada para salvaguardar tus fondos.

Con la SEEDER puedes hacer el proceso **semilla moneda**, generar entropía con
**dados**, y **calcular la última palabra de tu semilla sin esfuerzo**, además de
exportarla mediante un código QR.

Todo ello gracias a la genial idea de @Lunaticoin y mi trabajo.

![SEEDER, genera semillas offline](Images/Seeder_cover.jpg)

## La SEEDER no genera entropía

Esto es lo que hace distinta a la v2: **el dispositivo no tiene generador de números
aleatorios**. La entropía la pones tú, con una moneda o con un dado, y la SEEDER se
limita a hacer las cuentas de BIP39 delante de ti.

Y esas cuentas las puedes rehacer por tu lado. La SEEDER te enseña la entropía en
hexadecimal en la pantalla `Entropy (hex)`. Con ese hex y cualquier herramienta BIP39
offline obtienes exactamente las mismas palabras. Si no coinciden, tira el aparato.

| Modo | Entradas | Entropía |
|---|---|---|
| Moneda | 128 tiradas (12 palabras) / 256 (24) | los bits, tal cual, sin pasar por ningún hash |
| Dado   | 50 tiradas (12 palabras) / 99 (24)   | `SHA-256` de los dígitos en ASCII |

El modo dado usa el mismo esquema que la Coldcard, así que puedes verificarlo desde
un terminal:

```bash
printf '3141592653...' | sha256sum
```

Con 24 palabras se usan los 32 bytes del hash; con 12 palabras, los **16 primeros**
(los 32 primeros caracteres del hex). Es lo mismo que hace la Coldcard.

Y el modo moneda no aplica ningún hash: los bits que lanzas **son** la entropía. Eso
te permite además meter una entropía que ya tengas y dejar que la SEEDER te calcule
la última palabra con su checksum.

## Requisitos

Cualquiera de las dos placas:

| Placa | Pantalla | Binario |
|---|---|---|
| LilyGO TTGO T-Display (ESP32) | 240x135 | `seeder-tdisplay-merged.bin` |
| LilyGO T-Display-S3 (ESP32-S3) | 320x170 | `seeder-tdisplay-s3-merged.bin` |

Y un cable USB-C.

Opcionalmente admite una LiPo por el conector JST. La placa no trae interruptor,
pero **sí se puede apagar por firmware**: cómo hacerlo, y qué esperar del consumo
en reposo, está en [HARDWARE.md](HARDWARE.md).

## Instalación

Todo sale de la [última release](https://github.com/BitMaker-hub/Seeder/releases).
Hay dos caminos: uno cómodo y otro comprobable. Elige según lo que te juegues.

### Vía rápida — desde el navegador

Descarga `seeder-firmware-merged.bin` y flaséalo con
[esptool-js](https://espressif.github.io/esptool-js/) en el offset **0x0**.
Un solo archivo, nada que instalar.

> Cómodo, pero estás confiando en que la página te sirvió el binario correcto.
> Para una semilla de verdad, haz también la comprobación de abajo.

### Vía manual — desde el terminal, y verificable

Descarga la release entera, incluido el `SHA256SUMS`, y comprueba que lo que te
has bajado es lo que la CI publicó:

```bash
sha256sum -c SHA256SUMS
```

Flashea:

```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800   write_flash -z 0x0 seeder-firmware-merged.bin
```

Y comprueba que el chip contiene de verdad ese binario:

```bash
esptool.py --port /dev/ttyUSB0 --no-stub verify_flash 0x0 seeder-firmware-merged.bin
```

En Windows el puerto es `COM3` o similar. Ese último paso es el que importa y
está explicado en [SECURITY.md](SECURITY.md): con `--no-stub` la aplicación de la
SEEDER ni se ejecuta, responde el bootloader de la ROM del chip, así que un
firmware manipulado no puede mentir sobre lo que hay en la flash.

## Compilar desde fuente

```bash
pio run -e seeder
```

Las librerías (`uBitcoin`, `TFT_eSPI`) van fijadas dentro de `lib/` a propósito:
un generador de semillas debe compilar igual hoy que dentro de cinco años.

## Controles

Dos botones y nada más. **MOVE** es el de arriba y **OK** el de abajo.

| Dónde estás | MOVE | OK |
|---|---|---|
| Menú y elección de palabras | cambia la opción | acepta |
| Lanzando la moneda | cara | cruz |
| Tirando el dado | 1 → 6 | acepta la tirada |
| Leyendo la semilla | página siguiente | página anterior |
| Última página (`Exit`) | vuelve a la primera | mantener para salir y borrar |

Durante la captura, **mantener OK tres segundos vuelve al menú** y borra lo que
llevaras metido: si te has equivocado en la tirada 40 de 99 no hace falta
desenchufar el aparato. A los ~1,2 s aparece el aviso con una barra que se llena;
si sueltas antes, esa pulsación no cuenta como cara, cruz ni tirada.

## Llevarla a Sparrow

La SEEDER exporta dos QR distintos, y **no dan el mismo poder**:

| Pantalla | Qué lleva | Quien lo escanee |
|---|---|---|
| `Export` | las 12 o 24 palabras | **puede gastar** |
| `Watch only` | el zpub de cuenta | sólo ve el saldo |

**Sólo lectura** — es el que puedes escanear con el ordenador delante:

1. `File > New Wallet…`, le pones nombre y `Create Wallet`.
2. Se abre en *Settings* con **Single Signature** y **Native Segwit (P2WPKH)**
   ya puestos. Déjalos: son justo los de la SEEDER.
3. En *Keystores*, el icono del **ojo**, `Watch Only Wallet`.
4. En la fila `xpub:`, el botón de la **cámara**, y le enseñas el QR.
5. La SEEDER te muestra la huella maestra en esa misma pantalla. Cópiala en
   *Master fingerprint*, que Sparrow lo deja en `00000000` y ese cero acaba
   metido en cada PSBT que construya.
6. `Apply`.

> El botón de QR que hay junto al campo `Descriptor:` **no** sirve para esto:
> con una clave suelta no hace nada y no avisa. Tiene que ser el de la fila
> `xpub:`.

**La semilla**, si de verdad la quieres importar: `BIP39 keystore`, la **flecha
del desplegable** junto a `Use 24 Words`, y ahí dentro `Scan QR…`. No hay botón
visible y no está documentado. Necesita Sparrow **1.7.7 o posterior**.

Pero piénsatelo: meter las palabras en un ordenador conectado tira por tierra
todo lo demás. Para mirar el saldo te basta el zpub.

## Verificación

No te fíes de la SEEDER: compruébala. La pantalla `Entropy (hex)` te enseña los
bytes de los que salieron tus palabras, y con ese hex y cualquier herramienta BIP39
**offline** puedes rehacer las cuentas. Si no coinciden, no uses el aparato.

Hazlo con **tu propia entropía**, no con los vectores publicados: un firmware
malicioso reconoce las tiradas de prueba y se porta bien sólo ahí.

El modelo de amenazas completo, y qué no cubre, está en [SECURITY.md](SECURITY.md).

> La semilla nunca sale del dispositivo ni se escribe en flash: sólo vive en RAM y
> desaparece al desconectarlo. El firmware tampoco imprime nada por el puerto serie.

## TUTORIAL

Tutorial completo en YouTube:

[![Ver video aquí](https://img.youtube.com/vi/2K7ztWxtyY8/0.jpg)](https://youtu.be/2K7ztWxtyY8)
