# Vectores de prueba

Cómo comprobar que tu SEEDER hace las cuentas bien, sin fiarte de ella.

Ninguno de estos vectores es una semilla que debas usar: son entropías
degeneradas, elegidas justamente porque se pueden reproducir a mano.

## Modo moneda — los bits son la entropía

En el modo moneda no hay hash: los bits que lanzas **son** la entropía. Así que
si sacas siempre cruz, la entropía es todo ceros, y eso da el vector de prueba
más conocido de BIP39. Cualquier herramienta del mundo te dará lo mismo.

**12 palabras — 128 tiradas, todas cruz**

```
Entropy (hex)  00000000000000000000000000000000
abandon abandon abandon abandon abandon abandon
abandon abandon abandon abandon abandon about
```

**24 palabras — 256 tiradas, todas cruz**

```
Entropy (hex)  0000...0000  (32 bytes a cero)
abandon x23 art
```

## Modo dado — SHA-256 de las tiradas

El dado siempre vuelve al 1 después de aceptar una tirada, así que **pulsando
sólo OK** metes una tirada de 1 cada vez. 50 veces para 12 palabras, 99 para 24.

**12 palabras — 50 unos**

```
$ printf '11111111111111111111111111111111111111111111111111' | sha256sum
3dac51a65ec9fcfc409a1b5f1defe92ba723843118ea511971ab46b36859495f

Entropy (hex)  3DAC51A65EC9FCFC409A1B5F1DEFE92B     <- los 16 primeros bytes
diet glad hat rural panther lawsuit act drop gallery urge where fit
```

**24 palabras — 99 unos**

```
$ printf '111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111' | sha256sum
fa098eb852b2660348b21bb00ad03a49cc177ea07ebe34f46b40baa85313525e

Entropy (hex)  FA098EB852B2660348B21BB00AD03A49CC177EA07EBE34F46B40BAA85313525E
wheel erase puppy pistol chapter accuse carpet drop quote final attend near
scrap satisfy limit style crunch person south inspire lunch meadow enact tattoo
```

## Y con tu semilla de verdad

El mismo procedimiento vale para la semilla que vayas a usar, sin publicarla:
apunta el `Entropy (hex)` que te enseña la SEEDER, mételo en cualquier
herramienta BIP39 **offline** y comprueba que salen las mismas palabras.

Si no salen, no uses el aparato.
