# Unidad 00 — C en el PC: del primer `if` a los bits, el preprocesador y los módulos

**Objetivo:** leer y escribir C sin miedo, con todo lo que hace falta para
entender cualquier archivo de `src/juego/`: variables, tipos de ancho fijo
(`u8`/`u16`/`s16`), `if`/`for`/`while`, funciones, arrays, `struct`,
punteros, **operadores de bits y máscaras**, **el preprocesador**
(`#define`, macros, `#include`, `#ifdef`, `-D`), **programas de varios
archivos** (`.h`/`.c`, prototipos, `extern`, `static`) y **`enum`, `switch` y
tablas `const`**.

**Prerrequisitos:** ninguno. **Práctica:** PC (`gcc`). **Tiempo orientativo:**
cuatro o cinco tardes: una o dos para las partes 1–4 y otras dos o tres para
las partes 5–8.

Todo en esta unidad se compila y se ejecuta en el PC. No hay nada de SNES
todavía: primero el lenguaje y después la consola. Al final, en «Dónde está en
el juego», se ve que el código del protagonista, del mando y de la secuencia
de pantallas usa exactamente estas piezas.

## Antes de empezar: compilar un programa

Un **programa en C** es un archivo de texto `.c`. Un **compilador** es el
programa que lo traduce a instrucciones que la CPU entiende y deja un
ejecutable (`.exe` en Windows). En el PC el compilador es **`gcc`**, y en este
equipo ya está instalado (WinLibs). Para comprobarlo, en PowerShell:

```
gcc --version
```

Para compilar y ejecutar cualquier archivo de esta unidad, desde esta carpeta:

```
cd E:\ProyectoSNES\aprendizaje\unidad-00-c-en-el-pc
gcc -std=c99 -Wall -Wextra -o demo_01.exe demo_01_logica.c
.\demo_01.exe
```

- `-std=c99` fija la versión del lenguaje (C de 1999). Es la misma que usan
  las pruebas de PC del juego (`tests/Makefile:20`).
- `-Wall -Wextra` activan los **avisos** (*warnings*): mensajes del
  compilador sobre código que compila pero probablemente está mal. La regla
  del proyecto es **cero avisos**.
- `-o demo_01.exe` es el nombre del ejecutable que se crea. Los `.exe` no se
  guardan en git (`.gitignore`).

Dos partes se compilan de otra manera, y cada una lo explica: la 6 añade
opciones `-D` a la línea y la 7 pasa **varios** `.c` a la vez.

## Conceptos de las partes 1–4

### Programa, `main` y `printf`

Todo programa de C empieza a ejecutarse en la **función `main`** (qué es una
función se explica más abajo). `#include <stdio.h>` pega al principio del
archivo las declaraciones de la biblioteca estándar de entrada/salida, entre
ellas `printf`, que escribe texto en la consola. Dentro del texto, `%d` es un
hueco que se rellena con un número entero y `\n` es un salto de línea:

```c
printf("Tarou empieza con %d vidas.\n", vidas);   /* Tarou empieza con 3 vidas. */
```

Otros huecos que aparecen en la unidad: `%u` (entero sin signo), `%s` (una
cadena de texto), `%X` (hexadecimal, parte 5) y `%02X` (hexadecimal con 2
cifras como mínimo, rellenando con ceros).

En la SNES no hay consola ni `printf`: el juego escribe texto en un fondo de
la pantalla (`consoleDrawText`, unidad 06). Pero en el PC `printf` es la
manera más rápida de ver qué hace un programa.

> En esta unidad las cadenas de `printf` van **sin acentos ni eñes**
> («anios», «choco»): la consola de Windows no siempre los muestra bien y la
> salida dejaría de coincidir con la esperada. Los acentos van en los
> comentarios, que el compilador ignora.

### Comentario

Texto para personas que el compilador ignora: `/* así */` (puede ocupar
varias líneas) o `// así` (hasta el final de la línea). Regla del proyecto:
**cada línea de código del juego lleva su comentario en español**.

### Variable y tipo

Una **variable** es un trozo de memoria con nombre que guarda un valor que
puede cambiar. Su **tipo** dice cuántos bytes ocupa y qué valores admite. Un
**bit** es un 0 o un 1, y un **byte** son 8 bits (la parte 5 los mira de
cerca).

```c
int vidas = 3;        /* declara la variable vidas, de tipo int, con valor 3 */
vidas = vidas - 1;    /* le asigna un valor nuevo: 2                          */
```

`int` es el entero «por defecto» de C, pero **su tamaño depende del
compilador**: con `gcc` en el PC ocupa 4 bytes y con `tcc816`, el compilador
de la SNES, solo 2. Por eso el juego no usa `int` y usa tipos cuyo tamaño
está escrito en el nombre (→ glosario: `u8`, `u16`, `s16`):

| Tipo | Bytes | Valores | En el juego |
|---|---|---|---|
| `u8` | 1 | 0..255 | estados, contadores pequeños, banderas 0/1 |
| `u16` | 2 | 0..65535 | contadores de frames, posiciones en píxeles, el mando |
| `s16` | 2 | −32768..32767 | velocidades y posiciones en punto fijo (pueden ser negativas) |

La `u` es *unsigned* (sin signo) y la `s` *signed* (con signo). Hay dos razones
para usarlos:

1. **El mismo resultado en los dos compiladores.** La lógica del juego
   (`src/juego/tarou.c`, `camara.c`...) se compila dos veces: con `tcc816`
   para la ROM y con `gcc` para las pruebas de `tests/`. Si un tipo midiera
   distinto en cada uno, las pruebas del PC no demostrarían nada de la SNES.
2. **La CPU de la SNES (el 65816) trabaja con 8 o 16 bits**, y la consola solo
   tiene 128 KB de RAM. Un valor de 16 bits es su tamaño natural, y un byte
   ahorrado es un byte que queda para otra cosa.

¿De dónde salen esos nombres? En el PC, de cuatro líneas que se copian al
principio del archivo (las mismas que `src/sistema/tipos.h:33-38`; qué hace
`typedef` se explica con las `struct`):

```c
#include <stdint.h>          /* enteros de tamaño fijo del C estándar */
typedef uint8_t u8;          /* u8 es otro nombre para uint8_t        */
typedef uint16_t u16;
typedef int16_t s16;
```

**Qué usa cada parte.** Las partes 1 y 2 usan `int` a propósito: son los
primeros programas y `int` es lo que se ve en cualquier libro de C (y las
soluciones 1 y 2 son las que escribió el autor cuando aprendía). **Desde la
parte 3, todo usa los tipos del juego.** La `demo_03` imprime cuánto mide
cada uno en este PC:

```
Bytes: u8=1 u16=2 s16=2 int=4
```

Un `u8` o un `u16` se imprime con `%d` igual que un `int`: al pasar a
`printf`, C los convierte a `int`.

### Expresión y sentencia

Una **expresión** es algo que se calcula y da un valor: `vidas - 1`,
`edad < 3`, `duplicar(vidas)`. Una **sentencia** es una orden completa y
termina en `;`: `vidas = vidas - 1;`. Un **bloque** es un grupo de
sentencias entre llaves `{ }`.

### Decidir: `if`, `else if`, `else` (y el operador `?:`)

`if (condición) { ... }` ejecuta el bloque solo si la condición es cierta. En
C, «cierto» es cualquier valor distinto de 0 y «falso» es 0. Los operadores
de comparación son `<`, `<=`, `>`, `>=`, `==` (igual) y `!=` (distinto), y
se combinan con `&&` (y), `||` (o) y `!` (no). Una comparación vale 1 si es
cierta y 0 si no.

```c
if (edad < 3)       { /* cachorro */ }
else if (edad < 8)  { /* adulto   */ }
else                { /* viejo    */ }
```

**La trampa clásica:** `=` **asigna** y `==` **compara**. `if (edad = 0)`
compila, pone `edad` a 0 y nunca entra. El autor del proyecto se encontró con
este error resolviendo el ejercicio 1, y su nota está en
`solucion/ejercicio_01_logica.c`. Con `-Wall`, `gcc` avisa de este caso.

El **operador condicional** `condición ? a : b` es un `if` dentro de una
expresión: vale `a` si la condición es cierta y `b` si no.
`(f > 0) ? PAD[f - 1] : 0` aparece en el ejercicio 5 y `MAX(a, b)` del
ejercicio 6 se escribe con él. En el juego hay una prohibición concreta: un
`?:` que elige entre dos **cadenas** como argumento de `printf` o de
cualquier función de argumentos variables hace fallar a `tcc816` (CLAUDE.md).
La `demo_08` usa una tabla de nombres en su lugar.

### Repetir: `for` y `while`

`for (inicio; condición; paso) { ... }` hace el inicio una vez y después
repite «si la condición es cierta, ejecuta el bloque y da el paso»:

```c
for (u8 i = 0; i < 5; i++) { ... }   /* i vale 0, 1, 2, 3, 4 */
```

`i++` suma 1 a `i`, igual que `i = i + 1`. Una variable declarada dentro del
`for` (`u8 i`) solo existe dentro de él.

`while (condición) { ... }` repite mientras la condición sea cierta. Es un
`for` sin inicio ni paso. En la SNES, el corazón de todo juego es un bucle que
no termina nunca (`while (1)` en `src/sistema/main.c`, unidad 04).

### Función: parámetros, valor devuelto y `void`

Una **función** es un bloque de código con nombre que se puede llamar desde
otro sitio. Recibe **parámetros** (valores de entrada, entre paréntesis) y
puede **devolver** un valor con `return`. El tipo escrito antes del nombre es
el tipo de lo que devuelve:

```c
int duplicar(int numero) { return numero * 2; }   /* recibe un int, devuelve un int */
void saludarTarou(int edad) { printf(...); }      /* void: no devuelve nada         */
```

Al **llamarla**, los valores que se pasan se llaman **argumentos**:
`duplicar(vidas)`. `void` significa «nada»: una función `void` no devuelve
valor (solo tiene un efecto) y `main(void)` no recibe parámetros. La
**firma** o **prototipo** de una función es su primera línea terminada en
`;` (`int duplicar(int numero);`); anuncia que la función existe para poder
llamarla antes de que aparezca su cuerpo. Los `.h` del juego están llenos de
prototipos (parte 7; → glosario: `void`).

### Array e índice

Un **array** es una fila de variables del mismo tipo, seguidas en memoria, con
un solo nombre. Cada una se elige con su **índice** entre corchetes, que
**empieza en 0**:

```c
u16 puntaje[5] = {120, 340, 90, 500, 275};  /* puntaje[0] es 120, puntaje[4] es 275 */
```

Un array de 5 elementos tiene índices 0..4: `puntaje[5]` está fuera y C **no
lo comprueba**; lee o escribe la memoria de al lado. En la SNES casi todo son
arrays: un tile son 32 bytes seguidos, un tilemap es un array de `u16`, las
piezas de un metasprite son un array de structs. **`sizeof`** dice cuántos
bytes ocupa un tipo o una variable: `sizeof(puntaje)` es 10 (5 × 2).

### Conversión de tipo (*cast*)

Escribir un tipo entre paréntesis delante de una expresión la convierte a ese
tipo: `(s16)(p->x + 4)`. En C, una suma de dos `s16` se calcula como `int`, y
el *cast* deja claro (al compilador y al lector) que el resultado vuelve a
caber en un `s16`. El código del juego está lleno de ellos (por ejemplo
`(u8)(m->izquierda || m->derecha)` en `src/juego/tarou.c:162`).

### `struct` y `typedef`

Una **`struct`** agrupa varias variables con nombre (sus **campos**) en una
sola. Se lee un campo con un punto: `tarou.x`.

```c
typedef struct
{
    s16 x, y;       /* posición */
    s16 vx;         /* velocidad */
} Personaje;

Personaje tarou = {100, 144, 2};   /* los valores, en el orden de los campos */
tarou.x = tarou.x + tarou.vx;      /* 102 */
```

**`typedef`** da un nombre nuevo a un tipo. Sin él habría que escribir
`struct Personaje` cada vez; con `typedef struct { ... } Personaje;` se
escribe `Personaje` a secas. Es lo que hace el juego: en `src/juego/tarou.h`
el protagonista es `typedef struct { ... } Tarou;` y se escribe `Tarou`, no
`struct Tarou`. Y es lo que hace `src/sistema/tipos.h` con los enteros:
`typedef uint8_t u8;` (en el PC; en la SNES los da `<snes.h>`, que en
`snestypes.h` dice `typedef unsigned char u8;`).

`sizeof(Personaje)` dice cuántos bytes ocupa (6: tres `s16`).

### `enum` (primer contacto)

Un **`enum`** da nombre a números enteros: `enum { DIR_DERECHA = 0,
DIR_IZQUIERDA = 1 };`. El código dice `DIR_IZQUIERDA` en vez de un `1`
misterioso. El ejercicio 4 lo usa así; la parte 8 lo ve a fondo.

### Puntero: `&`, `*` y `->`

Cada variable vive en una **dirección** de memoria (un número que dice dónde
está). Un **puntero** es una variable que guarda una dirección:

```c
u8 vidas = 3;
u8 *pv = &vidas;     /* u8 *: "puntero a u8". &vidas: la dirección de vidas */
*pv = 5;             /* *pv: "lo que hay donde apunta pv". Ahora vidas vale 5 */
```

- `&x` es **la dirección de** `x`.
- `*p` es **lo apuntado por** `p` (leerlo o escribirlo es leer o escribir el
  original).
- `p->campo` es el campo de la struct a la que apunta `p`; es lo mismo que
  `(*p).campo`.

**Por qué importan: por valor frente a por puntero.** Cuando se pasa una
variable a una función, C le pasa **una copia** (paso *por valor*): si la
función la cambia, cambia la copia y el original sigue igual. Si se le pasa
su dirección (paso *por puntero*), la función puede cambiar el original. Los
pasos 3 y 4 de `demo_04_structs_punteros.c` lo muestran con la misma struct:

```
3. dentro de mover_copia la copia vale x=102
3. despues de mover_copia el original sigue en x=100
4. frame 1: x=102
```

Todo el código del protagonista funciona así: `tarou_actualizar(Tarou *t, ...)`
recibe un puntero y escribe `t->x_fp`, `t->estado`... sobre la única `Tarou`
que existe, `static Tarou tarou;` en `src/pantallas/pantalla_nivel1.c:94`.

**`const`** delante del tipo del puntero (`const Tarou *t`) promete que la
función **solo lee** lo apuntado; si intentara escribir, el compilador daría
error. **Parámetro de salida:** una función que tiene que «devolver» una
struct suele recibir un puntero a donde escribirla. `tarou_caja_cuerpo(const
Tarou *t, Caja *c)` lee al protagonista (entrada, `const`) y rellena la caja
(salida, `c`).

Un puntero también sirve para **recorrer** un array: `e++` sobre un puntero
a struct avanza una struct entera (no un byte). La `demo_08` recorre así una
tabla de eventos hasta su marca de fin.

El paso 6 de la `demo_04` adelanta la trampa de los paréntesis en las macros,
que la parte 6 explica entera.

## Parte 5 — Bits: binario, hexadecimal y máscaras

### Binario y hexadecimal

Un número se guarda en bits. En **binario** (base 2) cada cifra es un bit, y
cada posición vale el doble que la de su derecha: 1, 2, 4, 8, 16... El bit de
la derecha es el **bit 0** (el de menor peso) y en un `u16` el de la
izquierda es el **bit 15**.

```
202 en binario:  1100 1010
                 │││  │ │└ bit 0 = 0
                 ││└──┴─┴── bits 3 (8) y 1 (2) encendidos
                 └┴─────── bits 7 (128) y 6 (64) encendidos
128 + 64 + 8 + 2 = 202
```

El binario es largo de escribir. El **hexadecimal** (base 16) usa las cifras
0–9 y A–F (A = 10 ... F = 15), y **cada cifra hexadecimal son exactamente 4
bits**: `1100 1010` = `C` `A` = `0xCA`. En C, un número que empieza por `0x`
está en hexadecimal. Por eso el código de la SNES escribe las máscaras en
hex: `0x2000` se lee de un vistazo como «solo el bit 13» (`0010 0000 0000
0000`). C99 no tiene una forma estándar de escribir binario en el código
(`0b...` es una extensión) ni de imprimirlo, así que la `demo_05` trae su
propia función `imprimir_binario`.

### Los seis operadores de bits

Trabajan bit a bit, cada posición por separado (salida real de la `demo_05`,
con `a = 0xCA` y `b = 0x0F`):

| Operador | Nombre | Regla por bit | Ejemplo | Resultado |
|---|---|---|---|---|
| `a & b` | Y (AND) | 1 si los dos son 1 | `1100 1010 & 0000 1111` | `0000 1010` |
| `a \| b` | O (OR) | 1 si alguno es 1 | `1100 1010 \| 0000 1111` | `1100 1111` |
| `a ^ b` | O exclusivo (XOR) | 1 si son distintos | `1100 1010 ^ 0000 1111` | `1100 0101` |
| `~a` | NO (NOT) | da la vuelta a cada bit | `~1100 1010` | `0011 0101` |
| `a << n` | desplazar a la izquierda | corre los bits n sitios; entran ceros | `0x05 << 2` | `0x14` |
| `a >> n` | desplazar a la derecha | corre los bits n sitios; se pierden los de la derecha | `0x50 >> 4` | `0x05` |

No hay que confundirlos con los lógicos: `&&`, `||` y `!` miran el número
entero (cero o no cero) y dan 0 o 1; `&`, `|` y `~` trabajan bit a bit.

Desplazar a la izquierda n sitios multiplica por 2ⁿ, y a la derecha divide
(si el número no es negativo): `x >> 4` es `x / 16`. El juego lo usa para
pasar de píxeles a celdas de 16 px (`src/juego/colision.c:65`). Qué pasa
cuando los bits se salen por la izquierda, con los negativos y cuánto
cuestan `/` y `%` en la SNES es tema de la unidad 01.

**Cuidado con `~` y los tipos pequeños:** antes de operar, C convierte un
`u8` o un `u16` a `int` (32 bits en `gcc`), así que `~a` enciende también los
bits 8..31. Se recorta con un cast: `(u8)~a`, `(u16)(valor & ~mascara)`.

### Máscaras: probar, encender, apagar, alternar

Una **máscara** es un número con encendidos justo los bits que interesan. Con
una máscara y un operador se hace cualquier cosa con esos bits sin tocar los
demás. Con la macro del SDK `BIT(n)` (`(1 << n)`, `snestypes.h:34`):

| Quiero... | Se escribe | Por qué |
|---|---|---|
| saber si el bit está encendido | `(x & BIT(3)) != 0` | AND deja solo ese bit; si queda algo, estaba |
| encenderlo | `x = x \| BIT(3)` | OR con 1 pone 1; con 0 deja lo que había |
| apagarlo | `x = x & ~BIT(3)` | la máscara al revés tiene un 0 justo ahí |
| alternarlo | `x = x ^ BIT(3)` | XOR con 1 da la vuelta; con 0 deja igual |

Así se guardan **ocho datos de sí/no en un solo byte**, o doce botones en un
`u16`.

### Ejemplo 1: el mando

El SDK lee el mando en cada VBlank y deja una palabra de 16 bits con un bit
por botón (`input.h:49-63`):

```
bit:   15 14  13     12    11 10  9    8     7  6  5  4   3..0
       B  Y  SELECT START  ↑  ↓   ←    →     A  X  L  R   (siempre 0)
```

`KEY_B` es `BIT(15)` = `0x8000`, `KEY_RIGHT` es `BIT(8)` = `0x0100`. Con
derecha y B pulsados a la vez la palabra vale `0x8100`. Para saber si B está
pulsado, `src/juego/mando.c` hace exactamente lo de la tabla de arriba:

```c
m->saltar = (u8)((mantenidos & KEY_B) != 0);   /* 1 si el bit 15 está encendido */
```

**Recién pulsado** es «encendido ahora y apagado en el frame anterior»: una
sola expresión, `ahora & ~antes`. Es la línea `src/juego/guion.c:44`, con la
que la ROM de prueba saca los «recién pulsados» de su guion. El paso 4 de la
`demo_05`:

```
4. ahora           = 0x8100 = 1000 0001 0000 0000
4. ahora & ~antes  = 0x8000 = 1000 0000 0000 0000
```

### Ejemplo 2: el byte de atributos de un sprite

Cada sprite de la OAM (la tabla de sprites de la SNES, unidad 08; →
glosario: OAM) tiene un byte de atributos con cinco datos dentro. Su forma
la da la macro `OAM_ATTR` del SDK (`sprite.h:228`):

```
bit:     7       6       5  4        3  2  1      0
       vflip   hflip   prioridad    paleta     bit 8 del tile
```

Armarlo es desplazar cada dato a su sitio y unirlos con `|`; leer un campo es
desplazarlo a la derecha y quedarse con sus bits con `&`:

```c
attr = 0x40 | (prioridad << 4) | (paleta << 1) | ((tile >> 8) & 1);   /* 0x67 */
paleta = (attr >> 1) & 7;                                            /* 3    */
```

Voltear un sprite es alternar el bit 6: `src/juego/metasprite.c:40` hace
`props | METASPRITE_HFLIP` (con `METASPRITE_HFLIP` = `0x40`). Y el HUD de
depuración pone el bit de prioridad de un tile del fondo con
`| 0x2000` (`src/juego/hud_debug.c:85`).

### La librería: PVSnesLib

En esta unidad no se llama a la librería (todo es PC), pero la `demo_05` y el
ejercicio 5 copian sus valores. Lo que se usará desde la unidad 05 (el mando)
y la 08 (los sprites):

| Nombre (firma exacta) | Qué es | Hardware | Cabecera |
|---|---|---|---|
| `KEY_A`, `KEY_B`... `KEY_Y` (`typedef enum KEYPAD_BITS`) | un bit por botón | la palabra del mando | [`input.h`](https://alekmaul.github.io/pvsneslib/input_8h.html) |
| `#define padsCurrent(value) (pad_keys[value])` | **macro**, no función: los botones mantenidos del mando `value` | autolectura del joypad (`$4218/$4219`) | [`input.h`](https://alekmaul.github.io/pvsneslib/input_8h.html) |
| `#define padsDown(value) (pad_keysdown[value])` | macro: los que bajaron en este frame | ídem | [`input.h`](https://alekmaul.github.io/pvsneslib/input_8h.html) |
| `#define BIT(n) (1 << n)` | el bit `n` encendido | ninguno | `snestypes.h` |
| `#define OAM_ATTR(priority, hflip, vflip, gfxoffset, paletteoffset) ...` | arma el byte de atributos | OAM | [`sprite.h`](https://alekmaul.github.io/pvsneslib/sprite_8h.html) |
| `void oamSet(u16 id, u16 xspr, u16 yspr, u8 priority, u8 hflip, u8 vflip, u16 gfxoffset, u8 paletteoffset);` | escribe una entrada de la OAM; arma ese byte por dentro | OAM (copia en RAM) | [`sprite.h`](https://alekmaul.github.io/pvsneslib/sprite_8h.html) |

Ejemplo del SDK: `tools/pvsneslib/snes-examples/input/controller/controller.c`
compara la palabra del mando con cada `KEY_*` en un `switch` (parte 8).

## Parte 6 — El preprocesador

El **preprocesador** es un paso que corre **antes** de compilar: lee las
líneas que empiezan por `#` y **pega texto**. No sabe C ni calcula nada; el
compilador recibe el resultado. Se puede ver ese resultado con
`gcc -E -P archivo.c` (`-E`: parar después del preprocesador; `-P`: sin las
líneas de marca). La `demo_06_expandir.c` existe solo para eso; su salida
real (`capturas/salida_demo_06_expandir_E.txt`):

```c
int main(void)
{
    int a = ((3) * 60);
    int b = ((2 + 1) * (2 + 1));
    int c = 2 + 1 * 2 + 1;
    int d = 0;
    return (a - 180) + (b - 9) + (c - 5) + d;
}
```

### `#define`: constantes

`#define NOMBRE valor` pide al preprocesador que reemplace cada `NOMBRE` por
`valor`. Así se escriben las constantes: `#define GRAVEDAD 4`. Una constante
puede usar otras:
`#define PANTALLAS_NIVEL (ANCHO_NIVEL / ANCHO_PANTALLA)`. Esa división no
cuesta nada en la SNES: son dos números fijos y **el compilador la hace al
compilar** (la regla «sin divisiones por frame» de CLAUDE.md es para
divisiones entre variables). Lo mismo hace `src/juego/animaciones.c` con
`DURACION_ATAQUE / TAROU_ATACAR_CUADROS`.

Por costumbre, las constantes van en MAYÚSCULAS. Las del protagonista están
en `src/juego/tarou.h:45-55` (`VEL_CAMINAR_MAX`, `GRAVEDAD`, `VEL_SALTO`...),
y las de los tiempos de pantalla en `src/sistema/pantallas.h`
(`#define DURACION_NINTENDO SEGUNDOS(3)`). (→ glosario: `#define`.)

### Macros con parámetros y la trampa de los paréntesis

Una **macro** es un `#define` con parámetros: `#define CUADRADO(n) ((n) * (n))`.
Al usarla, el preprocesador pega el **texto** del argumento donde dice `n`,
sin calcularlo. Por eso hay **dos** juegos de paréntesis obligatorios:

1. **Alrededor de cada parámetro.** Con `#define CUADRADO_MAL(n) n * n`,
   `CUADRADO_MAL(2 + 1)` se convierte en `2 + 1 * 2 + 1`, que da 5 y no 9.
   (El paso 6 de la `demo_04` lo enseña con `DOBLE_MAL`.)
2. **Alrededor de todo.** Con `#define SUMA_MAL(a, b) (a) + (b)`,
   `SUMA_MAL(1, 2) * 10` se convierte en `(1) + (2) * 10`, que da 21 y no 30.

Salida real de la `demo_06`:

```
2. CUADRADO(2 + 1)     = 9
2. CUADRADO_MAL(2 + 1) = 5
2. SUMA(1, 2) * 10     = 30
2. SUMA_MAL(1, 2) * 10 = 21
2. OBJ_PRIO(prio)      = 32
2. OBJ_PRIO(prio & 3)  = 0
2. OBJ_PRIO((prio & 3)) = 32
```

Las tres últimas líneas son **un caso real del SDK**: `sprite.h:60` dice
`#define OBJ_PRIO(prio) (prio << 4)`, sin paréntesis en el parámetro.
`OBJ_PRIO(prio & 3)` se convierte en `(prio & 3 << 4)`, y como `<<` va antes
que `&`, se calcula `prio & 48`: 0 en vez de 32. Lo mismo pasa con
`OBJ_PAL(palofs)` y con `BIT(n)`. Al usar macros ajenas, un argumento que no
sea un número o un nombre suelto se pasa **entre paréntesis**. Las macros del
juego llevan los suyos: `PF(n)` en `src/sistema/puntofijo.h:35`,
`SEGUNDOS(s)` en `pantallas.h`, `TILE(c)` en `src/juego/hud_debug.c:85`.

Una trampa más, que la `demo_06` no ejecuta: en `MAX(a, b)` escrito como
`((a) > (b) ? (a) : (b))` cada argumento aparece **dos veces**, así que un
argumento que hace algo (una llamada a una función que cambia cosas) se
ejecuta dos veces. A una macro se le pasan valores, no acciones.

### `#include` y guarda de inclusión

`#include "archivo.h"` pega ahí el contenido de ese archivo; con comillas lo
busca junto al `.c` (y en las carpetas que se den con `-I`), con `< >` entre
los del sistema. Un **`.h`** (cabecera) contiene lo que otros archivos
necesitan saber: tipos, constantes y prototipos (parte 7). Si un `.h` se
pegara dos veces en el mismo `.c`, sus tipos se definirían dos veces y no
compilaría. La **guarda de inclusión** lo evita:

```c
#ifndef TAROU_H      /* si todavía no está definido TAROU_H...       */
#define TAROU_H      /* ...defínelo y pega el resto del archivo       */
...
#endif               /* la segunda vez, TAROU_H ya existe: no pega nada */
```

La `demo_06` incluye `demo_06_caja.h` **dos veces seguidas** a propósito y
compila. Sin las tres líneas de la guarda, `gcc` dice
(`capturas/error_sin_guarda.txt`):

```
demo_06_caja.h:30:8: error: redefinition of 'struct Caja'
```

En la vida real nadie escribe el mismo `#include` dos veces: pasa sin querer,
cuando `main.c` incluye `tipos.h` y además `reloj.h`, que también lo incluye
(parte 7). Todo `.h` del proyecto lleva su guarda (→ glosario: `#ifndef`).

### `#ifdef`, `#ifndef`, `#else`, `#endif` y `-D`

Estas líneas deciden **qué código llega al compilador**. `#ifdef NOMBRE`
deja pasar lo que sigue solo si `NOMBRE` está definido; `#ifndef`, solo si
no; `#else` es la otra rama y `#endif` cierra. Lo que queda fuera no se
compila: no ocupa sitio en el programa ni tiene que ser correcto para la
otra máquina.

Un nombre se puede definir **desde la línea de `gcc`**, sin tocar el archivo:
`-DNOMBRE` equivale a `#define NOMBRE 1` y `-DNOMBRE=5` a `#define NOMBRE 5`.
El juego lo usa dos veces:

- **`PRUEBAS_PC`**: `tests/Makefile:20` compila con `-DPRUEBAS_PC`, y
  `src/sistema/tipos.h:33` usa `#ifdef PRUEBAS_PC` para tomar `u8`/`u16`/`s16`
  de `<stdint.h>` (en el PC) o de `<snes.h>` (en la ROM).
- **`GUION_PRUEBA`**: la ROM que se juega sola se compila con
  `CFLAGS="-DGUION_PRUEBA"`, y `src/juego/mando.c` cambia el mando de verdad
  por el guion del recorrido dentro de sus `#ifdef GUION_PRUEBA`.

Un patrón útil es el **valor por defecto que `-D` puede cambiar**:

```c
#ifndef VIDAS_INICIALES      /* si nadie la definió con -D...   */
#define VIDAS_INICIALES 3    /* ...vale 3                       */
#endif
```

La `demo_06` se compila de las dos maneras y sus capturas lo muestran
(`salida_demo_06_preprocesador.txt` y `..._con_D.txt`):

```
gcc -std=c99 -Wall -Wextra -o demo_06.exe demo_06_preprocesador.c
4. GUION_PRUEBA sin definir: el mando lo da el jugador
5. VIDAS_INICIALES = 3

gcc -std=c99 -Wall -Wextra -DGUION_PRUEBA -DVIDAS_INICIALES=5 -o demo_06d.exe demo_06_preprocesador.c
4. GUION_PRUEBA definido: el mando lo da el guion
5. VIDAS_INICIALES = 5
```

Sin el `#ifndef`, un `#define VIDAS_INICIALES 3` del archivo chocaría con el
de la línea de `gcc` y daría el aviso «`"VIDAS_INICIALES" redefined`» (el
esqueleto del ejercicio 6 lo da a propósito hasta que se resuelve el TODO 4).

## Parte 7 — Programas de varios archivos

### `.h` frente a `.c`

Un programa grande se parte en **módulos**: un par `.h`/`.c` por tema. El
**`.h`** es el *qué*: lo que el resto del programa puede usar (tipos,
constantes, prototipos y variables anunciadas con `extern`). El **`.c`** es
el *cómo*: los cuerpos de las funciones y las variables. Quien usa un módulo
solo incluye su `.h` y no necesita leer su `.c`.

`src/` está partido así:

| Módulo | El `.h` ofrece | El `.c` esconde |
|---|---|---|
| `src/juego/tarou.h` / `tarou.c` | `Tarou`, `Caja`, las constantes y 4 prototipos (`tarou_iniciar`, `tarou_actualizar`, `tarou_caja_cuerpo`, `tarou_caja_golpe`, líneas 85-97) | 17 funciones `static` (`cambiar_estado` en la 65, `acelerar` en la 90, `manejar_sueno` en la 302...) |
| `src/sistema/secuencia.h` / `secuencia.c` | `extern u16 frames_en_pantalla;` (línea 29) y 4 prototipos | la variable `pantalla_actual` y los `switch` que llaman a cada pantalla |
| `src/juego/mando.h` / `mando.c` | la struct `Mando` y `mando_leer` | los bits `KEY_*` (parte 5): la lógica nunca los ve |

Cada `.c` incluye **su propio `.h`**: así el compilador comprueba que las
firmas del `.c` coinciden con los prototipos que ven los demás.

### Prototipo, definición y declaración

- La **definición** de una función es la función entera, con su cuerpo; la
  de una variable es la línea que le reserva memoria (`u16 frames_totales;`).
  Cada cosa se define **una sola vez** en todo el programa.
- Una **declaración** solo anuncia que algo existe y cómo es, sin crearlo. El
  **prototipo** es la declaración de una función. Se puede declarar las
  veces que haga falta (un `.h` incluido en diez `.c` la declara diez veces).

Si `main.c` llama a una función de la que no ha visto ni el prototipo, `gcc`
(desde la versión 14) da **error**: por eso se incluye el `.h`.

### `extern`: una variable de otro archivo

**`extern`** delante de una variable es una declaración: «existe una variable
con este nombre y este tipo, definida en otro `.c`». No reserva memoria.
`src/sistema/secuencia.c:33` **define** `u16 frames_en_pantalla;` y
`src/sistema/secuencia.h:29` la **declara** con
`extern u16 frames_en_pantalla;` para que todas las pantallas puedan leerla
(→ glosario: `extern`). La `demo_07` hace lo mismo con `frames_totales`, y en
el ejercicio 7 `main.c` escribe `marcador_record`, que vive en `marcador.c`.

### `static`: tres usos de la misma palabra

1. **`static` en una variable de archivo** (fuera de las funciones): solo se
   ve dentro de su `.c`. `static Tarou tarou;` en
   `src/pantallas/pantalla_nivel1.c:94` y `static u8 visible;` en
   `src/juego/hud_debug.c:91`. Si otro archivo la pide con `extern`, el
   enlazador no la encuentra (`capturas/error_enlace_static.txt`):

   ```
   undefined reference to `frames_del_segundo'
   ```

   Así, dos `.c` pueden tener cada uno su `static u8 i;` sin chocar.
2. **`static` en una función**: solo se puede llamar desde su `.c`. Son las
   piezas internas de un módulo: `cambiar_estado`, `acelerar`, `saltar`... en
   `src/juego/tarou.c` (→ glosario: `static`).
3. **`static` en una variable dentro de una función** (*static local*): se
   crea una sola vez y **conserva su valor entre llamadas**; su `= 0` se
   aplica al arrancar el programa, no en cada llamada. En el ejercicio 7,
   `static u8 veces` cuenta las sumas de todo el programa y nadie de fuera
   puede ponerla a cero. El juego no usa ninguna: guarda ese tipo de estado en
   variables de archivo, que su función `_iniciar` puede reiniciar.

**Trampa de la SNES: las `static` no empiezan a 0.** En el C estándar, una
variable `static` o global sin valor inicial vale 0, y en el PC es verdad: la
`demo_07` lo imprime (`static sin inicializar vale 0 en el PC`). En la SNES,
la RAM arranca con basura y `tcc816` no la limpia. Por eso el proyecto da
valor a mano a cada variable en una función de inicio: `mando_iniciar` pone
`cuadro_guion = 0` (`src/juego/mando.c:37-42`), `secuencia_cambiar` pone
`frames_en_pantalla = 0` (`secuencia.c:42`) y, en el ejercicio 7,
`marcador_iniciar` pone `total = 0`.

### Compilar y enlazar

Hacer un `.exe` con varios `.c` son dos trabajos distintos:

1. **Compilar**: cada `.c` se traduce **por separado** a un **archivo
   objeto** (`.o`): código máquina con huecos donde llama a cosas de otros
   archivos. Al compilar `main.c`, `gcc` solo sabe de `reloj.c` lo que dice
   `reloj.h`.
2. **Enlazar**: el **enlazador** (*linker*, `ld`) junta todos los `.o`,
   rellena cada hueco con la dirección de la función o variable que le
   corresponde y escribe el `.exe`.

`gcc` hace las dos cosas con una sola línea si se le dan todos los `.c`:

```
gcc -std=c99 -Wall -Wextra -o demo_07.exe demo_07/main.c demo_07/reloj.c
```

O en pasos separados (`-c` = solo compilar):

```
gcc -std=c99 -Wall -Wextra -c demo_07/main.c -o main.o
gcc -std=c99 -Wall -Wextra -c demo_07/reloj.c -o reloj.o
gcc -o demo_07.exe main.o reloj.o
```

Si se olvida un `.c`, cada archivo **compila** bien, pero el **enlazador**
no encuentra lo que falta (`capturas/error_enlace_falta_reloj.txt`, rutas
acortadas):

```
ld: ccXXXXXX.o:main.c:(.text+0x79): undefined reference to `reloj_iniciar'
...
collect2.exe: error: ld returned 1 exit status
```

Distinguirlos ahorra tiempo: un error con `archivo.c:línea:` es del
compilador (mira ese archivo); un `undefined reference` es del enlazador (falta
un `.c` en la línea, o la cosa es `static`, o no está definida en ningún sitio).
Las pruebas del juego se enlazan así: `tests/Makefile:29` pasa a `gcc` todos
los `prueba_*.c` y todos los `.c` de lógica pura de `src/` en una línea. Para
la ROM, el `Makefile` del proyecto compila cada `.c` con `tcc816` y el
enlazador `wlalink` los junta (unidad 03).

## Parte 8 — `enum`, `switch` y `const`

### `enum`: nombres para números

Un **`enum`** (enumeración) es una lista de nombres a los que C da valores
enteros: el primero vale 0 (o lo que diga su `=`) y cada uno sin `=` vale el
anterior + 1. Con `typedef` se le da nombre de tipo, como hace
`src/juego/animaciones.h`:

```c
typedef enum
{
    ESTADO_PARADO = 0,
    ESTADO_CAMINANDO,       /* 1 */
    ...
    ESTADO_DESPERTANDO,     /* 12 */
    ESTADO_CANTIDAD         /* 13: no es un estado: cuántos hay */
} Estado;
```

El último elemento, que **no es un estado sino la cuenta**, es un patrón
del proyecto (`CANTIDAD_PANTALLAS` en `src/sistema/pantallas.h`, igual): da
el tamaño de las tablas que van en paralelo al `enum` y sirve de límite en
los `for`. Si se añade un estado antes de él, la cuenta sube sola.

`KEY_A`, `KEY_B`... del SDK (parte 5) también son un `enum`, con valores que
no siguen el orden (`BIT(7)`, `BIT(15)`...).

**Un `enum` pone nombres; no fija el tamaño.** En `gcc`, una variable de tipo
`Estado` ocupa 4 bytes. El juego guarda el estado en un `u8`
(`u8 estado;` en `Tarou`, `src/juego/tarou.h:73`) para que ocupe uno, y usa
los nombres del `enum` para darle valor.

### `switch`, `case`, `break`, `default`

**`switch (valor)`** salta al **`case`** cuyo número coincide con `valor` y
ejecuta desde ahí hasta el **`break`**, que sale del `switch`. **`default`**
recoge cualquier valor que no tenga su `case`. Es la forma clara de escribir
un `if / else if` largo que compara la misma variable con muchas constantes.

```c
switch (pantalla_actual)                    /* src/sistema/secuencia.c:55-66 */
{
    case PANTALLA_NINTENDO:     pantalla_nintendo_iniciar();     break;
    case PANTALLA_PUBLISHER:    pantalla_publisher_iniciar();    break;
    ...
    default:                    break;   /* Valor desconocido: no hacemos nada. */
}
```

Tres formas que aparecen en el juego:

- **Casos agrupados**: varios `case` seguidos sin nada entre ellos comparten
  el código. `tarou_actualizar` (`src/juego/tarou.c:509-535`) manda PARADO,
  CAMINANDO y CORRIENDO a `manejar_pie` con tres `case` y un solo
  `break`.
- **`return` dentro de un `case`**: sale de la función entera, así que no
  hace falta `break`. `tarou_sprite_tabla` (`src/juego/tarou_sprite.c:103-121`)
  devuelve la tabla de cada estado así.
- **`default` defensivo**: aunque «no pueda pasar», se pone. Un estado
  corrupto entonces no hace nada raro.

**La trampa del `break` olvidado.** Sin `break`, C **sigue ejecutando el
`case` de abajo** (se llama *fallthrough*, «caer al siguiente»):

```c
case PANTALLA_TITULO:
    pantalla_titulo_iniciar();          /* falta el break...                    */
case PANTALLA_NIVEL1:
    pantalla_nivel1_iniciar();          /* ...y al entrar al título se inicia
                                           TAMBIÉN el nivel                       */
    break;
```

Con `-Wextra`, `gcc` avisa («this statement may fall through»). Los casos
agrupados no avisan porque no tienen código entre ellos. El comentario de
`src/sistema/secuencia.c:52-54` lo recuerda.

Otro aviso útil: con `-Wall`, un `switch` sobre una variable **de tipo
`enum`** sin `default` avisa si falta algún `case`. El juego hace el `switch`
sobre un `u8` (`t->estado`), así que ese aviso no salta: el `default` es su
red.

### `const`: solo lectura

**`const`** dice que algo no se puede cambiar; si el código lo intenta, el
compilador da error (`capturas/error_const.txt`):

```
demo_08_enum_switch_const.c:127:17: error: assignment of read-only location 'DURACION[0]'
```

Ya salió en los parámetros puntero (`const Tarou *t`: la función solo lee).
Su otro gran uso son las **tablas `const`**: datos fijos que el programa
consulta y nunca cambia. En la SNES, `tcc816` las deja en la **ROM** y no
gastan RAM (el comentario de `src/juego/animaciones.c` lo dice: «viven en la
ROM»). Ejemplos:

- `const AnimDef ANIMACIONES[ESTADO_CANTIDAD]` (`src/juego/animaciones.c:31-49`):
  una fila `{cuadros, frames_por_cuadro, bucle}` por estado, **en el mismo
  orden que el `enum`**, así que `ANIMACIONES[t->estado]` es la animación del
  estado actual.
- `const char *const NOMBRES_ESTADO[ESTADO_CANTIDAD]` (`animaciones.c:51-55`):
  los nombres que muestra el HUD. Tiene **dos `const`**: `const char *` dice
  que las letras no se cambian; el `*const` de después, que los punteros de
  la tabla tampoco (no se puede hacer que la fila 0 apunte a otra cadena).
- `static const u16 controles[]` (`src/pantallas/pantalla_nivel1.c:97`) y
  `static const u16 POTENCIAS[4]` (`src/juego/hud_debug.c:139`): tablas
  `const` que además son `static` (de su archivo).
- `const EventoEscena ESCENA_NINTENDO[]` (`src/escena/escena_nintendo.c`): la
  animación del logo como **tabla de eventos escrita con macros**
  (`FIJAR(0, PISTA_MOSAICO, 15)`, `RAMPA(...)`, `FIN_ESCENA(244)`). Junta las
  partes 6 y 8: cada macro se expande a las llaves de una fila. La `demo_08`
  hace una versión pequeña y la recorre con un puntero hasta la fila de fin.

## Cómo se trabaja esta unidad

Ocho partes, cada una con una demo para leer y ejecutar y un ejercicio para
completar. Los ejercicios tienen los `TODO` numerados y su **salida esperada
exacta** escrita en el propio archivo. Todos compilan sin avisos tal como
están (los esqueletos no imprimen nada, o imprimen ceros o «?», hasta que se
completan).

| Parte | Demo | Ejercicio | Qué practica |
|---|---|---|---|
| 1 | `demo_01_logica.c` | `ejercicio_01_logica.c` | variable, `if`/`else if`/`else`, `for` |
| 2 | `demo_02_funciones.c` | `ejercicio_02_funciones.c` | función `void` con parámetro |
| 3 | `demo_03_tipos_arrays.c` | `ejercicio_03_tipos_arrays.c` | `u8`/`u16` con `typedef`, `sizeof`, array, índice desde 0, acumular y contar |
| 4 | `demo_04_structs_punteros.c` | `ejercicio_04_structs_punteros.c` | `struct`, `typedef`, punteros, `->`, `const`, parámetro de salida |
| 5 | `demo_05_bits.c` | `ejercicio_05_bits.c` | binario, hex, `& \| ^ ~ << >>`, máscaras, mando, atributos de la OAM |
| 6 | `demo_06_preprocesador.c` (+ `demo_06_caja.h`, `demo_06_expandir.c`) | `ejercicio_06_preprocesador.c` | constantes, macros con paréntesis, `#ifndef`, `#ifdef`, `-D` |
| 7 | `demo_07/` (`main.c`, `reloj.c`, `reloj.h`, `tipos.h`) | `ejercicio_07/` (TODO en `marcador.c`) | `.h`/`.c`, prototipos, `extern`, los tres `static`, enlazar |
| 8 | `demo_08_enum_switch_const.c` | `ejercicio_08_enum_switch_const.c` | `enum` con cuenta final, tablas `const`, `switch` agrupado y con `return` |

En cada parte:

1. Lee la demo, compílala y ejecútala. Antes de ejecutarla, intenta adivinar
   qué va a imprimir.
2. Abre el ejercicio, lee el enunciado y la salida esperada y completa los
   `TODO`.
3. Compílalo hasta que no haya ni errores ni avisos y compara tu salida con
   la de referencia (sección «Aprobación»).
4. Solo entonces mira `solucion/`. Las soluciones de los ejercicios 1 y 2 son
   las que escribió el autor del proyecto cuando aprendía C, con sus notas.

## Dónde está en el juego

Abre estos archivos después de la parte 8. No hace falta entender todavía qué
hace cada línea (eso viene en las unidades 01, 02, 05, 08 y 10); lo que hay
que **reconocer** es cada pieza del lenguaje.

| Qué buscar | Dónde |
|---|---|
| Guarda de inclusión | `src/juego/tarou.h:36-37` y `:99` |
| `#define` de constantes (`VEL_CAMINAR_MAX`, `GRAVEDAD`, `VEL_SALTO`...) | `src/juego/tarou.h:45-55` |
| `enum` sin nombre (`ESPADA_*`, `DIR_*`) | `src/juego/tarou.h:58-59` |
| `typedef struct` pequeño: `Caja` (igual que en el ejercicio 4) | `src/juego/tarou.h:62-66` |
| `typedef struct` del protagonista: `Tarou`, con campos `s16` y `u8` | `src/juego/tarou.h:69-81` |
| Prototipos con `Tarou *t` y `const Tarou *t` | `src/juego/tarou.h:85-97` |
| Función `static` con puntero: `cambiar_estado` | `src/juego/tarou.c:65-71` |
| `tarou_iniciar`: escribe cada campo con `t->` (compárala con tu `personaje_iniciar`) | `src/juego/tarou.c:73-84` |
| `acelerar` y `frenar`: el mismo recorte de velocidad que tu `personaje_acelerar`, en las dos direcciones | `src/juego/tarou.c:90-145` |
| Parámetro de salida: `tarou_caja_cuerpo(const Tarou *t, Caja *c)` | `src/juego/tarou.c:565-580` |
| `typedef` de enteros y `#ifdef PRUEBAS_PC` | `src/sistema/tipos.h:30-43` |
| Macros con paréntesis: `PF(n)`, `PF_ENTERO(x)` | `src/sistema/puntofijo.h:34-36` |
| Máscaras del mando: `(mantenidos & KEY_B) != 0` y `#ifdef GUION_PRUEBA` | `src/juego/mando.c` (`mando_leer`) |
| Recién pulsados: `ahora & ~antes` | `src/juego/guion.c:44` |
| Bits sumados con `\|`: `BOTON_IZQUIERDA 0x01` ... `BOTON_ESPADA 0x40` | `src/juego/guion.h:35-41` |
| Espejo de un sprite: `\| METASPRITE_HFLIP` (`0x40`) | `src/juego/metasprite.c:40` |
| Bit de prioridad de un tile: `TILE(c)` con `\| 0x2000` | `src/juego/hud_debug.c:85` |
| Desplazamientos en vez de multiplicar o dividir: `>> 4`, `<< 7` | `src/juego/colision.c:65` |
| `extern` de una variable de otro archivo | `src/sistema/secuencia.h:29` (definida en `secuencia.c:33`) |
| Variables `static` de archivo | `src/juego/hud_debug.c:91-95`, `src/pantallas/pantalla_nivel1.c:93-97` |
| Valor a mano en una función de inicio (las `static` no empiezan a 0 en la SNES) | `src/juego/mando.c:37-42`, `src/sistema/secuencia.c:42` |
| Varios `.c` en una línea de `gcc` | `tests/Makefile:20` y `:29` |
| `typedef enum` con cuenta final: `Estado` / `Pantalla` | `src/juego/animaciones.h:34-50`, `src/sistema/pantallas.h:36-47` |
| Tablas `const` en paralelo al `enum`, `const char *const` | `src/juego/animaciones.c:31-55` |
| `switch` con un `case` por pantalla y `default` | `src/sistema/secuencia.c:52-66` |
| `switch` con casos agrupados | `src/juego/tarou.c:509-535` |
| `switch` con `return` en cada `case` | `src/juego/tarou_sprite.c:103-121` |
| Tabla `const` de eventos hecha con macros | `src/escena/escena_nintendo.c` (`ESCENA_NINTENDO`) |

Fíjate en una diferencia con el ejercicio 4: los campos de `Tarou` se llaman
`x_fp`, `vx_fp`. El `_fp` significa **punto fijo**, la manera de tener
decimales sin decimales. Es el tema de la unidad 01.

## Trampas de esta unidad (resumen)

- `=` asigna y `==` compara (parte 1).
- `int` mide 4 bytes en `gcc` y 2 en `tcc816`: el juego usa `u8/u16/s16` (parte 3).
- Un índice fuera del array no da error: lee o pisa memoria ajena (parte 3).
- `~` sobre un `u8`/`u16` trabaja en `int`: recortar con un cast (parte 5).
- Macros: paréntesis en cada parámetro y alrededor de todo; con macros del SDK
  como `OBJ_PRIO`, el argumento entre paréntesis (parte 6).
- Un `.c` olvidado en la línea de `gcc` da `undefined reference`: es el
  enlazador, no el compilador (parte 7).
- Las `static` empiezan a 0 en el PC, **no en la SNES**: dar valor siempre
  en la función de inicio (parte 7).
- Un `break` olvidado en un `switch` ejecuta también el `case` de abajo (parte 8).
- Un `?:` que elige entre cadenas como argumento de `printf` rompe `tcc816`
  (parte 1; en el PC compila).

## Ejercicio y criterio de aprobación

**Aprobación:** los **ocho** ejercicios compilan con
`gcc -std=c99 -Wall -Wextra` **sin errores ni avisos** y su salida es
**idéntica** a la esperada; el 6, compilado de las dos maneras. La salida de
referencia de cada ejercicio está en `capturas/salida_ejercicio_NN_*.txt`
(generada con la solución). Para comparar sin hacerlo a ojo, en PowerShell:

```
gcc -std=c99 -Wall -Wextra -o e4.exe ejercicio_04_structs_punteros.c
.\e4.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_04_structs_punteros.txt) -SyncWindow 0
```

Si `Compare-Object` no imprime nada, las dos salidas son iguales. Si imprime
líneas, `=>` es lo que falta en la tuya y `<=` lo que sobra (`-SyncWindow 0`
compara línea a línea, en orden; sin esa opción, dos líneas cambiadas de sitio
pasarían por iguales). Cambia el número y el nombre para los ejercicios 1, 2,
3, 5 y 8. Los ejercicios 6 y 7 se compilan distinto:

```
gcc -std=c99 -Wall -Wextra -o e6.exe ejercicio_06_preprocesador.c
.\e6.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_06_preprocesador.txt) -SyncWindow 0

gcc -std=c99 -Wall -Wextra -DGUION_PRUEBA -DVIDAS_INICIALES=5 -o e6d.exe ejercicio_06_preprocesador.c
.\e6d.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_06_preprocesador_con_D.txt) -SyncWindow 0

gcc -std=c99 -Wall -Wextra -o e7.exe ejercicio_07\main.c ejercicio_07\marcador.c
.\e7.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_07.txt) -SyncWindow 0
```

Al terminar, borra `mia.txt`, los `.exe` y los `.o` si compilaste por pasos.

`capturas/` también tiene la salida de cada demo (`salida_demo_NN_*.txt`), por
si quieres comprobar tu predicción del paso 1, la salida de `gcc -E` de la
`demo_06_expandir.c` (con y sin `-DGUION_PRUEBA`) y los cuatro errores que
cita este README (`error_*.txt`).

## Lo que viene

- **Unidad 01:** enteros de 16 bits por dentro (desbordamiento, signo,
  complemento a dos), el punto fijo de `x_fp` y el coste de `/` y `%`. Da por
  sabidos el binario, el hexadecimal y los operadores de bits de la parte 5.
- **Unidad 02:** máquinas de estado con el `enum` y el `switch` de la parte 8:
  el `estado` de `Tarou`.
- **Unidad 03:** qué hace `tcc816` con este mismo C para convertirlo en una
  ROM de SNES, y cómo enlaza `wlalink` los objetos de cada `.c`.
