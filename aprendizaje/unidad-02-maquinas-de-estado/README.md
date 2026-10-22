# Unidad 02 — Máquinas de estado

**Objetivo:** modelar «qué está haciendo» algo con una variable y un
`switch`, y leer con soltura las dos máquinas de estado del juego: la de las
pantallas (`src/sistema/secuencia.c`) y la del protagonista
(`src/juego/tarou.c`). Al final se añade una prueba propia a
`tests/prueba_tarou.c` y se ve pasar.

**Prerrequisitos:** unidad 00 (C en el PC, con su bloque de `enum`, `switch`
y `const`) y unidad 01 (punto fijo: las velocidades de Tarou son 12.4).
**Práctica:** PC (`gcc`) + sobre el juego real (las pruebas de `tests/`).
**Tiempo orientativo:** una tarde.

## Conceptos

### Estado

El **estado** de algo es lo que está haciendo **ahora**, elegido de una lista
cerrada de posibilidades. Un semáforo está en verde, en ámbar o en rojo; nunca
en dos a la vez y nunca en «verde y medio». Tarou está en uno de 13 estados
(→ glosario: Estado): `PARADO`, `CAMINANDO`, `SALTANDO`... En el código, el
estado es **una sola variable**: `u8 estado` en la struct `Tarou`
(`src/juego/tarou.h`) o `Pantalla pantalla_actual` en
`src/sistema/secuencia.c:32`.

### Transición

Una **transición** es el paso de un estado a otro, y siempre tiene una
**causa**: un botón («pulsa B → `SALTANDO`»), el tiempo («6 frames en verde →
ámbar»), el final de una animación («`DESENVAINANDO` termina → `PARADO`») o la
física («los pies tocan el suelo → aterriza»). Escribir una máquina de estados
es, sobre todo, escribir la **tabla de transiciones**: para cada estado, qué
causas lo hacen salir y hacia dónde. La del protagonista está en
`docs/diseno-demo-jugable.md` §4; ábrela ahora y tenla al lado durante toda la
unidad.

### `enum`: nombres con valor entero (repaso)

Un **`enum`** da nombre a una lista de enteros (unidad 00). El primero vale lo
que se diga (o 0) y cada uno siguiente, uno más:

```c
enum { LUZ_VERDE = 0, LUZ_AMBAR, LUZ_ROJA, LUZ_CANTIDAD };   /* 0, 1, 2, 3 */
```

Tres costumbres del proyecto, las tres en `src/juego/animaciones.h:34-50`:

1. **Un último valor que cuenta**: `ESTADO_CANTIDAD` (13) no es un estado;
   sirve para dimensionar arrays (`ANIMACIONES[ESTADO_CANTIDAD]`).
2. **El estado como índice**: como los valores son 0, 1, 2..., el estado elige
   directamente la fila de una tabla: `NOMBRES_ESTADO[t->estado]` es su
   nombre para el HUD y `ANIMACIONES[t->estado]` su animación. Por eso el
   comentario dice «el orden importa».
3. **La variable se guarda en un `u8`**, no en el tipo del `enum`: un `enum`
   ocupa 2 bytes en `tcc816` (medido: `sizeof` de un `enum` da 2, como el
   `int`, unidad 01), y 13 estados caben en 1 byte.

### `switch` / `case` / `break` (repaso)

`switch (x)` salta al `case` cuyo valor coincide con `x`; `break` sale del
`switch`; `default` recoge cualquier otro valor. **Sin `break`, C sigue
ejecutando el `case` de abajo** (*fall-through*). Casi siempre es un error,
pero `tarou_actualizar` lo usa a propósito para agrupar estados que comparten
código:

```c
switch (t->estado)                   /* src/juego/tarou.c:509 */
{
    case ESTADO_PARADO:              /* tres case seguidos sin nada en medio: */
    case ESTADO_CAMINANDO:           /* los tres caen en manejar_pie          */
    case ESTADO_CORRIENDO:
        manejar_pie(t, m);
        break;
    ...
    default:
        break;                       /* Valor imposible: no hacer nada.       */
}
```

El `default` vacío es una defensa: si por un error `estado` valiera 200, el
juego no hace nada raro en vez de saltar a cualquier parte.

### Máquina de estados finita

Una **máquina de estados finita** es la suma de lo anterior: un número finito
de estados, **una variable** que dice cuál está activo, y en cada paso (aquí,
cada frame) se ejecuta **solo el código del estado activo**, que decide si hay
transición (→ glosario: Máquina de estados). Dibujada, la del ejercicio es:

```
                  derecha
   ┌────────┐ ─────────────► ┌───────────┐
   │ PARADO │                │ CAMINANDO │
   └────────┘ ◄───────────── └───────────┘
     │   ▲      sin derecha       │   ▲
   B │   │ toca el suelo        B │   │ toca el suelo
     ▼   │ sin derecha            ▼   │ con derecha
   ┌───────────────────────────────────┐
   │             SALTANDO              │   (B en el aire: nada)
   └───────────────────────────────────┘
```

Por qué es mejor que un montón de `if` sueltos con banderas (`saltando = 1`,
`caminando = 1`...): con banderas, nada impide que dos sean 1 a la vez y hay
que pensar en todas las combinaciones (con 4 banderas, 16). Con una variable
de estado, **las combinaciones imposibles no se pueden escribir**.

### El patrón `_iniciar` / `_actualizar`

Todo lo que vive varios frames en el juego se escribe con dos funciones:

- **`_iniciar`**: se llama **una vez**, al empezar. Deja todas las variables
  en un valor conocido. En la SNES esto no es opcional: las variables
  `static` **no empiezan a 0** (la RAM arranca con basura y `tcc816` no la
  limpia), así que lo que `_iniciar` no escriba vale cualquier cosa.
- **`_actualizar`**: se llama **una vez por frame** desde el bucle principal.
  Lee la entrada, decide la transición y mueve lo que haya que mover.

Así son `tarou_iniciar`/`tarou_actualizar` (`src/juego/tarou.c:73` y `:506`),
`secuencia_iniciar`/`secuencia_actualizar` y cada pantalla
(`pantalla_titulo_iniciar`/`pantalla_titulo_actualizar`...). En la demo:
`semaforo_iniciar`/`semaforo_actualizar`; en el ejercicio:
`perro_iniciar`/`perro_actualizar`.

Un detalle más del patrón: **una sola función cambia de estado**.
`cambiar_estado` (`src/juego/tarou.c:65`) no solo escribe `t->estado`:
reinicia la animación (`cuadro = 0`, `frames_cuadro = 0`,
`anim_terminada = 0`). `secuencia_cambiar` (`secuencia.c:39`) reinicia
`frames_en_pantalla`, apaga la pantalla y llama al `_iniciar` de la nueva. Si
alguien escribiera `t->estado = ESTADO_SALTANDO` a mano, la animación
empezaría a mitad: centralizar el cambio evita ese error para siempre.

### Una sola cosa a la vez… y cuándo usar una variable aparte

«Un estado a la vez» tiene un límite. Tarou puede caminar **con** la espada
fuera o **sin** ella, saltar con ella o sin ella... Si la espada fuera parte
del estado, habría que duplicarlos: `CAMINANDO_CON_ESPADA`,
`CAMINANDO_SIN_ESPADA`, `SALTANDO_CON_ESPADA`... de 13 estados a 26, y cada
transición escrita dos veces.

La regla: **si una propiedad es independiente de lo que se está haciendo y
dura a través de varios estados, va en una variable aparte**. Por eso `Tarou`
tiene `u8 espada` (`ESPADA_ENVAINADA` / `ESPADA_FUERA`) junto a `estado`
(→ glosario: Espada), y lo mismo `direccion` y `en_suelo`. El estado dice qué
hace; la espada, con qué; la dirección, hacia dónde. Las transiciones
**consultan** esas variables (`A` solo ataca si `t->espada == ESPADA_FUERA`,
`manejar_pie`) y algunos estados las **cambian** (`manejar_espada` pone
`ESPADA_FUERA` al terminar de desenvainar). La prueba que escribirás en la
práctica comprueba justo esto.

## Cómo funciona: tres máquinas, de la más simple a la real

### 1. El semáforo (`demo_02_semaforo.c`)

Tres estados, transiciones por tiempo y una por entrada (el botón de
peatones). Compílala y, antes de ejecutarla, predice en qué frame cambia cada
luz:

```
gcc -std=c99 -Wall -Wextra -o demo_02.exe demo_02_semaforo.c
.\demo_02.exe
```

```
frame  5:  VERDE (lleva 5)
frame  6:  AMBAR (lleva 0)  <- cambia
...
frame 14:  VERDE (lleva 2)
frame 15:  AMBAR (lleva 0)  [boton]  <- cambia
```

El primer verde dura 6 frames; el segundo, 3, porque en el frame 15 un peatón
pulsa y ya habían pasado los 2 de mínimo. `boton_pulsado` es una variable
aparte (como `espada`): recuerda la petición aunque la luz no pueda atenderla
todavía. `frames_en_luz` es el `frames_en_pantalla` de `secuencia.c`.

### 2. La secuencia de pantallas (`src/sistema/secuencia.c`)

La máquina de estados más grande del juego, y la más simple de leer:

- **Estado:** `pantalla_actual`, de tipo `Pantalla` (el `enum` de
  `src/sistema/pantallas.h:36`: `PANTALLA_NINTENDO`, `PANTALLA_PUBLISHER`...
  `PANTALLA_PROXIMAMENTE`).
- **Tiempo en el estado:** `frames_en_pantalla` (`secuencia.c:33`).
- **Cambio:** `secuencia_cambiar(nueva)` (`:39`), con un `switch` que llama al
  `_iniciar` de la pantalla nueva.
- **Cada frame:** `secuencia_actualizar()` (`:81`), un `switch` que llama al
  `_actualizar` de la pantalla actual; y `secuencia_vblank()` (`:108`), un
  tercer momento que solo usan dos pantallas.
- **Quién la mueve:** el bucle de `src/sistema/main.c:64-66`.

Aquí las transiciones **no** están en `secuencia.c`: cada pantalla decide
cuándo terminar y llama a `secuencia_cambiar(...)` ella misma (por ejemplo, el
título al pulsar Start). La máquina central solo sabe despachar.

### 3. Tarou (`src/juego/tarou.c`)

13 estados agrupados en cinco familias, cada una con su función (paso 1 de
`tarou_actualizar`, `:509`):

| Estados | Función | Qué hace |
|---|---|---|
| `PARADO`, `CAMINANDO`, `CORRIENDO` | `manejar_pie` (`:160`) | botones por prioridad R > A > B > Abajo > dirección; cada `return` es un botón que gana a los de abajo |
| `AGACHADO`, `AGACHADO_CAMINANDO` | `manejar_agachado` (`:244`) | solo B, soltar Abajo y la cruceta; bajo techo bajo no se levanta |
| `SALTANDO`, `CAYENDO` | `actualizar_aire` (`:361`) | solo control aéreo; las transiciones las hace la física |
| `DESENVAINANDO`, `ENVAINANDO`, `ATACANDO` | `manejar_espada` (`:281`) | **ignoran el mando**; esperan a `anim_terminada` y vuelven a `PARADO` |
| `ECHANDOSE`, `DORMIDO`, `DESPERTANDO` | `manejar_sueno` (`:302`) | un `switch` dentro del `switch`: cualquier actividad despierta |

Tres cosas que la tabla no dice y el código sí:

- **No todas las transiciones salen del `switch`.** `SALTANDO → CAYENDO`
  ocurre en el paso 2 (cuando `vy_fp >= 0`, el apogeo: el frame 24 del
  ejercicio de la unidad 01), y aterrizar o caer por un borde, en `mover_y`
  (paso 4), que llama a `aterrizar` (`:370`). En el ejercicio pasa lo mismo:
  la física del aire va **después** del `switch` y depende de `en_suelo`, no
  del estado.
- **Algunas transiciones van con un frame de retraso.** `avanzar_animacion`
  marca `anim_terminada` al **final** de un frame, y `manejar_espada` lo ve al
  **principio** del siguiente. Desenvainar dura 20 frames (con la copia fija,
  abajo) y el estado cambia en el 21. Y en ese frame 21 el mando no se lee
  (lo hizo `manejar_espada`, no `manejar_pie`): un B mantenido salta en el 22.
- **`?:` para elegir el estado nuevo.** `manejar_pie` (`:184`) escribe
  `cambiar_estado(t, t->espada == ESPADA_FUERA ? ESTADO_ENVAINANDO :
  ESTADO_DESENVAINANDO)`: R saca o guarda según la variable aparte. (Con
  números `?:` es seguro en `tcc816`; con cadenas como argumentos de una
  función variádica, como `printf`, rompe el compilador: por eso el ejercicio
  imprime «<- cambia» con un `if`.)

## La trampa de `tests/tarou_anim.h`: una copia fija

Cuántos cuadros tiene cada animación no está escrito en `animaciones.c`: lo
genera el script del arte (`scripts/preparar_personaje.py`) en
`res/personajes/tarou/tarou_anim.h`, y `animaciones.c` hace
`#include "tarou_anim.h"`. ¿Cuál de los dos `tarou_anim.h`? El que encuentre
primero el compilador en sus rutas `-I`:

- **La ROM** (`Makefile` de la raíz) añade `-Ires/personajes/tarou`: usa el
  del **arte real**.
- **Las pruebas** (`tests/Makefile:20`) compilan con `-I. -I../src ...`: el
  `-I.` va primero y encuentra **`tests/tarou_anim.h`**, una copia fija con
  los cuadros de los antiguos marcadores.

| Animación | `tests/tarou_anim.h` (pruebas) | `res/personajes/tarou/tarou_anim.h` (ROM) |
|---|---|---|
| reposo | 4 | 9 |
| caminar / correr | 8 / 8 | 8 / 8 |
| desenvainar / envainar | 4 / 4 | 9 / 9 |
| atacar | 5 | 1 |
| echarse / dormido / despertar | 4 / 4 / 4 | 6 / 9 / 10 |

Por qué existe: así las pruebas no se rompen cada vez que el arte cambia.
Consecuencia: **las cuentas de frames de una prueba se hacen con la copia
fija**. Desenvainar dura en las pruebas 4 × 5 = 20 frames
(`FRAMES_DESENVAINAR` en `tests/prueba_tarou.c`), pero en la consola
9 × 5 = 45. Una prueba que dijera 45 «porque el arte tiene 9 cuadros» fallaría,
y una prueba en verde **no** demuestra cuánto dura la animación en la ROM.
El ataque es un caso curioso: dura 20 frames con las dos (`DURACION_ATAQUE /
cuadros`: 5 × 4 o 1 × 20), pero el número de cuadro no coincide (`cuadro == 4`
en la prueba; en el arte real solo existe el 0).

`capturas/salida_pruebas_arte_real.txt` lo demuestra: las mismas pruebas
compiladas con el `tarou_anim.h` real (`-I../res/personajes/tarou` delante de
`-I.`) dan **374 pruebas, 19 fallos**, entre ellos tres de la prueba nueva de
esta unidad.

## Práctica sobre el juego: una prueba propia

Las pruebas de `tests/` son funciones que crean un `Tarou`, le dan un `Mando`
escrito a mano, avanzan N frames con `tarou_actualizar` y comprueban el
resultado con `COMPROBAR(condición)` (`tests/minitest.h`: cuenta la prueba y,
si la condición es falsa, imprime `FALLO archivo:línea: condición`). Lee antes
`prueba_desenvainar` y `prueba_aterrizaje_caminando` en `tests/prueba_tarou.c`:
tu prueba se parece a las dos juntas.

**Dónde trabajar.** No en `E:\ProyectoSNES` directamente: en una rama propia
o en un clon (`git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u02`). Los
cambios del alumno no se confirman en la rama principal.

**Qué escribir** (`ejercicio`: lo que la prueba tiene que comprobar):

1. Tarou parado pulsa R: el estado es `DESENVAINANDO`.
2. Durante el resto de la animación se mantienen **B y Derecha** (y `saltar`):
   al final de `FRAMES_DESENVAINAR - 1` frames sigue `DESENVAINANDO`, en el
   suelo y sin moverse (`x_fp == PF(100)`).
3. Un frame más: `PARADO` y `espada == ESPADA_FUERA`.
4. Un frame más: ahora sí lee el B mantenido y está `SALTANDO`, con la espada
   todavía fuera.
5. Suelta B (`saltar_pulsado = 0`) y deja pasar 48 frames: ha aterrizado
   (`en_suelo == 1`), está `CAMINANDO` (Derecha sigue mantenida) y la espada
   **sigue fuera**.

Escríbela como una función `static void prueba_...(void)` antes de
`/* Punto de atención 3 ... */` y añade su llamada en `pruebas_tarou()`, al
final del archivo. Si olvidas la llamada, `gcc` avisa
(`defined but not used`) y la cuenta no sube. Después:

```
cd E:\ProyectoSNES_trabajo\u02
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

`tests/Makefile` ya lista las cabeceras como dependencias, así que aquí no
hace falta `make clean` (en la ROM sí: ver `CLAUDE.md`).

**Para comprobar que tu prueba prueba algo:** cambia a propósito una cuenta
(por ejemplo `FRAMES_DESENVAINAR - 1` por `FRAMES_DESENVAINAR`) y mira cómo
aparece un `FALLO tests/prueba_tarou.c:NNN`. Una prueba que no puede fallar no
sirve. Devuélvela a su sitio.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| `enum Pantalla` (los estados de la secuencia) | `src/sistema/pantallas.h:36-47` |
| Estado y reloj: `pantalla_actual`, `frames_en_pantalla` | `src/sistema/secuencia.c:32-33` |
| El único cambio de estado: `secuencia_cambiar` | `src/sistema/secuencia.c:39` |
| El `switch` por frame: `secuencia_actualizar`, `secuencia_vblank` | `src/sistema/secuencia.c:81`, `:108` |
| El bucle que mueve la máquina | `src/sistema/main.c:64-66` |
| `enum Estado` (13 estados + `ESTADO_CANTIDAD`) | `src/juego/animaciones.h:34-50` |
| El estado como índice: `ANIMACIONES[]`, `NOMBRES_ESTADO[]` | `src/juego/animaciones.c` |
| `cambiar_estado` (reinicia la animación) | `src/juego/tarou.c:65` |
| `tarou_iniciar` / `tarou_actualizar` | `src/juego/tarou.c:73`, `:506` |
| `manejar_pie`, prioridad de botones con `return` | `src/juego/tarou.c:160` |
| `manejar_agachado`, `manejar_espada`, `manejar_sueno` | `src/juego/tarou.c:244`, `:281`, `:302` |
| Transiciones fuera del `switch`: apogeo y aterrizar | `tarou_actualizar` paso 2; `aterrizar` (`:370`) desde `mover_y` |
| La tabla de transiciones | `docs/diseno-demo-jugable.md` §4 |
| Las pruebas de la máquina de Tarou | `tests/prueba_tarou.c` |
| La copia fija de los cuadros | `tests/tarou_anim.h` |

## Trampas

- **Olvidar un `break`** hace que se ejecute también el `case` siguiente.
  En el PC, `gcc -Wextra` avisa (`this statement may fall through`) cuando un
  `case` con código cae en el siguiente; `tcc816` compila lo mismo **sin
  decir nada** (comprobado el 2026-10-22), y el código de pantallas no pasa
  por `gcc`. Si se quiere agrupar estados, hacerlo como `tarou_actualizar`:
  varios `case` seguidos **sin código entre ellos** (eso no avisa en ninguno).
- **Cambiar el estado a mano** (`t->estado = ...`) en vez de con
  `cambiar_estado`: la animación del estado nuevo empezaría a mitad.
- **El orden del `enum` importa** si el estado indexa tablas: insertar un
  estado en medio desplaza `ANIMACIONES[]`, `NOMBRES_ESTADO[]` y las tiras de
  `tarou_sprite.c`.
- **`static` sin `_iniciar`**: en la SNES empiezan con basura. Todo estado se
  inicializa en su `_iniciar`.
- **Las cuentas de frames de las pruebas usan `tests/tarou_anim.h`**, no el
  arte real. Una prueba en verde no dice cuánto dura una animación en la ROM.
- **Un frame de retraso** entre el fin de una animación y el cambio de
  estado, y otro antes de que el estado nuevo lea el mando. Al contar frames
  en una prueba, cuenta con los dos.

## Ejercicio y criterio de aprobación

**Parte 1 (PC):** `ejercicio_02_perro.c`, un perro con `PARADO`, `CAMINANDO`
y `SALTANDO`. Cada carácter de una cadena es un frame del mando: `.` nada,
`d` derecha, `b` pulsa B, `x` derecha + B. Completa los TODO 1 a 6 (el `enum`
y sus nombres, `saltar`, los tres `case` y `aterrizar`); `main` y la física
del aire ya están escritos. Se ejecuta de tres maneras:

```
gcc -std=c99 -Wall -Wextra -o ejercicio_02.exe ejercicio_02_perro.c
.\ejercicio_02.exe "..dd.b......ddxdxdddd."            # teclas como argumento
Get-Content entrada_ejemplo.txt | .\ejercicio_02.exe   # desde el archivo
.\ejercicio_02.exe                                     # las escribes y pulsas Enter
```

**Parte 2 (sobre el juego):** la prueba de la sección «Práctica sobre el
juego».

**Aprobación:**

1. El ejercicio compila sin avisos y, con `entrada_ejemplo.txt`, su salida es
   **idéntica** a `capturas/salida_ejercicio_02_perro.txt`, que termina en:
   ```
   Secuencia: PARADO > CAMINANDO > PARADO > SALTANDO > PARADO > CAMINANDO > SALTANDO > CAMINANDO > PARADO
   ```
   Para comparar:
   ```
   Get-Content entrada_ejemplo.txt | .\ejercicio_02.exe 2>$null > mia.txt
   Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_02_perro.txt) -SyncWindow 0
   ```
   (sin salida = iguales; `2>$null` esconde la pregunta «Teclas...», que va a
   la salida de errores para no mezclarse con el resultado).
2. En tu clon, `.\tests\pruebas.exe` pasa de `367 pruebas, 0 fallos` a
   `374 pruebas, 0 fallos` (si escribes 7 `COMPROBAR`, como la solución;
   con otro número, 367 + los tuyos, y **0 fallos**). Referencia:
   `capturas/salida_pruebas.txt`.

La solución: `solucion/ejercicio_02_perro.c` y, para la prueba,
`solucion/fragmento_prueba_tarou.c` (el trozo y dónde pegarlo) o
`solucion/prueba_tarou.patch` (`git apply` en el clon). Fíjate en el frame 17
de la salida del ejercicio: un `x` (B) en el aire no hace nada, porque en
`SALTANDO` el `case` no mira el botón.

## Lo que viene

- **Unidad 03:** qué hace `tcc816` con este C para convertirlo en una ROM.
- **Unidad 10:** la física de los estados de suelo y aire (aceleración,
  frenado, salto variable).
- **Unidad 11:** la animación de cada estado (`avanzar_animacion`, cuadros,
  bucles) y por qué el arte real cambia las cuentas.
