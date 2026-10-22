# Unidad 11 — Animación por estados

**Objetivo:** que cada estado de Tarou muestre su animación a su ritmo, con
bucle o sin él; que el cambio de estado reinicie la animación; y que el final
de una animación pueda **disparar** un cambio de estado. Y afinar ese ritmo
sin que las pruebas mientan.

**Prerrequisitos:** 02 (estado, `enum Estado`, `switch`, `cambiar_estado`, la
trampa de la copia fija `tests/tarou_anim.h`) y 09 (cuadro de 96×64, tira,
tabla metasprite, cómo se dibuja el cuadro actual); por debajo, 00, 01, 03-06
y 08. **Práctica:** PC + sobre el juego real. **Tiempo orientativo:** una o
dos tardes.

## Conceptos

### Cuadro y frames por cuadro

Un **cuadro** (*frame* de animación) es una de las imágenes de una tira: el
reposo de Tarou tiene 9 cuadros de 96×64 (unidad 09). Un **frame** es una
vuelta del bucle principal, 1/60 s (unidad 04). No son lo mismo: si un cuadro
cambiara cada frame, los 9 del reposo pasarían en 0,15 s. Cada animación dice
cuántos frames dura cada cuadro: sus **frames por cuadro**. Reposo, 12:
`9 × 12 = 108` frames por vuelta, 1,8 s, una respiración tranquila. Correr, 4:
las patas van rápido.

```
frame:   1 ... 12 | 13 ... 24 | 25 ...           (reposo, 12 frames por cuadro)
cuadro:  0        | 1         | 2  ...  8 | 0 ...  ← bucle
```

### Bucle o no

Una animación **con bucle** vuelve al cuadro 0 al acabar (reposo, caminar,
dormido). Una **sin bucle** se queda en el último cuadro (saltar: Tarou
encogido en el aire hasta que cae; atacar, desenvainar, despertar). Es un
campo de la tabla, `bucle`.

### `anim_terminada`: el final como disparador

Algunos estados no terminan por un botón sino **cuando su gesto acaba**:
desenvainar, envainar y atacar vuelven a `PARADO`; echarse pasa a `DORMIDO`;
despertar vuelve a `PARADO`. El reloj de la animación avisa con un campo de
`Tarou`: `anim_terminada = 1` cuando una animación sin bucle llega al final.
La máquina de estados (unidad 02) solo lo lee:

```c
static void manejar_espada(Tarou *t)          /* src/juego/tarou.c */
{
    if (!t->anim_terminada)
    {
        return;                               /* todavía desenvainando: nada */
    }
    if (t->estado == ESTADO_DESENVAINANDO)
    {
        t->espada = ESPADA_FUERA;             /* la espada cuenta como fuera al terminar */
    }
    ...
    cambiar_estado(t, ESTADO_PARADO);
}
```

Así la duración de un gesto **no está escrita en ningún sitio de la lógica**:
la decide su animación. Cambias el ritmo y el estado dura más o menos solo.

### El reloj: `avanzar_animacion`

Tres campos de `Tarou` (unidad 00: `src/juego/tarou.h`) llevan el reloj:
`cuadro`, `frames_cuadro` (cuántos frames lleva el cuadro actual) y
`anim_terminada`. `avanzar_animacion` se llama **al final** de cada
`tarou_actualizar` (paso 5):

```c
static void avanzar_animacion(Tarou *t)
{
    const AnimDef *a = &ANIMACIONES[t->estado];   /* la fila de este estado */
    t->frames_cuadro++;
    if (t->frames_cuadro >= a->frames_por_cuadro)
    {
        t->frames_cuadro = 0;
        t->cuadro++;
        if (t->cuadro >= a->cuadros)
        {
            if (a->bucle)
            {
                t->cuadro = 0;                    /* reposo, caminar...: otra vuelta */
            }
            else
            {
                t->cuadro = (u8)(a->cuadros - 1); /* ataque, salto...: se queda en el último */
                t->anim_terminada = 1;
            }
        }
    }
}
```

Dos consecuencias del orden (paso 1 transiciones, paso 5 animación):

- **El frame en que se entra en un estado ya cuenta.** `cambiar_estado` pone
  `cuadro = 0`, `frames_cuadro = 0` y `anim_terminada = 0`, y al final de ese
  mismo frame `avanzar_animacion` suma 1. `prueba_cambio_de_estado_reinicia`
  lo comprueba: tras agacharse, `frames_cuadro == 1`.
- **El estado cambia un frame después del final.** `anim_terminada` se pone al
  final del frame N; `manejar_espada` lo ve al principio del N+1. Desenvainar
  con 9 cuadros × 5 frames termina al final del frame 45 y Tarou está en
  `PARADO` en el 46 (la demo lo traza).

### Cambiar de estado reinicia la animación... si el estado es otro

`cambiar_estado` siempre reinicia. Por eso `manejar_pie` solo lo llama si el
estado **nuevo es distinto**:

```c
u8 nuevo = m->correr ? ESTADO_CORRIENDO : ESTADO_CAMINANDO;
if (t->estado != nuevo)
{
    cambiar_estado(t, nuevo);    /* correr → caminar reinicia; caminar → caminar, no */
}
```

Si lo llamara cada frame, Tarou se quedaría clavado en el cuadro 0 de caminar.

### La tabla de animaciones indexada por estado

`src/juego/animaciones.h` define una fila por estado:

```c
typedef struct
{
    u8 cuadros;                 /* cuántos cuadros tiene la tira */
    u8 frames_por_cuadro;       /* cada cuántos frames avanza */
    u8 bucle;                   /* 1 = vuelve al primero */
} AnimDef;
extern const AnimDef ANIMACIONES[ESTADO_CANTIDAD];
```

y `src/juego/animaciones.c` la rellena **en el mismo orden que el `enum
Estado`**: `ANIMACIONES[ESTADO_DESENVAINANDO]` es la fila de desenvainar
porque `ESTADO_DESENVAINANDO` vale 7 y es la octava fila. Una tabla indexada
por estado sustituye a un `switch` de 13 casos: añadir un estado es añadir una
fila (y, en el mismo orden, una tira en `tarou_sprite_tabla` y un nombre en
`NOMBRES_ESTADO`, que el HUD muestra en la fila `EST`). Es `const`: vive en la
ROM y no gasta WRAM.

¿Cuesta indexar una struct de 3 bytes por frame? En la unidad 09 una struct de
6 bytes hacía llamar a `tcc__mul`. Con 3 no: tcc816 multiplica por 3 con un
desplazamiento y una suma (`asl a` + `adc`). Medido con la receta de PLAN §6.4
en un clon (`capturas/animdef_3_bytes_asm.txt`): **0** llamadas a `tcc__mul`
en `tarou.asm`. La regla práctica sigue siendo la de CLAUDE.md (potencias de
2 o puntero en los bucles calientes), pero conviene **medir** antes de
rellenar una struct.

### El header generado: `tarou_anim.h`

La tabla no escribe cuántos cuadros tiene cada tira: lo lee de
`TAROU_<ANIM>_CUADROS`, que **genera** `scripts/preparar_personaje.py` a
partir del arte (`res/personajes/tarou/tarou_anim.h`; no se edita a mano). El
script recorta la exportación de Pixellab según `MAPEOS_PIXELLAB`: por
ejemplo `"desenvainar": ("Unsheathe", "east", 0, None, False)` (todos los
cuadros) y `"envainar"`, el mismo gesto **al revés**; `"despertar"` toma 10 de
los 11 cuadros de `Stand_up` porque una tira no puede pasar de 10 (unidad 09:
10 × 3 KB por banco). Así, si el artista añade un cuadro, la animación lo
recoge sin tocar C. Lo que sí se decide en C es el **ritmo**: los frames por
cuadro y el bucle.

### Duración fija con una división entre constantes

Atacar tiene que durar unos 20 frames (1/3 s) tenga los cuadros que tenga:

```c
#define DURACION_ATAQUE 20
{TAROU_ATACAR_CUADROS, DURACION_ATAQUE / TAROU_ATACAR_CUADROS, 0},
```

Es una división, y en la SNES dividir cuesta una rutina (`tcc__div`, unidad
01). Aquí no: **las dos son constantes**, así que la hace el compilador al
compilar y en la ROM queda un número (hoy `20 / 1 = 20`, la pose fija
`Attack_1`). Es entera: con 3 cuadros saldría `20 / 3 = 6`, 18 frames, no 20.

## La demo: `demo_11_animacion.c`

Aplica la regla de `avanzar_animacion` a la tabla real:

```
cd E:\ProyectoSNES\aprendizaje\unidad-11-animacion
gcc -std=c99 -Wall -Wextra -o demo_11.exe demo_11_animacion.c
.\demo_11.exe
```

Extracto (`capturas/salida_demo_11_animacion.txt`):

```
== 1. Las 13 animaciones con el ARTE REAL ==
  animacion          cuadros  fr/cuad  bucle  frames segundos
  reposo (PARADO)          9       12     si     108     1.80
  ...
  desenvainar              9        5     no      45     0.75   <- larga (gesto sin bucle de mas de 2/3 s)
  envainar                 9        5     no      45     0.75   <- larga ...
  echarse                  6        8     no      48     0.80   <- larga ...
  despertar               10        8     no      80     1.33   <- larga ...

== 4. Las mismas reglas con la COPIA FIJA de las pruebas (tests/tarou_anim.h) ==
  desenvainar        arte  9 x  5 =  45 frames | copia fija  4 x  5 =  20 frames
  despertar          arte 10 x  8 =  80 frames | copia fija  4 x  8 =  32 frames
```

Ese es el **problema real** del ejercicio. Los frames por cuadro de la tabla
(`docs/diseno-demo-jugable.md` §7) se fijaron con los marcadores, de 4
cuadros: desenvainar duraba 20 frames. Llegó el arte de Pixellab con 9
cuadros y, con el mismo ritmo, el gesto pasó a 0,75 s; despertar, a 1,33 s.
Durante ese tiempo el mando no hace nada (estos estados lo ignoran): el juego
se siente lento.

## Las dos trampas de las pruebas

### 1. `tests/tarou_anim.h` es una copia fija

Las pruebas de PC no usan el header del arte: `tests/Makefile` pone `-I.`
antes, así que `animaciones.c` toma `tests/tarou_anim.h`, con los cuadros de
los marcadores (reposo 4, desenvainar 4, despertar 4...). Es a propósito: las
pruebas no deben romperse cada vez que el artista cambia un cuadro. Pero
significa que **las cuentas de frames de las pruebas son las de la copia
fija**: `FRAMES_DESENVAINAR` en `tests/prueba_tarou.c` es `4 * 5`, no `9 * 5`.
Al cambiar un ritmo, se cambia en la prueba con los cuadros de la copia.

### 2. Lo que la copia fija esconde: el guion del recorrido

`prueba_recorrido` juega el tutorial con el guion (unidad 05) y **también**
usa la copia fija. El guion pulsa R (desenvainar) en el cuadro 1772 y A
(atacar) en el 1872: 100 frames de margen. Con la copia fija, desenvainar
dura `4 × fpc`; con el arte real, `9 × fpc`. Si se sube el ritmo a 12 frames
por cuadro:

| | copia fija (pruebas) | arte real (ROM) |
|---|---|---|
| desenvainar | 4 × 12 = 48 frames | 9 × 12 = **108** frames |
| ¿termina antes de A (100 frames)? | sí | **no**: A llega en mitad del gesto y se ignora |
| resultado | 367 pruebas en verde | la ROM de prueba no rompe el tronco y no termina |

Comprobado en un clon (`capturas/trampa_desenvainar_12.txt`): con 12 frames
por cuadro y la prueba ajustada, `367 pruebas, 0 fallos`; compilando las
mismas pruebas con el `tarou_anim.h` real, `prueba_recorrido` falla (`tronco_roto
== 1`, `e == PARTIDA_SALIDA`...). El script `pruebas_con_arte_real.ps1` de esta
unidad hace las dos compilaciones; en la segunda fallan siempre 16
comprobaciones que dependen de la copia fija (lo esperado), y lo que hay que
mirar es si aparece `prueba_recorrido`. Acortar un gesto nunca rompe el guion;
alargarlo, sí.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| `enum Estado` (el orden importa), `AnimDef`, `ANIMACIONES`, `NOMBRES_ESTADO` | `src/juego/animaciones.h` |
| La tabla con los ritmos y `DURACION_ATAQUE` | `src/juego/animaciones.c` |
| `cambiar_estado` (reinicia el reloj) | `src/juego/tarou.c` |
| `avanzar_animacion` (el reloj) y su llamada en el paso 5 de `tarou_actualizar` | `src/juego/tarou.c` |
| Disparadores por `anim_terminada`: `manejar_espada`, `manejar_sueno` | `src/juego/tarou.c` |
| «Solo si el estado es distinto»: `manejar_pie`, `manejar_agachado` | `src/juego/tarou.c` |
| Quién dibuja el cuadro: `tarou_sprite_tabla(estado, cuadro)` | `src/juego/tarou_sprite.c` |
| El header real (generado) | `res/personajes/tarou/tarou_anim.h` |
| La copia fija de las pruebas | `tests/tarou_anim.h` |
| De dónde salen los cuadros (`MAPEOS_PIXELLAB`, `MAX_CUADROS = 10`) | `scripts/preparar_personaje.py` |
| Pruebas del reloj: `prueba_animacion_bucle`, `prueba_animacion_sin_bucle`, `prueba_cambio_de_estado_reinicia`, `prueba_desenvainar`, `prueba_dormirse`, `prueba_despertar` | `tests/prueba_tarou.c` |
| Pruebas de la tabla (cuadros de la copia fija, ritmos, bucles) | `tests/prueba_animacion.c` |
| El guion: R en 1772, A en 1872 | `src/juego/guion_tutorial.c` |
| La tabla de ritmos «a afinar» | `docs/diseno-demo-jugable.md` §7 |

## Ejercicio y criterio de aprobación

Enunciado en `ejercicio_11.md`; solución en `solucion/ejercicio_11.patch`.
Resumen: con la demo, decidir ritmos nuevos para desenvainar, envainar y
despertar (la solución usa 3, 3 y 5: 0,45 s y 0,83 s), cambiarlos en
`animaciones.c`, ajustar `FRAMES_DESENVAINAR` y `FRAMES_DESPERTAR` de
`tests/prueba_tarou.c` (con los cuadros de la copia fija), comprobar el
recorrido con el arte real, compilar la ROM que se juega sola y ver el gesto
en Mesen.

**Aprobación:** `367 pruebas, 0 fallos` (`capturas/pruebas_solucion.txt`), el
recorrido con el arte real sin `prueba_recorrido` entre los fallos, la ROM de
prueba compilada sin avisos y capturas de Mesen con el nuevo ritmo
(desenvainar ante el tronco; despertar en la ROM normal).

## Lo que viene

- **Unidad 18:** añadir una animación entera, de la exportación de Pixellab a
  la ROM, pasando por esta tabla.
- **Unidad 19:** el arte y sus decisiones (por qué 10 cuadros, el mapeo).
