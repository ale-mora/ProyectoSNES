# Unidad 05 — El mando

**Objetivo:** leer los botones del mando de la SNES, separar «mantenido» de
«recién pulsado» y traducirlos a intenciones de juego sin que la lógica sepa
nada de bits.

**Prerrequisitos:** unidad 04 (el bucle, el VBlank y la rutina de la NMI) y,
por ella, la 03 y la 00 (operadores de bits `& | ^ ~ << >>` y máscaras: aquí
se dan por sabidos). **Práctica:** mini-ROM (+ una demo de PC de 30 líneas).
**Tiempo orientativo:** una tarde.

## Conceptos

### El mando es un número de 16 bits

El mando de la SNES tiene 12 botones: la cruceta (arriba, abajo, izquierda,
derecha), A, B, X, Y, L, R, START y SELECT (→ glosario: Mando). Cada uno es
**un bit** de un número de 16 bits: 1 si está pulsado, 0 si no. Los 4 bits
bajos siempre valen 0 en un mando normal. PVSnesLib da nombre a cada bit en
`input.h`:

```
bit:  15  14   13     12    11  10   9    8    7   6   5   4   3..0
      B   Y  SELECT START   UP DOWN LEFT RIGHT  A   X   L   R   0000
```

```c
KEY_B = BIT(15),  KEY_Y = BIT(14), KEY_SELECT = BIT(13), KEY_START = BIT(12),
KEY_UP = BIT(11), KEY_DOWN = BIT(10), KEY_LEFT = BIT(9), KEY_RIGHT = BIT(8),
KEY_A = BIT(7),   KEY_X = BIT(6),  KEY_L = BIT(5),  KEY_R = BIT(4),
```

Una **máscara** es un número con unos solo en los bits que interesan
(unidad 00). Para saber si B está pulsado se hace `pad & KEY_B`: el resultado
es distinto de 0 si y solo si el bit 15 está a 1. Para varios a la vez,
`pad & (KEY_LEFT | KEY_RIGHT)`.

### La autolectura del mando

La consola puede leer los mandos sola: al empezar cada VBlank, si está
habilitada la **autolectura** (bit 0 de `NMITIMEN $4200`), recorre los 16 bits
de cada mando (tarda unas 3 líneas) y los deja en los registros
`JOY1L/JOY1H` (`$4218/$4219`) para el mando 1. Mientras dura, el bit 0 de
`HVBJOY $4212` está a 1 y leer `$4218` da basura.

### Cómo lo lee PVSnesLib (en la rutina de la NMI)

Lo hace la misma rutina de la NMI de la unidad 04, al final y **solo en los
frames buenos** (si el bucle estaba en `WaitForVBlank`; en un frame perdido no
toca el mando, para que no cambie a mitad de la lógica). En orden (leído en
`vblank.asm` del código fuente de PVSnesLib):

1. Espera a que termine la autolectura (`HVBJOY` bit 0 a 0).
2. Guarda el valor anterior: `pad_keysold[0] = pad_keys[0]`.
3. Lee `$4218` (16 bits). Si alguno de los bits 0..3 vale 1, no es un mando
   normal y lo deja en 0. El resultado va a `pad_keys[0]`.
4. Calcula los recién pulsados:
   **`pad_keysdown[0] = (pad_keys[0] ^ pad_keysold[0]) & pad_keys[0]`**: los
   bits que cambiaron (`^`) y que ahora están a 1 (`& pad_keys`).

Y lo mismo para el mando 2. Así, cuando `WaitForVBlank()` vuelve, las tres
variables ya tienen lo de este frame.

### Mantenido, recién pulsado y recién soltado

- **Mantenido** (*held*): el bit está a 1 ahora. Dura todos los frames que el
  botón esté abajo. Sirve para «mientras»: caminar, correr, mantener B para
  alargar el salto.
- **Recién pulsado** (*down*, *pressed*): el bit está a 1 ahora y estaba a 0
  en el frame anterior. **Dura un solo frame.** Sirve para acciones de un
  disparo: saltar, atacar, desenvainar. Si el salto mirara el mantenido,
  Tarou saltaría otra vez nada más aterrizar mientras B siguiera abajo.
- **Recién soltado** (*up*): estaba a 1 y ahora está a 0. `antes & ~ahora`.

Como el SDK solo calcula `pad_keysdown` en los frames buenos y cada vuelta
del bucle es un frame (unidad 04), **cada pulsación se ve exactamente una vez
por vuelta del bucle**: ni se pierde ni se duplica (siempre que la pulsación
dure al menos un frame, que en la práctica es siempre).

La demo de PC lo enseña con un mando «grabado» (`demo_05_mascaras.c`; salida
en `capturas/salida_demo_05_mascaras.txt`):

```
frame  pad    down   up     B?  B?(u8)  switch del ejemplo
    1  8000   8000   0000   1   0       B PRESSED
    2  8000   0000   0000   1   0       B PRESSED
    3  8100   0100   0000   1   0       (nada)
    4  0100   0000   8000   0   0       RIGHT PRESSED
```

En el frame 1 B baja (`down = 8000`); en el 2 sigue mantenido pero ya no es
«recién»; en el 3 baja Derecha; en el 4 se suelta B (`up = 8000`).

### La trampa del `(u8)`

La columna `B?(u8)` de la demo es siempre 0. Es `(u8)(pad & KEY_B)`: el
resultado de la máscara es `0x8000`, y al meterlo en 8 bits se queda con los
8 bits **bajos**, que son 0. Con B (bit 15), Y, SELECT, START y la cruceta
(bits 8..15) pasa siempre. Por eso `src/juego/mando.c` escribe
`(u8)((mantenidos & KEY_B) != 0)`: primero compara (da 0 o 1) y luego
convierte.

### El `switch` del ejemplo del SDK y la prioridad

El ejemplo `tools/pvsneslib/snes-examples/input/controller/controller.c` hace
`switch (pad0) { case KEY_A: ... case KEY_B: ... }`. Compara el número
**entero** con cada máscara: con B y Derecha a la vez (`0x8100`) no coincide
con ningún `case` y no muestra nada (frame 3 de la demo). Para leer botones
hay que usar `&`, uno por uno.

Y cuando dos botones piden cosas distintas el mismo frame, alguien tiene que
decidir. El juego fija una **prioridad**
(`docs/diseno-demo-jugable.md` §4, «Prioridad el mismo frame»):
**START > SELECT > R > A > B > Abajo > dirección**. En C es una cadena de
`if/else` en ese orden: el primero que se cumple gana y los demás no se
miran. La mini-ROM la aplica en su fila `ACTION`.

### La `struct Mando`: de bits a intenciones

La lógica del juego (`src/juego/tarou.c`) **no** lee bits: recibe una
`struct Mando` (`src/juego/mando.h`) con campos con nombre, todos 0 o 1:

```c
typedef struct
{
    u8 izquierda, derecha, abajo, correr, saltar;                                /* mantenidos     */
    u8 saltar_pulsado, atacar_pulsado, espada_pulsado, hud_pulsado, salir_pulsado; /* recién pulsados*/
    u8 actividad;                                                                /* algo tocado    */
} Mando;
```

`mando_leer()` (`src/juego/mando.c`) es **la única función del juego que mira
los bits del pad**. Tres ventajas: la asignación de botones (B salta, Y
corre...) está en un solo sitio; `tarou.c` no incluye `<snes.h>` y se puede
compilar con `gcc` y probar en el PC (las pruebas rellenan la `struct` a
mano); y se puede jugar **sin mando**.

### Guion de mando: jugar sin manos

Un **guion de mando** (→ glosario: Guion de mando) es una tabla de tramos
«del frame A al frame B, mantener tales botones». Para un frame dado se hace
OR de los tramos activos (mantenidos); haciendo lo mismo con el frame anterior
salen los recién pulsados, igual que el SDK: `ahora & ~antes`. Como la lógica
solo ve la `struct Mando`, da igual de dónde venga:

- `src/juego/guion.c` (`guion_mando`) convierte un guion en una `struct
  Mando`.
- `src/juego/guion_tutorial.c` es el recorrido completo del nivel 1 (13
  tramos: caminar, saltar los escalones, agacharse en el túnel, correr y
  saltar el foso, romper el tronco...).
- En el PC, `tests/prueba_recorrido.c` juega el nivel entero con ese guion.
- En la SNES, la **ROM de prueba** (`-DGUION_PRUEBA`) usa el mismo guion en
  `mando_leer` en vez del mando: Mesen muestra el recorrido sin que nadie
  pulse nada. Del mando real solo escucha SELECT (HUD) y START (salir).

La mini-ROM de esta unidad tiene su propio guion pequeño (`-DGUION_MANDO`).
Un detalle que viene del juego: la tabla se recorre con un **puntero que
avanza** (`t++`) y no con `guion[i]`. Con `guion[i]` y una `struct` de 5 bytes,
`tcc816` llama a su rutina de multiplicar (`tcc__mul`) en cada acceso para
calcular `i × 5`; en el juego eso eran 72 multiplicaciones por frame y la ROM
de prueba iba mucho más lenta (`src/juego/guion.c`, comentario de
`mantenidos`). Los tramos de la mini-ROM, además, miden 8 bytes (potencia de
2) con un campo de relleno.

## La librería: PVSnesLib

Cabecera: [input.h](https://alekmaul.github.io/pvsneslib/input_8h.html)
(firmas copiadas de `tools/pvsneslib/pvsneslib/include/snes/input.h`, 4.6.0).

| Nombre | Qué es | Qué hace | Hardware |
|---|---|---|---|
| `#define padsCurrent(value) (pad_keys[value])` | **macro**, no función | los botones mantenidos del mando `value` (0 = mando 1) en este frame | lee la variable que rellenó la NMI desde `JOY1L/H $4218/$4219` |
| `#define padsDown(value) (pad_keysdown[value])` | **macro** | los recién pulsados (0 → 1) | ídem |
| `unsigned short padsUp(u16 value);` | **función** | los recién soltados (1 → 0): `pad_keysold[value] & ~pad_keys[value]` | ídem |
| `void padsClear(u16 value);` | función | pone a 0 las tres variables de ese mando | — |
| `extern u16 pad_keys[5]; pad_keysold[5]; pad_keysdown[5];` | variables | lo que rellena la rutina de la NMI (hasta 5 mandos con el adaptador MultiPlayer 5) | — |
| `KEY_A ... KEY_Y` (`enum KEYPAD_BITS`) | constantes | la máscara de cada botón | — |
| `#define REG_JOYxLH(a) (((vuint16 *)0x4218)[(a)])` | macro | el registro de autolectura del mando `a`, por si se quiere leer a mano | `$4218..$421F` |

Que `padsCurrent` y `padsDown` sean macros significa que no cuestan una
llamada: `padsCurrent(0)` se convierte en `pad_keys[0]` antes de compilar.
`padsUp` sí es una llamada (y en esta versión la documentación la anuncia
como `\fn padsUp(u16 value)`; la firma real devuelve `unsigned short`).

La autolectura y la rutina de la NMI las deja en marcha `consoleInit()` (la
llama el arranque del SDK antes de `main`, unidad 03) a través de `nmiSet`
(unidad 04). Ejemplo del SDK: `tools/pvsneslib/snes-examples/input/controller/controller.c`
(base de la mini-ROM; ver arriba la pega de su `switch`).

## La práctica, paso a paso

### Paso 1 — La demo de PC

```
cd E:\ProyectoSNES\aprendizaje\unidad-05-mando
gcc -std=c99 -Wall -Wextra -o demo_05.exe demo_05_mascaras.c
.\demo_05.exe
```

Antes de ejecutarla, predice las columnas `down` y `up` de los frames 5 y 7.
Compara con `capturas/salida_demo_05_mascaras.txt`.

### Paso 2 — La mini-ROM con el mando

En `rom/`, con la orden de compilar de la unidad 03 (`make`): «No warnings or
errors found.» (`capturas/compilacion_rom.txt`). Ábrela en Mesen y prueba con
el teclado (asignación de este equipo, `docs/mesen-y-depuracion.md` §2):
flechas = cruceta, **A** = botón B, **S** = A, **Z** = Y, **X** = X, **Q** = L,
**W** = R, **D** = START, **E** = SELECT. Cuidado: la tecla A del teclado es
el botón **B** del mando.

```
 HELD:
B . ... ... .. .. .. RT . . . .        ← B y Derecha mantenidos

 LAST DOWN AT FRAME   812
. . ... ... .. .. .. RT . . . .        ← lo último que bajó, congelado

 LAST UP   AT FRAME   790
. . ... ... .. .. .. .. A . . .

 ACTION WALK

 PAD $8100  FRAME   845
```

- `HELD` cambia mientras mantienes; `LAST DOWN` y `LAST UP` se quedan con lo
  último que bajó o subió y su frame (un recién pulsado dura un frame: sin
  congelarlo no se vería).
- `ACTION`: pulsa A con una flecha mantenida: `ATTACK` (A gana a la
  dirección) durante medio segundo y luego otra vez `WALK`. Mantén abajo y
  una flecha: `CROUCH`.
- `PAD`: el número crudo. Con B solo, `$8000`; con B y Derecha, `$8100`.

### Paso 3 — La mini-ROM que se juega sola

```
$env:CFLAGS="-DGUION_MANDO"; make clean; make; $env:CFLAGS=$null
```

Sale `(SCRIPTED: NO HANDS)` bajo el título y los botones salen de la tabla
`GUION` de `botones.c`: B un segundo (frames 60–119), un toque de B (150),
Derecha con A en medio (200–259, A en el 220), otro toque de B (300) y Abajo +
Izquierda con R en medio (330–399, R en el 350). Comprueba en la pantalla la
prioridad: en el frame 220 `ATTACK` gana a `WALK`; en el 350 `SWORD` gana a
`CROUCH`. Vuelve a compilar sin `CFLAGS` al terminar (`make clean; make`).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| La `struct Mando` y qué campo es mantenido y cuál recién pulsado | `src/juego/mando.h` |
| `mando_leer`: `padsCurrent(0)`, `padsDown(0)` y la traducción con `!= 0` | `src/juego/mando.c` (`mando_leer`) |
| La rama `#ifdef GUION_PRUEBA` de `mando_leer`: el guion para la lógica, el pad real solo para SELECT y START | `src/juego/mando.c` |
| `guion_mando`: mantenidos de ahora y de antes, `nuevos = ahora & ~antes` | `src/juego/guion.c` |
| `mantenidos()` recorriendo la tabla con un puntero (y por qué) | `src/juego/guion.c` |
| `TramoGuion` y las máscaras propias `BOTON_*` (8 bits: el guion no necesita los 16) | `src/juego/guion.h` |
| El recorrido del tutorial: 13 tramos con el obstáculo que resuelve cada uno | `src/juego/guion_tutorial.c` |
| Dónde se llama: `mando_leer(&mando)` al principio de cada frame; START y SELECT antes que la lógica | `src/pantallas/pantalla_nivel1.c` (`pantalla_nivel1_actualizar`, paso 1) |
| La asignación de botones (tabla «mantenido / pulsado») | `docs/diseno-demo-jugable.md` §3 |
| La prioridad el mismo frame | `docs/diseno-demo-jugable.md` §4, «Reglas transversales» |
| El teclado de Mesen en este equipo | `docs/mesen-y-depuracion.md` §2 |

Fíjate en que `mando_leer` calcula `actividad` como
`(mantenidos | pulsados) != 0`: cualquier cosa tocada despierta a Tarou si
estaba dormido (unidad 02).

## Trampas

- **`(u8)(pad & KEY_B)` vale siempre 0.** Compara con `!= 0` antes de
  convertir.
- **`switch (pad)` con máscaras** solo funciona con un botón a la vez. Usa
  `&`.
- **Recién pulsado dura un frame.** Si lo dibujas en pantalla, congélalo o
  no se verá; si lo miras en un frame que no corre la lógica (por ejemplo,
  dentro de un fundido que bloquea), se pierde.
- **Mantener un botón no es pulsarlo:** contar con `padsCurrent` suma 60 por
  segundo.
- **`?:` con cadenas** como argumento de `consoleDrawText` → `error: ICE 2`
  en `tcc816`. Usa `if/else` (la mini-ROM elige el texto de `ACTION` en una
  tabla).
- **Un guion indexado con `guion[i]`** sobre structs de 5 bytes llama a
  `tcc__mul` en cada acceso: recorre con puntero o rellena a 8 bytes.
- **El guion de teclas de `capturar_mesen.ps1`** pierde teclas si se usa el
  equipo durante la tanda (el foco de Mesen salta a su menú). Para verificar
  sin manos, lo fiable es un guion dentro de la ROM (paso 3; la ROM de
  prueba del juego).
- **La tecla A del teclado es el botón B** del mando en este equipo.

## Ejercicio y criterio de aprobación

El ejercicio está en **`ejercicio_05.md`**: añadir a la mini-ROM un contador
de «cuántas veces se pulsó B». Solución: `solucion/botones.c` (compilada en
una copia aparte, con y sin `-DGUION_MANDO`: «No warnings or errors found.»;
`capturas/compilacion_variantes.txt`).

**Aprobación:**

1. La demo de PC compila sin avisos y su salida coincide con
   `capturas/salida_demo_05_mascaras.txt`.
2. Captura de Mesen de tu mini-ROM con los botones reflejados en `HELD`
   (al menos dos a la vez) y el contador de B.
3. Captura de la versión con `-DGUION_MANDO` al final del guion (a partir del
   frame 400, unos 7 s) con **`B PRESSES 3`**: tres pulsaciones aunque B
   estuvo mantenido 60 frames en la primera.

## Lo que viene

- **Unidad 10:** la física del salto, que usa `saltar_pulsado` para empezar
  y `saltar` (mantenido) para alargarlo.
- **Unidad 16:** la ROM de prueba con guion y las capturas automáticas, a
  fondo.
- **Unidad 20:** el guion del tutorial y cómo se ajusta con una traza de
  posiciones cuando cambia el nivel.
