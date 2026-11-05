# Unidad 24 — Enemigos y oleadas: el ninja, el testigo de ataque y el bloqueo de cámara

> **Código real, rama principal.** Esta unidad enseña sobre el código del beat 'em up
> (commit `641d7c0`, 2026-10-26): `src/enemigos/ninja.c`, `ninja_sprite.c`,
> `src/juego/oleadas.c`, `camara.c`, `src/pantallas/pantalla_fase1.c` y el bloque
> `GUION_FASE1` de `src/juego/mando.c`. La demo se compila desde esta carpeta con
> `../../src` y el ejercicio se hace en un clon de la rama actual:
>
> ```text
> git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u24
> ```
>
> **La ficha de `PLAN.md` se escribió antes que el código y promete más de lo que hay.**
> Lo que **existe**: un solo enemigo (el ninja básico, con tajo), tres ranuras fijas, el
> testigo de ataque, las zonas por bloqueo de cámara y la ROM autojugable. Lo que **todavía
> no existe** y esta unidad no enseña como hecho: el ninja rápido (shuriken), el kendoka
> pesado, el jefe (`jefe.c`), los objetos rompibles, la comida y las armas (`objetos/*` son
> *stubs*), las variantes por paleta salvo las tres paletas provisionales de los ninjas, la
> lectura de oleadas desde Tiled, `scripts/preparar_zona.py` y los `zonaN_datos.h`. Todo eso
> aparece más abajo bajo «Por venir» y claramente rotulado.

**Objetivo:** entender cómo se comporta un enemigo de un beat 'em up con la IA más pequeña
que funciona (nace fuera de la pantalla, camina hacia el protagonista, se para a distancia,
se coloca en su carril, espera su turno y da un tajo), por qué hay un número **fijo** de
enemigos (tres ranuras, por la VRAM), cómo la fase se corta en **zonas** que bloquean la
cámara hasta vencer la oleada, y cómo se comprueba todo eso sin tocar el mando (la ROM
autojugable y su réplica en el PC).

**Prerrequisitos:** 02 (máquinas de estado), 05 (el mando y el guion de mando), 07 (cámara),
18 (herramientas y pruebas de PC), 20 (Tiled y los datos generados, solo por su concepto),
21–23 (presupuesto de sprites, el carril x/y y el combate; esta unidad usa sus resultados
sin repetirlos). **Práctica:** PC (gcc) + juego real en un clon. No hay mini-ROM ni Mesen
en esta unidad. **Tiempo orientativo:** dos tardes (teoría y demo; ejercicio).

## Conceptos

### Ranura fija

Una **ranura** (*slot*) es una posición fija de una tabla en la que cabe un enemigo. El juego
no crea ni destruye ninjas: tiene `NINJAS_MAX` = 3 entradas de `Ninja` en una tabla
(`static Ninja ninjas[NINJAS_MAX]` en `pantalla_fase1.c:84`) y marca cada una como libre
(`vivo = 0`) u ocupada (`vivo = 1`). Hay tres razones, de más a menos fuerte:

1. **La VRAM.** Cada ninja dibuja un cuadro de 64×96 px = 8×12 tiles de 8×8 = 96 tiles,
   que en 4 bpp son 3 072 bytes. Cada ranura tiene su **hueco de tiles** reservado y fijo
   (`HUECOS[] = {192, 288, 384}` en `ninja_sprite.c:50`: 192 + 96 = 288, 288 + 96 = 384).
   Tres huecos es lo que cupo en el reparto de la VRAM de la unidad 21.
2. **El VBlank.** Según la spec (§9.2, no remedido aquí) con tres ninjas y Tarou el
   VBlank llega a la línea 36 de 37 con **una** subida de cuadro por frame; dos subidas
   no caben. Más ninjas simultáneos serían más subidas.
3. **La CPU.** Con la tabla fija se recorre con un puntero (`n++`), sin `a[k]`: `Ninja` mide
   **16 bytes** (dos `u16` y doce `u8`; `ninja.h`), potencia de 2, así que incluso `n + k`
   sale con desplazamientos y no con `tcc__mul` (unidad 14).

### Los estados del ninja (IA como máquina de estados)

La IA de un ninja es una máquina de estados (unidad 02) con cinco estados
(`EstadoNinja`, `ninja.h`) más un campo aparte, `dentro`, que no es un estado sino una
**memoria**: «ya estuve en la pantalla».

| Estado | Qué hace | Sale a… |
|---|---|---|
| `NINJA_PARADO` | quieto; el cuadro de reposo (0) | `ANDANDO` si se mueve; `ATACANDO` si `ninja_intentar_atacar` lo manda |
| `NINJA_ANDANDO` | camina 1 px/frame en x hacia el objetivo y ajusta su y 1 px cada 2 frames | `PARADO` al llegar a `NINJA_DISTANCIA` (40 px) y a su carril |
| `NINJA_ATACANDO` | un tajo de 28 frames (`NINJA_ATAQUE_FRAMES`); su caja de golpe está activa en los frames 12..19 | `PARADO`, con `espera = NINJA_ESPERA` (50) |
| `NINJA_RECIBIR` | aturdido 16 frames: retrocede 2 px los primeros 6, sin IA | `PARADO` |
| `NINJA_DERRIBADO` | caído 36 frames: sale despedido 3 px los primeros 14; sin vida, al acabar desaparece (`vivo = 0`) | `PARADO` si le queda vida; libera la ranura si no |

`RECIBIR` y `DERRIBADO` los provoca `ninja_recibir` (`ninja.c:124`), que llama el combate de
la unidad 23; aquí solo importa que **cortan la IA**: `ninja_actualizar` empieza con
`if (n->estado == NINJA_RECIBIR || n->estado == NINJA_DERRIBADO) { aturdido(n); return; }`
(`ninja.c:149`), y lo mismo con `ATACANDO` (`:154`): un tajo empezado no se interrumpe por
su cuenta (solo un golpe recibido lo cancela, porque `ninja_recibir` cambia el estado).

El orden dentro de **un frame de un ninja** (`ninja_actualizar`, `ninja.c:145`) es:

1. si está tocado → `aturdido` y fin; si está atacando → avanza el tajo y fin;
2. el descanso se gasta: `if (n->espera != 0) n->espera--` (andando o parado);
3. **mira** hacia el objetivo (`mira_izq`) y **avanza en x** si está a más de
   `NINJA_DISTANCIA` — o si todavía no ha entrado (ver abajo);
4. cada dos frames (`reloj & 1`) corrige **un píxel su y** hacia la del objetivo;
5. `estado` = `ANDANDO` si se movió en x o en y, si no `PARADO`; el cuadro de la tira de andar
   avanza cada `NINJA_FRAMES_CUADRO` = 4 frames (con un AND, no con `%`).

No hay divisiones ni multiplicaciones. Fíjate en que **ir al carril** (paso 4) es parte de
«andar»: un ninja que ya está a 40 px pero en otro carril sigue en `ANDANDO` hasta igualar
la y, y `ninja_intentar_atacar` solo acepta ninjas en `PARADO`, así que no ataca de
refilón. Un ninja ataca alineado: `dy ≤ 8` y `dx ≤ NINJA_DISTANCIA + 8`
(`ninja.c:259`).

### Nacer fuera, entrar, y no salir: `dentro` y `ninja_limitar_x`

`ninja_oleada(n, camara, zona)` (`ninja.c:51`) rellena las ranuras **libres** hasta
`zona + 1` ninjas (tope `NINJAS_MAX`): zona 0 → 1, zona 1 → 2, zona 2 → 3, y de ahí 3. Los
saca **fuera de la pantalla**: el primero por la izquierda (`camara − NINJA_ENTRADA`), el
segundo por la derecha (`camara + 256 + NINJA_ENTRADA`), alternando con `nacidos & 1`, y
con `mira_izq` ya puesto hacia dentro. El carril de nacimiento sale de la **ranura** (no
del orden de nacimiento): `CARRIL_ENTRADA[] = {200, 188, 212}` (`ninja.c:29`), tres líneas
distintas dentro del carril 184..216 de `mundo.h`. El descanso inicial también depende de
la ranura: `NINJA_ESPERA_INICIAL + 15·k` = 30, 45, 60 frames, para que no ataquen todos en
cuanto llegan (`(k << 4) - k` es `15k` sin multiplicar).

Mientras `dentro == 0` el ninja **no se detiene a `NINJA_DISTANCIA`**: sigue andando hacia
el objetivo (`!n->dentro && n->x != obj_x`, `ninja.c:177`). Cuando su x entra en
`[x_min, x_max]`, `ninja_limitar_x` (`ninja.c:217`) pone `dentro = 1` y **desde entonces lo
mantiene dentro**: si se pasa del rango, lo recorta. La pantalla pasa
`[camara + 32, camara + 256 − 32]` (`MARGEN_NINJA`, `pantalla_fase1.c:79`: la celda de 64 px
cabe entera).

Por qué hace falta: si el ninja se detuviera a 40 px de Tarou nada más nacer, y Tarou
estuviera pegado al borde de la pantalla, el ninja se pararía **fuera** de la pantalla, no
podría atacar (exige `dentro`) ni ser golpeado, y la oleada no se vencería nunca. El campo
`dentro` es esa memoria de un bit.

> **Hallazgo (ejecutado en el PC, no en la ROM).** Los dos márgenes no coinciden: Tarou
> puede llegar a `camara + 24` (`MARGEN_PANTALLA`, `mundo.h`) pero los ninjas solo cuentan
> como «dentro» desde `camara + 32`. Con Tarou pegado a la izquierda y un ninja que llega
> desde ahí, el ninja avanza hasta la x de Tarou (280) y se queda **sin entrar** (288 es el
> mínimo): no ataca mientras Tarou no se mueva. Salida real del experimento en
> `capturas/hallazgo_margenes.txt`. No es un bloqueo duro (Tarou puede golpearlo o
> apartarse; el golpe no lo probé), pero es una grieta entre dos constantes que viven en
> archivos distintos: una prueba que recorriera el borde la encontraría.
>
> **Corregido después en el juego (2026-10-26).** Ahora, mientras no está dentro, el ninja
> anda recto hacia donde mira al nacer (hacia dentro de la pantalla) sin pararse sobre
> Tarou, hasta que `ninja_limitar_x` lo da por dentro; `tests/prueba_ninja.c` tiene la
> prueba de este caso (Tarou en `camara + 24`). Esta unidad describe `641d7c0`: para repetir
> el experimento, haz el clon en ese commit.

### El testigo de ataque (turnos)

El **testigo de ataque** (*attack token*) es una regla de justicia: como mucho
`NINJA_TURNOS` = 2 ninjas atacan a la vez (spec §6; los demás esperan). No es un objeto:
es un **contador** que se calcula en cada frame y se pasa, ninja a ninja, dentro del bucle
de `pantalla_fase1.c:249-263`:

```c
turnos = ninja_atacando(ninjas);                 /* Cuántos dan un tajo ahora.          */
for (k = 0; k < NINJAS_MAX; k++)
{
    if (ninjas[k].vivo)
    {
        ninja_actualizar(ninjas + k, cuerpo.x, cuerpo.y);
        ninja_limitar_x(ninjas + k, ..., ...);
        if (ninja_intentar_atacar(ninjas + k, cuerpo.x, cuerpo.y,
                                  (u8)(turnos < NINJA_TURNOS)))
        {
            turnos++;                            /* El turno queda cogido.              */
        }
    }
}
```

`ninja_atacando` (`ninja.c:237`) cuenta los ninjas vivos en `ATACANDO`; `ninja_intentar_atacar`
(`:249`) recibe `turno_libre` y solo empieza el tajo si, además, el ninja está vivo,
**dentro**, en `PARADO`, **descansado** (`espera == 0`) y alineado. Si lo empieza, devuelve 1
y el bucle gasta un turno (`turnos++`): el siguiente ninja del mismo frame ya ve el contador
subido. Sin ese `turnos++` todos los que cumplan las condiciones atacarían a la vez; el
ejercicio lo comprueba rompiéndolo a propósito.

Tres cosas que la demo hace visibles:

- **La cadencia.** Un tajo dura 28 frames y después el ninja descansa 50
  (`NINJA_ESPERA`): un ciclo de **78 frames** por ninja (la demo, parte A: tajos en los
  cuadros 287, 365 y 443). El descanso solo baja cuando el ninja no está atacando (el tajo
  se sale antes del `espera--`).
- **El orden de las ranuras manda.** Con el turno libre, se lo queda el primero que cumpla
  las condiciones **en orden de ranura**: la 0 antes que la 1 antes que la 2. Con dos
  turnos, las ranuras 0 y 1 suelen atacar a la vez y la 2 espera a que se libere uno
  (demo, parte D). No hay rotación: es justicia aproximada, y a la larga las tres
  atacan lo mismo (parte C: 10 tajos cada una en 900 frames).
- **«Rodean o esperan» (spec §6) no existe todavía**: los ninjas que no tienen turno se
  quedan **parados** a 40 px (`p` en la tira de la demo), no rodean ni cambian de carril.

El testigo también abarata la CPU (spec §6): `combate_ninjas_resolver` solo hace trabajo
para quien tiene `ninja_golpe_activo`. (Esa parte es de la unidad 23.)

### Zonas, oleadas y el bloqueo de cámara

Una **zona** es, en el código de hoy, **un punto de la cámara** y una oleada que sale al
llegar a él. (El glosario del proyecto define «zona» como un tramo de la fase con su
escenario; en `oleadas.c` es solo el punto de bloqueo. Cuando exista el escenario por zonas
las dos ideas se encontrarán.) `oleadas.c` es una máquina de dos estados por zona:

```
          la cámara llega al punto
LIBRE  ───────────────────────────►  BLOQUEADA   (oleadas_actualizar devuelve 1 UNA vez)
  ▲                                      │
  └──────────────────────────────────────┘
      oleadas_vencer: actual++ y pasa a la zona siguiente
```

- `ZONA_CAMARA[] = {256, 640, 1024, 1536}` (`oleadas.c:25`): posiciones de la **cámara** (no
  de Tarou). `static const`: se queda en ROM (una `const` global se copiaría a WRAM, unidad 13).
- `oleadas_limite(o)` (`:35`): hasta dónde puede llegar la cámara ahora = el punto de la
  zona en curso, o `0xFFFF` si ya no quedan zonas.
- `oleadas_actualizar(o, camara)` (`:44`): si no está bloqueada y la cámara llegó al punto,
  la bloquea y devuelve 1. Devuelve 1 **una sola vez** por zona: en ese frame la pantalla
  llama a `ninja_oleada` (`pantalla_fase1.c:244`).
- `oleadas_vencer(o)` (`:54`): libera y avanza. Sin efecto si no estaba bloqueada. La
  pantalla la llama cuando `oleadas.bloqueada && ninja_vivos(ninjas) == 0`
  (`pantalla_fase1.c:278`): **la oleada se vence cuando no queda ningún ninja vivo**.

Lo que **bloquea** de verdad es la cámara, no `oleadas`. `camara_avanzar(camara, x_centro,
limite)` (`camara.c:42`) calcula la meta (el centro de Tarou en el centro de la pantalla),
la recorta a `CAMARA_MAX` (1792) y a `limite`, **nunca retrocede** y avanza como mucho
`CAMARA_PASO` = 2 px por frame (para que `fondo_nivel_actualizar` no se salte una columna
de 8 px). Si `limite` es 256, la cámara se queda en 256 por mucho que Tarou camine: Tarou
llega al borde de la pantalla y `mundo_limitar_x` le impide salir. Es decir, **el límite
de zona no es un muro del mundo: es el tope del scroll**.

Dos consecuencias que se verificaron en el PC:

- un punto de zona **por encima de `CAMARA_MAX`** (1792) **nunca bloquea**: la cámara no
  puede llegar, `oleadas_actualizar` no se dispara y la oleada no sale (con límite 1800 la
  cámara se queda en 1792; `capturas/hallazgo_margenes.txt`);
- un punto de zona **por debajo de 40** haría nacer al ninja de la izquierda en
  `camara − 40` con `u16`, que da la vuelta (con cámara 10, `x = 65506`). Las cuatro zonas de hoy
  están lejos de eso; si pasas una oleada a Tiled, el script debería rechazarlo.

> **Corregido después en el juego (2026-10-26).** Las dos cosas están arregladas en la rama
> principal, con pruebas: `oleadas_alcanzable` (en `oleadas.c`) recorta cada punto de zona a
> `CAMARA_MAX`, que ahora está en `camara.h`. Así, un punto más allá del final se bloquea en el
> final. `ninja_oleada` hace nacer al de la izquierda en `x = 0` si la cámara está a menos de
> `NINJA_ENTRADA` del principio. Esta unidad describe `641d7c0`: para repetir los
> experimentos, haz el clon en ese commit. El ejercicio de la quinta zona (1792) sigue
> valiendo tal cual.

### Cómo se dibuja un ninja: `ninja_sprite.c`

El dibujo está separado de la lógica (la lógica es pura y se prueba en el PC; esto toca la
VRAM y la OAM). Cada ranura tiene **dos entidades fijas** en la tabla de la capa de sprites
(unidad 21/22): el cuerpo (entradas 2..4) y su sombra (5..7); Tarou usa la 0 y la 1.

- **Hueco fijo de tiles.** El cuerpo de la ranura `k` siempre dibuja desde el tile
  `HUECOS[k]`. Cuando cambia el cuadro de animación, el cuadro **entero** (≤ 3 KB) se sube
  al hueco de esa ranura, y no hay doble búfer: el hueco se pisa de golpe en un VBlank y la
  OAM pasa al cuadro nuevo en ese mismo frame.
- **Una subida por frame, por turnos.** `subir` (`ninja_sprite.c:100`) recorre las tres
  ranuras empezando por `turno`; sube la primera cuyo `cuadro` no sea el que ya hay
  (`muestra[k]`), lo apunta en la lista de DMA (`dma_lista_copias`), avanza `turno` a la
  siguiente ranura y **sale**. Si dos ninjas cambian de cuadro en el mismo frame, el
  segundo espera al siguiente, con su cuadro anterior aún entero en su hueco: se ve
  consistente, solo se retrasa un frame. (La espera máxima medida en la prueba de carga fue
  de 2 frames: spec §9.2, no remedida aquí.)
- **Espejo sin subir nada.** Mirar a la izquierda no sube tiles: cambia a la **tabla de
  piezas espejada** del mismo cuadro (`cu.espejo` en vez de `cu.piezas`) y activa
  `SPRITE_HFLIP`. Solo se recalcula si cambió el cuadro o la mirada (`mirada[k]`).
- **Dos tiras en el mismo hueco.** Los cuadros 0..7 son de andar y 8..14 de atacar;
  `tira_de` (`:53`) decide de cuál de las dos tiras ROM es un cuadro, y con ella su banco
  (`ninja_bancos[]`): el cuadro de dibujo es un solo número y no importa qué tira lo contiene.
- **Tres paletas.** Los ninjas 0, 1 y 2 usan las paletas de sprites 1, 2 y 3
  (`SPRITE_PALETA(k + 1)`): tres variantes del mismo arte, provisionales. Es la «variante
  por paleta» de la spec (§6), sin más lógica detrás.
- **Orden por profundidad.** `sprites_y[ENT_NINJA + k] = p->y` (los pies) y al final
  `sprites_ordenar()`: es la clave del orden de la unidad 22.
- **Tocado = parpadeo.** No hay arte de «recibir golpe»: en `RECIBIR` y `DERRIBADO`, cuando
  `reloj & 2`, el cuerpo se manda a `y0 = 240` (fuera de la pantalla), así parpadea.

### La ROM autojugable `-DGUION_FASE1`

Para verificar sin manos se compila la ROM con `CFLAGS=-DGUION_FASE1`: `main.c` arranca
directamente la fase 1 (`secuencia_iniciar(PANTALLA_FASE1)`, `main.c:58`), `pantalla_fase1.c`
enciende el HUD de depuración (`:174`) y `mando_leer` lee el mando de una **tabla de
tiempo** en vez del hardware (`mando.c:335`). Es la misma idea del guion de la unidad 05/20,
con otra forma: `GUION_F1[]` (`mando.c:54`) son **254 tramos** `{hasta, teclas}`, cada uno
vale «hasta el cuadro `hasta` (exclusive), mantener estas teclas» con las teclas en bits
(`G_IZQ`, `G_DER`, `G_ARR`, `G_ABA`, `G_SAL`, `G_ATK`). La cuenta de cuadros (`cuadro_guion`)
la lleva `mando_iniciar`/`mando_leer`.

La tabla cuenta la historia así: 30 cuadros quieto; derecha hasta el cuadro 200 (la cámara
se bloquea en 256 y sale el primer ninja); se vuelve a la izquierda; pulsa **A cada 10
cuadros** hasta cerca del cuadro 500 (el combo de tres golpes encadena con A pulsada a partir del
cuadro 5 de cada golpe); derecha otra vez hasta el cuadro 780 (zona 1: salen dos ninjas,
uno por cada lado); se vuelve a la izquierda en el 1008 y repite el combo; y acaba en el
cuadro 1320 (22 s).

**Limitación importante:** la tabla **no mira lo que pasa**; es en bucle abierto. Si cambias
una cifra de la IA, de las zonas o del combate, los ninjas llegan antes o después y las
pulsaciones de A caen en el vacío: la oleada ya no se vence y la ROM se queda bloqueada
sin avisar. Por eso esta unidad trae `traza_24_guion.c`, que repite la tabla en el PC con la
lógica real y dice, con números, si las dos oleadas siguen venciéndose.

### Cómo se pasaría a Tiled (diseño, **no existe**)

La spec (§7) prevé objetos `oleada` en la capa de objetos de Tiled (propiedades `tipos`,
`cantidad`, `lado`, `retardo`, `zona`) y `limite_zona` (la x donde se bloquea la cámara),
más un script `scripts/preparar_zona.py`, sucesor de `preparar_nivel.py`, que escribiría
un `zonaN_datos.h`. Nada de eso está escrito. Lo que sí hay hoy es la forma de los datos
que reemplazaría: las dos tablas de constantes de arriba (`ZONA_CAMARA` en `oleadas.c` y
`CARRIL_ENTRADA` en `ninja.c`) y la regla «zona + 1 ninjas, tope 3» dentro de `ninja_oleada`.
La unidad 20 enseña la tubería equivalente para el tutorial (Tiled → script → `.h` → pruebas
de PC con los datos reales); cuando se escriba `preparar_zona.py`, esta unidad debe
rehacerse sobre él.

### Por venir (no enseñar como hecho)

| Lo que dice la ficha / la spec | Estado hoy |
|---|---|
| Ninja rápido: mantiene distancia, cambia de carril, shuriken | no existe (solo `NINJA_*`, un tipo) |
| Proyectiles por y | no existe |
| Kendoka pesado (bloquea, agarra) | no existe |
| Jefe por fases de vida (`jefe.c`) | no existe |
| `objetos/*`: rompibles, comida, armas | solo *stubs* que compilan |
| «Rodear» o «cambiar de carril» como estado | no existe: o anda al carril de Tarou, o espera parado |
| Variantes con más vida o más velocidad por paleta | solo la paleta cambia |
| Oleadas con `tipos`, `cantidad`, `lado`, `retardo` desde Tiled | constantes en `ninja.c` y `oleadas.c` |
| «GO →» en pantalla | el HUD de depuración enseña `GO` / `BLOQ`; no hay cartel de juego |

## La librería: PVSnesLib

Esta unidad **casi no usa la librería**: la IA y las zonas son lógica pura (sin `snes.h`) y
el dibujo va por la capa de sprites propia del juego (unidad 21). El poco SDK que toca:

| Función (firma exacta) | Qué hace / registros | Dónde |
|---|---|---|
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | copia a la CGRAM por DMA desde el color `address`; el juego carga aquí las paletas de sprites 1..3 (`128 + 16`, 3 × 32 bytes) | `src/enemigos/ninja_sprite.c` (`ninja_sprite_cargar`) |
| `void setMode(u8 mode, u8 size);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | modo de vídeo (`BGMODE $2105`) | `src/pantallas/pantalla_fase1.c` (`pantalla_fase1_iniciar`) |
| `void bgSetScroll(u8 bgNumber, u16 x, u16 y);` ([background.h](https://alekmaul.github.io/pvsneslib/background_8h.html)) | scroll de la capa, **en el acto**: se llama solo en el VBlank | `pantalla_fase1.c` (`pantalla_fase1_vblank`) |
| `void setModeHdmaColor(u8 channel, u8 *hdmatable);` y `void setModeHdmaReset(u8 channels);` | el degradado del cielo por HDMA y su apagado | `pantalla_fase1.c` |
| `padsCurrent(value)` / `padsDown(value)` (macros) | el mando real (el guion `GUION_FASE1` lo sustituye pero sigue leyendo SELECT y START) | `src/juego/mando.c` |

Las demás cosas con aspecto de librería son **del juego**: `SPRITE_PRIORIDAD`,
`SPRITE_PALETA`, `SPRITE_HFLIP` (`juego/sprites.h`), `VRAM_SPRITES` (`sistema/comun.h`),
`sprites_ent`, `sprites_ordenar`, `dma_lista_*`, `p16_cuadro` y `subida_*`.

**Lo que el SDK trae y el juego no usa: el motor de objetos** (`object.h`,
[página](https://alekmaul.github.io/pvsneslib/object_8h.html)). Su `void objInitEngine(void);`,
`void objInitFunctions(u8 objtype, void *initfct, void *updfct, void *reffct);`,
`u16 objNew(u8 objtype, u16 x, u16 y);`, `void objKill(u16 objhandle);` y `objUpdateAll()` dan
justo lo que `ninja.c` hace a mano: una tabla de objetos por tipo, con funciones de
inicio/actualización por tipo (el ejemplo `snes-examples/systems/objects/moveobjects`
llama a `objInitEngine`, `objInitFunctions(0, &testInit, &testUpdate, NULL)`,
`objLoadObjects` y, en el bucle, `objUpdateAll`). El juego no lo usa. No hay una decisión escrita sobre ello;
la razón probable, por analogía con el motor de mapas de la unidad 20, es que cada objeto del SDK es una `t_objs` de **64 bytes**
con su física (la cabecera `object.h` la describe como «object definition (64 bytes)»), y
las tres ranuras del juego miden 16 bytes, caben en la lógica pura de PC y se prueban allí.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| Constantes del ninja: distancia, vida, tajo, descanso, turnos, entrada | `src/enemigos/ninja.h` |
| Carriles de nacimiento (`CARRIL_ENTRADA`) | `src/enemigos/ninja.c:29` |
| `ninja_iniciar_todos`, `ninja_oleada` | `src/enemigos/ninja.c:31`, `:51` |
| `aturdido`, `ninja_recibir` | `ninja.c:88`, `:124` |
| `ninja_actualizar` (la IA de un frame) | `ninja.c:145` |
| `ninja_limitar_x` y el campo `dentro` | `ninja.c:217` |
| `ninja_atacando`, `ninja_intentar_atacar`, `ninja_golpe_activo`, `ninja_vivos` | `ninja.c:237`, `:249`, `:270`, `:276` |
| Tabla de zonas, `oleadas_limite`, `_actualizar`, `_vencer` | `src/juego/oleadas.c:25`, `:35`, `:44`, `:54` |
| `camara_avanzar`, `CAMARA_PASO` | `src/juego/camara.c:42`, `camara.h` |
| El bucle de IA y el reparto de turnos | `src/pantallas/pantalla_fase1.c:239-281` |
| Hueco de tiles por ranura, `tira_de`, `subir`, el dibujo | `src/enemigos/ninja_sprite.c:50`, `:53`, `:100`, `:133` |
| La ROM autojugable: la tabla y su lectura | `src/juego/mando.c:54`, `:335`; arranque en `src/sistema/main.c:58` |
| Pruebas: IA, oleadas, ataque y turnos | `tests/prueba_ninja.c`, `tests/prueba_oleadas.c`, `tests/prueba_combate.c` (desde «El ninja ataca», línea ~272) |

## Trampas

| Trampa | Qué pasa | Cómo se evita |
|---|---|---|
| Olvidar `turnos++` tras `ninja_intentar_atacar` | los tres atacan a la vez | el bucle de `pantalla_fase1.c` y la prueba del ejercicio |
| Un punto de zona > 1792 | la cámara no llega, la oleada no sale | límite = `CAMARA_MAX` como mucho |
| Un punto de zona < 40 | `camara − NINJA_ENTRADA` da la vuelta en `u16` | no hay zonas ahí; validar en el futuro script |
| `MARGEN_PANTALLA` (24) distinto de `MARGEN_NINJA` (32) | un ninja queda «fuera» junto a Tarou (hallazgo de arriba) | ajustar uno de los dos y añadir la prueba |
| Cambiar una cifra de la IA y fiarse de la ROM autojugable | la tabla de teclas va por tiempo, no por estado | repetir `traza_24_guion.c` |
| Editar `oleadas.h` o `ninja.h` y no hacer `make clean` | `snes_rules` no sigue las cabeceras: `pantalla_fase1.obj` conserva los valores viejos | `make clean` antes de `make` (el `Makefile` de `tests/` sí las sigue) |
| Más de un ninja subiendo cuadro por VBlank | el VBlank pasa de la línea 37 y la VRAM pierde datos (la spec midió 43) | una subida por frame, por turnos |
| `ninjas[k]` en un bucle por frame | sería `tcc__mul` si el tamaño no fuera potencia de 2 | recorrer con puntero (`n++`) y mantener `Ninja` en 16 bytes |

## La demo

`demo_24_oleada.c` usa el **código real** (`ninja.c`, `oleadas.c`, `camara.c`) y reescribe
el bloque de IA de `pantalla_fase1.c` en una función (`un_cuadro`), con los turnos como
parámetro. Desde esta carpeta:

```
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_24.exe demo_24_oleada.c ../../src/enemigos/ninja.c ../../src/juego/oleadas.c ../../src/juego/camara.c
.\demo_24.exe
```

(`.\demo_24.exe a`, `b`, `c` o `d` ejecuta una sola parte.) Salida real en
`capturas/salida_demo_24_oleada.txt`. Qué mirar:

- **Parte A (sucesos):** la cámara llega a 256 en el cuadro 159 y la zona 0 se bloquea con
  1 ninja; el ninja entra en pantalla en el 230 (nació en `camara − 40` = 216, así que
  anda ~72 px hasta `camara + 32` = 288); da tajos en el 287, 365 y 443 (cada **78**
  cuadros); el «jugador» lo derriba en el 489 y la oleada se vence en el 524. En la zona 1
  nacen dos (uno por cada lado) y **atacan los dos a la vez** («2 de 2»): con el límite en
  2 no hay tercero que esperar.
- **Parte B:** la misma historia, un cuadro de cada 20, una columna por ranura.
- **Parte C:** tres ninjas, 900 cuadros, con 1, 2 y 3 turnos:

  ```
  turnos | max a la vez | tajos ranura 0 | ranura 1 | ranura 2 | total
       1 |            1 |              9 |        9 |        9 |    27
       2 |            2 |             10 |       10 |       10 |    30
       3 |            3 |             10 |       10 |       10 |    30
  ```

  Pasar de 1 a 2 turnos solo sube de 27 a 30 tajos en total: el límite casi no cambia
  *cuánto* se ataca, cambia **cuántos a la vez** golpean a Tarou. (Con 1 turno faltan 3
  tajos de 30 porque tres tajos de 28 cuadros no caben en el ciclo de 78: 84 > 78, y
  alguien espera.) Tres turnos es lo mismo que ninguno, porque solo hay 3 ranuras.
- **Parte D:** las tiras de estados con 1 y 2 turnos (cada letra son 6 cuadros): con 2,
  las ranuras 0 y 1 se **sincronizan** (atacan y descansan al unísono) y la 2 va
  desfasada; con 1, se turnan de uno en uno. Es el comportamiento real del ordenamiento
  por ranura.

Antes de ejecutarla, intenta predecir: ¿cuántos cuadros pasan entre el primer tajo de un
ninja y el siguiente? ¿Y cuántos ninjas atacan en el mismo cuadro con 2 turnos?

### La réplica de la ROM autojugable

`traza_24_guion.c` repite en el PC `pantalla_fase1_actualizar` con la tabla `GUION_F1` real
(la saca de `mando.c` con `sed`) y la lógica pura real (`mundo.c`, `combate.c`, `ninja.c`,
`oleadas.c`, `camara.c`). Los comandos están en su cabecera. Salida real
(`capturas/salida_traza_24_guion.txt`):

```
tramos de la tabla GUION_F1: 254
[f= 189] zona 0 bloqueada en camara 256: nacen 1 ninja(s); Tarou en x=384 vida=24
[f= 463] oleada VENCIDA; siguiente zona 1; Tarou vida=24
[f= 716] zona 1 bloqueada en camara 640: nacen 2 ninja(s); Tarou en x=768 vida=24
[f= 971] Tarou recibe un tajo: vida 24 -> 22
[f=1130] oleada VENCIDA; siguiente zona 2; Tarou vida=22
fin de la tabla (cuadro 1400): zona en curso 2, camara 738, Tarou x=866 vida=22, ninjas vivos 0
```

La tabla vence las dos oleadas (en los cuadros 463 y 1130, dentro de sus 1320) y Tarou
recibe **un** tajo en la segunda. Esto es una réplica en el PC: la ROM real puede
diferir en lo que gcc y tcc816 hagan distinto, y las capturas de Mesen siguen
pendientes (esta sesión no ejecutó Mesen).

## Ejercicio y criterio de aprobación

**Ejercicio (juego real, en un clon, con TDD):** `ejercicio_24.md`. Se añade una **quinta
zona** al final del nivel (cámara 1792 = `CAMARA_MAX`) escribiendo **primero** la prueba que
falla en `tests/prueba_oleadas.c`, luego dos líneas de implementación, se arregla la prueba
vieja que codificaba «cuatro zonas», se escribe una prueba de simulación del testigo en
`tests/prueba_ninja.c` y se comprueba que detecta el fallo si se rompe el `turnos++`.
Después se compila la ROM normal y la `-DGUION_FASE1` (solo compilar) y se repite la
réplica del guion. Solución: `solucion/`.

**Aprobación:**

1. la demo compila con `gcc -std=c99 -Wall -Wextra` sin avisos y su salida coincide con
   `capturas/salida_demo_24_oleada.txt` (es determinista);
2. con la prueba nueva y la implementación aún sin escribir, `pruebas.exe` da **3 fallos**,
   todos de `prueba_oleadas.c` (`capturas/pruebas_rojo_quinta_zona.txt`);
3. con la implementación y la prueba vieja sin arreglar, otros 3
   (`capturas/pruebas_tras_implementar_sin_arreglar_la_vieja.txt`);
4. con todo hecho: **568 pruebas, 0 fallos** (`capturas/pruebas_solucion.txt`; antes del
   ejercicio eran 560);
5. con el `turnos++` del test eliminado, falla `maximo == NINJA_TURNOS`
   (`capturas/pruebas_mutacion_sin_turnos_mas_mas.txt`);
6. la ROM normal y la `-DGUION_FASE1` compilan con «No warnings or errors found»
   (`capturas/compilacion.txt`) y la réplica del guion da la **misma** salida.


## Anexo (2026-11-05): qué cambió después

La unidad se escribió con el ninja provisional de 64×96 y una sola clase de enemigo. Desde
entonces el ninja tiene el arte de «Ninja 1» y **tres tipos**:

| Qué | Dónde | La idea |
|---|---|---|
| **Tres tipos, uno por ranura** | `TipoNinja` y `Ninja.tipo` (`src/enemigos/ninja.h`); `ninja_oleada` (`ninja.c`) | La ranura k nace siempre como el tipo k: normal (rojo, ~80 px), mediano (amarillo, ~90 px) y grande (morado, ~100 px). Las oleadas traen 1, luego normal + mediano, luego los tres. |
| **Vida y daño por tipo** | `VIDA_TIPO[] = {4, 6, 8}` y `DANIO_TIPO` (2, 3, 4) en `ninja.c`; `ninja_danio` | Tablas `static const` indexadas por el tipo, en vez de `if` por tipo. |
| **El grande, más lento** | `ninja_actualizar` (`ninja.c:208`): `(n->reloj & 3) == 0` | Anda 3 de cada 4 frames: una máscara, sin dividir. |
| **Qué se dibuja** | `ninja_dibujo()` y los `NINJA_DIB_*` (`ninja.h`) | Un número de dibujo (0..41) por estado y reloj: guardia (8), caminar (6), puñetazo (3, puño fuera mientras la caja está activa), recibir (6), derribado (7), levantarse (5), KO (7). `ninja_sprite.c` lo traduce a tira y cuadro (`tira_de`). |
| **Derribo largo** | `NINJA_DERRIBO_FRAMES` 64 | Cae (21), en el suelo (20) y se levanta (20); solo el KO tumbado parpadea antes de irse. |
| **El struct creció** | `Ninja`: **32 bytes** (antes 16) | Sigue siendo potencia de 2: `ninjas + k` sigue sin `tcc__mul`. Si un campo nuevo lo pasara a 33, habría que rellenarlo hasta 64 o recorrer con puntero. |
| **Arte por tipo** | `res/personajes/ninja/`, `ninja_mediano/`, `ninja_grande/`; celdas 96×96 y 128×112 | El mismo dibujo recoloreado (anexo de la unidad 19); cada tipo, su paleta de sprites (1, 2, 3). |

Lo que la tabla «Por venir» de arriba decía de las variantes («solo la paleta cambia») ya no
es cierto: cambian la paleta, el tamaño, la vida, el daño y la velocidad. El ninja rápido con
shuriken, el kendoka y el jefe siguen sin existir.

**Ejercicio breve (PC, 15 minutos):** en un clon, cambia la vida del grande a 10 en
`VIDA_TIPO` y ejecuta las pruebas de PC. ¿Falla alguna? Si no falla ninguna, escribe la que
faltaba: «el combo de 3 golpes (1 + 1 + 2) no tumba del todo al grande» en
`tests/prueba_ninja.c`. (Es el mismo razonamiento que el TODO 8 del ejercicio: una cifra de
diseño que ninguna prueba protege.)
