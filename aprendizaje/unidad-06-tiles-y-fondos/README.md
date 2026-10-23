# Unidad 06 — Tiles, paletas, tilemaps y fondos

**Objetivo:** cargar y mostrar un fondo sabiendo exactamente qué bytes van a
la VRAM y cuáles a la CGRAM, y cambiar colores y celdas del mapa con el
juego en marcha.

**Prerrequisitos:** 03 (la tubería: sección, banco, `Makefile`) y 04 (frame,
VBlank, `WaitForVBlank`, pantalla apagada). **Práctica:** mini-ROM + sobre el
juego real (visores de Mesen) + una cuenta en el PC. **Tiempo orientativo:**
dos tardes.

La SNES no dibuja píxeles sueltos: la CPU deja en la memoria de vídeo unos
cuadraditos de 8×8 y una cuadrícula que dice cuál va en cada sitio, y la PPU
(→ glosario: SNES) compone la imagen línea a línea, 60 veces por segundo. Esta
unidad es esa idea, byte a byte. Al final se mira la capa cercana del
tutorial, que mide 2080 px de ancho y cabe en 260 cuadraditos. (El tutorial
es la Revisión 2, el juego de plataformas, guardada en el tag
`plataformas-r2`; el juego actual es un beat 'em up,
`docs/diseno-beat-em-up.md`, con sus propias zonas. La lección sobre tiles,
mapas y deduplicación vale igual para ellas.)

## Material

| Archivo | Qué es |
|---|---|
| `demo_06_tiles_y_mapa.c` | PC: un tile codificado y decodificado, entradas de mapa, colores |
| `rom/` | mini-ROM: las montañas del título + texto; esqueleto del ejercicio (TODO 1..4) |
| `ejercicio_06.md` | el enunciado: paleta y mapa en marcha (A), Mesen (B), recuento de tiles (C) |
| `solucion/fondo.c` | la mini-ROM con los cuatro TODO resueltos |
| `capturas/` | salida de la demo, resumen de compilación de las dos ROM y cifras de la parte C |

Para la demo de PC (como en la unidad 00):

```
cd E:\ProyectoSNES\aprendizaje\unidad-06-tiles-y-fondos
gcc -std=c99 -Wall -Wextra -o demo_06.exe demo_06_tiles_y_mapa.c
.\demo_06.exe
```

## Conceptos

### Tile y bits por píxel

Un **tile** es un cuadrado de **8×8 píxeles** (→ glosario: Tile). No guarda
colores, sino **índices** de color: «este píxel usa el color 3 de su paleta».
Cuántos bits se reservan por píxel (**bpp**, *bits per pixel*) decide cuántos
colores puede usar el tile y cuánto ocupa:

| bpp | Colores por tile | Bytes por tile | En el juego |
|---|---|---|---|
| 2 | 4 | 16 | la fuente del título y del HUD (`ark_consola_2bpp`) |
| 4 | 16 | 32 | casi todo: logos, capas del título y del nivel, sprites |
| 8 | 256 | 64 | las escenas 1 y 2 de la intro (Mode 3) |

La cuenta es `64 píxeles × bpp / 8`. Paso 1 de la demo.

**Formato planar.** Los bits de un píxel no van juntos: la SNES guarda el tile
por **planos** (un plano = un bit de cada píxel). En 4bpp, los bytes 0..15
llevan los planos 0 y 1 intercalados fila a fila, y los bytes 16..31 los
planos 2 y 3. El bit 7 de cada byte es el píxel de la izquierda. El paso 2 de
la demo dibuja una flecha con índices 0..3 y la codifica:

```
   18 00 24 18 5A 3C BD 7E 3C 18 3C 18 3C 18 3C 00 | 00 00 ... 00
   ...11...      fila 0: plano 0 = 0x18 (00011000), plano 1 = 0x00
   ..1221..      fila 1: plano 0 = 0x24 (00100100), plano 1 = 0x18 (00011000)
```

Nunca se escribe esto a mano: lo hace `gfx4snes` a partir de un PNG. Pero
saberlo explica por qué un tile de 4bpp «son 32 bytes» y por qué no se puede
cambiar un píxel con una sola escritura.

### Paleta y color de 15 bits

Una **paleta** es un grupo de colores que comparten los tiles que la usan
(→ glosario: Paleta). Los colores viven en la **CGRAM** (→ glosario: CGRAM):
256 entradas de 2 bytes. Cada color son **15 bits**, 5 por componente:
`0bbbbbgggggrrrrr` (0..31 de rojo, verde y azul). PVSnesLib trae dos macros
(`video.h`): `RGB5(r, g, b)` con componentes 0..31 y `RGB8(r, g, b)` con
0..255 (las divide entre 8). Paso 4 de la demo: `RGB8(232, 120, 40)` =
`0x15FD`.

En Mode 1, las entradas 0..127 son de los fondos: **8 paletas de 16** (la
paleta `p` empieza en la entrada `p * 16`); las 128..255, de los sprites
(unidad 08). El mapa de la CGRAM del juego está en
`docs/hardware-snes-por-modulo.md` §4: paleta 0 la fuente, 1 la capa
cercana, 2 la lejana.

**El índice 0 es transparente.** Un píxel de índice 0 no pinta: deja ver la
capa de detrás. Donde ninguna capa pinta se ve el **color de fondo**
(*backdrop*), que es la entrada **0 de la CGRAM**. En el título y en el nivel
ese color lo cambia un HDMA en cada línea: es el cielo degradado (unidad 07 y
12). En la mini-ROM, el TODO 1 cambia la entrada 0 y el «cielo» cambia aunque
ningún tile tenga ese color.

### Tilemap (mapa)

Un **tilemap** es la cuadrícula que dice qué tile va en cada celda de 8×8 de
una capa (→ glosario: Tilemap). Cada celda es una **palabra de 16 bits**
(una *entrada*):

```
 bit  15   14   13   12 11 10   9 8 7 6 5 4 3 2 1 0
     voltV voltH prio  paleta    número de tile (0..1023)
```

- **número de tile**: cuál, contando desde la base de tiles de la capa;
- **paleta** (0..7): qué grupo de 16 colores usa; por eso el `.map` de las
  montañas, convertido con `-e 2`, tiene entradas `0x08xx`: `2 << 10`;
- **prioridad**: delante o detrás de otras capas (abajo, «Orden de las capas»);
- **volteo horizontal y vertical**: la PPU dibuja el tile espejado. **Gratis**:
  el mismo tile sirve para su versión izquierda y derecha (→ glosario: Volteo
  de tile).

PVSnesLib tiene macros para armar una entrada (`background.h`): `BG_TIL_NUM(n)`,
`BG_TIL_PAL(n)`, `BG_TIL_PRIO`, `BG_TIL_FLIPX`, `BG_TIL_FLIPY`. El paso 3 de la
demo las combina y decodifica:

```
  tile 17, paleta 2        0x0811 = tile   17  paleta 2  prioridad 0  voltH 0  voltV 0
  el mismo, voltH          0x4811 = tile   17  paleta 2  prioridad 0  voltH 1  voltV 0
  HUD: TILE('A')           0x2021 = tile   33  paleta 0  prioridad 1  voltH 0  voltV 0
```

Un mapa de 32×32 celdas son 256×256 px y ocupan 32 × 32 × 2 = **2 KB**. La
pantalla visible es de 256×224: 28 filas. Hay también mapas de 64×32 (dos
páginas de 32×32 seguidas, para el scroll horizontal), que son el tema de la
unidad 07.

### VRAM, en palabras

La **VRAM** (→ glosario: VRAM) mide 64 KB y se direcciona en **palabras de 2
bytes**: la dirección `0x4000` del código es el byte `0x8000`. Todo lo que el
juego y PVSnesLib llaman «dirección de VRAM» va en palabras. Cada capa tiene
dos punteros:

- **base de tiles**: dónde empieza su tile 0. Va en pasos de `0x1000`
  palabras (8 KB): 0x0000, 0x1000, 0x2000...
- **base del mapa**: dónde empieza su tilemap. Va en pasos de `0x400`
  palabras (2 KB, un mapa de 32×32).

El reparto del juego está en `src/sistema/comun.h` y lo respetan todas las
pantallas (y la mini-ROM, que copia sus números):

```
0x0000  tiles de sprites          (unidad 08)
0x2000  VRAM_FONDO_A   tiles de la capa cercana / fondo principal   (512 tiles de 4bpp)
0x4000  VRAM_FONDO_B   tiles de la capa lejana (montañas)
0x6000  VRAM_FUENTE    96 tiles de la fuente
0x6800  VRAM_MAPA_TEXTO  mapa 32x32 del texto
0x6C00  VRAM_MAPA_A    mapa 64x32 de la capa cercana
0x7400  VRAM_MAPA_B    mapa de la capa lejana
```

### Modos de vídeo

La PPU tiene 8 **modos** (0..7) que deciden cuántos fondos hay y con cuántos
colores. El juego usa tres (→ glosario: Mode 1, Mode 3, Mode 7):

| Modo | Fondo 0 | Fondo 1 | Fondo 2 | En el juego |
|---|---|---|---|---|
| Mode 1 | 16 colores | 16 colores | 4 colores | casi todo: título, nivel, logos |
| Mode 3 | 256 colores | 16 colores | — | escenas 1 y 2 de la intro |
| Mode 7 | 256 colores, rotar/escalar | — | — | probado en el logo del estudio |

**Ojo con los números:** el hardware llama a las capas BG1..BG4 y PVSnesLib
las numera **desde 0** (`bgSetEnable(0)` es BG1). Mesen usa los nombres del
hardware. En esta unidad, «fondo 0» es lo que PVSnesLib llama 0.

### Orden de las capas y bit de prioridad

En Mode 1, de delante a atrás (OBJ = sprites, unidad 08; `.1` = tiles con el
bit de prioridad a 1):

```
[BG3.1 si BG3_MODE1_PRIORITY_HIGH] OBJ3  BG1.1  BG2.1  OBJ2  BG1.0  BG2.0  OBJ1  [BG3.1]  OBJ0  BG3.0  color de fondo
```

En la mini-ROM, el texto va en el fondo 0 (BG1) y las montañas en el 1 (BG2):
el texto queda delante sin hacer nada. En el título y en el nivel el texto va
en el fondo 2 (BG3, 4 colores) y por eso se pide `setMode(BG_MODE1,
BG3_MODE1_PRIORITY_HIGH)`: con ese bit, los tiles de BG3 **que tengan la
prioridad a 1** pasan delante de todo. Por eso el HUD escribe sus letras con
`0x2000` (`TILE(c)` en `src/juego/hud_debug.c:85`): sin ese bit, el techo del
túnel tapaba el texto.

### `gfx4snes` y el PNG indexado

Un **PNG indexado** guarda, por píxel, un índice a una paleta incluida en el
archivo (y no el color directamente): es el formato que entiende `gfx4snes`,
la herramienta del SDK que convierte una imagen en tiles, paleta y mapa. El
índice 0 del PNG será el transparente. Las opciones que usa el `Makefile` del
juego (la lista completa sale con `gfx4snes -h`; la unidad 18 las estudia
todas):

| Opción | Qué hace | Ejemplo |
|---|---|---|
| `-s 8` | tamaño del bloque: 8 para fondos (32 o 64 para sprites) | todos los fondos |
| `-u 16` | colores a usar: 4 → 2bpp, 16 → 4bpp, 256 → 8bpp | fuente 2bpp: `-u 4` |
| `-o 16` | colores que se escriben en el `.pal` | la fuente: `-o 2` |
| `-p` | escribir la paleta (`.pal`) | |
| `-m` | escribir el mapa (`.map`) | solo fondos |
| `-e 2` | paleta que se pone en cada entrada del mapa | montañas: paleta 2 |
| `-y` | mapa en páginas de 32×32 (para 64×32) | capas de 512 px (unidad 07) |
| `-F` | contar como iguales los tiles volteados (pone los bits en el mapa) | solo en la capa cercana del nivel (desde el 2026-10-22, abajo) |
| `-R` | no reducir tiles repetidos | nunca |

Salen `.pic` (tiles), `.pal` (paleta) y `.map` (mapa), que `data.asm` pega en
la ROM con **`.incbin`** (directiva del ensamblador: «copia aquí los bytes de
este archivo») bajo una **etiqueta** (un nombre para esa dirección), y el C
las ve como `extern char montanas_til;`.

### Deduplicación de tiles

`gfx4snes` corta la imagen en celdas de 8×8 y guarda **cada dibujo distinto
una sola vez**; el mapa apunta varias celdas al mismo tile. Por eso una
imagen grande y repetitiva cabe en poco. Cifras medidas (parte C del
ejercicio, `capturas/salida_parte_c_gfx4snes.txt`):

| Imagen | Celdas | Sin reducir (`-R`) | Normal | Con `-F` |
|---|---|---|---|---|
| montañas del título (256×224) | 896 | — | **257** tiles (8224 B) | 248 |
| capa cercana del tutorial (2080×256) | 8320 | 8320 tiles (260 KB) | **281** tiles (8992 B) | 260 |

Sin reducir, la capa cercana no cabría ni en la VRAM entera ni en un banco de
la ROM. Reducida, son 281 tiles de los 512 de `VRAM_FONDO_A`. Cabe porque
`scripts/preparar_nivel.py` pega el arte **alineado a la rejilla de 8×8**
(una pieza de tierra repetida da siempre los mismos 4 tiles) y porque las
letras de los carteles son la fuente de consola, un tile por letra: la «E»
de `JUMP HIGHER` y la de `EXIT` son el mismo tile.

**Lo que encontró esta unidad (corregido el 2026-10-22):** el script varía la
tierra con volteos al azar contando con que «los volteos son gratis», pero el
`Makefile` convertía la capa **sin `-F`**: el `.map` del juego tenía **0**
celdas volteadas, y cada tile volteado se guardaba como uno distinto (281 en
vez de 260). Funcionaba igual (sobra VRAM), pero los volteos no se
aprovechaban. Tras medirlo aquí, el autor adoptó `-F` en la regla de
`res/nivel1/fondo_cerca.pic`: hoy la capa son 260 tiles (la ROM ganó 672
bytes). Las cifras de la tabla de arriba y de la parte C del ejercicio son
las de la conversión **sin** `-F`, para que se vea la diferencia.

### DMA

**DMA** (*direct memory access*) es una copia de un bloque de memoria hecha
por hardware, sin la CPU, a unos 2,7 MB/s (→ glosario: DMA / HDMA): mucho más
rápida que un bucle en C. Todas las funciones de carga de PVSnesLib copian
por DMA (canal 0). Como cualquier escritura en VRAM o CGRAM, **solo es segura
con la pantalla apagada o en el VBlank** (unidad 04): la mini-ROM carga todo
antes de `setScreenOn()` y hace sus cambios justo después de
`WaitForVBlank()`.

## La librería: PVSnesLib

PVSnesLib 4.6.0 (la del repo, `tools/pvsneslib/pvsneslib/include/snes/`). Las
firmas son las de las cabeceras. Las direcciones de VRAM, en palabras.

| Función (cabecera) | Qué hace | Hardware |
|---|---|---|
| `void setMode(u8 mode, u8 size);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | elige el modo (`BG_MODE1`, `BG_MODE3`...); `size` lleva banderas como `BG3_MODE1_PRIORITY_HIGH` | `BGMODE $2105` |
| `void bgInitTileSet(u8 bgNumber, u8 *tileSource, u8 *tilePalette, u8 paletteEntry, u16 tileSize, u16 paletteSize, u16 colorMode, u16 address);` ([background.h](https://alekmaul.github.io/pvsneslib/background_8h.html)) | copia `tileSize` bytes de tiles a `address`, la paleta (`paletteSize` bytes) a la paleta `paletteEntry` y apunta la base de tiles del fondo. `colorMode`: `BG_4COLORS`, `BG_16COLORS`, `BG_256COLORS` | DMA a `VMDATA $2118` y `CGDATA $2122`; `BG12NBA $210B`/`BG34NBA $210C` |
| `void bgInitMapSet(u8 bgNumber, u8 *mapSource, u16 mapSize, u8 sizeMode, u16 address);` (background.h) | copia el mapa a `address` y apunta la base del mapa; `sizeMode`: `SC_32x32`, `SC_64x32`... | DMA a `VMDATA`; `BG1SC..BG3SC $2107-$2109` |
| `void bgSetGfxPtr(u8 bgNumber, u16 address);` (background.h) | solo cambia la base de tiles (múltiplo de 0x1000) | `BG12NBA`/`BG34NBA` |
| `void bgSetMapPtr(u8 bgNumber, u16 address, u8 mapSize);` (background.h) | solo cambia la base y el tamaño del mapa | `BGnSC` |
| `void bgSetEnable(u8 bgNumber);` / `void bgSetDisable(u8 bgNumber);` (background.h) | enciende o apaga una capa en la pantalla principal | `TM $212C` |
| `void dmaCopyVram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | copia `size` bytes a la VRAM, avanzando 1 palabra por palabra | `VMAIN $2115` = 0x80, `VMADD $2116`, canal 0, `MDMAEN $420B` |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` (dma.h) | copia `size` bytes a la CGRAM desde la entrada `address` | `CGADD $2121`, `CGDATA $2122`, canal 0 |
| `setPaletteColor(paletteEntry, paletteColor)` (video.h, **macro**) | escribe un color en la entrada 0..255, sin DMA | `CGADD`, dos escrituras a `CGDATA` |
| `void consoleInitText(u8 palnum, u8 palsize, u8 *tilfont, u8 *palfont);` ([textfont.h](https://alekmaul.github.io/pvsneslib/textfont_8h.html)) | copia la fuente y su paleta (`palsize` en **bytes**) y prepara la consola de texto | VRAM y CGRAM por DMA |
| `void consoleSetTextGfxPtr(u16 vramfont);` / `void consoleSetTextMapPtr(u16 offsetfont);` / `void consoleSetTextOffset(u16 offsetfont);` (textfont.h) | dónde copia la consola la fuente y dónde escribe su mapa | — (direcciones) |
| `void consoleDrawText(u16 x, u16 y, char *fmt, ...);` (textfont.h) | escribe texto (con `%d`, como `printf`) en la celda (x, y); se copia a la VRAM en el siguiente VBlank | VRAM (en la NMI del SDK) |

Índice de cabeceras: <https://alekmaul.github.io/pvsneslib/files.html>.
Ejemplo del SDK equivalente: `tools/pvsneslib/snes-examples/graphics/Backgrounds/Mode1`
(un fondo de 16 colores con `bgInitTileSet` + `bgInitMapSet`), del que sale
la mini-ROM.

Detalles que la tabla no dice:

- `consoleDrawText` está en **textfont.h**, no en console.h.
- `setPaletteColor` es una **macro de tres sentencias sin llaves**:
  `if (x) setPaletteColor(0, c);` solo condiciona la primera (la de `CGADD`) y
  las otras dos se ejecutan siempre. Úsala siempre dentro de `{ }`.
- Ninguna función de PVSnesLib cambia **una** celda del mapa: se hace con
  `dmaCopyVram` de 2 bytes (o de una fila, 64) desde una variable, en el VBlank.
  Es lo que hacen los TODO 3 y 4: preparan 4 filas en la WRAM y las suben con
  un solo `dmaCopyVram` de 256 bytes al principio del VBlank siguiente.
- No hay que cambiar el registro a mano para nada de esta unidad. Cuando el
  juego lo hace (el `VMAIN = 0x81` de las columnas, unidad 07) es porque
  `dmaCopyVram` fija siempre 0x80.

## La mini-ROM (`rom/`)

Parte de `snes-examples/graphics/Backgrounds/Mode1` con la receta de
`aprendizaje/PLAN.md` §6.3 y el reparto de VRAM del juego:

- `rom/res/montanas.png` es `res/titulo/fondo_lejos.png` (la capa lejana del
  título, con el logo); `rom/res/fuente.bmp` es la fuente de consola del juego.
- `rom/Makefile` las convierte con las mismas opciones que el `Makefile` del
  juego y resume la compilación (`No warnings or errors found.`).
- `rom/data.asm` las pega con `.incbin`.
- `rom/src/fondo.c`: fuente en el fondo 0 (igual que `fuente_4bpp` de
  `comun.c`), montañas en el fondo 1 (igual que `fondo16_cargar`), `setMode`,
  capas y texto; después, el bucle con los TODO.

Compilada sin los TODO (`capturas/compilacion_rom.txt`) y con la solución
(`capturas/compilacion_solucion.txt`): `No warnings or errors found.` en las
dos; 257 tiles de fondo (8224 bytes de `res/montanas.pic`; la ROM escribe la
cuenta en pantalla, `BG TILES: 257`).

Recorrido del código, en el orden en que hay que hacerlo en cualquier
pantalla:

1. **Pantalla apagada** (lo está al arrancar): se puede copiar a la VRAM lo
   que haga falta.
2. **Tiles y paleta** con `bgInitTileSet`; **mapa** con `bgInitMapSet`. Los
   tamaños se calculan restando etiquetas: `&montanas_til_end - &montanas_til`.
3. **Modo y capas**: `setMode`, `bgSetEnable`/`bgSetDisable`.
4. **`setScreenOn()`**.
5. **Bucle**: `WaitForVBlank()` y, justo después, los cambios a VRAM/CGRAM
   **ya preparados** (el DMA de `bloque` si `pendiente` vale 1, y los
   colores); después, con todo el frame por delante, se prepara lo del
   siguiente. Es el mismo reparto que el juego hace con `src/juego/dma_lista.c`
   (los módulos apuntan copias durante el frame; `dma_ejecutar.asm` las
   dispara en el VBlank).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| El reparto de la VRAM | `src/sistema/comun.h:29-35` |
| Cargar un fondo de 16 colores: `bgInitTileSet` + `bgInitMapSet` + `bgSetEnable` | `fondo16_cargar`, `src/sistema/comun.c:95-101` |
| Lo mismo en 256 colores (Mode 3) y su aviso: pisa las 256 entradas de la CGRAM | `fondo256_cargar`, `src/sistema/comun.c:103-109`; `comun.h` |
| La fuente: `bgSetGfxPtr`, `bgSetMapPtr`, `consoleInitText` con 2 colores | `fuente_4bpp` y `fuente_2bpp`, `src/sistema/comun.c:54-73` |
| Mode 1 con solo el texto | `video_texto_mode1`, `src/sistema/comun.c:79-86` |
| Las reglas de `gfx4snes` (fuente `-u 4`/`-u 16`, fondos `-e 1`/`-e 2`, Mode 3 `-u 256`) | `Makefile:56-119` |
| Etiquetas `_til`, `_til_end`, `_pal`, `_map` | `data.asm` (p. ej. `.rodata_nivel1_lejos`, líneas 165-171) |
| Una entrada de mapa escrita a mano, con el bit de prioridad | `TILE(c)`, `src/juego/hud_debug.c:82-85` |
| La capa cercana del nivel: tiles y paleta 1 con el SDK, mapa de 64×32 | `fondo_nivel_cargar`, `src/juego/fondo_nivel.c:118-126` |
| Los volteos que el script pone y `gfx4snes` no aprovecha | `scripts/preparar_nivel.py:257-266` (`volteos`) y la regla de `res/nivel1/fondo_cerca.pic` del `Makefile` (con `-F` desde el 2026-10-22) |
| Mapas de VRAM, CGRAM y registros por módulo | `docs/hardware-snes-por-modulo.md` §2, §3, §4 |

Fíjate en `fondo_nivel_cargar`: carga tiles y paleta con `bgInitTileSet`,
pero **no** usa `bgInitMapSet`: el mapa del nivel (2080 px) no cabe en el de
la VRAM (512 px). Solo apunta la base con `bgSetMapPtr` y después copia
columnas sueltas. Eso es la unidad 07.

## Trampas

- **Direcciones de VRAM en palabras.** `0x4000` en el código es el byte
  `0x8000`. Mesen enseña unas veces una cosa y otras la otra: mira la unidad.
- **Base de tiles alineada a 0x1000 palabras; base de mapa, a 0x400.** Una
  dirección intermedia se redondea sin aviso y la capa muestra basura.
- **Escribir en la VRAM o la CGRAM con la pantalla encendida y fuera del
  VBlank** pierde datos sin aviso (unidad 04). Carga antes de
  `setScreenOn()` o justo después de `WaitForVBlank()`.
- **El bucle se come el VBlank.** El VBlank dura unas 37 líneas. La primera
  versión de la solución hacía, tras `WaitForVBlank()`, un bucle de 32 vueltas
  en C para armar una fila y luego su `dmaCopyVram`, cuatro veces. **Medido**:
  en Mesen, las 8 capturas (de ~3 a ~10 s) salieron idénticas byte a byte;
  la copia y el espejo no aparecieron nunca, sin un solo aviso. **Estimado**
  (contando instrucciones en el `.ps` de tcc816, antes del optimizador): cada
  vuelta de `fila[i] = mapa[(r << 5) + i]` son del orden de 100 instrucciones
  (cargas de `u8` estáticos, el `<< 5` hecho como bucle de 5 `asl`, aritmética
  de punteros de 24 bits), un par de líneas de pantalla; la primera fila
  tardaba decenas de líneas en armarse y **los cuatro DMA llegaban con la
  pantalla pintando**, cuando la PPU ignora las escrituras a la VRAM. Regla: en el VBlank, solo DMA de
  datos ya preparados; los bucles, antes o después.
- **Elegir el índice de color a ojo.** El color más usado de las montañas
  del título (el 7, 13 942 píxeles) no es el de las montañas sino el bosque
  verde azulado de las 8 filas de abajo; las montañas son el 5 (y el 4 en
  las sombras). Antes de tocar un color, cuenta qué píxeles lo usan (en
  Python con Pillow, o pasando el ratón en el Tile Viewer de Mesen).
- **`consoleInitText` también escribe en la CGRAM:** con la paleta 0 pisa la
  entrada 0, el color de fondo. Si cargas un fondo con la paleta 0 después de
  la fuente, pisas los colores de la fuente (por eso el juego da paletas
  distintas a cada cosa: 0 fuente, 1 cercana, 2 lejana).
- **`setPaletteColor` sin llaves** (arriba).
- **El índice 0 nunca pinta.** Si una imagen sale «agujereada», su color de
  relleno quedó en el índice 0 del PNG.
- **`consoleDrawText` con `%d` es lento**: formatea con una rutina larga. Para
  texto que cambia cada frame, el juego escribe el mapa a mano
  (`src/juego/hud_debug.c`): ese era el motivo de que el nivel bajase al 60 %.
- **Los volteos del tilemap solo son gratis si `gfx4snes` los busca** (`-F`).

## Ejercicio y criterio de aprobación

El enunciado completo está en `ejercicio_06.md`:

- **A.** Completar los TODO 1..4 de `rom/src/fondo.c`: dos colores a los 2 s,
  una copia de filas del mapa a los 4 s, la misma copia en espejo a los 6 s.
- **B.** Mirar la capa cercana del juego en el Tilemap Viewer y el Tile
  Viewer de Mesen, y el bit de prioridad del HUD.
- **C.** Contar tiles con y sin reducción, y con `-F`, y responder por qué.

**Aprobación:** captura de la mini-ROM con el fondo, los colores cambiados y
la copia del logo en espejo (la captura sin manos de 8 s del enunciado
empieza ~3 s tarde: colores cambiados desde la captura 0, copia derecha
seguro en la 2, espejo seguro desde la 4; la 1 y la 3 caen en el cambio), y las cifras de la parte C (las de
referencia están en `capturas/salida_parte_c_gfx4snes.txt`: 281, 8320 y 260).

## Lo que viene

- **Unidad 07:** mover esos fondos (scroll), una cámara que sigue a Tarou y
  el mapa-anillo por el que el nivel de 2048 px cabe en un mapa de 512.
- **Unidad 08:** los sprites, que usan tiles de la misma forma pero desde la
  OAM y con las paletas 128..255.
- **Unidad 12:** el color de fondo cambiado en cada línea (HDMA): el cielo.
