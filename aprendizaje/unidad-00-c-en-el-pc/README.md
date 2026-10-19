# Unidad 00 — C en el PC: lógica, funciones, arrays, structs y punteros

**Objetivo:** leer y escribir C sin miedo, con lo justo para entender
`src/juego/tarou.h` y `src/juego/tarou.c`: variables, tipos, `if`/`for`/`while`,
funciones, arrays, `struct`, punteros y `#define`.

**Prerrequisitos:** ninguno. **Práctica:** PC (`gcc`). **Tiempo orientativo:**
una o dos tardes.

Todo en esta unidad se compila y se ejecuta en el PC. No hay nada de SNES
todavía: primero el lenguaje y después la consola. Al final, en «Dónde está en
el juego», se ve que el código del protagonista usa exactamente estas piezas.

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
  las pruebas de PC del juego (`tests/Makefile`).
- `-Wall -Wextra` activan los **avisos** (*warnings*): mensajes del
  compilador sobre código que compila pero probablemente está mal. La regla
  del proyecto es **cero avisos**.
- `-o demo_01.exe` es el nombre del ejecutable que se crea. Los `.exe` no se
  guardan en git (`.gitignore`).

## Conceptos

### Programa, `main` y `printf`

Todo programa de C empieza a ejecutarse en la **función `main`** (qué es una
función se explica más abajo). `#include <stdio.h>` pega al principio del
archivo las declaraciones de la biblioteca estándar de entrada/salida, entre
ellas `printf`, que escribe texto en la consola. Dentro del texto, `%d` es un
hueco que se rellena con un número entero y `\n` es un salto de línea:

```c
printf("Tarou empieza con %d vidas.\n", vidas);   /* Tarou empieza con 3 vidas. */
```

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
**byte** son 8 bits; un **bit** es un 0 o un 1 (la unidad 01 lo explica a
fondo).

```c
int vidas = 3;        /* declara la variable vidas, de tipo int, con valor 3 */
vidas = vidas - 1;    /* le asigna un valor nuevo: 2                          */
```

`int` es el entero «por defecto» de C, pero **su tamaño depende del
compilador**: con `gcc` en el PC ocupa 4 bytes. Por eso el juego no usa `int`
y usa tipos cuyo tamaño está escrito en el nombre (→ glosario: `u8`, `u16`,
`s16`):

| Tipo | Bytes | Valores | En el juego |
|---|---|---|---|
| `u8` | 1 | 0..255 | estados, contadores pequeños, banderas 0/1 |
| `u16` | 2 | 0..65535 | contadores de frames, posiciones en píxeles |
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

Las demos y ejercicios de esta unidad usan `int` a propósito, porque es lo que
se ve en cualquier libro de C; la `demo_04` y el `ejercicio_04` ya usan `u8` y
`s16` como el juego.

### Expresión y sentencia

Una **expresión** es algo que se calcula y da un valor: `vidas - 1`,
`edad < 3`, `duplicar(vidas)`. Una **sentencia** es una orden completa y
termina en `;`: `vidas = vidas - 1;`. Un **bloque** es un grupo de
sentencias entre llaves `{ }`.

### Decidir: `if`, `else if`, `else`

`if (condición) { ... }` ejecuta el bloque solo si la condición es cierta. En
C, «cierto» es cualquier valor distinto de 0 y «falso» es 0. Los operadores
de comparación son `<`, `<=`, `>`, `>=`, `==` (igual) y `!=` (distinto), y
se combinan con `&&` (y), `||` (o) y `!` (no).

```c
if (edad < 3)       { /* cachorro */ }
else if (edad < 8)  { /* adulto   */ }
else                { /* viejo    */ }
```

**La trampa clásica:** `=` **asigna** y `==` **compara**. `if (edad = 0)`
compila, pone `edad` a 0 y nunca entra. El autor del proyecto se encontró con
este error resolviendo el ejercicio 1, y su nota está en
`solucion/ejercicio_01_logica.c`. Con `-Wall`, `gcc` avisa de este caso.

### Repetir: `for` y `while`

`for (inicio; condición; paso) { ... }` hace el inicio una vez y después
repite «si la condición es cierta, ejecuta el bloque y da el paso»:

```c
for (int i = 0; i < 5; i++) { ... }   /* i vale 0, 1, 2, 3, 4 */
```

`i++` suma 1 a `i`, igual que `i = i + 1`. Una variable declarada dentro del
`for` (`int i`) solo existe dentro de él.

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
firmas (→ glosario: `void`).

### Array e índice

Un **array** es una fila de variables del mismo tipo, seguidas en memoria, con
un solo nombre. Cada una se elige con su **índice** entre corchetes, que
**empieza en 0**:

```c
int puntaje[5] = {120, 340, 90, 500, 275};  /* puntaje[0] es 120, puntaje[4] es 275 */
```

Un array de 5 elementos tiene índices 0..4: `puntaje[5]` está fuera y C **no
lo comprueba**; lee o escribe la memoria de al lado. En la SNES casi todo son
arrays: un tile son 32 bytes seguidos, un tilemap es un array de `u16`, las
piezas de un metasprite son un array de structs.

### Conversión de tipo (*cast*)

Escribir un tipo entre paréntesis delante de una expresión la convierte a ese
tipo: `(s16)(p->x + 4)`. En C, una suma de dos `s16` se calcula como `int`, y
el *cast* deja claro (al compilador y al lector) que el resultado vuelve a
caber en un `s16`. El código del juego está lleno de ellos (por ejemplo
`(u8)(m->izquierda || m->derecha)` en `src/juego/tarou.c:134`).

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
`typedef uint8_t u8;` (en el PC; en la SNES los da `<snes.h>`).

`sizeof(Personaje)` dice cuántos bytes ocupa (6: tres `s16`).

### `enum`

Un **`enum`** da nombre a números enteros: `enum { DIR_DERECHA = 0,
DIR_IZQUIERDA = 1 };`. El código dice `DIR_IZQUIERDA` en vez de un `1`
misterioso. Se ve a fondo en la unidad 02 (máquinas de estado).

### Puntero: `&`, `*` y `->`

Cada variable vive en una **dirección** de memoria (un número que dice dónde
está). Un **puntero** es una variable que guarda una dirección:

```c
int vidas = 3;
int *pv = &vidas;    /* int *: "puntero a int". &vidas: la dirección de vidas */
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
que existe, `static Tarou tarou;` en `src/pantallas/pantalla_nivel1.c:72`.

**`const`** delante del tipo del puntero (`const Tarou *t`) promete que la
función **solo lee** lo apuntado; si intentara escribir, el compilador daría
error. **Parámetro de salida:** una función que tiene que «devolver» una
struct suele recibir un puntero a donde escribirla. `tarou_caja_cuerpo(const
Tarou *t, Caja *c)` lee al protagonista (entrada, `const`) y rellena la caja
(salida, `c`).

### `#define` y macros

`#define NOMBRE valor` pide al **preprocesador** (el paso que corre antes de
compilar, y que también resuelve los `#include`) que reemplace cada `NOMBRE`
por `valor`. Así se escriben las constantes: `#define GRAVEDAD 4`. Una
**macro** es un `#define` con parámetros: `#define DOBLE(n) ((n) * 2)`.

El preprocesador **pega texto, no calcula**. Por eso las macros llevan
paréntesis en todo. El paso 6 de la `demo_04` lo muestra: con
`#define DOBLE_MAL(n) n * 2`, `DOBLE_MAL(1 + 2)` se convierte en
`1 + 2 * 2`, que da 5 y no 6. Las macros del juego, como `PF(n)` en
`src/sistema/puntofijo.h:35`, llevan sus paréntesis por esto
(→ glosario: `#define`).

### `static` y `extern`

- **`static` en una función** (o en una variable fuera de las funciones): solo
  se ve dentro de su archivo `.c`. En `src/juego/tarou.c`, `cambiar_estado`,
  `acelerar`, `saltar`... son `static`: son piezas internas que solo usa
  `tarou_actualizar`. En `src/juego/hud_debug.c:61`, `static u8 visible;` es
  una variable que solo ve el HUD.
- **`extern`** anuncia que una variable existe y vive en **otro** archivo.
  `src/sistema/secuencia.c:32` define `u16 frames_en_pantalla;`, y
  `src/sistema/secuencia.h:29` dice `extern u16 frames_en_pantalla;` para que
  las pantallas puedan leerla (→ glosario: `extern`, `static`).

### `#include` y guarda de inclusión

`#include "archivo.h"` pega ahí el contenido de ese archivo. Un **`.h`**
(cabecera) contiene lo que otros archivos necesitan saber: tipos, constantes
y firmas de funciones. El `.c` tiene los cuerpos. Si un `.h` se pegara dos
veces en el mismo archivo, sus tipos se definirían dos veces y no compilaría.
La **guarda de inclusión** lo evita:

```c
#ifndef TAROU_H      /* si todavía no está definido TAROU_H...       */
#define TAROU_H      /* ...defínelo y pega el resto del archivo       */
...
#endif               /* la segunda vez, TAROU_H ya existe: no pega nada */
```

`#ifdef` es su pareja: «si está definido». `src/sistema/tipos.h:33` usa
`#ifdef PRUEBAS_PC` para elegir de dónde salen `u8`/`u16`/`s16`: de
`<stdint.h>` cuando compila `gcc` para las pruebas (`tests/Makefile` pasa
`-DPRUEBAS_PC`) o de `<snes.h>` cuando compila `tcc816` para la ROM
(→ glosario: `#ifndef`).

## Cómo se trabaja esta unidad

Cuatro partes, cada una con una demo para leer y ejecutar y un ejercicio para
completar. Los ejercicios tienen los `TODO` numerados y su **salida esperada
exacta** escrita en el propio archivo. Todos compilan sin avisos tal como
están (los esqueletos no imprimen nada o imprimen ceros hasta que se
completan).

| Parte | Demo | Ejercicio | Qué practica |
|---|---|---|---|
| 1 | `demo_01_logica.c` | `ejercicio_01_logica.c` | variable, `if`/`else if`/`else`, `for` |
| 2 | `demo_02_funciones.c` | `ejercicio_02_funciones.c` | función `void` con parámetro |
| 3 | `demo_03_tipos_arrays.c` | `ejercicio_03_tipos_arrays.c` | array, índice desde 0, acumular y contar |
| 4 | `demo_04_structs_punteros.c` | `ejercicio_04_structs_punteros.c` | `struct`, `typedef`, punteros, `->`, `const`, parámetro de salida |

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

Abre estos archivos después de la parte 4. No hace falta entender todavía qué
hace cada línea (eso viene en las unidades 01, 02 y 10); lo que hay que
**reconocer** es cada pieza del lenguaje.

| Qué buscar | Dónde |
|---|---|
| Guarda de inclusión | `src/juego/tarou.h:36-37` y `:98` |
| `#define` de constantes (`VEL_CAMINAR_MAX`, `GRAVEDAD`, `VEL_SALTO`...) | `src/juego/tarou.h:45-55` |
| `enum` sin nombre (`ESPADA_*`, `DIR_*`) | `src/juego/tarou.h:58-59` |
| `typedef struct` pequeño: `Caja` (igual que en el ejercicio 4) | `src/juego/tarou.h:62-66` |
| `typedef struct` del protagonista: `Tarou`, con campos `s16` y `u8` | `src/juego/tarou.h:69-81` |
| Firmas de funciones con `Tarou *t` y `const Tarou *t` | `src/juego/tarou.h:85-96` |
| Función `static` con puntero: `cambiar_estado` | `src/juego/tarou.c:37-43` |
| `tarou_iniciar`: escribe cada campo con `t->` (compárala con tu `personaje_iniciar`) | `src/juego/tarou.c:45-56` |
| `acelerar` y `frenar`: el mismo recorte de velocidad que tu `personaje_acelerar`, en las dos direcciones | `src/juego/tarou.c:62-117` |
| Parámetro de salida: `tarou_caja_cuerpo(const Tarou *t, Caja *c)` | `src/juego/tarou.c:435-450` |
| `typedef` de enteros y `#ifdef PRUEBAS_PC` | `src/sistema/tipos.h:30-43` |
| Macros con paréntesis: `PF(n)`, `PF_ENTERO(x)` | `src/sistema/puntofijo.h:34-36` |
| `extern` de una variable de otro archivo | `src/sistema/secuencia.h:29` (definida en `secuencia.c:32`) |
| Variables `static` de archivo (entre ellas un puntero, `static u16 *celda;`) | `src/juego/hud_debug.c:61-65` |

Fíjate en una diferencia con el ejercicio 4: los campos de `Tarou` se llaman
`x_fp`, `vx_fp`. El `_fp` significa **punto fijo**, la manera de tener
decimales sin decimales. Es el tema de la unidad 01.

## Ejercicio y criterio de aprobación

**Aprobación:** los cuatro ejercicios compilan con
`gcc -std=c99 -Wall -Wextra` **sin errores ni avisos** y su salida es
**idéntica** a la esperada. La salida de referencia de cada ejercicio está en
`capturas/salida_ejercicio_NN_*.txt` (generada con la solución). Para
comparar sin hacerlo a ojo, en PowerShell:

```
gcc -std=c99 -Wall -Wextra -o e4.exe ejercicio_04_structs_punteros.c
.\e4.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_04_structs_punteros.txt) -SyncWindow 0
```

Si `Compare-Object` no imprime nada, las dos salidas son iguales. Si imprime
líneas, `=>` es lo que falta en la tuya y `<=` lo que sobra (`-SyncWindow 0`
compara línea a línea, en orden; sin esa opción, dos líneas cambiadas de sitio
pasarían por iguales). Cambia el número
y el nombre para los ejercicios 1, 2 y 3. Al terminar, borra `mia.txt` y los
`.exe`.

`capturas/` también tiene la salida de cada demo (`salida_demo_NN_*.txt`), por
si quieres comprobar tu predicción del paso 1.

## Lo que viene

- **Unidad 01:** enteros de 16 bits por dentro (binario, desbordamiento,
  signo) y el punto fijo de `x_fp`.
- **Unidad 02:** `enum` y `switch` para las máquinas de estado: el
  `estado` de `Tarou`.
- **Unidad 03:** qué hace `tcc816` con este mismo C para convertirlo en una
  ROM de SNES.
