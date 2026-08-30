# Modelo de seguridad de la SEEDER

Este documento explica **de qué te protege la SEEDER, de qué no**, y cómo comprobar
por tu cuenta que hace lo que dice. Está escrito para que puedas desconfiar del
aparato con criterio, que es la única forma sana de usar uno.

Si algo de aquí no cuadra con lo que ves en tu dispositivo, no lo uses.

---

## La idea de fondo

**La SEEDER no genera entropía.** No tiene generador de números aleatorios. La
entropía la pones tú con una moneda o con un dado, y el aparato se limita a hacer
las cuentas de BIP39 delante de ti.

Eso no es una limitación, es el diseño. Un generador de semillas que produce la
aleatoriedad por su cuenta te pide que confíes en su silicio, en su firmware y en
quien te lo vendió. La SEEDER te pide que confíes en tu moneda.

Y como la entropía es tuya, **puedes rehacer las cuentas por fuera**: la pantalla
`Entropy (hex)` te enseña exactamente los bytes de los que salieron tus palabras.

| Modo | Entrada | Entropía |
|---|---|---|
| Moneda | 128 tiradas (12 palabras) / 256 (24) | los bits, tal cual, sin hash |
| Dado | 50 tiradas (12 palabras) / 99 (24) | `SHA-256` de los dígitos en ASCII |

---

## Decisiones de diseño

- **La semilla nunca se escribe en flash.** Sólo vive en RAM y desaparece al
  desconectar. No hay EEPROM, no hay NVS, no hay persistencia de ningún tipo.
- **La semilla nunca sale por el puerto serie.** El firmware no imprime *nada*
  por UART. `SEEDER_DEBUG` sólo habilita trazas de botones y jamás toca la
  semilla ni la entropía.
- **Borrado explícito.** Al salir de las páginas de la semilla se sobrescriben en
  memoria las palabras, la entropía, el zpub y las tiradas. uBitcoin ya hace
  `memzero` de la clave privada y el chain code al destruirlas.
- **Sin red.** No se inicializan WiFi ni Bluetooth. No hay OTA.
- **Sin passphrase BIP39** por ahora: con dos botones no es usable, y una
  passphrase incómoda es una passphrase mala.
- Derivación **BIP84** (`m/84'/0'/0'`), direcciones nativas segwit.

---

## Cómo verificar tu SEEDER

### 1. La comprobación que importa, y no necesita nada instalado

Genera una semilla, apunta el `Entropy (hex)` que te muestra, y comprueba en
cualquier herramienta BIP39 **offline** que de esa entropía salen esas palabras.

Si no coinciden, el aparato te está mintiendo.

> **Hazlo con tu propia entropía, no con los vectores publicados.** Los de
> [TESTVECTORS.md](TESTVECTORS.md) sirven para cazar errores, pero no una puerta
> trasera deliberada: un firmware malicioso reconoce perfectamente que 128 cruces
> seguidas es alguien probándolo, se porta bien ahí, y roba en la siguiente. Una
> generación real no la puede distinguir de una prueba, porque *es* real.

### 2. Que el chip contiene el binario publicado

```bash
esptool.py --port COM3 --no-stub verify_flash 0x0 seeder-firmware-merged.bin
```

Con `--no-stub` **la aplicación de la SEEDER ni se ejecuta**: las líneas DTR/RTS
meten al chip en modo descarga por hardware y quien responde es el bootloader de
la ROM de máscara, grabado en silicio e inmodificable. Un firmware malicioso puede
mentirte en su pantalla, pero no puede mentir sobre lo que hay en la flash, porque
no participa en la lectura.

### 3. Que ese binario sale de este código

Cada release lleva su `SHA256SUMS` y se construye con GitHub Actions desde este
repositorio. O compílalo tú:

```bash
pio run -e seeder
```

Las librerías van fijadas dentro de `lib/` precisamente para que el resultado sea
el mismo hoy y dentro de cinco años.

---

## Lo que deliberadamente NO hacemos

**No mostramos el hash del firmware en pantalla.** Sería teatro: el firmware que
dibuja el hash es el mismo que habría que verificar, así que a uno malicioso le
basta con imprimir una constante. Ningún programa puede demostrar su propia
integridad ante sí mismo. La verificación tiene que venir de fuera, y por eso está
el punto 2 de arriba.

**No ciframos la flash.** En este aparato no hay ningún secreto que proteger: el
código es público y la semilla no toca la flash. Cifrarla no ganaría nada y
rompería el `verify_flash`, que es la única comprobación externa que tienes.

Es una decisión consciente con un coste conocido: sin cifrado existe un ataque
*time-of-check to time-of-use* que requiere acceso físico al aparato y un
interposer en el bus SPI. Aceptamos ese riesgo —de laboratorio y dirigido— a
cambio de que cualquiera pueda verificar su dispositivo.

---

## Unidades firmadas

> Aplica a las unidades vendidas ya flasheadas. Las que montes tú quedan
> completamente abiertas, sin candado de ningún tipo.

Las unidades provisionadas llevan **Secure Boot v2** activado: sólo arranca
firmware firmado con la clave de Bitronics. Si alguien te flashea otra cosa, el
aparato no arranca. Se recupera volviendo a flashear el firmware firmado.

Lo que Secure Boot te garantiza y lo que no:

- **Sí**: que el firmware que corre es el que firmó Bitronics.
- **No**: que ese firmware sea bueno. Concentra la confianza en Bitronics, no la
  elimina. Lo que la elimina es lo de la sección anterior.

El modo descarga por UART **se deja habilitado a propósito**, en contra de la
recomendación por defecto de Espressif, para que puedas seguir haciendo
`verify_flash` y para que una placa mal flasheada sea recuperable y no un ladrillo.

---

## Riesgos que este diseño no cubre

Con honestidad, porque un modelo de amenazas que sólo lista lo que sí resuelve no
sirve de nada:

- **Un firmware manipulado que te dé las palabras correctas y además las filtre**
  por WiFi o BLE. Pasaría todas las comprobaciones de arriba. El ESP32 lleva las
  dos radios en el chip. La única defensa real es compilarlo y flashearlo tú.
- **El QR exporta el mnemónico en claro.** Si lo escaneas con el móvil, tu semilla
  acaba en un teléfono con internet. No lo hagas.
- **La cadena de suministro del propio chip.** Todo lo anterior asume un ESP32
  genuino con su ROM genuina.
- **Miradas.** La semilla se muestra en pantalla. Genera en un sitio sin cámaras y
  sin gente detrás.
- **Ataques físicos** sobre un dispositivo en marcha: canales laterales, glitching,
  extracción de RAM. Fuera de alcance.

---

## Reportar un problema

Si encuentras un fallo de seguridad, escribe a **bitmaker@bitronics.com** antes
de publicarlo, y danos margen para sacar una corrección.
