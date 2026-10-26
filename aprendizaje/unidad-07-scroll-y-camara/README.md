# Unidad 07 — Scroll, cámara y parallax

**Objetivo:** mover el mundo escribiendo los registros de scroll, con una
cámara que sigue al protagonista y se para en los bordes del nivel, dos capas
a distinta velocidad (parallax) y un nivel más ancho que el mapa de la VRAM.

**Prerrequisitos:** 06 (tiles, tilemaps, VRAM en palabras, `gfx4snes`) y, a
través de ella, 03 y 04 (VBlank, pantalla apagada). **Práctica:** mini-ROM +
PC (`tests/prueba_camara.c` del juego). **Tiempo orientativo:** dos tardes.

En la SNES nada «se mueve» por el escenario: Tarou se queda más o menos en el
centro de la pantalla y lo que cambia es **qué trozo** de cada fondo enseña la
PPU. Esta unidad es ese desplazamiento, la cuenta que lo decide (la cámara)
y el truco con el que un nivel de 2048 px cabe en un mapa de 512.

## Material

| Archivo | Qué es |
|---|---|
| `demo_07_anillo.c` | PC: cámara, scroll de las dos capas y columnas del anillo para varias x |
| `rom/` | mini-ROM: arbustos y montañas con la cruceta; esqueleto con TODO 1..3 |
| `ejercicio_07.md` | A: recorte, parallax y una columna del anillo; B: `camara_para_ancho` y sus pruebas |
| `solucion/` | `parallax.c` (mini-ROM) y la parte B: `camara.c`, `camara.h`, `prueba_camara.c`, `camara_ancho.patch` |
| `capturas/` | salida de la demo, compilaciones y la salida de las pruebas con la solución |

```
gcc -std=c99 -Wall -Wextra -o demo_07.exe demo_07_anillo.c
.\demo_07.exe
```

## Conceptos

### Registro de scroll

Cada fondo tiene dos **registros de scroll**: el horizontal (`BGnHOFS`) y el
vertical (`BGnVOFS`). Valen «cuántos píxeles del mapa quedan a la izquierda
(o arriba) del borde de la pantalla». Con `HOFS = 100`, la columna 0 de la
pantalla enseña la columna 100 del mapa. En Mode 1, los del fondo 0 son
`$210D`/`$210E` y los del fondo 1, `$210F`/`$2110` (→ glosario: BG / capas).

Son **registros de doble escritura** (→ glosario: Registro de doble
escritura): reciben un valor de 10 bits en dos escrituras seguidas de 8 al
mismo registro, primero el byte bajo y después el alto. `bgSetScroll` lo hace
por nosotros; desensamblada (bytes de la ROM del juego, en `$8E:D184`) hace
exactamente eso:

```
sta $210D,y      ; byte bajo de x   (y = número de fondo × 2)
xba              ; cambia los bytes de A
sta $210D,y      ; byte alto de x
...              ; lo mismo con la y en $210E,y
```

Consecuencia que importa: **`bgSetScroll` escribe el registro en el acto**,
sin esperar al VBlank. La PPU usa el valor nuevo desde la línea siguiente;
si se llama a mitad del dibujo, la parte de arriba de la pantalla sale con
el scroll viejo. En la mini-ROM se llama justo después de `WaitForVBlank()`
(ver «Dónde está en el juego» para lo que hace el nivel).

### Mapas de 64×32 y la vuelta a los 512 px

Un mapa de 32×32 mide 256 px de ancho: tanto como la pantalla. Para
desplazarse hace falta más mapa, y la PPU admite 64×32 (`SC_64x32`): **dos
páginas de 32×32 seguidas** en la VRAM, la izquierda en la dirección base y
la derecha 0x400 palabras después (2 KB). No es un mapa de 64 columnas por
filas: la fila 0 de la página derecha está en `base + 0x400`, no en
`base + 32`. Por eso `gfx4snes` necesita **`-y`** para estas capas (escribe el
mapa por páginas, con el alto múltiplo de 256: por eso
`scripts/preparar_fondo_nivel.py` rellena la imagen hasta 256 de alto aunque
la pantalla mida 224).

El scroll **da la vuelta**: la PPU toma el valor del registro módulo el ancho
del mapa. Con 64×32, la columna 512 del «mundo» es otra vez la columna 0 del
mapa. Por eso el título puede sumar 1 a `scroll_cerca` para siempre: el valor
es un `u16` que crece y los arbustos se repiten cada 512 px.

### Bucle sin costura (original + espejo)

Para que la vuelta no se note, el píxel 511 de la capa tiene que «pegar» con
el píxel 0. El truco del proyecto: dibujar 256 px y añadir a la derecha su
**espejo** horizontal. El borde derecho del espejo es el borde izquierdo del
original: siempre casan. Lo hace `scripts/preparar_fondo_nivel.py` con las
montañas (`ImageOps.mirror`, y además borra el logo), y los arbustos del
título están hechos igual. La capa lejana del nivel mide 512 px y se repite
cuatro veces a lo largo de los 2048 del nivel.

### Cámara

La **cámara** es cuánto se ha desplazado el nivel respecto a la pantalla:
la x del mundo que cae en la columna 0 de la pantalla (→ glosario: Cámara).
Con ella, todo se reduce a una resta: **pantalla = mundo − cámara**. Así
dibuja `tarou_sprite_dibujar` a Tarou (`src/juego/tarou_sprite.c:204`).

La regla del juego (`src/juego/camara.c`): el centro de la celda de Tarou
(su x + 48, media celda de 96) en el centro de la pantalla (128):

```
camara = x + 48 − 128, recortada a 0 .. ANCHO_NIVEL − ANCHO_PANTALLA = 0 .. 1792
```

**Recorte** (*clamp*): si un valor se sale de un intervalo, se deja en el
extremo. Sin el recorte, en el borde izquierdo la cámara enseñaría lo que hay
«antes» del nivel. Y se calcula en **`s16`**, porque cerca del borde la
cuenta da negativa. Paso 1 de la demo:

```
1. x = 10: en s16, 10 + 48 - 128 = -70; en u16 = 65466 (por eso camara_para usa s16)
```

Un `u16` no tiene negativos (unidad 01): −70 «da la vuelta» a 65466, que es
mayor que 1792, y el recorte lo dejaría en el **borde derecho**. La tabla de
la demo enseña los dos topes: con `x` de 0 a 80 la cámara vale 0, en 81 vale
1, y desde 1872 se queda en 1792.

### Parallax

**Parallax** (→ glosario: Parallax): capas que se desplazan a velocidades
distintas dan sensación de profundidad, porque lo lejano parece moverse
menos. En el nivel:

```c
bgSetScroll(0, camara, 0);        /* capa cercana: 1:1, es el nivel        */
bgSetScroll(1, camara >> 1, 0);   /* capa lejana: la mitad                 */
```

`>> 1` es dividir entre 2 sin división (unidad 01: `/` llama a una rutina
lenta). Cualquier relación que sea suma de potencias de 2 sale igual de
barata: 1/4 es `>> 2`; 3/4, `(c >> 1) + (c >> 2)`.

### El mapa-anillo

El nivel mide 2048 px = 256 columnas de tiles; el mapa de la VRAM, 64. No
cabe. La solución (→ glosario: Streaming de columnas): usar el mapa de 64×32
como un **anillo**. La columna `c` del nivel va al sitio `c mod 64` del mapa,
y como el scroll también da la vuelta a los 512 px, lo que la PPU enseña en
la posición `camara` coincide con lo que hay en el nivel en esa misma x. Solo
hay que escribir cada columna **antes** de que entre en pantalla, encima de
una que ya salió.

`nivel_columna_vram` (`src/juego/nivel.c`) convierte la columna en palabras
desde el principio del mapa, sin dividir:

```c
return (u16)(((col & 32) << 5) | (col & 31));
```

- `col & 31`: la columna dentro de su página (0..31);
- `col & 32`: 0 o 32 según la página; `<< 5` lo convierte en 0 o 0x400;
- los bits por encima de 32 (64, 128) se pierden: es el «mod 64».

La demo lo comprueba (paso 3: las columnas 4, 68, 132 y 196 caen en
`+0x004`). Cada frame, `fondo_nivel_actualizar` mira si la cámara entró en
una columna nueva (`camara >> 3` cambió) y apunta dos: la siguiente a la
última visible (`col + 33`) y la anterior a la primera (`col − 1`). La
pantalla enseña 32 columnas, o 33 cuando el scroll no es múltiplo de 8.
Como la cámara avanza como mucho 2 px por frame, nunca se salta una. Las
columnas fuera del nivel (−1, que en `u16` es 65535, o 256) no se apuntan. Lo
ves en la tabla de la demo:

```
      x  camara  cerca(&511)  lejos=cam/2(&511)  col | columnas que entran (anillo)
    432    352    352           176  (176)        44 |  der  77 -> +0x00D   izq  43 -> +0x40B
   1500   1420    396           710  (198)       177 |  der 210 -> +0x012   izq 176 -> +0x410
```

### Escribir una columna: `VMAIN = 0x81`

En la ROM el mapa del nivel está guardado **columna a columna**
(`scripts/mapa_a_columnas.py`): las 32 entradas de una columna son 64 bytes
seguidos. En la VRAM, en cambio, dos filas seguidas de una columna están a
32 palabras (una fila del mapa). La PPU tiene un registro para eso: **VMAIN**
(`$2115`, `REG_VMAIN` en `video.h`) dice cuánto avanza la dirección de la
VRAM tras cada escritura. Con `0x80` avanza 1 palabra (lo normal: tiles,
filas); con **`0x81` avanza 32**: cada palabra escrita baja una fila.

El TODO 3 de la mini-ROM lo hace con la CPU, desde C, con las macros de
PVSnesLib:

```c
REG_VMAIN = 0x81;                                /* +32 palabras tras cada escritura */
REG_VMADDLH = VRAM_MAPA_A + columna_vram(col);   /* fila 0 de esa columna            */
for (i = 0; i < 32; i++) REG_VMDATALH = ENTRADA_RAYA;
REG_VMAIN = 0x80;                                /* como lo espera el SDK            */
```

## La librería: PVSnesLib

| Función (cabecera) | Qué hace | Hardware |
|---|---|---|
| `void bgSetScroll(u8 bgNumber, u16 x, u16 y);` ([background.h](https://alekmaul.github.io/pvsneslib/background_8h.html)) | escribe el scroll horizontal y vertical de un fondo, **en el acto** | `BGnHOFS`/`BGnVOFS` `$210D-$2114`, dos escrituras de 8 bits cada uno |
| `void bgInitMapSet(u8 bgNumber, u8 *mapSource, u16 mapSize, u8 sizeMode, u16 address);` (background.h) | con `SC_64x32` copia los 4 KB de las dos páginas y declara el mapa de 64×32 | DMA a `VMDATA`; `BGnSC` (bits de tamaño) |
| `void bgSetMapPtr(u8 bgNumber, u16 address, u8 mapSize);` (background.h) | declara base y tamaño sin copiar nada: lo que usa el nivel, que copia columnas sueltas | `BGnSC $2107-$2109` |
| `void setParallaxScrolling(u8 bgrnd);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | scroll distinto por franjas de líneas con HDMA (tabla `HDMATable16`); no lo usa el juego | HDMA |
| `padsCurrent(value)` / `padsDown(value)` ([input.h](https://alekmaul.github.io/pvsneslib/input_8h.html)) | **macros** (`pad_keys[value]`, `pad_keysdown[value]`): mantenidos y recién pulsados (unidad 05) | los lee la NMI del SDK |
| `REG_VMAIN`, `REG_VMADDLH`, `REG_VMDATALH` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | macros de registro: modo de avance, dirección y dato de la VRAM | `$2115`, `$2116`, `$2118` |

Ejemplos del SDK: `tools/pvsneslib/snes-examples/graphics/Backgrounds/Mode1Scroll`
(un mapa de 64×64 movido con la cruceta y `bgSetScroll`) y
`graphics/Effects/ParallaxScrolling` (parallax por franjas con HDMA). La
mini-ROM parte del primero y hace el parallax con dos capas, como el juego.

**Por qué `fondo_nivel.c` no usa PVSnesLib para las columnas.** No hay
ninguna función del SDK que copie a la VRAM con un avance de 32:
`dmaCopyVram` pone siempre `VMAIN = 0x80`. Así que el juego baja al
registro, pero no escribe `REG_VMAIN` desde `fondo_nivel.c`: apunta cada
columna en la **lista de DMA** (`dma_lista_anadir(origen, destino, 64,
banco, DMA_VMAIN_COLUMNA)`, con `DMA_VMAIN_COLUMNA = 0x81` en
`src/juego/dma_lista.h`) y en el VBlank `src/juego/dma_ejecutar.asm` pone
`VMAIN`, la dirección y el canal 0 de DMA para cada entrada y deja `VMAIN`
en 0x80 al acabar. Dos motivos, los dos medidos:

1. **Velocidad.** Un DMA de 64 bytes es mucho más rápido que 32 escrituras de
   la CPU desde C (las de la mini-ROM sirven para aprender, no para cada
   frame).
2. **El presupuesto del VBlank.** Hasta el 2026-10-20 `fondo_nivel.c`
   preparaba y disparaba sus DMA en C dentro del VBlank: dos columnas
   costaban 9 de las 37 líneas, y junto a los tiles de Tarou se salía del
   VBlank (cabecera de `src/juego/fondo_nivel.c`). Con la lista, el C solo
   apunta durante el frame y la rutina en ensamblador dispara todo de un
   tirón (unidades 09, 15 y 20).

## La mini-ROM (`rom/`)

- `res/cerca.png` = `res/titulo/fondo_cerca.png` (arbustos, 512×256, 294
  tiles); `res/lejos.png` = `res/nivel1/fondo_lejos.png` (montañas sin logo,
  512×256, 354 tiles); `res/fuente2bpp.bmp` = la fuente del título.
- Las dos capas con `-y` y `SC_64x32`, en las direcciones del juego
  (`VRAM_MAPA_A` 0x6C00, `VRAM_MAPA_B` 0x7400); el texto en el fondo 2 (BG3)
  con `BG3_MODE1_PRIORITY_HIGH`, como el título; el cielo, un color liso en la
  entrada 0 de la CGRAM (en el juego es un HDMA, abajo).
- La raya roja: un tile macizo propio copiado al hueco 511 de
  `VRAM_FONDO_A` con `dmaCopyVram`, con la paleta 3 (libre).
- El texto que cambia (`CAM`, `NEAR`/`FAR`, `COL`/`RING`) no va con
  `consoleDrawText`: se escribe a mano en un tilemap en la WRAM, una fila por
  frame, con los números hechos por restas, y se sube por DMA (64 bytes) en el
  VBlank, como `src/juego/hud_debug.c`. `RING` es la columna del anillo
  (0..63) y `+n`, la palabra de su fila 0 dentro del mapa: 0..31 en la
  página izquierda, 1024..1055 (0x400 + c − 32) en la derecha; por eso la
  columna 42 del anillo sale como `+1034`.
- El bucle: lógica (mando, cámara, preparar una fila de texto) →
  `WaitForVBlank()` → scroll, la fila de texto y la columna, al principio del
  VBlank.

Compilación del esqueleto y de la solución: `No warnings or errors found.`
(`capturas/compilacion_rom.txt`, `capturas/compilacion_solucion.txt`).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| La regla de la cámara, en `s16`, con los dos recortes | `camara_para`, `src/juego/camara.c:26-40` |
| Sus pruebas: 0, 80, 81, 200, 1872, 1952; y las del anillo | `tests/prueba_camara.c` |
| `ANCHO_NIVEL` 2048, `COLUMNAS_NIVEL` 256, `ANCHO_CELDA` 96, `SUELO_Y` 176 | `src/juego/nivel.h` |
| `nivel_columna_vram` | `src/juego/nivel.c:22-28` |
| Paso 3 del frame: cámara y las dos `bgSetScroll` | `src/pantallas/pantalla_nivel1.c:219-222` |
| Lo mismo al entrar y al reaparecer (con la pantalla apagada) | `pantalla_nivel1.c:141-142` y `:170-171` |
| El título: los arbustos avanzan 1 px por frame, sin cámara | `pantalla_titulo.c:113-116` |
| Qué columnas se escriben y cuándo: `col + 33` y `col − 1` | `fondo_nivel_actualizar`, `src/juego/fondo_nivel.c:128-151` |
| Las 64 columnas de golpe, con la pantalla apagada (15 antes, 49 después) | `fondo_nivel_rellenar`, `fondo_nivel.c:92-116` |
| Una columna a la lista de DMA, con su banco y `DMA_VMAIN_COLUMNA` | `apuntar_columna`, `fondo_nivel.c:75-90` |
| Por qué 512 px y 256 de alto (`-y`) | `scripts/preparar_fondo_nivel.py` (`preparar`, `--alto`) |
| Pantalla = mundo − cámara (el sprite) | `src/juego/tarou_sprite.c:204` |

**El cielo es un truco de HDMA.** Ninguna capa pinta el cielo: es el color
de fondo (entrada 0 de la CGRAM, unidad 06), y un canal de HDMA lo cambia en
cada línea (`setModeHdmaColor(HDMA_CHANNEL6, ...)` en `pantalla_nivel1.c` y
`pantalla_titulo.c`). Por eso el degradado no se desplaza con la cámara: no
depende del scroll, solo de la línea. Cómo funciona, en la unidad 12.

**Lo que encontró esta unidad (corregido el 2026-10-22):** hasta ese día, el
paso 3 del nivel llamaba a `bgSetScroll` después de la lógica, a mitad del
dibujo (la fila `LIN` del HUD dice en qué línea: 70–120 en el tutorial). Como
`bgSetScroll` escribe en el acto, las líneas de arriba de ese punto se
dibujaban con el scroll del frame anterior y las de abajo con el nuevo,
mientras las columnas que entran y la OAM de Tarou no llegaban hasta el
VBlank: en teoría, un corte de 1–2 px en la capa cercana mientras Tarou
camina. Ahora el paso 3 solo calcula `camara` y las dos llamadas a
`bgSetScroll` están en `pantalla_nivel1_vblank`, antes de
`dma_lista_ejecutar`: scroll, columnas y sprite cambian a la vez. El Event
Viewer de Mesen (unidad 16) enseña en qué línea se escribe `$210D`: antes,
a mitad de la pantalla; ahora, dentro del VBlank.

## Trampas

- **La cámara se calcula en `s16`.** En `u16`, la resta cerca del borde
  izquierdo da 65466 y el recorte manda la cámara al final del nivel.
- **`PF_ENTERO` solo con valores ≥ 0** (unidad 01): la cámara se calcula con
  la x de la celda, que nunca es negativa.
- **Un mapa de 64×32 son dos páginas**, no filas de 64: sin `-y`, el mapa sale
  por filas y la mitad derecha aparece desordenada. La columna `c` de la
  página derecha está en `0x400 + c − 32`.
- **`bgSetScroll` no espera al VBlank** (arriba).
- **`consoleDrawText` con `%d` en cada frame frena el bucle.** Medido en la
  primera versión de la mini-ROM (Mesen, captura con guion): con tres
  líneas y cinco números por frame, cada vuelta costaba ~3 frames; la cámara
  de 2 px por vuelta iba a ~40 px/s en vez de 120 y la NMI copiaba el texto a
  medio escribir (filas que no casaban entre sí). Para texto que cambia cada
  frame: números por restas y el tilemap a mano (`hud_debug.c`).
- **Un número que se acorta deja cifras viejas** si solo se escribe encima:
  hay que borrar la fila (o rellenar con espacios) antes. `preparar_fila`
  hace `memset` de la fila entera cada vez.
- **Una línea de texto de 32 letras empezando en x = 1 no cabe**: la consola
  sigue en la fila de abajo. El mapa tiene 32 columnas (0..31).
- **`VMAIN` hay que dejarlo en 0x80.** Las funciones del SDK
  (`dmaCopyVram`, la consola) lo dan por supuesto; `dma_ejecutar.asm` lo
  restaura al acabar.
- **No escribir una columna lejana en el anillo.** Dos columnas separadas 64
  comparten sitio: si se escribe la que no toca, se estropea la que se ve.
  `fondo_nivel_actualizar` solo reescribe las del tronco que están a la vista
  (`(u16)(parche_col + i + 1 - col) <= 34`).
- **Los pies 4 px dentro de la hierba son a propósito.** La física usa el
  borde exacto de las celdas (`SUELO_Y` = 176 es la fila del suelo base), pero
  el arte de Pixellab es una vista lateral un poco elevada: con los pies justo
  en el borde, Tarou parecía flotar. Se hunde en dos mitades: **2 px los pone
  el arte** (la banda de hierba de la capa cercana empieza 2 px por encima
  del borde de la celda) y **2 px el dibujo** (`HUNDIR_PIES` en
  `src/juego/tarou_sprite.c:93`, sumado a la y en la línea 205). Es solo
  dibujo: no hay scroll vertical y la cámara ni la colisión lo ven. Si ves a
  Tarou «metido» en el suelo, no lo corrijas en la cámara ni en la física
  (unidades 19 y 20; `docs/diseno-tutorial-movimiento.md` §7).

## Ejercicio y criterio de aprobación

Enunciado completo en `ejercicio_07.md`:

- **A.** Mini-ROM: recortar la cámara (TODO 1), la capa lejana a la mitad
  (TODO 2) y pintar una columna del anillo con `VMAIN = 0x81` (TODO 3).
- **B.** Juego real, en un clon: `camara_para_ancho(x, ancho_nivel)`, con
  `camara_para` como una llamada a ella, y comprobaciones nuevas en
  `tests/prueba_camara.c` para niveles de 1024, 512 y 256 px.

**Aprobación:** captura de la mini-ROM con las capas desplazadas, la raya y
el tope de 1792 (comando con guion en el enunciado), y `tests/pruebas.exe` en
verde con las pruebas nuevas. Con la solución: `375 pruebas, 0 fallos`
(`capturas/salida_pruebas_solucion.txt`; sin ella, 367) y la ROM del juego
compilando sin avisos tras `make clean`
(`capturas/compilacion_juego_solucion.txt`).

## Lo que viene

- **Unidad 08:** los sprites, que no se desplazan con el scroll: se colocan
  en coordenadas de pantalla (mundo − cámara).
- **Unidad 12:** el HDMA del cielo y el scroll distinto por líneas.
- **Unidad 20:** la copia de columnas por DMA con la lista, el parche del
  tronco y la colisión con el mismo nivel.
