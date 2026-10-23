# Unidad 01 — Enteros de 16 bits y punto fijo

> **Lección válida, juego histórico.** Esta unidad enseña sobre el código de
> la Revisión 2 (plataformas, tag `plataformas-r2`); el juego actual es un
> beat 'em up ([`docs/diseno-beat-em-up.md`](../../docs/diseno-beat-em-up.md)).
> Los archivos del juego que cita (`src/juego/tarou.h`, `src/juego/tarou.c`,
> `tests/prueba_tarou.c`, con sus números de línea) se leen en ese tag, por
> ejemplo con `git show plataformas-r2:src/juego/tarou.h`, o en un clon fijado
> en él:
>
> ```text
> git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u01
> git -C E:\ProyectoSNES_trabajo\u01 checkout -q plataformas-r2
> ```
>
> **Qué se traslada al beat 'em up:** todo el concepto. `puntofijo.h` se
> queda, x e y del carril siguen en 12.4, y la gravedad de este ejercicio es
> la del salto en z (`pantalla_y = y − z`, spec §3) y la de los lanzamientos.

**Objetivo:** hacer aritmética con decimales en una CPU que no los tiene, y
saber qué pasa cuando un entero de 8 o 16 bits se queda sin sitio. Al
terminar se entiende cada número de `src/sistema/puntofijo.h` y de las
constantes de movimiento de `src/juego/tarou.h`.

**Prerrequisitos:** unidad 00 (C en el PC, incluido su bloque de bits).
**Práctica:** PC (`gcc`). **Tiempo orientativo:** una tarde.

Todo se compila y se ejecuta en el PC, pero con **el mismo `puntofijo.h` que
usa la ROM**: las demos y el ejercicio lo incluyen directamente desde `src/`,
igual que las pruebas de `tests/`. Lo que aquí da 69 px es lo que da en la
consola.

## Antes de empezar: compilar con las cabeceras del juego

Las demos hacen `#include "sistema/puntofijo.h"`. Ese archivo incluye
`sistema/tipos.h`, que da `u8`/`u16`/`s16` de `<stdint.h>` **solo si está
definido `PRUEBAS_PC`** (unidad 00: `#ifdef`); si no, intenta traer `<snes.h>`,
que en el PC no existe. Por eso la línea de compilación lleva dos opciones más
que en la unidad 00:

```
cd E:\ProyectoSNES\aprendizaje\unidad-01-enteros-y-punto-fijo
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I..\..\src -o demo_01_enteros.exe demo_01_enteros.c
.\demo_01_enteros.exe
```

- `-DPRUEBAS_PC` define la macro `PRUEBAS_PC`, como si el archivo empezara
  por `#define PRUEBAS_PC`.
- `-I..\..\src` añade `E:\ProyectoSNES\src` a las carpetas donde se buscan
  los `#include "..."`. Así `"sistema/puntofijo.h"` se encuentra.

Son exactamente las opciones de `tests/Makefile`
(`-std=c99 -Wall -Wextra -DPRUEBAS_PC -I. -I../src ...`).

## Conceptos y cómo funciona

### Repaso de una línea: bits, bytes y hexadecimal

Un **bit** es un 0 o un 1, un **byte** son 8 bits, `0x` delante de un número
lo escribe en **hexadecimal** (base 16: un dígito por cada 4 bits, `0xFF` =
255) y `<<`/`>>` desplazan los bits a izquierda o derecha. Todo eso, con los
operadores `& | ^ ~`, está en la unidad 00; aquí se usa sin volver a
explicarlo.

### Rango de un tipo entero

El **rango** de un tipo son los valores que puede guardar. Con *n* bits hay
2ⁿ combinaciones (→ glosario: `u8`, `u16`, `s16`):

| Tipo | Bits | Rango | Combinaciones |
|---|---|---|---|
| `u8` | 8 | 0..255 | 256 |
| `u16` | 16 | 0..65535 | 65536 |
| `s16` | 16 | −32768..32767 | 65536 |

`u16` y `s16` tienen las **mismas 65536 combinaciones de bits**; lo único
que cambia es cómo se leen. Eso es el complemento a dos, dos apartados más
abajo.

### Desbordamiento (*overflow*)

**Desbordamiento** es lo que pasa cuando el resultado de una cuenta no cabe
en el tipo donde se guarda: los bits que sobran se pierden y queda solo la
parte baja. Un `u8` que vale 255 (`0xFF`, ocho unos) y suma 1 da `0x100`,
que necesita 9 bits; el noveno se tira y queda `0x00`. Paso 1 de
`demo_01_enteros.c`:

```
1. u8:  255 + 1 = 0
1. u8:  0 - 1 = 255
1. u16: 65535 + 1 = 0
```

Es como el cuentakilómetros de un coche viejo que pasa de 99999 a 00000. El
65816 (la CPU de la SNES) no avisa: **el desbordamiento es silencioso**. Por
eso el juego recorta los valores *antes* de que se salgan del rango, y no
después (lo verás en `mover_x`, más abajo).

### Signo: complemento a dos

Para guardar negativos en 16 bits, C (y el 65816, y casi todas las CPU) usa el
**complemento a dos**: el bit más alto (el 15) dice si es negativo, y un
negativo −*x* se guarda como 65536 − *x*. La receta práctica para negar un
número es **invertir todos sus bits y sumar 1**. Paso 2 de la demo:

```
2. s16      1 se guarda como 0x0001 (leido sin signo: 1)
2. s16     -1 se guarda como 0xFFFF (leido sin signo: 65535)
2. s16    -96 se guarda como 0xFFA0 (leido sin signo: 65440)
2. s16 -32768 se guarda como 0x8000 (leido sin signo: 32768)
2. ~96 + 1 = 0xFFA0, el mismo patron que -96
```

La ventaja que hace que todo el mundo lo use: **sumar y restar es la misma
operación con signo y sin signo**. La CPU suma bits y no necesita saber si el
número «es» negativo; el `s16` o el `u16` solo dicen a C cómo **leer** y
**comparar** el resultado. Por eso `t->x_fp += t->vx_fp` funciona igual con
una velocidad de +24 que de −24.

Con signo también se desborda, pero por el otro lado (paso 3):

```
3. s16: 32767 + 1 = -32768
```

`0x7FFF + 1 = 0x8000`, que leído con signo es el negativo más grande.

> Rigor de C: convertir a `s16` un valor que no cabe (`(s16)32768`) es
> «definido por la implementación» en el estándar. `gcc` y `tcc816` hacen lo
> mismo, quedarse con los 16 bits de abajo, y el proyecto cuenta con ello.

### Punto fijo

La SNES no tiene **coma flotante** (números con decimales como `1.5` en
hardware; en C, `float`), y el proyecto prohíbe `float`. Pero Tarou camina a
1 px por frame, corre a 2 y la gravedad le suma un cuarto de píxel cada frame.
La solución es el **punto fijo** (→ glosario: Punto fijo): guardar el número
**multiplicado por una potencia de 2** en un entero normal, y acordar que los
bits de abajo son la fracción.

En el juego se multiplica por 16 (2⁴): 4 bits de fracción. 1,0 px se guarda
como 16, 1,5 px como 24 y 0,25 px como 4. A ese formato se le llama **12.4**:
12 bits para la parte entera y 4 para la fracción, en un `s16`.

```
   bit 15                          4 3       0
   ┌──────────────────────────────┬──────────┐
   │   parte entera (12 bits)     │ fracción │      x_fp = 184 = 0x00B8
   │        0000 0000 1011        │   1000   │      entero 11, fracción 8/16
   └──────────────────────────────┴──────────┘      = 11,5 px
```

Las tres operaciones que importan:

- **Sumar, restar y comparar** valores 12.4 entre sí es sumar, restar y
  comparar enteros. `x_fp + vx_fp` ya es 12.4.
- **De entero a 12.4**: desplazar 4 a la izquierda (× 16).
- **De 12.4 al píxel**: desplazar 4 a la derecha (÷ 16, tirando la fracción).
  La fracción es lo que queda en los 4 bits de abajo: `x_fp & 15`.

El paso 1 de `demo_01_puntofijo.c` mueve un valor a 1,5 px por frame (24) y lo
enseña: el píxel avanza 1, 2, 1, 2... y la fracción guarda el medio píxel
que falta hasta que se completa.

```
1. inicio: x_fp=160 -> pixel 10, fraccion 0/16
1. frame 1: x_fp=184 -> pixel 11, fraccion 8/16
1. frame 2: x_fp=208 -> pixel 13, fraccion 0/16
1. frame 3: x_fp=232 -> pixel 14, fraccion 8/16
```

El sprite se dibuja en píxeles enteros, pero la física acumula la fracción:
en 8 frames recorre exactamente 12 px.

### `PF()` y `PF_ENTERO()`: el punto fijo del juego

`src/sistema/puntofijo.h:34-36` son tres líneas:

```c
#define PF_BITS 4                                   /* 4 bits de fracción = 1/16 px.       */
#define PF(n) ((s16)((n) << PF_BITS))               /* 1 → 16, 448 → 7168.                 */
#define PF_ENTERO(x) ((u16)((x) >> PF_BITS))        /* Solo para x >= 0: 7183 → 448.       */
```

- `PF(n)`: de píxeles enteros a 12.4. `PF(1)` es 16, `PF(144)` es 2304.
- `PF_ENTERO(x)`: de 12.4 al píxel. `PF_ENTERO(2212)` es 138.
- Las dos llevan sus paréntesis por la trampa de las macros de la unidad 00.

**Por qué 12.4 y no otra división.** 4 bits de fracción dan pasos de 1/16 px,
suficientes para la gravedad (4/16) y el andar agachado (8/16). Los 12 bits
enteros con signo llegan hasta 32767 / 16 = **2047,9 px**. El nivel 1 mide
2048 px, y la celda de Tarou (96 px de ancho) nunca empieza más allá de
1952 px. Paso 4 de `demo_01_enteros.c`:

```
4. PF(1952) = 31232 (X_MAX_FP de tarou.c: cabe)
4. PF(2047) = 32752 (el ultimo pixel entero que cabe)
4. PF(2048) = -32768 (el ancho del nivel ya NO cabe)
```

`PF(2048)` se desborda a negativo: el ancho del nivel **no se puede escribir
en 12.4**. El juego no lo necesita: `X_MAX_FP` (`src/juego/tarou.c:38`) es
`PF(ANCHO_NIVEL - ANCHO_CELDA)` = 31232, y `mover_x` recorta la posición ahí.
Un nivel de más de 2048 px obligaría a cambiar de formato (o a guardar la x en
otro tipo).

**Por qué `VEL_SALTO` es `(-96)` y no `PF(-6)`.** Desplazar a la izquierda un
número negativo es comportamiento indefinido en C99, y `gcc -Wextra` lo dice
(`capturas/aviso_pf_negativo.txt`):

```
warning: left shift of negative value [-Wshift-negative-value]
```

Por eso las constantes negativas de `tarou.h` se escriben ya multiplicadas:
`VEL_SALTO (-96)`, `VEL_SALTO_CORTE (-32)`. Las positivas sí usan `PF`:
`VEL_TERMINAL PF(6)`, `VEL_CAMINAR_MAX PF(1)`.

### Por qué las posiciones no son negativas: la trampa de `PF_ENTERO`

La regla del proyecto (comentario de `puntofijo.h`): las **posiciones** nunca
son negativas, solo las **velocidades** pueden serlo, y las velocidades nunca
se pasan a píxel. La razón es el `(u16)` de `PF_ENTERO`. Paso 3 de
`demo_01_puntofijo.c`:

```
3. y_fp=-16 -> PF_ENTERO da 65535 (no -1)
3. comparar en punto fijo: y_fp < 0 es 1 (correcto)
```

−16 (un píxel por encima de la fila 0) desplazado 4 bits da −1, y el `(u16)`
lo lee como 65535 (complemento a dos). Un código que hiciera
`if (PF_ENTERO(t->y_fp) < 10)` para «¿está arriba del todo?» diría que no
justo cuando Tarou ha saltado por encima de la pantalla. Dos defensas, las dos
en el juego:

1. **Recortar antes de convertir.** `mover_x` (`src/juego/tarou.c:416-425`)
   pone `x_fp` a 0 en cuanto se hace negativa, y a `X_MAX_FP` si se pasa.
2. **Comparar en punto fijo**, no en píxeles: `if (t->x_fp < 0)`,
   `if (y_fp < y_min_fp)`. Una comparación de dos `s16` respeta el signo.

> El desplazamiento en sí no es el problema. Medido el 2026-10-22: `tcc816`
> compila `v >> 4` de un `s16` como un desplazamiento **aritmético**
> (`cmp #$8000` + `ror`, que copia el bit de signo; `capturas/tcc816_costes.txt`)
> y `gcc` también. −16 >> 4 da −1 en los dos. Lo que lo estropea es leer ese
> −1 como `u16`.

### Punto fijo 8.8: el motor de escenas

El mismo truco con otra proporción. En **8.8** el byte alto es la parte
entera y el bajo la fracción, en 256avos: `0x0100` es 1,0 y `0x0080` es 0,5.
Lo usa el motor de escenas (`src/escena/escena.h`) para valores pequeños
(brillo 0..15, mosaico 0..15) que tienen que cambiar **menos de 1 por frame**.
El fundido de entrada del logo es `RAMPA(0, 32, PISTA_BRILLO, 0, 15)`: de 0 a
15 en 32 frames, 15/32 = 0,47 por frame. En 8.8 eso es un paso de 120
(120/256 = 0,47). Paso 4 de la demo:

```
4. paso de la rampa 0 -> 15 en 32 frames: 120 (= 120/256 por frame)
4. frame  4: valor=0x01E0 -> brillo 1
...
4. frame 32: valor=0x0F00 -> brillo 15
```

- El valor vive en `Pista.valor` (`u16`, `escena.h:92`) y se avanza con
  `p->valor = (u16)(p->valor + p->paso)` (`escena.c:88`).
- El entero se saca con `>> 8` (`ESCENA_VALOR`, `escena.h:111`): leer el byte
  alto.
- El último frame se pone **exacto** (`(u16)p->hasta << 8`, `escena.c:92`):
  31 × 120 = 3720 = `0x0E88` daría brillo 14, no 15. Redondear al final es más
  barato que calcular un paso perfecto.

¿Por qué 8.8 aquí y 12.4 en Tarou? Porque el brillo no pasa de 255 (8 bits
enteros sobran) y necesita pasos finos (1/256); las posiciones del nivel
necesitan 2047 px enteros y les basta con 1/16.

### El coste de `/` y `%` en el 65816

El 65816 sabe sumar, restar y desplazar bits, pero **no tiene instrucción de
multiplicar ni de dividir**. Cuando el C dice `a / b`, `tcc816` llama a una
**rutina** de su librería (`tcc__udiv` sin signo, `tcc__div` con signo) que
divide con un bucle de restas y desplazamientos. Medido con el compilador del
proyecto (`capturas/tcc816_costes.txt`, salida de `816-tcc` de PVSnesLib
4.6.0):

| C (con `a`, `b` `u16`; `v` `s16`) | Lo que genera `tcc816` |
|---|---|
| `a / b` | `jsr.l tcc__udiv` (una llamada a la rutina) |
| `a % b` | `jsr.l tcc__udiv` (la misma rutina; el resto sale en otro registro) |
| `a / 16` | cuatro `lsr a` (desplazar: barato) |
| `a % 16` | `and #15` (una máscara: barato) |
| `a >> 4` | cuatro `lsr a` (igual que `a / 16`) |
| `v / 16` (**con signo**) | `jsr.l tcc__div` (¡la rutina!) |
| `v >> 4` (con signo) | cuatro `cmp #$8000` + `ror a` |
| `a * 3` | `asl a` + `adc` (x·2 + x) |

Tres lecciones:

1. **Dividir entre una variable siempre llama a la rutina.** El proyecto lo
   pagó: el comentario de `numero()` en `src/juego/hud_debug.c:133-138` cuenta
   que con ~40 divisiones por frame (para pasar números a decimal) el juego
   bajaba a **un cuarto de su velocidad**. Ahora resta potencias de diez.
2. **Dividir entre una potencia de 2 sin signo es gratis**: el compilador lo
   cambia por desplazamientos o una máscara.
3. **Con signo, `/ 16` no es `>> 4`.** La división de C redondea hacia el 0
   (−1 / 16 = 0) y el desplazamiento hacia abajo (−1 >> 4 = −1), así que el
   compilador no puede cambiar una por otra y llama a `tcc__div`. Por eso
   `PF_ENTERO` está escrito con `>>` y no con `/ 16`.

La regla del proyecto (`CLAUDE.md`): **ninguna división en código que corre
cada frame**. Si una cuenta necesita dividir entre constantes, que la haga el
compilador al compilar: `DURACION_ATAQUE / TAROU_ATACAR_CUADROS` en
`src/juego/animaciones.c:41` y el paso de `RAMPA` son constantes, y en la ROM
no queda ninguna división.

> La CPU de la SNES (el chip 5A22, que lleva dentro el 65816) tiene además un
> multiplicador y un divisor por hardware en registros propios; `tcc816` no
> los usa. Los registros se ven en la unidad 13.

### `int` es de 16 bits en tcc816: `255 * 256` no cabe

En C, una cuenta entre enteros pequeños (`u8`, `s16`...) se hace en `int`. En
el PC un `int` mide 32 bits; en `tcc816`, **16** (medido: `sizeof(int) == 2`,
y también `sizeof(long) == 2`). Entonces 255 × 256 = 65280 no cabe en un
`int` de la SNES (máximo 32767). Paso 5 de `demo_01_enteros.c`, que simula en
el PC el `int` de 16 bits:

```
5. 255 * 256 a 16 bits = -256 (deberia ser 65280)
5. ... y dividido entre 32 = -8 (deberia ser 2040)
5. (255 * 128 / 32) * 2 a 16 bits = 2040 (correcto)
```

La macro `RAMPA` (`src/escena/escena.h:79-81`) calcula el paso 8.8 como
`(hasta - desde) * 128 / frames * 2` y no `* 256 / frames` para que el
producto intermedio quepa siempre en 16 bits (255 × 128 = 32640 ≤ 32767). Se
pierde 1/256 de precisión, que da igual porque el último frame se pone exacto.

> **Matiz medido el 2026-10-22.** Con PVSnesLib 4.6.0, cuando **todo son
> constantes** (como en las tablas de `RAMPA`), `tcc816` calcula el valor al
> compilar con más bits y acierta igual con `* 256` que con `* 128 ... * 2`
> (las dos tablas salen idénticas en `capturas/tcc816_costes.txt`). Pero en
> cuanto un operando es una **variable**, el producto se calcula en el frame a
> 16 bits (`xba` + `and #$ff00`) y desborda. El proyecto reparte la cuenta
> para no depender de lo que el compilador decida plegar: es la regla segura.

## Cómo se trabaja esta unidad

| Parte | Archivo | Qué practica |
|---|---|---|
| 1 | `demo_01_enteros.c` | desbordamiento, complemento a dos, límite del 12.4, `int` de 16 bits |
| 2 | `demo_01_puntofijo.c` | 12.4 a 1,5 px/frame, velocidad negativa, trampa de `PF_ENTERO`, 8.8 |
| 3 | `ejercicio_01_gravedad.c` | el salto completo de Tarou en 12.4 (TODO 1 a 6) |

1. Lee cada demo, **intenta predecir su salida** y después compílala y
   ejecútala. Las salidas de referencia están en `capturas/salida_demo_*.txt`.
2. Abre el ejercicio. Antes de escribir código, haz a mano los dos primeros
   frames (lo pide el propio archivo).
3. Completa los TODO, compila sin avisos y compara con la salida esperada.
4. Solo entonces mira `solucion/ejercicio_01_gravedad.c`.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| Las macros `PF_BITS`, `PF`, `PF_ENTERO` | `src/sistema/puntofijo.h:34-36` |
| Sus pruebas: `PF(1) == 16`, `PF_ENTERO(PF(100) + 15) == 100`, restar por debajo de 0 da negativo | `tests/prueba_puntofijo.c` |
| Constantes de movimiento en 12.4: `GRAVEDAD 4`, `VEL_TERMINAL PF(6)`, `VEL_SALTO (-96)` | `src/juego/tarou.h:45-55` |
| Campos `x_fp`, `y_fp`, `vx_fp`, `vy_fp` (`s16`, 12.4) | `src/juego/tarou.h` (`typedef struct ... Tarou`) |
| El límite derecho `X_MAX_FP` = `PF(1952)` = 31232 | `src/juego/tarou.c:38` |
| Gravedad: `t->vy_fp += GRAVEDAD`, recortada a `VEL_TERMINAL` (paso 2) | `tarou_actualizar`, `src/juego/tarou.c:545` |
| Integración: `t->y_fp += t->vy_fp` (paso 4) | `mover_y`, `src/juego/tarou.c:478` |
| Recorte antes de convertir: `if (t->x_fp < 0) t->x_fp = 0;` | `mover_x`, `src/juego/tarou.c:416-425` |
| `PF_ENTERO` para pasar a píxel antes de mirar la colisión | `mover_x` y `mover_y`, `src/juego/tarou.c` |
| Redondear al borde de una celda de 16 px con una máscara, sin dividir: `(pies_antes + 15) & ~15` | `mover_y`, `src/juego/tarou.c:482` |
| 8.8: `Pista.valor`, `RAMPA` con `* 128 ... * 2`, `ESCENA_VALOR` (`>> 8`) | `src/escena/escena.h:79-111` |
| 8.8 por frame y valor exacto al final | `escena_avanzar`, `src/escena/escena.c:85-93` |
| Pasar a decimal sin dividir (restando potencias de diez) | `numero`, `src/juego/hud_debug.c:133-173` |
| La prueba de la altura del salto | `prueba_apogeo_y_altura`, `tests/prueba_tarou.c:165-182` |

Un detalle para el ojo crítico: el comentario de `VEL_SALTO` en `tarou.h` dice
«~72 px de altura». Es la cuenta de la física continua (v² / 2g = 6² / (2 ·
0,25) = 72). El juego avanza a saltos de un frame y da **69**: es lo que mide
el ejercicio.

## Trampas

- **El desbordamiento no avisa.** Ni la SNES ni C se quejan: el valor da la
  vuelta. Recortar antes de que se salga del rango (`mover_x`).
- **`PF_ENTERO` solo con valores ≥ 0.** Un negativo da un número enorme
  (65535 para −16). Comparar en punto fijo con signo (`y_fp < 0`), nunca el
  píxel convertido. Es la trampa que más reaparece (unidad 20, colisión).
- **12.4 no llega a 2048.** `PF(2048)` es −32768. Si un nivel futuro es más
  ancho, hay que cambiar de formato.
- **`PF(-6)` es comportamiento indefinido.** Las constantes negativas se
  escriben ya en 12.4: `(-96)`.
- **El `int` de `tcc816` es de 16 bits.** Un producto intermedio de más de
  32767 desborda en la SNES aunque en el PC (y en las pruebas) dé bien. Las
  macros con constantes grandes se reparten (`* 128 ... * 2`).
- **Ninguna `/` ni `%` por frame.** Entre una variable o con signo llaman a
  `tcc__udiv`/`tcc__div`. Entre potencias de 2 y sin signo, el compilador las
  cambia por desplazamientos y máscaras.
- **Las pruebas de PC no ven ninguna de las dos últimas.** `gcc` tiene `int`
  de 32 bits y divide en hardware: una cuenta que desborda en la SNES pasa las
  pruebas en verde. Por eso esas reglas se cumplen por costumbre, no porque
  algo avise.

## Ejercicio y criterio de aprobación

`ejercicio_01_gravedad.c`: simular el salto de Tarou con `VEL_SALTO = -96` y
`GRAVEDAD = 4`, en el orden de `tarou_actualizar` (primero la gravedad,
después mover), desde y = 144 px (la celda con los pies en el suelo de las
pruebas).

**Aprobación:** el ejercicio compila con
`gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I..\..\src` **sin avisos** y su
salida es **idéntica** a `capturas/salida_ejercicio_01_gravedad.txt`. En
particular imprime:

```
Apogeo en el frame 23: y minima 75 px
Altura maxima: 69 px
Aterriza en el frame 47
Con el orden al reves: 75 px
```

69 px es lo que acepta `prueba_apogeo_y_altura` (`y` mínima entre 73 y 77 px
partiendo de 144: 144 − 69 = 75), y el frame 47 es el del comentario de
`prueba_aterrizaje` («Aterriza en el frame 47»). Con el orden al revés (mover
y después sumar la gravedad) el primer paso sería de −96 en vez de −92 y el
salto subiría 75 px: el orden de las dos líneas cambia 6 px de altura.

Para comparar sin hacerlo a ojo, en PowerShell:

```
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I..\..\src -o e1.exe ejercicio_01_gravedad.c
.\e1.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_01_gravedad.txt) -SyncWindow 0
```

Si `Compare-Object` no imprime nada, son iguales. Al terminar, borra `mia.txt`
y los `.exe`.

**Para ir más allá (sin solución):** cambia `GRAVEDAD` a 3 y predice la
altura antes de ejecutar. Después cambia `VEL_SALTO` a `(-80)`. ¿Qué
constante tocarías para un salto más alto pero igual de largo en el tiempo?

## Lo que viene

- **Unidad 02:** el `estado` de `Tarou` y las máquinas de estado: cuándo se
  pasa de `SALTANDO` a `CAYENDO` (cuando `vy_fp >= 0`, el frame 24 de tu
  ejercicio).
- **Unidad 10:** la física completa (aceleración, frenado, salto variable con
  `VEL_SALTO_CORTE`) sobre este mismo punto fijo.
- **Unidad 14:** leer el ensamblador de `capturas/tcc816_costes.txt` instrucción
  a instrucción.
