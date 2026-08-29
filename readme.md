# SEEDER - Sovereign seed generator

Un dispositivo DIY que te permite crear tus propias semillas de Bitcoin en cuestión
de segundos, sin depender de nadie ni de nada para salvaguardar tus fondos.

Con la SEEDER puedes hacer el proceso **semilla moneda**, generar entropía con
**dados**, y **calcular la última palabra de tu semilla sin esfuerzo**, además de
exportarla mediante un código QR.

Todo ello gracias a la genial idea de @Lunaticoin y mi trabajo.

![image](https://github.com/BitMaker-hub/Seeder/blob/master/Images/Seeder_entry.png)

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

- Módulo TTGO T-Display
- Cable USB-C

## Instalación

Descarga `seeder-firmware-merged.bin` de la [última
release](https://github.com/BitMaker-hub/Seeder/releases) y flaséalo con
[esptool-js](https://espressif.github.io/esptool-js/) en el offset **0x0**. Es un
único archivo, no hace falta nada más.

Cada release lleva su `SHA256SUMS` y se construye desde este repositorio con
GitHub Actions, así que puedes comprobar que el binario que flasheas sale del
código que estás leyendo.

## Compilar desde fuente

```bash
pio run -e seeder
```

Las librerías (`uBitcoin`, `TFT_eSPI`) van fijadas dentro de `lib/` a propósito:
un generador de semillas debe compilar igual hoy que dentro de cinco años.

> La semilla nunca sale del dispositivo ni se escribe en flash: sólo vive en RAM y
> desaparece al desconectarlo. El firmware tampoco imprime nada por el puerto serie.

## TUTORIAL

Tutorial completo en YouTube:

[![Ver video aquí](https://img.youtube.com/vi/2K7ztWxtyY8/0.jpg)](https://youtu.be/2K7ztWxtyY8)
