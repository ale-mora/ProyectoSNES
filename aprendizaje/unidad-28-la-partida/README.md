# Unidad 28 — La partida: el selector, los personajes como datos y la pausa, el GAME OVER y el STAGE CLEAR

> **Código real, rama principal.** Esta unidad enseña sobre lo que rodea al combate (commit
> `6845b5f`, 2026-11-05): `src/pantallas/pantalla_selector.c`, `src/juego/personaje.c/.h`,
> `src/juego/protagonista_sprite.c/.h` (`ProtaSprite`), `src/juego/estado_fase.c/.h` y
> `src/pantallas/pantalla_fase1.c` (cómo obedece a la partida). La demo se compila desde esta
> carpeta con `../../src`; el ejercicio se hace en un clon:
>
> ```text
> git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u28
> git -C E:\ProyectoSNES_trabajo\u28 checkout -q 6845b5f
> ```
>
> **No hay ficha en `PLAN.md` para esta unidad**: se añadió el 2026-11-05 con el código ya
> hecho. La 26 (el relevo entre protagonistas en mitad de la pelea) sigue por escribir: el
> selector **no** es el relevo; elige a una sola protagonista antes de empezar.

**Objetivo:** ver cómo un juego pasa de «una demo con Tarou» a «una partida»: elegir
personaje sin escribir `if (tarou)` por todo el código (los personajes como **datos** y el
dibujo como **instancias**), y meter la pausa, el KO, el GAME OVER con su cuenta atrás y el
STAGE CLEAR en una **máquina de estados pura** que se prueba en el PC y a la que la pantalla
solo obedece.

**Prerrequisitos:** 02 (máquinas de estado), 04 (el bucle y el VBlank), 05 (el mando y los
flancos), 11 (animación por tablas), 18 (pruebas de PC), 21–24 (la fase del beat 'em up), 25
(la media luz). **Práctica:** PC (gcc) + juego real en un clon, con capturas de Mesen.
**Tiempo orientativo:** una tarde y media.

## Conceptos

### Personajes como datos

Un **dato** es algo que el código **lee**; una **decisión** es algo que el código **pregunta**
(`if`). Tarou y Bárbara hacen lo mismo (andar, saltar, tres golpes, recibir, caer KO): lo que
cambia es el **dibujo** de cada cosa. Por eso `personaje.h` define una sola lista de
animaciones, `AnimProtaId` (`ANIM_GUARDIA` … `ANIM_KO`), y **una tabla por personaje**
(`ANIMS_TAROU` y `ANIMS_BARBARA`, `personaje.c:36` y `:49`) con, para cada animación, una
`AnimProta` de 4 bytes: qué tira, cuántos cuadros, cada cuántos frames cambia y si hace bucle.
`personaje_anims(personaje)` devuelve la tabla; `personaje_ancho/_alto` la celda (96×64 y
112×96) y `personaje_alcance` el golpe (52 y 60). La lógica que elige la animación a partir del
cuerpo (`personaje_anim_de`) es **la misma** para los dos. La demo (parte A) imprime las dos
tablas lado a lado.

`AnimProta` mide 4 bytes **a propósito**: una potencia de 2, así `tabla[i]` se compila con
desplazamientos y no con `tcc__mul` (unidad 14). Las tablas son `static const`: se quedan en
la ROM (una tabla `const` global iría a la WRAM, copiada al arrancar).

### El dibujo como instancia: `ProtaSprite`

Una **instancia** es una variable de un tipo `struct` con todo el estado de **una** cosa; las
funciones reciben un puntero a ella. Antes había un `tarou_sprite.c` con variables `static`:
solo podía haber **un** Tarou. Ahora `ProtaSprite` (`protagonista_sprite.h:43`) guarda qué
personaje es, su entidad de la tabla de sprites, su paleta, sus **dos búferes** de VRAM (para
subir un cuadro mientras se enseña el otro, unidad 09), su tabla de animaciones, el tope de
bytes por VBlank y el plan de subida. Así:

- **la fase** tiene **una** (`static ProtaSprite prota`), cargada con `personaje_elegido`;
- **el selector** tiene **dos a la vez** (`tarou` en tiles 0–191, paleta 0; `barbara` en
  192–383, paleta 2; `pantalla_selector.c:130-131`), cada una con **la mitad** del tope de
  subida (`TOPE_CADA_UNO` 768 bytes: 1,5 KB entre las dos, lo que cabe en el VBlank).

### El selector: flancos propios y la paleta oscura

`pantalla_selector.c` enseña a los dos animados y marca uno. Dos detalles de hardware:

- **El flanco propio.** Un **flanco** es el frame en que un botón **pasa** de suelto a
  pulsado. El SDK da `padsDown`, pero solo dura un VBlank y un frame largo se lo puede saltar;
  el selector guarda `antes = padsCurrent(0)` y calcula `actual & ~antes` cada frame
  (`:155`). Al entrar se inicializa con lo que ya está pulsado (`:139`): así el Start del
  título **no** cuenta como Start del selector.
- **La paleta oscura.** El no marcado se dibuja oscuro **sin tocar sus tiles**: se copia su
  paleta a la mitad de brillo en otra paleta de sprites (`paleta_oscura`, `:70`; la máscara
  `0x3DEF` de la unidad 25) y `nombres` (`:83`) solo cambia **qué paleta** usa cada
  instancia (`tarou.paleta = 0 o 1`). Es un byte de atributos en la OAM: gratis.

Al confirmar, `personaje_elegido = marcado` y la secuencia pasa a `PANTALLA_FASE1`.
`personaje_elegido` es una global de `personaje.c`; `main.c` la pone a Tarou al arrancar
(`:60`): en la SNES las variables **no empiezan a 0** (crt0 solo limpia 10 bytes).

### La partida como máquina de estados pura

`estado_fase.c` es una **máquina de estados** (unidad 02) con cinco estados y un contador
(`EstadoFase`, 4 bytes):

```text
            Start                          ko                 60 frames
 JUGANDO ─────────► PAUSA        JUGANDO ─────► KO ──────────────────────► GAME OVER
    ▲                 │  Select → TITULO                                   │ cuenta 9..0, 1 por segundo
    └──── Start ──────┘                       Start → REINICIAR ◄──────────┤
                                              al pasar de 0     → TITULO ◄─┘
 JUGANDO ── fase vencida ──► STAGE CLEAR ── 180 frames ──► PROXIMAMENTE
```

Lo importante es **la frontera**: la máquina no sabe nada de la SNES. Cada frame la pantalla
le pasa los botones **recién pulsados** y dos condiciones, y ella devuelve una **acción**
(`ACCION_NADA`, `_TITULO`, `_REINICIAR`, `_PROXIMAMENTE`):

```c
k = estado_fase_actualizar(&partida, mando.salir_pulsado, mando.hud_pulsado,
                           (u8)(cuerpo.vida == 0 && cuerpo.stun == 0),     /* ko         */
                           (u8)(oleadas.actual >= OLEADAS_ZONAS));          /* vencida    */
```

(`pantalla_fase1.c:304`). La pantalla obedece: sale al título, **reinicia** (`salir(PANTALLA_FASE1)`:
la secuencia vuelve a llamar a `pantalla_fase1_iniciar`, la fase desde el principio con el
mismo personaje) o va a «STAGE 2 / COMING SOON». Además pregunta dos cosas:
`estado_fase_mundo_avanza` (si no, el bloque del mundo no se ejecuta: todo se queda quieto
pero se sigue dibujando) y `estado_fase_atenuada` (la media luz de la unidad 25). Por eso toda
la lógica de la partida tiene pruebas de PC (`tests/prueba_estado_fase.c`), y la pantalla solo
tiene el «pegamento».

Dos reglas finas que la demo enseña: en el **KO** el mundo **sigue** un segundo (se ve caer a
la protagonista) y Start no pausa; en el **STAGE CLEAR** ni Start ni Select lo cortan. Y la
cuenta del CONTINUE no divide: cuenta frames hasta 60 y baja un número (sin el `/` lento de
tcc816).

### Los textos: filas del HUD

PAUSE, GAME OVER, CONTINUE? n y STAGE CLEAR van en las filas 6 y 7 de la capa de texto (BG3),
con las funciones del HUD de depuración (`hud_debug_empezar/texto/numero/enviar`): `aviso()`
(`pantalla_fase1.c:197`) las escribe **cada frame** mientras dura el estado (si una copia se
pierde por el VBlank, la siguiente la repone) y las borra una vez al volver a jugar. Como BG3
no entra en la media luz, el texto se lee a pleno brillo.

## La librería: PVSnesLib

| Función (firma exacta) | Qué hace / registros | Dónde |
|---|---|---|
| `padsCurrent(value)` (macro, [pad.h](https://alekmaul.github.io/pvsneslib/pad_8h.html)) | los botones pulsados ahora del mando `value` (copia de `$4218`) | `pantalla_selector.c` (el flanco propio) |
| `padsDown(value)` | los recién pulsados, pero solo durante un VBlank | `mando.c` (la fase; el selector no se fía de ella) |
| `void consoleDrawText(u16 x, u16 y, char *fmt, ...);` ([console.h](https://alekmaul.github.io/pvsneslib/console_8h.html)) | texto en la consola (BG3) | `nombres()` del selector |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` | la paleta oscura | `paleta_oscura()` |

La máquina de la partida y las tablas de personajes **no usan la librería**: son lógica pura.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| Lista de animaciones, tablas, celdas, alcance | `src/juego/personaje.h`, `personaje.c:36`, `:49`, `:61-98` |
| La instancia de dibujo | `ProtaSprite` en `src/juego/protagonista_sprite.h:43`; `protagonista_sprite_cargar` `protagonista_sprite.c:111`, `_actualizar` `:182` |
| El selector | `src/pantallas/pantalla_selector.c` (`paleta_oscura` `:70`, `nombres` `:83`, flanco `:155`) |
| A quién se juega | `personaje_elegido` (`personaje.c:31`), puesto en `src/sistema/main.c:60` |
| La máquina de la partida | `src/juego/estado_fase.c:35` (`estado_fase_actualizar`), `:101`, `:106` |
| Cómo la obedece la fase | `pantalla_fase1.c:304-330` (acciones, media luz, HUD solo jugando), `aviso` `:197` |
| Pruebas | `tests/prueba_estado_fase.c`, `tests/prueba_personaje.c` |
| La ROM autojugable con Bárbara | `-DGUION_FASE1 -DGUION_BARBARA` (`main.c:58`) |

## Trampas

| Trampa | Qué pasa | Cómo se evita |
|---|---|---|
| `if (personaje == TAROU)` repartidos por el código | cada personaje nuevo obliga a tocarlo todo | tablas por personaje y una sola lógica |
| `static` de estado en el módulo de dibujo | solo cabe un personaje en pantalla | una instancia (`struct`) por personaje |
| Confiar en `padsDown` en una pantalla con frames largos | un Start que no llega | flanco propio con `padsCurrent` |
| Olvidar inicializar `antes` al entrar | el Start del título confirma el selector | `antes = padsCurrent(0)` al entrar |
| Escribir los registros de la media luz al cambiar de estado | se escriben fuera del VBlank | marcar `cambiar_luz` y escribir en `_vblank` |
| Cambiar una regla de la partida en la pantalla | sin pruebas que lo vean | la regla en `estado_fase.c`, con su prueba |
| Pensar que «reiniciar» deja el estado limpio solo | `static` con valores viejos | `pantalla_fase1_iniciar` inicializa **todo** |

## La demo

`demo_28_partida.c` usa el código real (`personaje.c`, `estado_fase.c`). Desde esta carpeta:

```text
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -I../../res/personajes/tarou -I../../res/personajes/barbara -o demo_28.exe demo_28_partida.c ../../src/juego/personaje.c ../../src/juego/estado_fase.c ../../src/juego/mundo.c ../../src/juego/animaciones.c
.\demo_28.exe
```

Salida real en `capturas/salida_demo_28_partida.txt`. A: las dos tablas (fíjate en que las
**mismas** animaciones tienen tiras con números distintos: las de Tarou van detrás de las del
plataformas). B: pausa y reanudar, Select en pausa → TITULO; KO en el frame 50 → GAME OVER en
el 110 → Start en el 300 con la cuenta en 6 → REINICIAR; la última oleada en el 20 → STAGE
CLEAR → PROXIMAMENTE en el 200 aunque se pulse Start. C: sin tocar nada, el GAME OVER vuelve al
título en el frame **660** (1 s de KO + 10 de cuenta, del 9 al 0).

Antes de ejecutarla, predice: si se pulsa Start en el frame del KO, ¿se pausa? (No: el KO
gana; mira el orden de los `if` en el caso `FASE_JUGANDO`.)

## Ejercicio y criterio de aprobación

**Ejercicio (juego real, en un clon, con TDD):** `ejercicio_28.md`. Al entrar en la fase,
«READY?» 2 s con el mundo quieto, que nada corta, y luego a jugar. Prueba primero; después los
nombres; después la regla, que **rompe** las pruebas viejas (que suponían «iniciar = jugando»)
y se arreglan con una ayuda; luego el texto en pantalla y la ROM autojugable, que va por
tiempo. Solución: `solucion/`.

**Aprobación:**

1. la demo compila sin avisos y su salida coincide con `capturas/salida_demo_28_partida.txt`;
2. con la prueba nueva y solo los nombres: **4 fallos** (`capturas/pruebas_1_rojo.txt`);
3. con la regla y las pruebas viejas sin tocar: **513 fallos** de las viejas
   (`capturas/pruebas_2_viejas_rojas.txt`: una es un bucle de 500 frames);
4. con las viejas actualizadas: **2481 pruebas, 0 fallos** (`capturas/pruebas_3_verde.txt`);
5. la ROM normal y la `-DGUION_FASE1` compilan sin avisos (`capturas/compilacion.txt`);
6. en Mesen, «READY?» al entrar, un Start pulsado durante él **no** pausa, y luego se juega
   (`capturas/mesen_ready.png`, `mesen_despues_del_ready.png`, `mesen_secuencia.png`).

## Por venir (no enseñar como hecho)

| Lo que dicen la spec y el plan | Estado hoy |
|---|---|
| El relevo DKC en mitad de la pelea (L), con las dos barras de vida (unidad 26) | no existe: el selector elige una sola |
| Vidas extra y CONTINUE limitados | no existe: CONTINUE infinitos |
| Marcador, puntuación y HUD de juego definitivo | solo la barra de vida y el HUD de depuración |
| La fase 2 de verdad | «STAGE 2 / COMING SOON» |
