# Glosario SNES y C para leer el código del proyecto

Los comentarios del código dicen `→ glosario: TÉRMINO` cuando usan un concepto
que se explica aquí una sola vez. Está ordenado de lo más general a lo más
específico; cada entrada dice **qué es**, **por qué importa en este proyecto**
y, cuando aplica, **qué función de PVSnesLib lo toca**.

## Hardware

- **SNES / Super Nintendo** — consola de 16 bits (1990). Tres chips que
  importan: la **CPU** (65816, ejecuta nuestro C compilado), la **PPU**
  (dibuja la imagen) y el **SPC700** (reproduce el sonido, con su propio
  programa y su propia RAM de 64 KB).
- **Frame** — una imagen completa. En NTSC la SNES dibuja **60 frames por
  segundo**, así que "3 segundos" en el código son `3 * 60 = 180` frames. Todo
  el tiempo del juego se cuenta en frames, no en segundos.
- **VBlank (vertical blank) / NMI** — el instante, al final de cada frame, en
  que la PPU terminó de dibujar y "descansa" antes del siguiente. Es la
  **única** ventana segura para escribir en la memoria de vídeo sin
  romper la imagen. La PPU avisa con una interrupción llamada NMI.
  `WaitForVBlank()` detiene el programa hasta ese instante: por eso el bucle
  principal la llama una vez por vuelta, y una vuelta del bucle = un frame.
  El mando también se lee ahí: si no se llama a `WaitForVBlank()`, el mando
  "no responde".
- **VRAM** — la memoria de vídeo (64 KB). Guarda **tiles** y **tilemaps** de
  fondo, y los gráficos de los sprites. La CPU no dibuja píxeles: copia datos
  a VRAM y la PPU los muestra. Direcciones como `0x3000` o `0x6800` en el
  código son posiciones dentro de la VRAM (en hexadecimal).
- **CGRAM** — la memoria de **paletas** (256 colores de 15 bits, 5 bits por
  componente RGB). `RGB8(r,g,b)` convierte un color de 0-255 al formato SNES.
- **OAM** — la tabla de **sprites** (128 entradas: posición, tile, paleta,
  prioridad). `oamSet(...)` escribe una entrada; la librería la copia a la PPU
  en cada VBlank.
- **DMA / HDMA** — copia rápida de bloques de memoria hecha por hardware.
  **DMA** (`dmaCopyVram`) copia tiles/mapas a VRAM durante VBlank mucho más
  rápido que la CPU. **HDMA** escribe un valor en un registro de la PPU **en
  cada línea de la pantalla**: así se hace el cielo con degradado
  (`setModeHdmaGradient`) sin gastar CPU.
- **Banco / LoROM** — la ROM del cartucho se ve en trozos de **32 KB** llamados
  bancos. Nuestro proyecto tiene 8 (256 KB). Un asset (imagen, música) no puede
  cruzar de un banco a otro; `data.asm` reparte cada uno en una sección
  `superfree` y el enlazador elige el banco.

## Gráficos

- **Tile** — cuadradito de **8×8 píxeles**. Todo fondo y sprite se compone de
  tiles. Un tile de 16 colores ocupa 32 bytes ("4bpp": 4 bits por píxel).
- **Tilemap (mapa)** — cuadrícula que dice qué tile va en cada celda de la
  pantalla (32×32 celdas = 256×256 px) y con qué paleta. Cambiar un fondo es
  cargar tiles + tilemap (`bgInitTileSet` + `bgInitMapSet`).
- **Paleta** — grupo de colores (16 en Mode 1) que un tile o sprite usa.
  Hay 8 paletas para fondos y 8 para sprites. El **color 0** de cada paleta
  es transparente en sprites y en las capas superiores.
- **BG (background) / capas** — la PPU dibuja hasta 4 fondos superpuestos
  (BG1 delante... BG4 detrás), cada uno con su tilemap y su **scroll**
  (`bgSetScroll`). En PVSnesLib se numeran **desde 0**: `bgSetDisable(1)`
  apaga el segundo fondo.
- **Mode 1** — modo de vídeo base del proyecto: BG1 y BG2 de 16 colores, BG3
  de 4 colores (ideal para texto). `setMode(BG_MODE1, 0)`.
- **Mode 7** — modo especial: una sola capa de 256 colores que la PPU puede
  **rotar y escalar** en tiempo real. Se usa en el logo de desarrolladores.
  `setMode7`, `setMode7Rot`, variables `m7sx`/`m7sy` para el zoom.
- **Sprite** — imagen móvil independiente del fondo (el personaje). Tamaños
  8×8 a 64×64. Límite: 128 en pantalla, 32 por línea horizontal.
- **Fade (fundido) / brillo** — la PPU tiene un control de brillo de 0 (negro)
  a 15 (normal). `setBrightness(n)` lo fija; `setFadeEffectEx(FADE_IN, f)` lo
  anima en `f` pasos. **Ojo:** estas funciones bloquean el programa mientras
  dura el efecto.
- **Mosaico** — efecto de la PPU que agranda los píxeles de una capa hasta
  volverla bloques. `setMosaicEffect(MOSAIC_IN, MOSAIC_BG1)`.
- **Consola de texto** — PVSnesLib trae una fuente y una capa de texto de
  32 columnas × 28 filas (celdas de 8×8). `consoleDrawText(x, y, "...")`
  escribe en la celda (x, y); la librería copia el texto a VRAM en el
  siguiente VBlank.

## Entrada y sonido

- **Mando (pad)** — `padsCurrent(0)` devuelve los botones **mantenidos** del
  mando 1 como bits; `padsDown(0)` solo los **recién pulsados** en este frame.
  Se prueba un botón con `&`: `if (padsDown(0) & KEY_START)`.
- **Soundbank / módulo .it** — la música se compone en un tracker (formato
  Impulse Tracker) y `smconv` la empaqueta en un "soundbank" dentro de la ROM.
  `spcBoot()` arranca el chip de sonido; `spcLoad(MOD_X)` carga una canción;
  `spcPlay(0)` la reproduce; `spcProcess()` debe llamarse cada frame.

## C (lo que aparece en este proyecto)

- **`u8`, `u16`** — enteros sin signo de 8 bits (0-255) y 16 bits (0-65535),
  definidos por PVSnesLib. Se prefieren a `int` porque la CPU es de 16 bits y
  cada byte de RAM cuenta (hay 128 KB).
- **`#include`** — pega el contenido de otro archivo. `<snes.h>` es la
  librería; `"comun.h"` es nuestro.
- **`#define`** — constante o macro que el preprocesador reemplaza antes de
  compilar. `#define FPS 60` → cada `FPS` pasa a ser `60`.
- **`#ifndef` / `#define` / `#endif` en un `.h`** — "guarda de inclusión":
  evita que el mismo header se pegue dos veces en un archivo.
- **`enum`** — lista de nombres con valor entero (0, 1, 2...). Nuestro
  `enum Pantalla` da nombre a cada estado de la secuencia.
- **`switch` / `case`** — elige un bloque según el valor de una variable; es
  el corazón de la **máquina de estados** (`secuencia.c`).
- **`extern`** — "esta variable/función existe en otro archivo". Así las
  pantallas leen `frames_en_pantalla`, que vive en `secuencia.c`.
- **`static`** (en variable global) — la variable solo se ve dentro de su
  archivo `.c`.
- **`void`** — "sin valor": una función `void` no devuelve nada; `f(void)` no
  recibe parámetros.
- **Máquina de estados** — patrón: una variable dice en qué estado estamos; en
  cada frame se ejecuta solo el código de ese estado; al terminar se cambia la
  variable. Aquí los estados son las pantallas.
