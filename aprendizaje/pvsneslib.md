# PVSnesLib en el curso: índice de la librería

Índice de lo que el curso enseña de **PVSnesLib**: qué función o macro, su
firma, en qué unidad se ve y qué archivo del juego la usa. Recoge además lo que
el curso midió o leyó en el fuente y que la documentación no dice o dice mal, y
lo que el juego decidió **no** usar. Las firmas están copiadas de las cabeceras
del repositorio. La columna «En el juego» se ha comprobado con `grep` sobre
`src/` sin contar los comentarios (fecha: 2026-10-22).

> **Nota del 2026-10-23: el juego cambia de género.** La columna «En el
> juego» describe el código de la Revisión 2 (el plataformas), congelado en
> el tag `plataformas-r2`. Con la Revisión 3, un beat 'em up
> (`docs/diseno-beat-em-up.md` §10), **salen** del juego `src/juego/colision.c`,
> `src/juego/partida.c`, `src/juego/guion_tutorial.c`, `res/nivel1/*` del
> tutorial (y con ellos `scripts/preparar_nivel.py`, al que sucede
> `preparar_zona.py`) y la física de plataformas de `tarou.c`; **cambian de
> nombre** `src/pantallas/pantalla_nivel1.c` (→ `pantalla_fase1.c`) y
> `src/juego/tarou_sprite.c` (→ `protagonista_sprite.c`). Las citas a esos
> archivos se dejan como están: siguen siendo ciertas en el tag, que es donde
> las prácticas de las unidades 00–20 se hacen. Se marcan con «(tag)» las de
> los archivos que salen.

## 1. Qué es y dónde está

**PVSnesLib** es el SDK en C para la SNES que usa el proyecto: el compilador
`816-tcc` (tcc816), el ensamblador `wla-65816`, el enlazador `wlalink`, las
herramientas (`gfx4snes`, `smconv`, `snesbrr`, `tmx2snes`...), la regla de
compilación `snes_rules` y una librería de funciones **escrita en
ensamblador** que se llama desde C. La versión es la **4.6.0**
(`tools/pvsneslib/pvsneslib/include/snes/libversion.h`:
`_PVSNESLIB_STRING_ "PVSnesLib V4.6.0"`).

| Qué | Dónde |
|---|---|
| Cabeceras (las firmas de verdad) | `tools/pvsneslib/pvsneslib/include/snes/*.h`; `#include <snes.h>` las incluye todas |
| `memset` y la libc | `tools/pvsneslib/devkitsnes/include/string.h` (fuera de `snes/`) |
| La librería ya ensamblada | `tools/pvsneslib/pvsneslib/lib/LoROM_FastROM/` (`crt0_snes.obj`, `libc.obj`, `libm.obj`, `libtcc.obj`); el repositorio **no** trae su fuente |
| El fuente de la librería (`vblank.asm`, `videos.asm`, `dmas.asm`, `crt0_snes.asm`...) | GitHub, etiqueta de la misma versión: <https://github.com/alekmaul/pvsneslib/tree/4.6.0/pvsneslib/source> |
| Documentación web | índice de cabeceras: <https://alekmaul.github.io/pvsneslib/files.html>; cada cabecera tiene su página `<cabecera>_8h.html` (p. ej. <https://alekmaul.github.io/pvsneslib/video_8h.html>) |
| Ejemplos del SDK | `tools/pvsneslib/snes-examples/` (`audio/`, `graphics/`, `hello_world/`, `input/`, `systems/`; las ROM ya compiladas en `bin/`) |

Enlaza siempre a la **página de la cabecera**, no a las anclas `#a...` de
cada función: esas anclas son un hash que cambia de una versión de la
documentación a otra.

**La regla del curso: primero la librería, después el registro.** Cada unidad
enseña antes la función de PVSnesLib que hace el trabajo y, solo si el juego
la necesita, baja después al registro `REG_xxx` desde C (las macros de
registro de `video.h`, `background.h`, `dma.h`...; una asignación como
`REG_MOSAIC = x;` es un `sta $2106`) y, en las unidades 14 y 15, al
ensamblador. Cuando el juego no usa una función, la unidad lo dice y explica
por qué (sección 4).

Unidades del curso (carpetas de `aprendizaje/`):
[00](unidad-00-c-en-el-pc/) ·
[01](unidad-01-enteros-y-punto-fijo/) ·
[02](unidad-02-maquinas-de-estado/) ·
[03](unidad-03-tuberia-de-compilacion/) ·
[04](unidad-04-frame-y-vblank/) ·
[05](unidad-05-mando/) ·
[06](unidad-06-tiles-y-fondos/) ·
[07](unidad-07-scroll-y-camara/) ·
[08](unidad-08-sprites-y-oam/) ·
[09](unidad-09-metasprites/) ·
[10](unidad-10-fisica/) ·
[11](unidad-11-animacion/) ·
[12](unidad-12-hdma-y-efectos/) ·
[13](unidad-13-memoria/) ·
[14](unidad-14-asm-leer/) ·
[15](unidad-15-asm-escribir/) ·
[16](unidad-16-mesen/) ·
[17](unidad-17-audio/) ·
[18](unidad-18-herramientas/) ·
[19](unidad-19-del-arte-al-juego/) ·
[20](unidad-20-colision/).
Las unidades 01, 02, 10, 11 y 18 no tienen sección «La librería»: son lógica
pura en el PC o herramientas (la 18 trata `gfx4snes`). Las 14 y 15
(ensamblador) tienen su sección, recién escritas.

## 2. Tabla por cabecera

Leyenda de la columna «Tipo»: **F** función (una llamada `jsl`, con sus
parámetros en la pila), **M** macro (el preprocesador la sustituye antes de
compilar: no cuesta una llamada), **V** variable, **T** tipo o constante.
«—» en «En el juego» = el juego no la usa.

### `console.h` — [console_8h.html](https://alekmaul.github.io/pvsneslib/console_8h.html)

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `consoleInit` | F | `void consoleInit(void);` | inicialización genérica; la llama el arranque (`crt0`) **antes** de `main`: instala la rutina de la NMI y la lectura del mando | [03](unidad-03-tuberia-de-compilacion/), [05](unidad-05-mando/) | — (la llama el SDK) |
| `snes_vblank_count` | V | `extern u16 snes_vblank_count;` | sube 1 en cada VBlank, también en los perdidos | [04](unidad-04-frame-y-vblank/), [16](unidad-16-mesen/) | `src/juego/hud_debug.c` (la `VB` del HUD) |
| `consoleMesenBreakpoint` | F | `void consoleMesenBreakpoint(void);` | ejecuta `WDM`: Mesen se para si tiene marcado «Break on WDM» | [16](unidad-16-mesen/) | — |
| `consoleNocashMessage` | F | `void consoleNocashMessage(char *fmt, ...);` | un `printf` a la ventana de depuración de no$sns | [16](unidad-16-mesen/) | — |

### `textfont.h` — [textfont_8h.html](https://alekmaul.github.io/pvsneslib/textfont_8h.html)

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `consoleSetTextMapPtr` | F | `void consoleSetTextMapPtr(u16 offsetfont);` | dirección de VRAM (palabras) del mapa de texto | [03](unidad-03-tuberia-de-compilacion/), [06](unidad-06-tiles-y-fondos/) | `src/sistema/comun.c` |
| `consoleSetTextGfxPtr` | F | `void consoleSetTextGfxPtr(u16 vramfont);` | dirección de VRAM de los tiles de la fuente | [03](unidad-03-tuberia-de-compilacion/), [06](unidad-06-tiles-y-fondos/) | `src/sistema/comun.c` |
| `consoleSetTextOffset` | F | `void consoleSetTextOffset(u16 offsetfont);` | desplazamiento del primer tile de la fuente | [06](unidad-06-tiles-y-fondos/) | `src/sistema/comun.c` |
| `consoleInitDefaultText` | F | `void consoleInitDefaultText(u8 palnum);` | copia la fuente que trae la librería y su paleta | [03](unidad-03-tuberia-de-compilacion/), [15](unidad-15-asm-escribir/) | — (la usan las mini-ROM de la 03 y la 15) |
| `consoleInitText` | F | `void consoleInitText(u8 palnum, u8 palsize, u8 *tilfont, u8 *palfont);` | copia una fuente propia y su paleta (`palsize` en **bytes**) | [06](unidad-06-tiles-y-fondos/) | `src/sistema/comun.c` |
| `consoleDrawText` | F | `void consoleDrawText(u16 x, u16 y, char *fmt, ...);` | escribe texto con formato (`%d`, `%u`, `%x`) en un búfer en RAM; la NMI lo copia a la VRAM. **Está en `textfont.h`, no en `console.h`** | [03](unidad-03-tuberia-de-compilacion/), [04](unidad-04-frame-y-vblank/), [06](unidad-06-tiles-y-fondos/), [15](unidad-15-asm-escribir/), [17](unidad-17-audio/) | `src/sistema/comun.c` (no el HUD de depuración: ver sección 4) |
| `consoleVblank` | F | sin declarar: `extern void consoleVblank(void);` | la rutina que `consoleInit` instala en la NMI y copia el texto; **ninguna cabecera la declara** | [04](unidad-04-frame-y-vblank/) | — |

### `video.h` — [video_8h.html](https://alekmaul.github.io/pvsneslib/video_8h.html)

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `setMode` | F | `void setMode(u8 mode, u8 size);` | modo de vídeo (`BG_MODE1`...) y banderas (`BG3_MODE1_PRIORITY_HIGH`); `BGMODE $2105` | [03](unidad-03-tuberia-de-compilacion/), [06](unidad-06-tiles-y-fondos/), [15](unidad-15-asm-escribir/), [17](unidad-17-audio/) | `src/sistema/comun.c`, `src/pantallas/pantalla_intro.c`, `pantalla_nivel1.c`, `pantalla_titulo.c` |
| `setScreenOn` | F | `void setScreenOn(void);` | **espera un VBlank** y escribe `$0F` en `INIDISP` | [03](unidad-03-tuberia-de-compilacion/), [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/), [15](unidad-15-asm-escribir/) | `src/sistema/main.c` |
| `setScreenOff` | F | `void setScreenOff(void);` | *forced blank*: `$80` en `INIDISP`; la VRAM se puede escribir cuando se quiera | [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/), [13](unidad-13-memoria/), [20](unidad-20-colision/) | `src/sistema/secuencia.c`, `src/pantallas/pantalla_nivel1.c` |
| `setBrightness` | F | `void setBrightness(u8 level);` | brillo 1..15; **con 0, forced blank** (sección 3) | [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/) | `src/escena/efectos.c`, `src/pantallas/pantalla_devs.c`, `src/sistema/main.c` |
| `setFadeEffect` | F | `void setFadeEffect(u8 mode);` | fundido `FADE_IN`/`FADE_OUT` de 16 frames; bloquea | [12](unidad-12-hdma-y-efectos/) | — |
| `setFadeEffectEx` | F | `void setFadeEffectEx(u8 mode, u8 framesNumber);` | fundido de 16 × `framesNumber` frames; bloquea | [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/) | `src/sistema/comun.c` (`fade_in`, `fade_out`) |
| `setMosaicEffect` | F | `void setMosaicEffect(u8 mode, u8 bgNumbers);` | anima el mosaico (`MOSAIC_OUT`/`MOSAIC_IN`, `MOSAIC_BG1..4`); bloquea | [12](unidad-12-hdma-y-efectos/) | `src/pantallas/pantalla_intro.c` |
| `setColorEffect` | F | `void setColorEffect(u8 colorMathA, u8 colorMathB);` | escribe `CGWSEL $2130` y `CGADSUB $2131` | [12](unidad-12-hdma-y-efectos/) | — (el juego escribe `REG_CGWSEL`/`REG_CGADSUB`) |
| `setColorIntensity` | F | `void setColorIntensity(u8 colorApply, u8 intensity);` | el color fijo, `COLDATA $2132` | [12](unidad-12-hdma-y-efectos/) | — (el juego escribe `REG_COLDATA`) |
| `setPaletteColor` | M | `#define setPaletteColor(paletteEntry, paletteColor) \` (tres sentencias) | un color en la entrada 0..255, sin DMA (`CGADD`, `CGDATA` dos veces) | [06](unidad-06-tiles-y-fondos/) | — |
| `setPalette` | M | `#define setPalette(palette, paletteEntry, paletteSize) dmaCopyCGram(palette, paletteEntry, paletteSize)` | otro nombre de `dmaCopyCGram` | [09](unidad-09-metasprites/) | — |
| `RGB5`, `RGB8` | M | `#define RGB5(r, g, b) ((r) \| ((g) << 5) \| ((b) << 10))`; `#define RGB8(r, g, b) (((r) >> 3) \| (((g) >> 3) << 5) \| (((b) >> 3) << 10))` | un color BGR555 con componentes 0..31 o 0..255 | [06](unidad-06-tiles-y-fondos/) | — |
| `REG_VMAIN`, `REG_VMADDLH`, `REG_VMDATALH` | M | `#define REG_VMAIN (*(vuint8 *)0x2115)` ... | registros de la VRAM: modo de avance, dirección, dato | [07](unidad-07-scroll-y-camara/), [15](unidad-15-asm-escribir/), [20](unidad-20-colision/) | — en C; `src/juego/dma_ejecutar.asm` escribe `$2115`/`$2116` por dirección |
| `REG_INIDISP` | M | `#define REG_INIDISP (*(vuint8 *)0x2100)` | brillo y forced blank escritos a mano | [15](unidad-15-asm-escribir/) | — |
| `mirrorINIDISP` | V | `extern u8 mirrorINIDISP;` | copia en la WRAM del último valor escrito en `INIDISP` por las funciones del SDK | [15](unidad-15-asm-escribir/) | — |
| `REG_CGWSEL`, `REG_CGADSUB`, `REG_COLDATA` | M | `#define REG_CGWSEL (*(vuint8 *)0x2130)`; `#define REG_CGADSUB (*(vuint8 *)0x2131)`; `#define REG_COLDATA (*(vuint8 *)0x2132)` | matemática de color y color fijo | [12](unidad-12-hdma-y-efectos/) | `src/escena/efectos.c` |

### `background.h` — [background_8h.html](https://alekmaul.github.io/pvsneslib/background_8h.html)

Los fondos se numeran **0..3** (el hardware los llama BG1..BG4). Direcciones de
VRAM en palabras.

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `bgSetGfxPtr` | F | `void bgSetGfxPtr(u8 bgNumber, u16 address);` | base de los tiles de un fondo (`BG12NBA $210B`/`BG34NBA $210C`) | [03](unidad-03-tuberia-de-compilacion/), [06](unidad-06-tiles-y-fondos/), [15](unidad-15-asm-escribir/), [17](unidad-17-audio/) | `src/sistema/comun.c` |
| `bgSetMapPtr` | F | `void bgSetMapPtr(u8 bgNumber, u16 address, u8 mapSize);` | base y tamaño del mapa (`SC_32x32`, `SC_64x32`...), sin copiar nada (`BGnSC $2107-$210A`) | [03](unidad-03-tuberia-de-compilacion/), [06](unidad-06-tiles-y-fondos/), [15](unidad-15-asm-escribir/), [07](unidad-07-scroll-y-camara/), [20](unidad-20-colision/) | `src/sistema/comun.c`, `src/juego/fondo_nivel.c` |
| `bgInitTileSet` | F | `void bgInitTileSet(u8 bgNumber, u8 *tileSource, u8 *tilePalette, u8 paletteEntry, u16 tileSize, u16 paletteSize, u16 colorMode, u16 address);` | tiles a la VRAM y paleta a la CGRAM por DMA, y apunta la base de tiles | [06](unidad-06-tiles-y-fondos/), [19](unidad-19-del-arte-al-juego/), [20](unidad-20-colision/) | `src/sistema/comun.c`, `src/juego/fondo_nivel.c` |
| `bgInitMapSet` | F | `void bgInitMapSet(u8 bgNumber, u8 *mapSource, u16 mapSize, u8 sizeMode, u16 address);` | copia el mapa entero y apunta su base | [06](unidad-06-tiles-y-fondos/), [07](unidad-07-scroll-y-camara/) | `src/sistema/comun.c` |
| `bgSetEnable` | F | `void bgSetEnable(u8 bgNumber);` | enciende una capa en la pantalla principal (`TM $212C`) | [06](unidad-06-tiles-y-fondos/) | `src/sistema/comun.c`, `src/juego/fondo_nivel.c`, `src/juego/hud_debug.c`, `src/pantallas/pantalla_intro.c`, `pantalla_titulo.c` |
| `bgSetDisable` | F | `void bgSetDisable(u8 bgNumber);` | la apaga | [03](unidad-03-tuberia-de-compilacion/), [06](unidad-06-tiles-y-fondos/), [15](unidad-15-asm-escribir/), [17](unidad-17-audio/) | `src/sistema/comun.c`, `src/juego/hud_debug.c`, `src/pantallas/pantalla_intro.c` |
| `bgSetScroll` | F | `void bgSetScroll(u8 bgNumber, u16 x, u16 y);` | scroll horizontal y vertical, escrito **en el acto** (`BGnHOFS`/`BGnVOFS`, doble escritura) | [07](unidad-07-scroll-y-camara/), [20](unidad-20-colision/) | `src/pantallas/pantalla_nivel1.c`, `pantalla_titulo.c` |
| `BG_TIL_NUM` y compañía | M | `#define BG_TIL_NUM(n) (n<<0)` ... | arman una entrada del mapa (tile, paleta, prioridad, volteos) | [06](unidad-06-tiles-y-fondos/) | — |
| `BG_16COLORS`, `SC_32x32`, `SC_64x32`, `BG_MODE1`... | T | constantes | modos de color, tamaños de mapa, modos de vídeo | [06](unidad-06-tiles-y-fondos/), [07](unidad-07-scroll-y-camara/) | `src/sistema/comun.c`, `src/juego/fondo_nivel.c`, pantallas |
| `REG_MOSAIC`, `REG_WOBJSEL`, `REG_WH0`, `REG_WH1` | M | `#define REG_MOSAIC (*(vuint8 *)0x2106)` ... | mosaico, ventanas y matemática de color | [12](unidad-12-hdma-y-efectos/) | `src/escena/efectos.c` (y `REG_MOSAIC` en `src/pantallas/pantalla_intro.c`) |

### `sprite.h` — [sprite_8h.html](https://alekmaul.github.io/pvsneslib/sprite_8h.html)

El `id` de las funciones `oam*` es **entrada × 4** (0, 4... 508): el
desplazamiento en bytes dentro del búfer.

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `oamInit` | F | `void oamInit(void);` | prepara el búfer de la OAM y esconde los 128 sprites | [08](unidad-08-sprites-y-oam/) | `src/sistema/main.c`, `src/pantallas/personajes.c` |
| `oamInitGfxSet` | F | `void oamInitGfxSet(u8 *tileSource, u16 tileSize, u8 *tilePalette, u16 paletteSize, u8 tilePaletteNumber, u16 address, u8 oamsize);` | copia tiles y paleta y fija base y tamaños (`OBSEL $2101`) | [08](unidad-08-sprites-y-oam/) | — |
| `oamInitGfxAttr` | F | `void oamInitGfxAttr(u16 address, u8 oamsize);` | solo base y tamaños, sin copiar | [08](unidad-08-sprites-y-oam/), [09](unidad-09-metasprites/), [19](unidad-19-del-arte-al-juego/) | `src/juego/tarou_sprite.c`, `src/pantallas/personajes.c` |
| `oamSet` | F | `void oamSet(u16 id, u16 xspr, u16 yspr, u8 priority, u8 hflip, u8 vflip, u16 gfxoffset, u8 paletteoffset);` | una entrada entera: posición, prioridad, volteos, tile, paleta | [00](unidad-00-c-en-el-pc/), [08](unidad-08-sprites-y-oam/), [09](unidad-09-metasprites/), [19](unidad-19-del-arte-al-juego/) | `src/juego/tarou_sprite.c`, `src/pantallas/personajes.c` |
| `oamSetEx` | F | `void oamSetEx(u16 id, u8 size, u8 hide);` | tamaño (`OBJ_SMALL`/`OBJ_LARGE`) y visibilidad (`OBJ_SHOW`/`OBJ_HIDE`) | [08](unidad-08-sprites-y-oam/), [09](unidad-09-metasprites/), [19](unidad-19-del-arte-al-juego/) | `src/juego/tarou_sprite.c`, `src/pantallas/personajes.c` |
| `oamSetXY` | F | `void oamSetXY(u16 id, u16 xspr, u16 yspr);` | solo la posición | [08](unidad-08-sprites-y-oam/), [09](unidad-09-metasprites/) | `src/juego/tarou_sprite.c`, `src/pantallas/personajes.c` |
| `oamSetVisible` | F | `void oamSetVisible(u16 id, u8 hide);` | enseña o esconde (`OBJ_HIDE` lleva el sprite a x 255, y 240) | [08](unidad-08-sprites-y-oam/) | — (sección 4) |
| `oamClear` | F | `void oamClear(u16 first, u8 numEntries);` | esconde un rango; `oamClear(0, 0)` esconde las 128 | [08](unidad-08-sprites-y-oam/), [09](unidad-09-metasprites/) | `src/juego/tarou_sprite.c` |
| `oamUpdate` | F | `void oamUpdate(void);` | copia el búfer a la OAM; la NMI del SDK ya lo hace sola | [08](unidad-08-sprites-y-oam/) | — |
| `OAM_ATTR` | M | `#define OAM_ATTR(priority, hflip, vflip, gfxoffset, paletteoffset) ((vflip << 7) \| (hflip << 6) \| (priority << 4) \| (paletteoffset << 1) \| ((gfxoffset >> 8) & 1))` | arma el byte de atributos | [00](unidad-00-c-en-el-pc/) | — |
| `OBJ_SMALL`, `OBJ_LARGE`, `OBJ_SHOW`, `OBJ_HIDE` | M | 0, 1, 0, 1 | argumentos de `oamSetEx` | [08](unidad-08-sprites-y-oam/) | `src/juego/tarou_sprite.c`, `src/pantallas/personajes.c` |
| `OBJ_SIZE16_L32`, `OBJ_SIZE32_L64`... | M | constantes | los dos tamaños de sprite de `OBSEL` | [09](unidad-09-metasprites/) | `src/juego/tarou_sprite.c` (16/32), `src/pantallas/personajes.c` (32/64) |
| `t_metasprite` | T | `typedef struct { u16 dx, dy; u16 dtile; u8 props; u8 unused; } t_metasprite;` | una pieza de las tablas que escribe `gfx4snes -T` | [09](unidad-09-metasprites/) | `src/juego/metasprite.c/.h`, `src/juego/tarou_sprite.c`, `src/pantallas/personajes.c` |
| `metasprite_end`, `METASPR_ITEM`, `METASPR_TERM` | M | `#define metasprite_end -128`; `#define METASPR_ITEM(dx,dy,dt,a) {(dx),(dy),(dt),(a)}`; `#define METASPR_TERM {metasprite_end}` | terminador y constructores de una tabla de metasprite | [09](unidad-09-metasprites/) | — (el juego compara con su propia constante) |
| `oamInitDynamicSprite` | F | `void oamInitDynamicSprite(u16 gfxsp0adr, u16 gfxsp1adr, u16 oamsp0init, u16 oamsp1init, u8 oamsize);` | arranca el **motor dinámico** (no admite 64×64) | [09](unidad-09-metasprites/) | — (sección 4) |
| `oamMetaDrawDyn32` | F | `void oamMetaDrawDyn32(u16 id, s16 x,s16 y, u8 *sprmeta,u8 *gfxptr);` | dibuja un metasprite de piezas de 32×32 **leyendo las tablas `t_metasprite` de `gfx4snes -T`** y encola la copia de sus tiles | [09](unidad-09-metasprites/) | — (sección 4) |
| `oamMetaDrawDyn16` | F | `void oamMetaDrawDyn16(u16 id, s16 x,s16 y, u8 *sprmeta,u8 *gfxptr, u16 sprsize);` | lo mismo con piezas de 16×16 (lleva un parámetro más) | [09](unidad-09-metasprites/) | — |
| `oamMetaDrawDyn8` | F | `void oamMetaDrawDyn8(u16 id, s16 x,s16 y, u8 *sprmeta,u8 *gfxptr);` | lo mismo con piezas de 8×8 | [09](unidad-09-metasprites/) | — |
| `oamDynamicMetaDraw` | F | `void oamDynamicMetaDraw(u16 id, s16 x, s16 y, u8 *sprmeta);` | otro formato (`t_metasprites`, 16 bytes, terminado en `0xFFFF`): **no** sirve para `gfx4snes -T` | [09](unidad-09-metasprites/) | — |
| `oamInitDynamicSpriteEndFrame` | F | `void oamInitDynamicSpriteEndFrame(void);` | cierra el frame del motor y esconde lo no usado | [09](unidad-09-metasprites/) | — |
| `oamVramQueueUpdate` | F | `void oamVramQueueUpdate(void);` | ejecuta la cola: un DMA por pieza, justo después de `WaitForVBlank()` | [09](unidad-09-metasprites/) | — |
| `oambuffer` | V | `extern t_sprites oambuffer[128];` | estado del motor por entrada (`oamrefresh = 1` pide recopiar) | [09](unidad-09-metasprites/) | — |

### `dma.h` — [dma_8h.html](https://alekmaul.github.io/pvsneslib/dma_8h.html)

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `dmaCopyVram` | F | `void dmaCopyVram(u8 *source, u16 address, u16 size);` | copia a la VRAM por el canal 0 con `VMAIN = 0x80` (avance de 1 palabra) | [06](unidad-06-tiles-y-fondos/), [08](unidad-08-sprites-y-oam/), [13](unidad-13-memoria/), [20](unidad-20-colision/) | `src/pantallas/personajes.c` |
| `dmaCopyVram7` | F | `void dmaCopyVram7(u8 *source, u16 address, u16 size, u8 vrammodeinc, u16 dmacontrol);` | la misma copia **eligiendo `VMAIN`** y el control del DMA | [20](unidad-20-colision/) | — (sección 4) |
| `dmaCopyCGram` | F | `void dmaCopyCGram(u8 *source, u16 address, u16 size);` | copia a la CGRAM desde el color `address` | [06](unidad-06-tiles-y-fondos/), [08](unidad-08-sprites-y-oam/), [09](unidad-09-metasprites/), [13](unidad-13-memoria/), [19](unidad-19-del-arte-al-juego/) | `src/juego/tarou_sprite.c`, `src/pantallas/personajes.c` |
| `dmaCopyOAram` | F | `void dmaCopyOAram(u8 *source, u16 address, u16 size);` | copia a la OAM | [13](unidad-13-memoria/) | — |
| `dmaClearVram` | F | `void dmaClearVram(void);` | pone a 0 toda la VRAM | [13](unidad-13-memoria/), [15](unidad-15-asm-escribir/) | — |
| `setModeHdmaColor` | F | `void setModeHdmaColor(u8 channel, u8* hdmatable);` | degradado por líneas del **color 0** de la CGRAM (tabla de `gfx4snes -n`) | [12](unidad-12-hdma-y-efectos/) | `src/pantallas/pantalla_titulo.c`, `pantalla_nivel1.c` |
| `setModeHdmaReset` | F | `void setModeHdmaReset(u8 channels);` | escribe `channels` tal cual en `HDMAEN` | [12](unidad-12-hdma-y-efectos/) | `src/pantallas/pantalla_titulo.c`, `pantalla_nivel1.c` |
| `setModeHdmaWindowReset` | F | `void setModeHdmaWindowReset(u8 channels);` | lo mismo y además `TMW = 0` | [12](unidad-12-hdma-y-efectos/) | — |
| `setModeHdmaGradient` | F | `void setModeHdmaGradient(u8 maxLevels);` | degradado de brillo por franjas (canal 3) | [12](unidad-12-hdma-y-efectos/) | — |
| `setModeHdmaShadeUpDown` | F | `void setModeHdmaShadeUpDown(void);` | brillo que crece hacia el centro (canal 3) | [12](unidad-12-hdma-y-efectos/) | — |
| `setParallaxScrolling` | F | `void setParallaxScrolling(u8 bgrnd);` | scroll horizontal por franjas (`HDMATable16`, canal 3) | [07](unidad-07-scroll-y-camara/), [12](unidad-12-hdma-y-efectos/) | — |
| `setModeHdmaWaves` | F | `void setModeHdmaWaves(u8 channel, u8 bgrnd);` | ondas con HDMA indirecto | [12](unidad-12-hdma-y-efectos/) | — |
| `setModeHdmaWavesMove` | F | `void setModeHdmaWavesMove(void);` | anima las ondas (una vez por frame) | [12](unidad-12-hdma-y-efectos/) | — |
| `setModeHdmaWindow` | F | `void setModeHdmaWindow(u8 channels, u8 bgrnd, u8 bgrndmask,u8* hdmatableL,u8* hdmatableR);` | ventana con forma: bordes izquierdo y derecho por línea | [12](unidad-12-hdma-y-efectos/) | — |
| `REG_DMAPn`, `REG_BBADn`, `REG_A1TnLH`, `REG_A1Bn`, `REG_DASnLH`, `REG_MDMAEN`, `REG_HDMAEN` | M | `#define REG_DMAP0 (*(vuint8 *)0x4300)` ... `#define REG_HDMAEN (*(vuint8 *)0x420C)` | los registros de cada canal de DMA/HDMA como variables de C | [12](unidad-12-hdma-y-efectos/), [13](unidad-13-memoria/), [20](unidad-20-colision/) | `src/escena/efectos.c` (canal 6 y `REG_HDMAEN`) |

### `input.h` — [input_8h.html](https://alekmaul.github.io/pvsneslib/input_8h.html)

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `padsCurrent` | **M** | `#define padsCurrent(value) (pad_keys[value])` | botones mantenidos del mando `value` | [00](unidad-00-c-en-el-pc/), [05](unidad-05-mando/), [07](unidad-07-scroll-y-camara/) | `src/juego/mando.c`, `src/pantallas/pantalla_titulo.c` |
| `padsDown` | **M** | `#define padsDown(value) (pad_keysdown[value])` | recién pulsados en este frame | [00](unidad-00-c-en-el-pc/), [05](unidad-05-mando/), [07](unidad-07-scroll-y-camara/), [17](unidad-17-audio/) | `src/juego/mando.c`; `src/pantallas/pantalla_nintendo.c`, `pantalla_publisher.c`, `pantalla_devs.c`, `pantalla_intro.c`, `pantalla_titulo.c` |
| `padsUp` | F | `unsigned short padsUp(u16 value);` | recién soltados (`pad_keysold & ~pad_keys`) | [05](unidad-05-mando/) | — |
| `padsClear` | F | `void padsClear(u16 value);` | pone a 0 las tres variables de un mando | [05](unidad-05-mando/) | — |
| `pad_keys`, `pad_keysold`, `pad_keysdown` | V | `extern u16 pad_keys[5];` (y las otras dos igual) | lo que rellena la NMI en los frames buenos | [05](unidad-05-mando/) | a través de las macros |
| `KEY_A` ... `KEY_Y` | T | `typedef enum KEYPAD_BITS` | la máscara de cada botón | [00](unidad-00-c-en-el-pc/), [05](unidad-05-mando/) | `src/juego/mando.c` y las pantallas (`KEY_START`) |
| `REG_JOYxLH` | M | `#define REG_JOYxLH(a) (((vuint16 *)0x4218)[(a)])` | el registro de autolectura, para leerlo a mano | [05](unidad-05-mando/) | — |

### `interrupt.h` — [interrupt_8h.html](https://alekmaul.github.io/pvsneslib/interrupt_8h.html)

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `WaitForVBlank` | F | `void WaitForVBlank(void);` | pone `vblank_flag = 1` y duerme (`wai`) hasta que la NMI la borra | [03](unidad-03-tuberia-de-compilacion/), [04](unidad-04-frame-y-vblank/), [09](unidad-09-metasprites/), [14](unidad-14-asm-leer/), [15](unidad-15-asm-escribir/), [17](unidad-17-audio/) | `src/sistema/main.c` |
| `nmiSet` | F | `void nmiSet(void (*vblankRoutine)(void));` | cambia la rutina que la NMI llama en **cada** VBlank | [04](unidad-04-frame-y-vblank/), [05](unidad-05-mando/), [14](unidad-14-asm-leer/) | — (sección 4) |
| `vblank_flag` | V | `extern u8 vblank_flag;` | 1 mientras el bucle espera en `WaitForVBlank` | [04](unidad-04-frame-y-vblank/), [14](unidad-14-asm-leer/) | — |
| `lag_frame_counter` | V | `extern u16 lag_frame_counter;` | VBlanks que llegaron con el bucle sin terminar | [04](unidad-04-frame-y-vblank/) | — |

### `sound.h` — [sound_8h.html](https://alekmaul.github.io/pvsneslib/sound_8h.html)

Todo en la [unidad 17](unidad-17-audio/). El juego **aún no tiene sonido**:
`src/audio/audio.c` es un esqueleto que solo las nombra en un comentario.

| Nombre | Tipo | Firma | Qué hace |
|---|---|---|---|
| `spcBoot` | F | `void spcBoot(void);` | copia el driver snesmod al SPC700; lo primero de `main` |
| `spcSetBank` | F | `void spcSetBank(u8 *bank);` | dónde empieza el soundbank |
| `spcAllocateSoundRegion` | F | `void spcAllocateSoundRegion(u8 size);` | reserva `size` × 256 bytes para efectos BRR; detiene el módulo |
| `spcLoad` | F | `void spcLoad(u16 musIndex);` | copia un módulo al SPC700 (lento) |
| `spcLoadEffect` | F | `void spcLoadEffect(u16 sfxIndex);` | carga un efecto del `.it` de efectos |
| `spcPlay` | F | `void spcPlay(u8 startPos);` | encola «toca desde `startPos`» |
| `spcStop` | F | `void spcStop(void);` | detiene la música (volver a tocar empieza de 0) |
| `spcPauseMusic` | F | `void spcPauseMusic(void);` | detiene guardando la posición |
| `spcResumeMusic` | F | `void spcResumeMusic(void);` | sigue donde se pausó |
| `spcSetModuleVolume` | F | `void spcSetModuleVolume(u8 vol);` | volumen 0..255 |
| `spcFadeModuleVolume` | F | `void spcFadeModuleVolume(u16 vol, u16 fadespeed);` | fundido del volumen |
| `spcFlush` | F | `void spcFlush(void);` | espera a que la cola se vacíe |
| `spcProcess` | F | `void spcProcess(void);` | entrega la cola; una vez por frame, antes de `WaitForVBlank()` |
| `spcEffect` | F | `void spcEffect(u16 pitch, u16 sfxIndex, u8 volpan);` | dispara un efecto cargado |
| `spcSetSoundEntry` | F | `void spcSetSoundEntry(u8 vol, u8 panning, u8 pitch, u16 length, u8 *sampleaddr, brrsamples *ptr);` | registra un efecto BRR |
| `spcPlaySound` | F | `void spcPlaySound(u8 sndIndex);` | dispara un BRR registrado (índice 0 = el último) |
| `spcPlaySoundV` | F | `void spcPlaySoundV(u8 sndIndex, u16 volume);` | lo mismo con volumen 0..15 |

### `map.h` — [map_8h.html](https://alekmaul.github.io/pvsneslib/map_8h.html)

Todo en la [unidad 20](unidad-20-colision/). El juego no lo usa (sección 4);
`src/niveles/niveles.c` es un esqueleto que solo las nombra en un comentario.

| Nombre | Tipo | Firma | Qué hace |
|---|---|---|---|
| `mapLoad` | F | `void mapLoad(u8 *layer1map, u8 *layertiles, u8 *tilesprop);` | carga mapa (`.m16`), metatiles (`.t16`) y propiedades (`.b16`) desde la ROM |
| `mapUpdate` | F | `void mapUpdate(void);` | prepara lo que hay que redibujar según la cámara |
| `mapVblank` | F | `void mapVblank(void);` | lo copia a la VRAM, en el VBlank |
| `mapUpdateCamera` | F | `void mapUpdateCamera(u16 xpos, u16 ypos);` | mueve la cámara (con su zona muerta) |
| `mapGetMetaTile` | F | `u16 mapGetMetaTile(u16 xpos, u16 ypos);` | el metatile de una posición |
| `mapGetMetaTilesProp` | F | `u16 mapGetMetaTilesProp(u16 xpos, u16 ypos);` | su propiedad (`T_SOLID`, `T_PLATE`, `T_SPIKE`...) |
| `mapSetMapOptions` | F | `void mapSetMapOptions(u8 optmap);` | `MAP_OPT_1WAY`, `MAP_OPT_BG2` |

### `object.h` — [object_8h.html](https://alekmaul.github.io/pvsneslib/object_8h.html)

Todo en la [unidad 20](unidad-20-colision/). El juego no lo usa.

| Nombre | Tipo | Firma | Qué hace |
|---|---|---|---|
| `objCollidMap` | F | `void objCollidMap(u16 objhandle);` | choca un objeto con el mapa (`tilestand`, `tileabove`, velocidad) |
| `objCollidMapWithSlopes` | F | `void objCollidMapWithSlopes(u16 objhandle);` | lo mismo con rampas |
| `objCollidObj` | F | `u16 objCollidObj(u16 objhandle1, u16 objhandle2);` | choque entre dos objetos (cajas) |
| `objUpdateXY` | F | `void objUpdateXY(u16 objhandle);` | integra la velocidad |

### `snestypes.h` (sin enlace propio en el curso)

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `u8`, `s8`, `u16`, `s16` | T | `typedef unsigned char u8;` `typedef signed short s16;`... | enteros de tamaño fijo (en tcc816 `int` mide 16 bits) | [00](unidad-00-c-en-el-pc/), [01](unidad-01-enteros-y-punto-fijo/) | en todo el código, vía `src/sistema/tipos.h` |
| `vuint8`, `vuint16` | T | `typedef volatile unsigned char vuint8;`... | el tipo de las macros `REG_xxx` | [13](unidad-13-memoria/) | `src/escena/efectos.c` (`REG_DASB6`) |
| `BIT` | M | `#define BIT(n) (1 << n)` | el bit `n` encendido | [00](unidad-00-c-en-el-pc/) | — |

`snestypes.h` también define `bool`, `s32`/`u32` (como `long long`): el
proyecto no los usa (regla de `CLAUDE.md`).

### Fuera de `snes/*.h`

| Nombre | Tipo | Firma | Qué hace | Unidad | En el juego |
|---|---|---|---|---|---|
| `memset` | F | `void *memset(void *b, int c, size_t n);` (`devkitsnes/include/string.h`) | rellena memoria; está en ensamblador (parámetros en `4,s`, `8,s`, `10,s`; `rtl`): 3 instrucciones por byte | [14](unidad-14-asm-leer/), [15](unidad-15-asm-escribir/) | `src/juego/hud_debug.c` |
| `tcc__mul`, `tcc__div`, `tcc__udiv` | F | sin cabecera: las llama el compilador | multiplicar y dividir cuando no es potencia de 2 (lentas) | [01](unidad-01-enteros-y-punto-fijo/), [14](unidad-14-asm-leer/) | las genera tcc816 |
| `tcc__start`, `VBlank` | — | sin cabecera: arranque (`crt0_snes`) y vector NMI | arranque antes de `main`; rutina de la NMI | [03](unidad-03-tuberia-de-compilacion/), [04](unidad-04-frame-y-vblank/), [13](unidad-13-memoria/), [14](unidad-14-asm-leer/) | — |
| `snes_rules`, `buildWithSummary` | — | `tools/pvsneslib/devkitsnes/snes_rules` | la tubería de `make` del SDK | [03](unidad-03-tuberia-de-compilacion/), [18](unidad-18-herramientas/) | `Makefile` |
| `gfx4snes` | — | `tools/pvsneslib/devkitsnes/tools/` | PNG/BMP → `.pic`, `.pal`, `.map`, `_meta.inc` (`-T`), tabla HDMA (`-n`) | [06](unidad-06-tiles-y-fondos/), [18](unidad-18-herramientas/), [19](unidad-19-del-arte-al-juego/) | `Makefile` |
| `smconv`, `snesbrr` | — | herramientas del SDK | soundbank desde `.it`; WAV → BRR | [17](unidad-17-audio/) | — |
| `tmx2snes` | — | `tools/pvsneslib/devkitsnes/tools/tmx2snes.exe` | Tiled → `.m16`/`.b16`/`.o16` del motor de mapas | [20](unidad-20-colision/) | — (el juego tiene `scripts/preparar_nivel.py`, tag) |

**Registros que el SDK no define** y el juego define él: `REG_DASB6`
(`$4367`, banco de los datos del HDMA indirecto del canal 6) en
`src/escena/efectos.c:47` (unidades [12](unidad-12-hdma-y-efectos/) y
[13](unidad-13-memoria/)); `REG_SLHV` (`$2137`) y `REG_OPVCT` (`$213D`),
la línea del haz para el medidor del VBlank, en
`src/pantallas/pantalla_nivel1.c:83-84` ([04](unidad-04-frame-y-vblank/)).

**Cabeceras que el curso no trata:** `fixed.h`, `gamestates.h`, `lzss.h`,
`pixel.h`, `scores.h`.

## 3. Correcciones medidas por el curso

Lo que la documentación de PVSnesLib no dice, o dice mal, y el curso comprobó;
y, marcado «cabecera», lo que la cabecera sí avisa pero es fácil pasar por
alto. «Fuente» = leído en el fuente de la etiqueta 4.6.0 en GitHub; «ROM» =
leído en los bytes de una ROM compilada; «medido» = medido en Mesen o
compilando.

### De la librería

| Corrección | Cómo se supo | Unidad |
|---|---|---|
| `setBrightness(0)` escribe **`$80`** en `INIDISP`: *forced blank*, no «brillo 0» (`lda #$80` si el nivel es 0, `and #$0F` si no) | fuente y ROM | [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/) |
| `setFadeEffectEx(FADE_OUT, f)` acaba en `$00`: negro **con la pantalla encendida**, y lo que se copie a la VRAM fuera del VBlank se pierde (el fallo del título al volver del nivel; arreglo: `setScreenOff()` antes de cada `_iniciar`, `src/sistema/secuencia.c`) | fuente y medido | [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/) |
| `setFadeEffect`, `setFadeEffectEx` y `setMosaicEffect` **bloquean**: llaman a `WaitForVBlank` dentro; el fundido dura 16 × f frames (con `f = 0`, 256 VBlanks por paso) | fuente y mini-ROM | [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/) |
| `setMosaicEffect` da 16 pasos de 3 VBlanks: **48 frames**. `MOSAIC_OUT` hace crecer los bloques (0 → 15) y `MOSAIC_IN` los deshace: la documentación (y un comentario del fuente) lo dicen al revés | fuente | [12](unidad-12-hdma-y-efectos/) |
| `setScreenOn()` **espera un VBlank** antes de escribir `$0F` | cabecera («Calls WaitForVBlank() before enabling the screen») y fuente | [04](unidad-04-frame-y-vblank/), [12](unidad-12-hdma-y-efectos/) |
| `setModeHdmaColor` cambia el **color 0 de la CGRAM** (modo 3, destino `$21`: `CGADD` dos veces y `CGDATA` dos veces), no `COLDATA`; además apaga **todos** los demás canales de HDMA y espera dos interrupciones | fuente | [12](unidad-12-hdma-y-efectos/) |
| `setModeHdmaReset(channels)` escribe `channels` tal cual en `HDMAEN`: `setModeHdmaReset(0)` apaga todos | fuente | [12](unidad-12-hdma-y-efectos/) |
| `setModeHdmaGradient`, `setModeHdmaShadeUpDown` y `setParallaxScrolling` usan **los tres el canal 3**: no pueden ir a la vez | fuente | [12](unidad-12-hdma-y-efectos/) |
| Lo que escribe una tabla de HDMA se ve desde la línea **siguiente** (la raya gris bajo el logo; `tabla[0] = arriba - 1` en `src/escena/efectos.c`) | medido en Mesen | [12](unidad-12-hdma-y-efectos/) |
| `bgSetScroll` escribe los registros **en el acto**, sin sombras ni espera al VBlank | descrito en la unidad (sin decir si se leyó en el fuente) | [07](unidad-07-scroll-y-camara/) |
| La NMI del SDK **ya copia la OAM** (544 bytes por DMA, canal 7) en cada VBlank bueno: no hace falta `oamUpdate()` | fuente | [04](unidad-04-frame-y-vblank/), [08](unidad-08-sprites-y-oam/), [14](unidad-14-asm-leer/) |
| Orden de la NMI: copia la OAM (si el bucle esperaba), llama a `nmi_handler` (**siempre**, también en frames perdidos), suma `snes_vblank_count` y, solo en frames buenos, borra `vblank_flag` y lee el mando; si no, suma `lag_frame_counter` | fuente | [04](unidad-04-frame-y-vblank/), [05](unidad-05-mando/), [14](unidad-14-asm-leer/) |
| El SDK gasta **5 de las 37 líneas** del VBlank antes de devolver el control (la `a` de `V5`) | medido (HUD) | [04](unidad-04-frame-y-vblank/), [09](unidad-09-metasprites/) |
| `consoleVblank` existe en la librería pero **ninguna cabecera la declara**: con `nmiSet` hay que escribir `extern void consoleVblank(void);` y llamarla, o el texto deja de actualizarse | compilado (`-DNMI_PROPIA`) | [04](unidad-04-frame-y-vblank/) |
| `consoleInit()` la llama el arranque antes de `main`; el programa no la llama | fuente | [03](unidad-03-tuberia-de-compilacion/) |
| El arranque (`crt0_snes`) **solo borra parte de la `.bss`**: las etiquetas `SECTIONSTART_.bss` salen repetidas (un aviso del `.log`) y el bucle borra el tamaño de una sola; en el juego, **10 bytes** de unos 1 800. Las `static` empiezan con basura | fuente y ROM | [13](unidad-13-memoria/) |
| `padsUp` es función (no macro como `padsCurrent`/`padsDown`) y devuelve `unsigned short`; la documentación la anuncia como `\fn padsUp(u16 value)` | cabecera | [05](unidad-05-mando/) |
| La NMI pone a 0 la palabra del mando si sus bits 0..3 no valen 0 (no es un mando normal) | fuente | [05](unidad-05-mando/) |
| `setPaletteColor` es una macro de **tres sentencias sin llaves**: `if (x) setPaletteColor(...)` solo condiciona la primera. Úsala dentro de `{ }` | cabecera | [06](unidad-06-tiles-y-fondos/) |
| Ninguna función cambia **una** celda del mapa: se hace con `dmaCopyVram` de 2 bytes | cabecera | [06](unidad-06-tiles-y-fondos/) |
| `dmaCopyVram` fija **siempre** `VMAIN = 0x80`: no sirve para copiar columnas (avance de 32) | descrito en las unidades | [06](unidad-06-tiles-y-fondos/), [07](unidad-07-scroll-y-camara/), [20](unidad-20-colision/) |
| `dmaCopyVram7` sí recibe `VMAIN`, pero su documentación no dice si lo deja como estaba: ponlo en `0x80` tú mismo | cabecera | [20](unidad-20-colision/) |
| `oamSetVisible(id, OBJ_HIDE)` lleva el sprite a y = 240, y con sprites de 64 (o de 32 en el nivel) asoma por abajo | cabecera y medido | [08](unidad-08-sprites-y-oam/) |
| El motor dinámico de sprites no admite 64×64, y `oamDynamicMetaDraw` lee otra struct: la que lee las tablas de `gfx4snes -T` es `oamMetaDrawDyn32` | cabecera | [09](unidad-09-metasprites/) |
| El motor dinámico gastaba **32 líneas** del VBlank en los tiles de Tarou (6 DMA de 512 B + preparación), no las 18 del dato | medido (HUD) | [09](unidad-09-metasprites/) |
| El motor de mapas solo funciona en el fondo 0 («Map engine must be used on Background #0») y no tiene ninguna función para escribir en el mapa | cabecera | [20](unidad-20-colision/) |
| `spcAllocateSoundRegion` detiene el módulo: va antes de `spcLoad`/`spcPlay`. El índice de `spcPlaySound` va al revés (0 = el último registrado) | cabecera | [17](unidad-17-audio/) |
| El `switch (pad0)` del ejemplo `input/controller` compara la palabra entera: con dos botones a la vez no coincide ningún `case`. Hay que usar `&` | medido (demo) | [05](unidad-05-mando/) |
| Las direcciones de la librería en el `.sym` no cambian entre compilaciones (`memset` `$81:8371`, `tcc__mul` `$81:8000`, `tcc__div` `$81:814F`, `VBlank` `$80:81AF`) | ROM | [14](unidad-14-asm-leer/) |
| `memset` (ensamblador) cuesta 3 instrucciones por byte; un `*p++ = 0` en C, muchas más (se cuentan en el TODO 11 de la 15; en el HUD, vaciar una fila pasó de ~30 líneas a ~3, cabecera de `src/juego/hud_debug.c`) | ROM y medido | [14](unidad-14-asm-leer/), [15](unidad-15-asm-escribir/) |
| `gfx4snes -T` empaqueta los bloques de 32×32 en filas de 128 px: una tira de 1 cuadro ocupa **4 KB** en la ROM, no 3 | medido | [18](unidad-18-herramientas/) |

### Del compilador (tcc816), no de la librería

| Corrección | Cómo se supo | Unidad |
|---|---|---|
| `int` mide **16 bits** y un `enum`, 2 bytes | compilado | [01](unidad-01-enteros-y-punto-fijo/), [02](unidad-02-maquinas-de-estado/) |
| `?:` con cadenas como argumento variádico (p. ej. de `consoleDrawText`) da `error: ICE 2` | compilado | [04](unidad-04-frame-y-vblank/), [05](unidad-05-mando/), [09](unidad-09-metasprites/), [17](unidad-17-audio/) |
| Con todo constantes, `* 256` y `* 128 ... * 2` dan lo mismo (se pliega al compilar); con una variable, el producto se hace a 16 bits y desborda | compilado | [01](unidad-01-enteros-y-punto-fijo/) |
| Un `case` sin `break` cae en el siguiente **sin aviso** (gcc sí avisa) | compilado | [02](unidad-02-maquinas-de-estado/) |
| Las tablas `const` **globales** van a la WRAM (`$7F`, copiadas al arrancar); las `static const`, a la ROM (`.rodata`, que sale con banco `$0E` en vez de `$8E`) | `.sym` | [13](unidad-13-memoria/) |
| `/` y `%` (también `u8 / 2` y `s16 / 2`) llaman a `tcc__div`; indexar structs de tamaño no potencia de 2 llama a `tcc__mul` | `.asm` generado | [01](unidad-01-enteros-y-punto-fijo/), [14](unidad-14-asm-leer/) |

## 4. Lo que el juego no usa, y por qué

| Lo que no usa | Qué usa en su lugar | Por qué | Unidad |
|---|---|---|---|
| El **motor dinámico de sprites** (`oamInitDynamicSprite`, `oamMetaDrawDyn32`, `oamVramQueueUpdate`, `oamInitDynamicSpriteEndFrame`) | motor propio desde el 2026-10-21: `oamSet` por pieza, dos celdas de VRAM y los 3 KB del cuadro nuevo repartidos en dos VBlanks por la lista de DMA (`src/juego/tarou_sprite.c`, `metasprite.c`) | su subida costaba 32 líneas; con las columnas del fondo el VBlank pasaba de 37 y Tarou salía «a trozos» | [09](unidad-09-metasprites/) |
| `dmaCopyVram` para las columnas del fondo y el HUD (y `dmaCopyVram7`) | la **lista de DMA** (`src/juego/dma_lista.c` + `dma_ejecutar.asm`): se apunta durante el frame y en el VBlank solo se escriben registros, con `VMAIN = 0x81` para columnas | `dmaCopyVram` fija `VMAIN = 0x80`; cada llamada desde C cuesta cientos de instrucciones dentro del VBlank (dos columnas: 9 de 37 líneas) | [07](unidad-07-scroll-y-camara/), [20](unidad-20-colision/) |
| El **motor de mapas** (`mapLoad`, `mapUpdate`, `mapVblank`...) y `tmx2snes` | rejilla propia de 128×14 (`src/juego/colision.c`, tag), streaming de columnas (`fondo_nivel.c`, se queda) y `scripts/preparar_nivel.py` desde Tiled (tag; en el beat 'em up, `preparar_zona.py`) | no se puede cambiar el mapa en marcha (el tronco), el arte es de rejilla doble, trae su propia cámara | [20](unidad-20-colision/) |
| El **motor de objetos** (`objCollidMap`, `objUpdateXY`...) | la física de Tarou (`src/juego/tarou.c`; la de plataformas, tag), probada en el PC | habría sustituido una física ya escrita y probada (dos pies, túnel, agacharse) | [20](unidad-20-colision/) |
| `nmiSet` (rutina propia en la NMI) | `secuencia_vblank()` justo después de `WaitForVBlank()` en `src/sistema/main.c` | solo corre en frames buenos, ve los datos del frame terminados y es C normal; el precio es empezar unas líneas más tarde | [04](unidad-04-frame-y-vblank/) |
| `setFadeEffectEx`/`setMosaicEffect` **para el logo Nintendo** (sí los usa en otras pantallas: `comun.c`, `pantalla_intro.c`) | el motor de escenas (`src/escena/`): brillo con `setBrightness`, mosaico, ventana y luz con `REG_xxx`, un byte por frame | las funciones bloquean 32-48 frames y no se pueden mezclar; la ventana con HDMA indirecto no existe en la librería | [12](unidad-12-hdma-y-efectos/) |
| `setColorEffect`, `setColorIntensity`, `setModeHdmaWindow` | `REG_CGWSEL`, `REG_CGADSUB`, `REG_COLDATA`, `REG_WOBJSEL`, `REG_WH0/1` y el canal 6 a mano (`src/escena/efectos.c`) | el destello necesita HDMA indirecto movido cambiando 2 bytes | [12](unidad-12-hdma-y-efectos/) |
| `setParallaxScrolling` | dos capas con `bgSetScroll` a distinta velocidad | el juego hace el parallax por capas, no por franjas | [07](unidad-07-scroll-y-camara/) |
| `consoleDrawText` en el **HUD de depuración** (sí en `comun.c` para los textos de las pantallas) | escribir el mapa de texto en RAM con `memset` y punteros, una fila de 64 bytes por frame a la lista de DMA (`src/juego/hud_debug.c`) | su formateo de `%d` hacía correr el juego al 60 % (perdía VBlanks); copiar todo con `dmaCopyVram` costaba 5 líneas | [15](unidad-15-asm-escribir/) (el ejercicio parte de `hud_debug.c`; la explicación está en la cabecera de ese archivo) |
| `oamSetVisible` / el escondite y = 240 | su propia función `esconder` en `tarou_sprite.c` | con sprites grandes, y = 240 asoma | [08](unidad-08-sprites-y-oam/) |
| `oamUpdate` | nada: la NMI ya copia la OAM | sería una copia doble | [08](unidad-08-sprites-y-oam/) |
| `consoleMesenBreakpoint`, `consoleNocashMessage` | el HUD (se ve en las capturas) y el `.sym` | — | [16](unidad-16-mesen/) |
| Las funciones `spc*` | (el juego aún no suena) | pendiente: la unidad 17 lo practica en su mini-ROM | [17](unidad-17-audio/) |

## 5. Ejemplos del SDK citados por las unidades

Todos bajo `tools/pvsneslib/snes-examples/` (rutas comprobadas; el nombre
`Mode1ContinuosScroll` está escrito así en el SDK).

| Ejemplo | Qué enseña | Unidad |
|---|---|---|
| `hello_world/src/hello_world.c` | consola de texto; base de las mini-ROM | [03](unidad-03-tuberia-de-compilacion/), [14](unidad-14-asm-leer/), [15](unidad-15-asm-escribir/) |
| `systems/timer/src/timer.c` | contar VBlanks; base de la mini-ROM | [04](unidad-04-frame-y-vblank/) |
| `graphics/Backgrounds/Mode1ContinuosScroll/Mode1ContinuosScroll.c` | rutina propia `myconsoleVblank` con `nmiSet` | [04](unidad-04-frame-y-vblank/), [14](unidad-14-asm-leer/) |
| `input/controller/controller.c` | leer el mando (y la pega de su `switch`) | [00](unidad-00-c-en-el-pc/), [05](unidad-05-mando/) |
| `graphics/Backgrounds/Mode1` | un fondo de 16 colores con `bgInitTileSet` + `bgInitMapSet` | [06](unidad-06-tiles-y-fondos/) |
| `graphics/Backgrounds/Mode1Scroll` | mapa de 64×64 movido con `bgSetScroll` | [07](unidad-07-scroll-y-camara/) |
| `graphics/Effects/ParallaxScrolling` | parallax por franjas con HDMA | [07](unidad-07-scroll-y-camara/), [12](unidad-12-hdma-y-efectos/) |
| `graphics/Sprites/SimpleSprite` | un sprite de 32×32 con `oamInitGfxSet` + `oamSet` + `oamSetEx` | [08](unidad-08-sprites-y-oam/) |
| `graphics/Sprites/DynamicEngineMetaSprite` | el motor dinámico y `gfx4snes -T` sobre una tira | [09](unidad-09-metasprites/), [19](unidad-19-del-arte-al-juego/) |
| `graphics/Effects/GradientColors` | `setModeHdmaColor` con tabla de `gfx4snes -n`; base de la mini-ROM | [12](unidad-12-hdma-y-efectos/) |
| `graphics/Effects/HDMAGradient` | `setModeHdmaGradient` en Mode 3; un asset de más de 32 KB en dos secciones | [12](unidad-12-hdma-y-efectos/), [13](unidad-13-memoria/) |
| `graphics/Effects/Fading`, `Mosaic`, `Window`, `WindowCircular`, `Waves`, `Transparency`, `TransparentWindow` | fundidos, mosaico, ventanas, ondas, transparencias | [12](unidad-12-hdma-y-efectos/) |
| `systems/memory_mapping` | otra ROM: HiROM + FastROM | [13](unidad-13-memoria/) |
| `systems/breakpoints` | `consoleMesenBreakpoint` | [16](unidad-16-mesen/) |
| `systems/debug` | `consoleNocashMessage` | [16](unidad-16-mesen/) |
| `audio/music`, `audio/music2`, `audio/effects`, `audio/effectsandmusic`, `audio/tada` | música, pausa, efectos, BRR | [17](unidad-17-audio/) |
| `systems/games/likemario/src/LikeMario.c` (ROM: `bin/likemario.sfc`) | música + efecto BRR en un juego | [17](unidad-17-audio/) |
| `graphics/maps/tiled` | el motor de mapas con un mapa de Tiled | [20](unidad-20-colision/) |
| `systems/objects/mapandobjects` | Mario con el motor de objetos | [20](unidad-20-colision/) |
