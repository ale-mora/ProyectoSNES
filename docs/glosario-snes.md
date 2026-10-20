# Glosario SNES y C para leer el código del proyecto

Los comentarios del código dicen `→ glosario: TÉRMINO` cuando usan un concepto
que se explica aquí una sola vez. Está ordenado de lo más general a lo más
específico; cada entrada dice **qué es**, **por qué importa en este proyecto**
y, cuando aplica, **qué función de PVSnesLib lo toca**.

## Vocabulario del proyecto

Términos acordados el 2026-10-16 (Revisión 1 del MVP). Un concepto, una
palabra: si un documento o comentario usa otra, es un error a corregir.

- **Etapa** — fase del plan de desarrollo (`docs/plan-desarrollo.md`, Etapa 0
  a 6). Nunca una zona del juego ni una fase de vida del personaje.
- **Revisión** — iteración numerada del producto mínimo viable (MVP).
  Revisión 1: Tarou adulto camina, corre, salta, se agacha, usa la espada y
  duerme en el nivel 1. Revisión 2: Bárbara.
- **Nivel** — zona jugable con principio y fin (`nivel.c`, `PANTALLA_NIVEL1`).
  El nivel 1 mide 512 px de ancho.
- **Escenario** — lo que se ve y se pisa dentro de un nivel: las capas de fondo
  y la geometría del suelo. Es una parte del nivel, no un sinónimo.
- **Pantalla** — un estado de la máquina de estados de `secuencia.c`
  (`PANTALLA_TITULO`, `PANTALLA_NIVEL1`...). El ancho físico de 256 px se llama
  «ancho de pantalla» (`ANCHO_PANTALLA`), nunca «una pantalla».
- **Protagonista** — el personaje que el jugador controla en este momento. Hay
  dos: **Tarou** (perro samurái, Revisión 1) y **Bárbara** (mujer samurái,
  Revisión 2). Cada uno tiene su archivo (`tarou.c`, `barbara.c`).
- **Jugador** — la persona que sostiene el mando. Nunca el personaje.
- **Edad** — fase de vida de Tarou: cachorro, adulto, viejo. La Revisión 1 usa
  solo el adulto.
- **Enemigo** — entidad que daña al protagonista. Los sprites del lobo samurái
  y de la samurái que hoy salen en la secuencia de arranque serán enemigos
  cuando toque (Etapa 3).
- **Estado** (del protagonista) — qué está haciendo ahora, una sola cosa a la
  vez: parado, caminando, corriendo, saltando, cayendo, agachado, agachado
  caminando, desenvainando, envainando, atacando, echándose, dormido,
  despertando. Es una variable; cambia según el mando y la física.
- **Espada** — si la espada está envainada o desenvainada. Persiste entre
  estados (se puede correr con ella fuera). Solo se ataca con ella fuera.
- **Dirección** — hacia dónde mira el protagonista: izquierda o derecha.
  Decide el espejo horizontal (`hflip`) del sprite.
- **Stub** — archivo que compila pero no hace nada: reserva el sitio de un
  subsistema futuro y documenta en su cabecera el contrato previsto y la Etapa
  que lo construirá («PENDIENTE: Etapa N»).

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
  (`setModeHdmaColor` con una tabla de un color por línea) sin gastar CPU.
- **Línea del haz (contadores H/V)** — la PPU dibuja la imagen de arriba
  abajo, una línea cada 63,5 µs (262 líneas por frame en NTSC; las 225..261
  son el VBlank). Leer el registro `$2137` congela los contadores y `$213D`
  devuelve en qué línea va el haz. El HUD de depuración lo usa como medidor de
  coste: si la lógica termina en la línea L, usó (37 + L) / 262 del frame.
- **`snes_vblank_count`** — contador de VBlanks del SDK (sube 1 en cada NMI).
  Comparado con `frames_en_pantalla` dice si el juego pierde frames; comparado
  con el reloj, si el emulador va a 60 por segundo.
- **Banco / LoROM** — la ROM del cartucho se ve en trozos de **32 KB** llamados
  bancos. Nuestro proyecto tiene 16 (512 KB). Un asset (imagen, música) no puede
  cruzar de un banco a otro; `data.asm` reparte cada uno en una sección
  `superfree` y el enlazador elige el banco.

## Gráficos

- **Tile** — cuadradito de **8×8 píxeles**. Todo fondo y sprite se compone de
  tiles. Un tile de 16 colores ocupa 32 bytes ("4bpp": 4 bits por píxel).
- **Tilemap (mapa)** — cuadrícula que dice qué tile va en cada celda de la
  pantalla (32×32 celdas = 256×256 px) y con qué paleta. Cambiar un fondo es
  cargar tiles + tilemap (`bgInitTileSet` + `bgInitMapSet`).
- **Tileset** — colección de tiles pensada para combinarse (bordes, rellenos,
  esquinas). El suelo del nivel sale de un tileset *Sidescroller* de Pixellab:
  tiles de **borde** (hierba sobre tierra, con las filas de arriba vacías) y
  de **relleno** (tierra opaca), de 16×16. → `scripts/preparar_suelo.py`.
- **Volteo de tile** — cada celda del tilemap puede mostrar su tile espejado
  en horizontal o en vertical sin gastar tiles nuevos: así se varía la tierra
  del suelo para que no se vean franjas.
- **Paleta** — grupo de colores (16 en Mode 1) que un tile o sprite usa.
  Hay 8 paletas para fondos y 8 para sprites. El **color 0** de cada paleta
  es transparente en sprites y en las capas superiores.
- **BG (background) / capas** — la PPU dibuja hasta 4 fondos superpuestos
  (BG1 delante... BG4 detrás), cada uno con su tilemap y su **scroll**
  (`bgSetScroll`). En PVSnesLib se numeran **desde 0**: `bgSetDisable(1)`
  apaga el segundo fondo.
- **Mode 1** — modo de vídeo base del proyecto: BG1 y BG2 de 16 colores, BG3
  de 4 colores (ideal para texto). `setMode(BG_MODE1, 0)`.
- **Mode 3** — un fondo de 256 colores (BG1) + uno de 16 (BG2). Se usa en las
  escenas fijas de la intro. Cada tile ocupa 64 bytes: una pantalla llena no
  cabe en un banco de 32 KB, por eso las escenas miden 256×120.
- **Mode 7** — modo especial: una sola capa de 256 colores que la PPU puede
  **rotar y escalar** en tiempo real con una matriz de 4 valores (A, B, C, D en
  formato 8.8) y un centro (M7X, M7Y). Se probó para el logo del estudio y se
  descartó en la demo (ver spec §10).
- **Punto fijo (formatos 8.8 y 12.4)** — un entero que guarda una fracción:
  se reservan algunos bits bajos para los «decimales». La SNES no tiene
  decimales (no hay unidad de coma flotante), así que es la única forma de
  tener velocidades como 1,5 px por frame. **8.8**: byte alto entero, byte bajo
  fracción (0x0100 = 1.0, 0x0080 = 0.5); lo usan las matrices de Mode 7.
  **12.4**: 12 bits de entero y 4 de fracción (1/16 px) en un `s16`; lo usan la
  posición y la velocidad del protagonista, porque 12 bits llegan a 2047 px y
  todo sigue siendo aritmética de 16 bits. El píxel se obtiene desplazando:
  `x >> 4`.
- **Registro de doble escritura** — varios registros de la PPU (M7A..D, M7X,
  M7Y, scroll) reciben 16 bits en dos escrituras seguidas de 8: primero el byte
  bajo, luego el alto.
- **Sprite** — imagen móvil independiente del fondo (el personaje). Tamaños
  8×8 a 64×64. Límite: 128 en pantalla, 32 por línea horizontal, y **512 tiles
  de gráficos** en total (un sprite de 64×64 gasta 64), en dos tablas de 256:
  un sprite grande no puede cruzar de una a otra (al bajar de fila el número
  de tile da la vuelta dentro de su tabla). La coordenada y "da la vuelta"
  cada 256 px: un sprite de 64 en y = 240 asoma por arriba.
- **Metasprite** — sprite grande compuesto por varios sprites de hardware que
  se mueven como uno solo. Tarou, en su celda de 96×64, son hasta 6 sprites de 32×32. PVSnesLib
  genera la tabla de piezas con `gfx4snes -T` y lo dibuja con
  `oamMetaDrawDyn32`.
- **Sprite dinámico** — técnica en la que todos los cuadros de animación viven
  en la ROM y, en cada VBlank, solo se copia a VRAM el cuadro que se va a
  mostrar. Así el límite de 512 tiles deja de importar. PVSnesLib:
  `oamInitDynamicSprite`, `oamVramQueueUpdate`, `oamInitDynamicSpriteEndFrame`.
- **Fade (fundido) / brillo** — la PPU tiene un control de brillo de 0 (negro)
  a 15 (normal). `setBrightness(n)` lo fija; `setFadeEffectEx(FADE_IN, f)` lo
  anima en `f` pasos. **Ojo:** estas funciones bloquean el programa mientras
  dura el efecto.
- **Mosaico** — efecto de la PPU que agranda los píxeles de una capa hasta
  volverla bloques. `setMosaicEffect(MOSAIC_OUT, ...)` lo agranda (la doc del
  SDK lo tiene al revés) y `REG_MOSAIC = 0` lo apaga.
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

## Mecánicas del juego (Revisión 1)

- **Velocidad, aceleración y frenado** — en píxeles por frame, en punto fijo.
  Cada frame: si hay una dirección pulsada, `velocidad += aceleración` hasta
  un máximo; si no, la velocidad se acerca a 0 restando el frenado. Después
  `posición += velocidad`.
- **Gravedad** — aceleración vertical constante que se suma a la velocidad
  vertical en cada frame mientras el protagonista está en el aire. La
  velocidad de caída se limita a una **velocidad terminal**.
- **Salto de altura variable** — al pulsar B se asigna una velocidad vertical
  hacia arriba; si se suelta B mientras todavía sube, esa velocidad se
  recorta y el salto queda más corto. Mantener B da el salto completo.
- **Suelo** — altura `y` del nivel sobre la que se apoyan los pies. En la
  Revisión 1 es plano: `nivel_suelo_en(x)` devuelve una constante. Cuando haya
  desniveles solo cambia el interior de esa función.
- **Hundir los pies (perspectiva)** — el arte de Pixellab es una vista
  lateral un poco elevada: las patas del lado lejano acaban más arriba que las
  cercanas. Si la hierba empieza justo debajo de los pies, el personaje parece
  flotar sobre una raya; por eso la banda de hierba se dibuja 4 px por encima
  de `SUELO_Y` (`--hundir 4`) y los pies quedan dentro de ella.
- **Celda** — el rectángulo fijo en que se dibuja cada cuadro del
  protagonista: 96×64 px (`ANCHO_CELDA` × `ALTO_CELDA`), su última fila es la
  de los pies. Era 64×64 hasta que llegó el arte real, más ancho.
- **Cámara** — cuánto se ha desplazado el nivel respecto a la pantalla. Sigue
  al protagonista y se recorta entre 0 y `ANCHO_NIVEL - ANCHO_PANTALLA`. Se
  aplica escribiendo el scroll de cada capa (`bgSetScroll`).
- **Parallax** — capas de fondo que se desplazan a velocidades distintas (la
  lejana, a la mitad de la cámara) para dar sensación de profundidad.
- **Caja de colisión / caja de golpe (hitbox)** — rectángulo que representa el
  cuerpo del protagonista (colisión) o el alcance de la espada (golpe). En la
  Revisión 1 se calculaban y se mostraban en el HUD; desde el tutorial
  (Revisión 2) el cuerpo choca con el escenario y el golpe rompe el tronco.
- **Colisión por celdas (rejilla de colisión)** — el escenario se describe como
  una rejilla de celdas de 16×16 px y cada celda dice qué ES: vacío, sólido,
  plataforma o zarza. La física solo pregunta «¿qué hay en este píxel?» y mira
  la celda que lo contiene: columna `x >> 4`, fila `y >> 4`, índice
  `(fila << 7) | columna`. → `src/juego/colision.c`.
- **AABB** (*axis-aligned bounding box*) — una caja alineada con los ejes. Dos se
  tocan si cada una empieza antes de que acabe la otra, en x y en y: la prueba
  de colisión más barata. → `se_tocan` en `src/juego/partida.c`.
- **Resolución por ejes** — mover primero en x y corregir contra las paredes,
  después en y contra suelos y techos. Así una esquina nunca atrapa al
  personaje. → `mover_x` y `mover_y` en `src/juego/tarou.c`.
- **Plataforma de un solo sentido** — sostiene al caer encima, pero se atraviesa
  desde abajo y de lado: solo cuenta si los pies CRUZAN su borde de arriba
  bajando.
- **Punto de control** — sitio donde se reaparece tras caer o pincharse. En el
  tutorial, cada cartel; se apunta el último por el que pasó el centro de
  Tarou. → `src/juego/partida.c`.
- **Streaming de columnas (mapa-anillo)** — el nivel (2048 px) no cabe en el
  mapa de la VRAM (512 px), así que el mapa se usa como un anillo: cuando la
  cámara avanza, se escribe por DMA la columna que va a entrar, encima de una
  que ya salió. El scroll de la PPU también da la vuelta a los 512 px, así que
  lo que se ve coincide. → `src/juego/fondo_nivel.c`.
- **Rejilla doble (*dual grid*)** — tileset en el que cada celda de arte cubre
  las ESQUINAS de cuatro celdas de lógica y se elige por cuáles de ellas son
  sólidas (16 combinaciones). Así es el tileset Sidescroller de Pixellab; lo
  aplica `scripts/preparar_nivel.py`.
- **Guion de mando** — una tabla de «del cuadro A al B, mantener tal botón» que
  hace de mando. Sirve para probar el nivel entero en el PC y para que una ROM
  de prueba se juegue sola. → `src/juego/guion.c`.
- **HUD de depuración** — texto en la capa BG3 con posición, velocidad, estado,
  espada y cámara. Se alterna con SELECT, arranca apagado y no es parte del
  juego: es una herramienta para verificar y aprender.
- **Animación de espera (dormir)** — tras unos segundos sin tocar el mando, el
  protagonista se echa y duerme; cualquier botón lo despierta y, mientras se
  levanta, el mando se ignora.

## C (lo que aparece en este proyecto)

- **`u8`, `u16`** — enteros sin signo de 8 bits (0-255) y 16 bits (0-65535),
  definidos por PVSnesLib. Se prefieren a `int` porque la CPU es de 16 bits y
  cada byte de RAM cuenta (hay 128 KB).
- **`s16`** — entero con signo de 16 bits (−32768 a 32767). Se usa para
  velocidades y posiciones en punto fijo, que pueden ser negativas (hacia la
  izquierda o hacia arriba).
- **`struct`** — tipo compuesto que agrupa varias variables con nombre (sus
  campos) en una sola (posición, velocidad, estado...). El protagonista es
  una `struct` (`Tarou`, en `src/juego/tarou.h`) para que todo su estado
  viaje junto. Un campo se lee con `.` (`tarou.x_fp`) o, a través de un
  puntero, con `->` (`t->x_fp`).
- **`typedef`** — da un nombre nuevo a un tipo que ya existe.
  `typedef struct { ... } Tarou;` permite escribir `Tarou` en vez de
  `struct ...`, y `typedef uint8_t u8;` (`src/sistema/tipos.h`, en el PC) da
  el nombre `u8` al entero de 8 bits del C estándar.
- **Array** — fila de variables del mismo tipo seguidas en memoria, con un
  nombre y un índice que empieza en 0 (`a[0]` es la primera). C no comprueba
  que el índice esté dentro: `a[5]` en un array de 5 lee la memoria de al lado.
- **Puntero** — variable que guarda una dirección de memoria. `&x` es la
  dirección de `x`; `*p` es lo que hay donde apunta `p`; `p->campo` es un
  campo de la struct apuntada. Pasar un puntero a una función le permite
  cambiar el original (`tarou_actualizar(Tarou *t, ...)`); pasar el valor le
  da una copia.
- **`const`** — "solo lectura". En un parámetro puntero (`const Tarou *t`)
  promete que la función no escribe lo apuntado; el compilador lo hace
  cumplir. En una tabla (`const AnimDef ANIMACIONES[]`), sus valores no
  cambian.
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
- **`static`** (en una variable global o en una función) — solo se ve dentro
  de su archivo `.c`. Las piezas internas de `src/juego/tarou.c`
  (`cambiar_estado`, `acelerar`, `saltar`...) son funciones `static`; solo
  `tarou_iniciar`, `tarou_actualizar` y las de cajas se ven desde fuera.
- **`volatile`** — le dice al compilador que una variable puede cambiar "por
  fuera" (es un registro de hardware) y que no guarde su valor en caché: cada
  lectura de `REG_OPVCT` tiene que ir de verdad al chip.
- **`void`** — "sin valor": una función `void` no devuelve nada; `f(void)` no
  recibe parámetros.
- **Máquina de estados** — patrón: una variable dice en qué estado estamos; en
  cada frame se ejecuta solo el código de ese estado; al terminar se cambia la
  variable. Aquí los estados son las pantallas.
