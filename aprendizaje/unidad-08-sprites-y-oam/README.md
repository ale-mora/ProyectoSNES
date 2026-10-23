# Unidad 08 — Sprites estáticos y la OAM

**Objetivo:** poner un personaje en pantalla con la OAM, animarlo cambiando
de tile y entender los dos límites que más han mordido al proyecto: la
frontera de los 256 tiles y el sprite escondido que asoma por arriba.

**Prerrequisitos:** 06 (tiles, paletas, CGRAM, VRAM en palabras, `gfx4snes`)
y, a través de ella, 03 y 04. **Práctica:** mini-ROM. **Tiempo
orientativo:** una o dos tardes.

Los fondos de las unidades 06 y 07 son cuadrículas: cada tile en su celda. Un
personaje necesita otra cosa: una imagen que se pueda poner en **cualquier**
píxel, encima de los fondos, y moverla sin reescribir ningún mapa. Eso es un
sprite.

## Material

| Archivo | Qué es |
|---|---|
| `rom/` | mini-ROM: Bárbara (64×64) animada y movida con la cruceta; esqueleto con TODO 1..2 (el lobo) |
| `ejercicio_08.md` | A: el lobo con otra paleta; B: el cruce de tablas (tile 192); C: y = 240 |
| `solucion/sprites.c` | la mini-ROM completa (con `#ifndef` para los experimentos B y C) |
| `capturas/` | compilaciones (esqueleto, solución, `TILE_LOBO=192`, `LOBO_Y=240`) y la comprobación de la fórmula del tile |

## Conceptos

### Sprite (OBJ)

Un **sprite** (en la documentación de Nintendo, **OBJ**) es una imagen
cuadrada de 8×8 a 64×64 px que la PPU dibuja en una posición cualquiera de la
pantalla, por encima o por debajo de los fondos según su prioridad
(→ glosario: Sprite). Usa tiles de 4bpp, como un fondo de 16 colores, pero
no tiene mapa: un sprite grande son tiles **seguidos** en una forma fija
(abajo). Hay 128 a la vez; un personaje mayor que 64×64 se arma con varios
(metasprite, unidad 09).

### La OAM

La **OAM** (*object attribute memory*, → glosario: OAM) es la tabla de los
128 sprites, dentro de la PPU: 544 bytes. Cada entrada tiene:

| Campo | Bits | Qué es |
|---|---|---|
| x | 9 | columna de pantalla (−256..255; el bit 8 va en la tabla alta) |
| y | 8 | fila de pantalla (0..255: **da la vuelta**) |
| tile | 9 | primer tile del sprite (0..511) |
| paleta | 3 | paleta de sprites 0..7 = CGRAM `128 + 16 × p` |
| prioridad | 2 | 0..3: entre qué capas de fondo va (orden de capas, unidad 06) |
| volteo H / V | 1 + 1 | espejo horizontal y vertical, gratis |
| tamaño | 1 | pequeño o grande (tabla alta) |

Son 4 bytes por sprite (512) más una **tabla alta** de 32 bytes con 2 bits
por sprite (el bit 8 de x y el tamaño). PVSnesLib guarda una copia en la WRAM
(`oamMemory`) que las funciones `oam*` modifican, y la NMI del SDK la copia a
la OAM en cada VBlank: por eso basta con llamar a `oamSet` durante el frame y
a `WaitForVBlank()`, como en `SimpleSprite`. Y por eso el primer parámetro
de esas funciones es el **número de entrada × 4**: es el desplazamiento en
bytes dentro de la copia.

### Tamaños: solo dos a la vez

Todos los sprites de la pantalla comparten **dos** tamaños, «pequeño» y
«grande», que se eligen para toda la OAM en el registro `OBSEL` (`$2101`); cada
sprite dice si es pequeño o grande. PVSnesLib nombra las combinaciones
(`sprite.h`):

| Constante | Pequeño | Grande | En el juego |
|---|---|---|---|
| `OBJ_SIZE8_L16` | 8×8 | 16×16 | |
| `OBJ_SIZE16_L32` | 16×16 | 32×32 | el nivel: las piezas de Tarou son «grandes» de 32 |
| `OBJ_SIZE32_L64` | 32×32 | 64×64 | intro y título: Tarou en piezas «pequeñas» de 32, Bárbara «grande» de 64 |

(Hay tres más: `OBJ_SIZE8_L32`, `OBJ_SIZE8_L64`, `OBJ_SIZE16_L64`.)

### 512 tiles, en dos tablas de 256

Los tiles de sprites viven en una zona de la VRAM de 16 KB: **512 tiles de
4bpp** como máximo (→ glosario: VRAM). El juego la pone en `VRAM_SPRITES` =
0x0000 (`src/sistema/comun.h`). Esos 512 son, para la PPU, **dos tablas de
256** (el bit 8 del número de tile elige la tabla).

Un sprite grande no se guarda «de corrido»: la PPU ve la VRAM de sprites como
una hoja de **16 tiles de ancho** (128 px). Un sprite de 64×64 que empieza en
el tile N toma 8 tiles de la fila de N, 8 de la fila de debajo (N + 16), y así
8 filas:

```
fila 0:  N      N+1    ...  N+7
fila 1:  N+16   N+17   ...  N+23
...
fila 7:  N+112  ...         N+119
```

Y aquí está la trampa: al bajar de fila la PPU suma 16 **dentro de los 8
bits bajos** del número de tile: el bit 8 (la tabla) no cambia. **Un sprite
grande no puede cruzar de una tabla a otra:** si la pasa, da la vuelta dentro
de la suya. Con N = 192:

```
fila 0..3:  192, 208, 224, 240     (bien)
fila 4..7:  (192 + 64) & 255 = 0, 16, 32, 48   ← tiles del principio de la tabla 0
```

Verificado en Mesen el 2026-10-19 con Bárbara en el tile 192 (cabecera de
`src/pantallas/personajes.c`, bloque «LÍMITE»): la mitad de abajo de sus
cuadros mostraba tiles de Tarou. Por eso Bárbara pasó al tile 256 (la tabla
1 entera) y Tarou se quedó con la 0. Es la parte B del ejercicio.

### Cómo empaqueta `gfx4snes -s 64`

`gfx4snes -s 64` corta la tira (aquí 256×64: 4 cuadros) en **bloques de
64×64** y los coloca **de dos en dos en filas de 128 px**, la misma hoja de 16
tiles de ancho que lee la PPU. Así cada bloque queda en la forma que un
sprite grande espera, y el bloque `n` empieza en el tile:

```
(n / 2) * 128 + (n % 2) * 8          →   0, 8, 128, 136
```

`n / 2` es la fila de bloques (cada una, 8 filas de tiles × 16 = 128 tiles) y
`n % 2` la columna (8 tiles). La mini-ROM lo escribe sin dividir:
`((n >> 1) << 7) | ((n & 1) << 3)`. Comprobado con un script sobre el `.pic`
de Bárbara: los píxeles opacos de cada cuadro de la tira coinciden con esos
tiles (`capturas/comprobacion_empaquetado_s64.txt`), y en Mesen se comprueba
con el Tile Viewer (ejercicio, parte A3). Para piezas de 32 (`-s 32`) son 4
por fila: `(n / 4) * 64 + (n % 4) * 4` (`tile_del_bloque32` en
`personajes.c`).

Una tira de 4 cuadros de 64×64 son 4 × 64 = 256 tiles (8 KB): **una tabla
entera**.

### Paletas de sprites

Las entradas 128..255 de la CGRAM son de los sprites: 8 paletas de 16, la `p`
en `128 + 16 × p` (→ glosario: Paleta). Como en los fondos, el índice 0 es
transparente. En el juego: Tarou en la 0 (128..143), Bárbara en la 1
(144..159) (`docs/hardware-snes-por-modulo.md` §4). Dos sprites con el mismo
dibujo y distinta paleta son gratis en VRAM: es el truco clásico de los
enemigos de colores.

### Esconder un sprite (y por qué y = 240 falla)

La OAM no tiene un bit de «apagado»: un sprite se esconde **poniéndolo fuera
de la pantalla**. La pantalla mide 224 líneas y la y de la OAM tiene 8 bits:
**da la vuelta a los 256**. Un sprite de 64 de alto en y = 240 ocupa las
líneas 240..303; las 256..303 son, para la PPU, las 0..47: sus 48 filas de
abajo **asoman por arriba**. El truco clásico (y = 240) solo vale para
sprites de 16 de alto o menos. Es la parte C del ejercicio.

El juego esconde así (`esconder` en `src/pantallas/personajes.c` y en
`src/juego/tarou_sprite.c`): tamaño que no llegue a dar la vuelta, **y = 224**
(la primera línea bajo la pantalla; 224 + 32 = 256) y **x = 256** (el bit 8
de x: fuera por la derecha), con `oamSetEx(..., OBJ_HIDE)` + `oamSetXY(...,
256, 224)`. Al volver a enseñarlo, `oamSet` repone posición y `oamSetEx`
tamaño. `tarou_sprite_ocultar` usa `oamClear(0, 0)` (las 128 entradas).

### Límites por línea

Además de 128 sprites en total: como mucho **32 sprites** y **34 trozos de 8
px** por línea de pantalla. Lo que pasa de ahí no se dibuja en esa línea. Un
Tarou de 96 px de ancho son 12 trozos en las líneas que cruzan sus 3 piezas.

## La librería: PVSnesLib

Firmas de `tools/pvsneslib/pvsneslib/include/snes/sprite.h`
([sprite.h](https://alekmaul.github.io/pvsneslib/sprite_8h.html)). El `id` es
**entrada × 4** (0, 4, 8... 508).

| Función | Qué hace | Hardware |
|---|---|---|
| `void oamInit(void);` | prepara la copia de la OAM y esconde los 128 sprites | búfer en WRAM |
| `void oamInitGfxSet(u8 *tileSource, u16 tileSize, u8 *tilePalette, u16 paletteSize, u8 tilePaletteNumber, u16 address, u8 oamsize);` | copia los tiles a `address` (VRAM, pasos de 8 K palabras), la paleta a la de sprites `tilePaletteNumber` y fija los tamaños | DMA a VRAM y CGRAM; `OBSEL $2101` |
| `void oamInitGfxAttr(u16 address, u8 oamsize);` | solo base y tamaños, sin copiar (el juego copia aparte) | `OBSEL` |
| `void oamSet(u16 id, u16 xspr, u16 yspr, u8 priority, u8 hflip, u8 vflip, u16 gfxoffset, u8 paletteoffset);` | escribe la entrada entera: posición, prioridad 0..3, volteos, primer tile (0..511), paleta 0..7 | búfer; la NMI lo copia (`OAMADD/OAMDATA $2102-$2104`) |
| `void oamSetEx(u16 id, u8 size, u8 hide);` | tamaño (`OBJ_SMALL`/`OBJ_LARGE`) y `OBJ_SHOW`/`OBJ_HIDE` (tabla alta) | búfer |
| `void oamSetXY(u16 id, u16 xspr, u16 yspr);` | solo la posición | búfer |
| `void oamSetVisible(u16 id, u8 hide);` | enseña o esconde; según su cabecera, `OBJ_HIDE` lo lleva a x = 255, y = 240, y al volver a enseñarlo hay que reponer la posición con `oamSetXY` | búfer |
| `void oamClear(u16 first, u8 numEntries);` | esconde un rango (con 0 entradas, las 128) | búfer |
| `void oamUpdate(void);` | copia el búfer a la OAM («Write all OBJ descriptors to OAM»). La NMI del SDK ya lo hace en cada VBlank: ni `SimpleSprite`, ni la mini-ROM, ni el juego la llaman | `OAMADD/OAMDATA` |
| `void dmaCopyVram(u8 *source, u16 address, u16 size);` / `void dmaCopyCGram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | tiles y paleta de un segundo personaje (unidad 06) | DMA |

`OBJ_SMALL` = 0, `OBJ_LARGE` = 1, `OBJ_SHOW` = 0, `OBJ_HIDE` = 1 (macros de
`sprite.h`). Ejemplo del SDK equivalente:
`tools/pvsneslib/snes-examples/graphics/Sprites/SimpleSprite` (un sprite de
32×32 con `oamInitGfxSet` + `oamSet` + `oamSetEx`), del que parte la
mini-ROM. Para esta unidad no hay que bajar a ningún registro a mano.

**Por qué `y = 240` no se ve en la tabla:** la advertencia de
`oamSetVisible` describe el escondite clásico, que con sprites de 64 (y con
el Tarou de 32 de alto en el nivel) asoma; por eso el juego no la usa y tiene
su propio `esconder`.

## La mini-ROM (`rom/`)

- `res/barbara.png` y `res/lobo.png`: las tiras del pack
  (`res/personajes/barbara_samurai.png` y `lobo_samurai.png`), 4 cuadros de
  64×64, convertidas con la regla del juego (`gfx4snes -s 64 -o 16 -u 16 -p`).
- Bárbara: `oamInitGfxSet` con `OBJ_SIZE32_L64` a 0x0000 (tiles 0..255),
  paleta de sprites 0. Entrada 0, grande. Un cuadro cada 8 frames; la cruceta
  la mueve 1 px por frame sin salir de la pantalla.
- El lobo (TODO 1 y 2): sus 2 primeros cuadros (4 KB) en el tile
  `TILE_LOBO` = 256 (VRAM 0x1000), paleta 1, entrada 1, volteado.
- **La raya oscura a la izquierda del lobo es del arte, no un fallo.** En la
  tira original (`res/personajes/lobo_samurai.png`) la espada llega hasta el
  borde de cada celda de 64 px, y el contorno de la punta de la espada del
  cuadro siguiente se cuela 1 px en la última columna de la celda anterior
  (x = 63, 127 y 191, filas 22..27, índice 14, casi negro), separado del
  cuerpo. Con el volteo horizontal esa columna pasa a x = 0 del sprite: una
  rayita vertical de 6 px a la izquierda, a la altura de la punta de la
  espada (arriba del lomo). La mini-ROM usa la tira tal cual; para el juego (etapa de
  enemigos) habrá que limpiar esas columnas en el PNG.
- Con `-DTILE_LOBO=192` (parte B) el lobo casi desaparece y Bárbara pierde
  el cuerpo en sus cuadros 2 y 3: el porqué completo, con una simulación de
  la VRAM hecha en el PC, está en el enunciado.
- `#ifndef TILE_LOBO` / `#ifndef LOBO_Y`: los experimentos se compilan con
  `CFLAGS` sin tocar el código (como `-DGUION_PRUEBA` en el juego).

Las cuatro compilaciones (esqueleto, solución, `-DTILE_LOBO=192`,
`-DLOBO_Y=240`): `No warnings or errors found.` (`capturas/`).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| El bloque «LÍMITE»: dos tablas de 256, Bárbara en el 192 se corrompía, por qué la tira estática de Tarou son 2 cuadros (192 tiles) | cabecera de `src/pantallas/personajes.c` (líneas 18-26) |
| La fórmula de `-s 64` y la de `-s 32` | `tile_del_bloque64` y `tile_del_bloque32`, `src/pantallas/personajes.c:102-111` |
| Bárbara en el tile 256 = VRAM 0x1000 (256 × 16 palabras) | `TILE_BASE_BARBARA`, `VRAM_BARBARA`, `personajes.c:83-84` |
| Carga: `oamInit`, `oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE32_L64)`, `dmaCopyVram`, `dmaCopyCGram` | `personajes_cargar`, `personajes.c:133` |
| Esconder bien (y = 224, x = 256) y la explicación del fallo de y = 240 | `esconder`, `personajes.c:113-124`; `personajes_ocultar` |
| Dibujar: `oamSet` + `oamSetEx` por pieza, con volteo en espejo | `dibujar`, `personajes.c:153` |
| El nivel: `OBJ_SIZE16_L32` (piezas «grandes» de 32), `esconder` con `OBJ_LARGE` | `tarou_sprite_cargar` y `esconder`, `src/juego/tarou_sprite.c:148-169` |
| Mapa de la OAM (entradas 0..5 Tarou, 6 Bárbara) y de la VRAM de sprites | `docs/hardware-snes-por-modulo.md` §3 y §5 |

La OAM del juego en una línea: Tarou usa las entradas 0..5 (hasta 6 piezas
de 32×32, unidad 09) y Bárbara la 6; las 7..127 están libres para enemigos y
objetos. Y una nota para la unidad 14: `tile_del_bloque64` del juego recibe un
`u8` y divide; el compilador llama a su rutina de división (`tcc__div`). La
mini-ROM usa desplazamientos.

## Trampas

- **`id` = entrada × 4.** `oamSet(1, ...)` no es la entrada 1: es un byte
  dentro de la entrada 0, y lo estropea.
- **Un sprite grande no cruza la frontera de los 256 tiles** (ni la de una
  fila de 16: un 64×64 que empiece en el tile 12 también da la vuelta, ahora
  en horizontal). Elige primeros tiles alineados: múltiplos de 8 dentro de su
  fila.
- **y = 240 no esconde un sprite alto**: asoma por arriba. Esconder con
  y = 224, x = 256 y un tamaño que no llegue a 256.
- **Tiles de sprite en palabras**: el tile `t` está en `VRAM_SPRITES + t × 16`
  (16 palabras = 32 bytes por tile de 4bpp).
- **Paleta de sprites `p` = CGRAM `128 + 16p`**, no `16p`: con `16p` se pisa
  una paleta de fondo.
- **Los sprites no siguen al scroll**: su x es de pantalla. En el nivel,
  pantalla = mundo − cámara (unidad 07).
- **`oamSetEx(OBJ_HIDE)` y después `oamSet`**: `oamSet` no vuelve a enseñar
  el sprite; hay que llamar a `oamSetEx(..., OBJ_SHOW)` (como hace `dibujar`).

## Ejercicio y criterio de aprobación

Enunciado completo en `ejercicio_08.md`:

- **A.** El lobo: tiles en el 256 y paleta de sprites 1 (TODO 1), su entrada
  de la OAM volteada (TODO 2); comprobar la fórmula del tile en el Tile Viewer.
- **B.** Compilar con `-DTILE_LOBO=192` y explicar la corrupción.
- **C.** Compilar con `-DLOBO_Y=240` y explicar por qué asoma por arriba.

**Aprobación:** captura con los dos sprites (cada uno con su paleta), la
fórmula verificada en el Tile Viewer de Mesen (tiles 0, 8, 128, 136 y 256) y
la captura del cruce de tablas.

## Lo que viene

- **Unidad 09:** un personaje de 96×64 con hasta 6 piezas (metasprite) y
  cuadros que se copian a la VRAM según hacen falta, porque 13 animaciones no
  caben en 512 tiles.
- **Unidad 16:** el cruce de tablas como caso de depuración en Mesen.
