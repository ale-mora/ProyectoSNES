# Unidad 09 — Metasprites y sprites dinámicos

**Objetivo:** dibujar un personaje más grande que un sprite (Tarou mide 96×64)
y con más cuadros de los que caben en la VRAM: cada cuadro se compone de varias
piezas (metasprite) y solo el cuadro que se ve está en la VRAM (sprite
dinámico). Primero con el motor de PVSnesLib, después con el motor propio
que usa el juego desde el 2026-10-21, y entender **por qué** se cambió.

**Prerrequisitos:** unidad 08 (sprites, OAM, las dos tablas de 256 tiles,
`gfx4snes -s`, esconder un sprite); a través de ella, 06 (tiles, paletas,
DMA), 04 (frame, VBlank, forced blank), 03 (tubería, secciones, bancos) y 00.
**Práctica:** mini-ROM + PC + sobre el juego real. **Tiempo orientativo:** dos
o tres tardes.

## La idea en una página

```
ROM (4 MB)                                   VRAM (64 KB)                 OAM (128 entradas)
┌───────────────────────────────┐            ┌──────────────────┐         ┌────────────────┐
│ tarou_correr.pic  (8 cuadros) │  copiar    │ tiles 0..127     │◄────────│ 0: x,y,tile 0  │
│ tarou_reposo.pic  (9 cuadros) │ ─ solo el ─►│  (el cuadro que  │         │ 1: x+32, tile 4│
│ ... 13 tiras, 88 cuadros      │  cuadro    │   se ve ahora)   │         │ ...            │
│ = 264 KB de tiles             │  que toca  │ tiles 128..255   │         │ 5: x+64,y+32   │
└───────────────────────────────┘            └──────────────────┘         └────────────────┘
          + tabla metasprite por cuadro: qué pieza (bloque de 32×32) va en qué (dx, dy)
```

Tres ideas y una restricción:

1. **Metasprite:** un cuadro de 96×64 son 3×2 piezas de 32×32; cada pieza es
   una entrada de la OAM. Se mueven juntas sumando su `(dx, dy)` a la posición
   del personaje.
2. **Sprite dinámico:** los 88 cuadros de Tarou (264 KB) no caben en los 16 KB
   de tiles de sprites de la VRAM. Viven en la ROM y, cuando el cuadro cambia,
   se copian sus tiles (3 KB) a la VRAM.
3. **Espejo:** mirar a la izquierda no necesita otra tira: cada pieza cambia de
   lado y se voltea con un bit de la OAM.
4. **Restricción:** la VRAM solo se escribe en el VBlank (unidad 04), que dura
   37 líneas del haz, y el DMA copia unos 170 bytes por línea. 3 KB son 18
   líneas. Ahí está toda la historia de esta unidad.

## Conceptos

### Metasprite

Un **metasprite** es un sprite grande hecho de varios sprites de hardware
(«piezas») que se dibujan juntos (→ glosario: Metasprite). La SNES no tiene
sprites de 96×64: los tamaños de la OAM son cuadrados (8, 16, 32 o 64 px,
unidad 08). Tarou se dibuja en una **celda** de 96×64 (→ glosario: Celda)
partida en 6 piezas de 32×32:

```
 dx:   0        32       64
     ┌────────┬────────┬────────┐ dy 0
     │ pieza0 │ pieza1 │ pieza2 │
     ├────────┼────────┼────────┤ dy 32
     │ pieza3 │ pieza4 │ pieza5 │
     └────────┴────────┴────────┘
     posición de la pieza = (x + dx, y + dy)
```

¿Por qué 32×32 y no 64×64? Con `OBJ_SIZE16_L32` (pequeño 16×16, grande 32×32)
la celda de 96 px se cubre exacta con 3 piezas; con 64×64 harían falta 2×1 y
sobrarían 32 px de ancho de tiles vacíos en cada cuadro. Además el juego
puede **omitir** las piezas vacías, como se ve enseguida.

### La tabla metasprite de `gfx4snes -T`

`gfx4snes` es el conversor de imágenes del SDK (unidad 06). Con `-T` escribe,
además de los tiles, un archivo `<nombre>_meta.inc` con **una tabla por
cuadro**. La regla del juego (`Makefile`, regla patrón de Tarou) es:

```
$(GFXCONV) -s 32 -o 16 -u 16 -p -T -X 96 -Y 64 -P 3 -i $<
```

`-s 32` = piezas de 32×32; `-T` = tabla metasprite; `-X 96 -Y 64` = tamaño de
la celda (cuántas piezas forman un cuadro); `-P 3` = prioridad 3 en el
atributo de cada pieza. Un cuadro real (`res/personajes/tarou/tarou_correr_meta.inc`,
se genera al compilar):

```c
const t_metasprite tarou_correr_metasprite0[] = {
	METASPR_ITEM(0, 0, 0, OBJ_PAL(0) | OBJ_PRIO(3)),
	METASPR_ITEM(32, 0, 1, OBJ_PAL(0) | OBJ_PRIO(3)),
	METASPR_ITEM(64, 0, 2, OBJ_PAL(0) | OBJ_PRIO(3)),
	METASPR_ITEM(0, 32, 24, OBJ_PAL(0) | OBJ_PRIO(3)),
	METASPR_ITEM(32, 32, 25, OBJ_PAL(0) | OBJ_PRIO(3)),
	METASPR_ITEM(64, 32, 26, OBJ_PAL(0) | OBJ_PRIO(3)),
	METASPR_TERM
};
const t_metasprite* tarou_correr_metasprites[8] = { ... };   /* una tabla por cuadro */
```

Cada entrada es una `t_metasprite` (de `snes/sprite.h`, 8 bytes):

| Campo | Tipo | Qué es |
|---|---|---|
| `dx`, `dy` | `u16` | posición de la pieza dentro de la celda |
| `dtile` | `u16` | número de **bloque** de 32×32 dentro de la tira |
| `props` | `u8` | atributo de la OAM: `vhoopppc` (v/h = volteo, oo = prioridad, ppp = paleta) |
| `unused` | `u8` | relleno para que la entrada mida 8 bytes |

Tres detalles que el código del juego respeta:

- **El terminador.** La tabla termina con `METASPR_TERM`, una entrada con
  `dx = metasprite_end = -128`. Como `dx` es `u16`, se lee como **`0xFF80`**
  (`METASPRITE_FIN` en `src/juego/metasprite.h`).
- **Puede haber menos de 6 piezas.** `gfx4snes -T` **omite las piezas
  totalmente transparentes**. Tarou dormido está tumbado: los cuadros 0, 1 y
  8 de `dormido` tienen **4** piezas (las dos de arriba a los lados están
  vacías). Nadie puede suponer «siempre 6»: se recorre hasta el terminador
  (`metasprite_piezas`).
- **La numeración de bloques** sigue la imagen, fila a fila. La tira de
  correr mide 768×64 px = 24×2 bloques: la fila de arriba son los bloques
  0..23 y la de abajo, 24..47. Por eso el cuadro 0 usa 0, 1, 2 y 24, 25, 26.
  Reposo (9 cuadros, 864 px) tiene 27 por fila: su cuadro 0 usa 0, 1, 2, 27, 28, 29.

### Cómo guarda la tira los bloques (`.pic`)

El `.pic` no guarda la imagen tal cual: la reordena en filas de **4 bloques**
(128 px de ancho, la misma anchura que la hoja de sprites de la VRAM: 16
tiles). Lo dice el registro de compilación de la mini-ROM
(`capturas/compilacion_rom.txt` guarda el resumen; el detalle está en el `.log`):

```
gfx4snes: convert image from 864x64 to 128x448...
gfx4snes: image converted to 4x14 blocks of 32x32...
gfx4snes: saving 896 tiles in file [res/tarou_reposo.pic]...
```

```
.pic de reposo (128 px de ancho)         bloque n → fila n / 4, columna n % 4
┌────┬────┬────┬────┐ fila 0  (2048 B)   cada fila de bloques = 4 filas de tiles
│ 0  │ 1  │ 2  │ 3  │                     de 16 tiles x 32 B = 512 B cada una
├────┼────┼────┼────┤ fila 1
│ 4  │ 5  │ 6  │ 7  │                     54 bloques → 14 filas (la última a medias)
│ .. │    │    │    │                     = 14 x 2048 = 28 672 B (896 tiles)
```

Los 9 cuadros son 27 648 bytes de tiles útiles; el `.pic` mide 28 672 porque
la última fila de 4 queda a medias. Esto importa para copiar un bloque: la
fila de tiles `k` del bloque `n` está en `(n/4)·2048 + k·512 + (n%4)·128`, y
son 128 bytes seguidos. Si los bloques `n` y `n+1` están en la misma fila de 4,
sus filas de tiles van pegadas: se copian juntas.

### Sprite dinámico

Un **sprite dinámico** guarda todos los cuadros en la ROM y copia a la VRAM
solo el que se muestra (→ glosario: Sprite dinámico). Las cuentas de Tarou:

| | Cifra |
|---|---|
| Un cuadro de 96×64 a 4 bpp | 6 piezas × 16 tiles × 32 B = **3 072 B** (3 KB) |
| Los 88 cuadros del arte actual | ~264 KB: imposible en los 16 KB de sprites de la VRAM |
| Tiles de sprites que direcciona la OAM | 512 (dos tablas de 256, unidad 08) |
| Un banco de ROM (LoROM) | 32 KB → como mucho **10 cuadros** por animación (30 KB) |

El último dato es una regla del proyecto: cada tira va en su propia sección
`superfree` de `data.asm` (`.rodata_tarou_<anim>`), y una sección no puede
cruzar un banco (unidad 03; a fondo en la 13). `scripts/preparar_personaje.py`
se niega a generar una tira de más de 10 cuadros: por eso `despertar` usa 10
de los 11 cuadros del arte.

### Espejo horizontal, pieza a pieza

Para mirar a la izquierda no hay tiras espejadas en la ROM. Se espeja **la
tabla**: cada pieza cambia de lado dentro de la celda y se voltea.

```
mirando a la derecha            mirando a la izquierda
 dx 0    32    64                dx 0    32    64
┌─────┬─────┬─────┐             ┌─────┬─────┬─────┐
│  A  │  B  │  C  │    ──►      │ Ↄ   │ ᗺ   │ ᗅ   │   C pasa a dx 0, A a dx 64:
└─────┴─────┴─────┘             └─────┴─────┴─────┘   dx' = 64 − dx (64 = 96 − 32)
                                                      y cada pieza con el bit h (0x40)
```

Cada pieza muestra **el mismo bloque**, volteado. La de en medio (dx 32) se
queda en su sitio. Lo hace `metasprite_espejar` en `src/juego/metasprite.c`
(probado en `tests/prueba_metasprite.c`): copia campo a campo (tcc816 no copia
structs con `=`), pone `0x40` en `props` y escribe el terminador **justo
detrás de la última pieza**, sean 6 o 4.

## La librería: PVSnesLib

Cabecera principal: [`sprite.h`](https://alekmaul.github.io/pvsneslib/sprite_8h.html)
(índice: [files.html](https://alekmaul.github.io/pvsneslib/files.html)).
Ejemplo del SDK: `tools/pvsneslib/snes-examples/graphics/Sprites/DynamicEngineMetaSprite/`
(su `.c` está en la raíz del ejemplo, no en `src/`).

| Función (firma exacta) | Qué hace | Hardware |
|---|---|---|
| `void oamInitDynamicSprite(u16 gfxsp0adr, u16 gfxsp1adr, u16 oamsp0init, u16 oamsp1init, u8 oamsize);` | Arranca el motor dinámico: dónde van en la VRAM los tiles de las piezas grandes (`gfxsp0adr`) y pequeñas (`gfxsp1adr`), desde qué entrada de la OAM reparte cada tamaño y los tamaños (`OBJ_SIZE16_L32`: 16×16 y 32×32; 64×64 no lo admite). | `OBSEL` ($2101), buffer de la OAM |
| `void oamMetaDrawDyn32(u16 id, s16 x, s16 y, u8 *sprmeta, u8 *gfxptr);` | Dibuja un metasprite de piezas de 32×32: recorre la tabla `t_metasprite` hasta `metasprite_end`, ocupa una entrada de la OAM por pieza desde `id` y, si `oambuffer[id].oamrefresh` vale 1, **encola** la copia de los bloques desde `gfxptr` (la tira, puntero de 24 bits). | buffer de la OAM, cola de copias |
| `void oamInitDynamicSpriteEndFrame(void);` | Cierra el frame del motor: esconde las entradas que este frame no usó y deja los contadores listos para el siguiente. | buffer de la OAM |
| `void oamVramQueueUpdate(void);` | Ejecuta la cola: un DMA por pieza encolada hacia la VRAM. **Justo después de `WaitForVBlank()`**. | DMA, VRAM ($2116-$2119) |
| `void WaitForVBlank(void);` ([`interrupt.h`](https://alekmaul.github.io/pvsneslib/interrupt_8h.html)) | Espera a la NMI; la NMI del SDK copia el buffer de la OAM a la OAM y lee el mando. | NMI |
| `#define setPalette(palette, paletteEntry, paletteSize)` ([`video.h`](https://alekmaul.github.io/pvsneslib/video_8h.html)) | Macro: es `dmaCopyCGram`. La paleta de sprites 0 empieza en el color 128. | CGRAM |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` ([`dma.h`](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | Copia una paleta a la CGRAM por DMA (la usa el juego). | CGRAM |
| `void oamInitGfxAttr(u16 address, u8 oamsize);` | Sin motor: dónde empiezan los tiles de sprites y qué dos tamaños hay. La usa el juego. | `OBSEL` |
| `void oamSet(u16 id, u16 xspr, u16 yspr, u8 priority, u8 hflip, u8 vflip, u16 gfxoffset, u8 paletteoffset);` | Escribe una entrada de la OAM. **`id` es la entrada × 4** (desplazamiento en bytes del buffer). | buffer de la OAM |
| `void oamSetEx(u16 id, u8 size, u8 hide);` | Tamaño (`OBJ_SMALL`/`OBJ_LARGE`) y visibilidad (`OBJ_SHOW`/`OBJ_HIDE`) de una entrada. | buffer de la OAM (tabla alta) |
| `void oamSetXY(u16 id, u16 xspr, u16 yspr);` | Solo la posición. | buffer de la OAM |
| `void oamClear(u16 first, u8 numEntries);` | Esconde entradas; `oamClear(0, 0)` esconde las 128. | buffer de la OAM |
| `typedef struct { u16 dx, dy; u16 dtile; u8 props; u8 unused; } t_metasprite;` | Una pieza de la tabla; `METASPR_ITEM`, `METASPR_TERM` y `metasprite_end` (-128) son macros de la misma cabecera. | — |
| `extern t_sprites oambuffer[128];` | El estado del motor por entrada; `oambuffer[id].oamrefresh = 1` pide volver a copiar los tiles. | — |

Ojo: `oamDynamicMetaDraw(u16 id, s16 x, s16 y, u8 *sprmeta)` también existe,
pero lee **otra** struct (`t_metasprites`, 16 bytes, terminada en 0xFFFF y con
un puntero de gráficos por pieza): no sirve para las tablas de `gfx4snes -T`.
La que encaja es `oamMetaDrawDyn32` (y sus hermanas `Dyn16`/`Dyn8`).

## La mini-ROM: el motor del SDK

`rom/` parte de `DynamicEngineMetaSprite` (receta de PLAN §6.3) con las tiras
de **reposo** y **caminar** de Tarou convertidas igual que en el juego. La
cruceta lo hace caminar; sin cruceta, reposa. El bucle de `rom/src/main.c`:

```c
if (anim != ult_anim || cuadro != ult_cuadro || izquierda != ult_izq)
{
    oambuffer[0].oamrefresh = 1;       /* cuadro nuevo: que el motor copie sus tiles */
    ...
}
if (izquierda) { espejar(tabla); tabla = espejo; }   /* tabla espejada en la RAM */
oamMetaDrawDyn32(0, x, Y_TAROU, (u8 *)tabla, tiles); /* 6 entradas desde la 0    */
oamInitDynamicSpriteEndFrame();
WaitForVBlank();
oamVramQueueUpdate();                  /* aquí, en el VBlank, van los 3 KB             */
```

Es exactamente lo que hacía `src/juego/tarou_sprite.c` hasta el 2026-10-20
(se puede leer con `git show 8a32523:src/juego/tarou_sprite.c`).

Compilar (PowerShell, dentro de `rom/`):

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
```

Debe terminar en `No warnings or errors found.` (`capturas/compilacion_rom.txt`)
y dejar `u09_metasprite.sfc`. En Mesen se ve el texto
`UNIT 09 - METASPRITE 96X64`, a Tarou respirando y, con la cruceta, caminando
en las dos direcciones; abajo, `ANIM WALK  FRAME n`.

Tres diferencias con el ejemplo del SDK, todas reglas del proyecto: no hay
`bool` ni `int` en las variables (sí en `main`, que C exige), las `static` se
inicializan en `main` (la WRAM arranca con basura) y el texto no usa
`cond ? ">" : " "` como argumento de `consoleDrawText`: con cadenas en `?:`
dentro de una función variádica, tcc816 da un **error interno**. Se escribe
con `if`/`else`.

## Por qué el juego dejó el motor del SDK: Tarou a trozos

En la mini-ROM el motor funciona: en el VBlank no hay nada más. En el nivel,
el VBlank lo comparten tres cosas. Las cifras, medidas con la columna `V`
del HUD (líneas del VBlank al entrar y al salir de `pantalla_nivel1_vblank`;
→ glosario: Presupuesto del VBlank) y contadas en
`docs/diseno-tutorial-movimiento.md` §8, «Trampas encontradas» (2026-10-20):

```
VBlank: 37 líneas                                       (después la PPU dibuja)
├─ SDK (OAM + mando) ......  5
├─ tiles de Tarou ......... 32   ← oamVramQueueUpdate: 6 DMA de 512 B + preparación
├─ columnas del fondo .....  9
└─ total ................. 46   ← 9 líneas de más
```

Lo que llega a la VRAM con la PPU ya dibujando **se pierde sin aviso**. El
síntoma: piezas que faltan, trozos del cuadro anterior, parpadeo. Se
encontró **midiendo** (la línea del haz al entrar y salir del VBlank), no
mirando capturas: es el quinto caso de la unidad 16. Y en la Revisión 1 el
motor ya estaba en 37, al límite, sin que nadie lo hubiera medido.

¿Por qué 32 líneas si 3 KB son 18? `3072 / 170 ≈ 18` es solo el **dato**
(el DMA mueve un byte cada 8 ciclos maestros y una línea son 1364: ~170 B). El
resto es la preparación de cada DMA en el código del motor (registros, banco,
dirección por pieza). Cuando el coste fijo por copia es grande, hay que
copiar menos veces o prepararlo antes.

## El motor propio del juego (desde el 2026-10-21)

Cuatro piezas, cada una en su archivo:

### 1. Una entrada de la OAM por pieza, con `oamSet`

`tarou_sprite_dibujar` (`src/juego/tarou_sprite.c`, paso 2) recorre la tabla
del cuadro **mostrado** y llama a `oamSet` por pieza. El tile de cada pieza es
**fijo dentro de la celda** de la VRAM, no depende del bloque de la tira:

```c
dx = izquierda ? (u16)(METASPRITE_ESPEJO_DX - tabla[i].dx) : tabla[i].dx;
tile = (u16)(base + (((tabla[i].dy >> 5) << 6) | ((tabla[i].dx >> 5) << 2)));
oamSet((u16)(OAM_TAROU + i) * 4, (u16)(x + dx), (u16)(y + tabla[i].dy),
       PRIORIDAD_TAROU, izquierda, 0, tile, PALETA_TAROU);
```

Fila de la celda `dy/32` × 64 tiles + columna `dx/32` × 4 tiles: la pieza de
arriba a la izquierda usa el tile 0, la de en medio el 4, la de la derecha el
8; abajo, 64, 68 y 72. El espejo es el parámetro `hflip` de `oamSet` y el
`64 − dx`: aquí no hace falta una tabla espejada en la RAM. Las entradas que el
cuadro no usa (dormido: 4 piezas) se esconden con `esconder()`, que pone
`x = 256` **y** `y = 224`, porque con el clásico `y = 240` un sprite grande
asoma por arriba (unidad 08).

### 2. Dos celdas en la VRAM: el doble búfer

```
VRAM de sprites (VRAM_SPRITES = 0x0000), 16 tiles de ancho
tiles   0..127  ┌──────────────┐  búfer 0: una celda de 3×2 bloques (8 filas de 16 tiles)
                │ 0  4  8      │
                │ 64 68 72     │
tiles 128..255  ├──────────────┤  búfer 1 (VRAM_BUFER(1) = 0x0800 palabras)
                │128 132 136   │
                │192 196 200   │
                └──────────────┘  las dos en la PRIMERA tabla de 256: ningún
                                  sprite cruza a la segunda (unidad 08)
```

Mientras la OAM enseña el cuadro desde un búfer, el cuadro nuevo se escribe en
el **otro**, como mucho `BYTES_POR_VBLANK = 1536` bytes por VBlank (media
celda, unas 9 líneas). Cuando está entero, la OAM pasa a enseñarlo y los
búferes se intercambian (`bufer_mostrado ^= 1`) (→ glosario: Doble búfer).
El precio: **el dibujo del cuadro va un frame por detrás** de la lógica (la
posición no). A 60 FPS no se nota.

### 3. El plan de copias: `metasprite_copias`

Qué trozos de la tira hay que copiar y adónde. Es lógica pura
(`src/juego/metasprite.c`, probada en `tests/prueba_metasprite.c`): para cada
pieza calcula su bloque en la tira y su sitio en la celda, junta piezas
vecinas con bloques seguidos en la misma fila de 4 y emite 4 copias por
tramo (las 4 filas de tiles del bloque). `demo_09_copias.c` la ejecuta en el
PC con cuadros reales:

```
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_09.exe demo_09_copias.c ../../src/juego/metasprite.c
.\demo_09.exe
```

Extracto de la salida (`capturas/salida_demo_09_copias.txt`):

```
correr, cuadro 0: 8 copias
  copia  0: tira+    0 -> VRAM 0x0000  384 bytes
  ...
    -- VBlank 1: 1536 bytes, unas 9 lineas de DMA
  copia  4: tira+12288 -> VRAM 0x0400  384 bytes
  ...
    -- VBlank 2: 1536 bytes, unas 9 lineas de DMA
  total 3072 bytes = 18 lineas de DMA (redondeado) si fuera de una vez (de las 37 del VBlank)
correr, cuadro 1: 16 copias        ← el bloque 3 cierra su fila de 4: no se junta con el 4
dormido, cuadro 0: 12 copias       ← 4 piezas, 2048 bytes
```

### 4. La lista de DMA

`tarou_sprite.c` no copia nada: **apunta** las copias con
`dma_lista_anadir(origen, vram, bytes, banco, DMA_VMAIN_TILES)`
(`src/juego/dma_lista.h`) y en el VBlank `dma_lista_ejecutar()`, una rutina en
ensamblador (`src/juego/dma_ejecutar.asm`, unidad 15), dispara un DMA por
entrada con unos 60 ciclos de preparación cada uno (→ glosario: Lista de
DMA). El banco de cada tira no lo sabe C: sale de la tabla
`tarou_til_bancos` de `data.asm` (`.db :tarou_reposo_til, ...`; unidad 13).
Tarou apunta **antes** que las columnas del fondo
(`pantalla_nivel1_actualizar`, paso 4): si algo no cupiera, que no sea él.

Resultado medido hoy (CLAUDE.md, ROM FastROM, 2026-10-21): `V5 7` en un frame
normal y `V5 17..20` en los frames en que sube media celda de Tarou. Por
debajo de 37 con margen.

### El primer cuadro, antes del fundido

Al entrar en el nivel la pantalla está apagada (`secuencia_cambiar` llama a
`setScreenOff` antes de cada `_iniciar`). `pantalla_nivel1_iniciar` aprovecha
el forced blank (unidad 04) y sube el primer cuadro entero **antes** del
`fade_in`:

```c
do
{
    dma_lista_vaciar();
    tarou_sprite_dibujar(&tarou, camara);   /* apunta media celda */
    dma_lista_ejecutar();                   /* y la copia ya: pantalla apagada */
} while (!tarou_sprite_al_dia(&tarou));
```

Sin este bucle, durante los 32 frames del fundido se veía lo que hubiera en la
VRAM. `reaparecer()` hace lo mismo con la pantalla apagada.

### `HUNDIR_PIES`

`tarou_sprite_dibujar` dibuja 2 px más abajo que la física
(`y = PF_ENTERO(t->y_fp) + HUNDIR_PIES`): el arte es una vista un poco
elevada y, con los pies en el borde exacto, Tarou parecía flotar. Solo es
dibujo; la física y las pruebas usan el borde exacto (unidad 19).

## Trampas

- **`copias[i]` con una struct de 6 bytes llama a `tcc__mul`.** El 65816 no
  multiplica: tcc816 calcula `i × 6` con una rutina (`tcc__mul`) en **cada
  acceso**. Con 8 bytes, `i × 8` son tres `asl`. `CopiaBloque` tenía 6 bytes y
  el frame que planificaba un cuadro costaba 130 líneas; se le añadió el campo
  `libre` (relleno). Medido en un clon con la receta de PLAN §6.4
  (`capturas/tcc_mul_copiabloque.txt`): con 8 bytes, **0** llamadas en
  `metasprite.asm` y **0** en `tarou_sprite.asm`; sin `libre` (6 bytes),
  **3** y **5**. Es la misma trampa que `guion[i]` (5 bytes) en `guion.c`.
- **Suponer 6 piezas.** Dormido tiene cuadros de 4. Recorrer hasta
  `METASPRITE_FIN` y esconder las entradas sobrantes.
- **El terminador del espejo**, justo detrás de la última pieza, no en la
  posición 6.
- **Los `_meta.inc` definen tablas globales.** Solo los incluye
  `tarou_sprite.c`; `personajes.c` incluye el suyo
  (`tarou_reposo_estatico_meta.inc`). Si dos `.c` incluyen el mismo, el
  enlazador ve la tabla dos veces.
- **Escribir la VRAM fuera del VBlank** (o pasarse de la línea 37) se pierde
  sin aviso. La columna `V` del HUD lo vigila.
- **`?:` con cadenas como argumento variádico**: error interno de tcc816.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| `t_metasprite` en el PC, `METASPRITE_FIN`, `CopiaBloque` con `libre` | `src/juego/metasprite.h` |
| `metasprite_piezas`, `metasprite_espejar`, `metasprite_copias` | `src/juego/metasprite.c` |
| Sus pruebas (6, 3 y 0 piezas, tabla rota, plan de copias) | `tests/prueba_metasprite.c` |
| Las dos celdas (`VRAM_BUFER`, `TILES_POR_BUFER`), `BYTES_POR_VBLANK`, `HUNDIR_PIES`, `esconder` | `src/juego/tarou_sprite.c`, constantes |
| `apuntar_copias` (el tope de 1536 bytes) y `tarou_sprite_dibujar` (paso 1: subida; paso 2: OAM) | `src/juego/tarou_sprite.c` |
| `DmaItem`, `dma_lista_anadir`, `DMA_VMAIN_TILES` | `src/juego/dma_lista.h`, `dma_lista.c` |
| `.rodata_tarou_*` (una sección por tira) y `tarou_til_bancos` | `data.asm` |
| La regla patrón de Tarou (`-s 32 ... -T -X 96 -Y 64 -P 3`) | `Makefile` |
| El bucle `tarou_sprite_al_dia` antes del `fade_in`; el orden Tarou → columnas → HUD | `src/pantallas/pantalla_nivel1.c` (`_iniciar`, `reaparecer`, `_actualizar` paso 4) |
| La versión con el motor del SDK | `git show 8a32523:src/juego/tarou_sprite.c` |
| El fallo de Tarou a trozos y sus números | `docs/diseno-tutorial-movimiento.md` §7 y §8 |

## Ejercicio y criterio de aprobación

El enunciado completo está en `ejercicio_09.md`; la solución, en
`solucion/rom/` (compilada: `capturas/compilacion_solucion.txt`).

1. **Mini-ROM:** añadir una tercera animación, `dormido`, con Abajo
   (TODO 1..7), y mostrar en pantalla cuántas piezas tiene el cuadro.
2. **Sprite Viewer de Mesen:** medir las 6 piezas de 32×32 de un cuadro
   normal y las 4 de los cuadros 0, 1 y 8 de dormido.
3. **Juego real (sin modificarlo):** ver en el Sprite Viewer cómo las piezas
   saltan de los tiles 0.. a los 128.. al cambiar de cuadro, y leer `V` en el
   HUD.
4. **Opcional, en un clon:** quitar `libre` de `CopiaBloque` y contar los
   `tcc__mul`.

**Aprobación:** la solución compila con `No warnings or errors found.`;
capturas de Mesen con Tarou mirando a la derecha y a la izquierda (el
metasprite en ambas direcciones) y dormido con `PIECES 4`; las cifras del
Sprite Viewer anotadas.

## Lo que viene

- **Unidad 11:** qué cuadro toca en cada frame (la tabla de animaciones).
- **Unidad 13:** bancos, `.db :etiqueta` y por qué 10 cuadros por sección.
- **Unidad 15:** `dma_ejecutar.asm` por dentro.
- **Unidad 16:** encontrar el fallo de Tarou a trozos con el depurador.
