# Glosario SNES y C para leer el código del proyecto

Los comentarios del código dicen `→ glosario: TÉRMINO` cuando usan un concepto
que se explica aquí una sola vez. Está ordenado de lo más general a lo más
específico; cada entrada dice **qué es**, **por qué importa en este proyecto**
y, cuando aplica, **qué función de PVSnesLib lo toca**. La última sección,
«Herramientas y depuración», reúne lo que no está en la consola sino alrededor
de ella: `make`, el enlazador, Mesen y la cadena de arte de Pixellab. Una
marca «(unidad NN)» al final de una entrada dice qué unidad del curso
(`aprendizaje/unidad-NN-*/`) la enseña con detalle.

## Vocabulario del proyecto

Términos acordados el 2026-10-16 (Revisión 1 del MVP). Un concepto, una
palabra: si un documento o comentario usa otra, es un error a corregir.

- **Etapa** — fase del plan de desarrollo (`docs/plan-desarrollo.md`, Etapa 0
  a 6). Nunca una zona del juego ni una fase de vida del personaje.
- **Revisión** — iteración numerada del producto mínimo viable (MVP).
  Revisión 1: Tarou adulto camina, corre, salta, se agacha, usa la espada y
  duerme en el nivel 1. Revisión 2: Bárbara.
- **Nivel** — zona jugable con principio y fin (`nivel.c`, `PANTALLA_NIVEL1`).
  El nivel 1 (el tutorial de movimiento) mide 2048 px de ancho: 8 pantallas.
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
  "no responde". Por dentro, `WaitForVBlank` pone `vblank_flag = 1` y repite
  la instrucción `wai` hasta que la rutina NMI lo vuelve a poner a 0
  (unidad 04).
- **Frame perdido (*lag frame*)** — vuelta del bucle que no llega a
  `WaitForVBlank` antes de la NMI: la lógica se pasó del frame, así que esa
  vuelta dura dos VBlanks y el juego se ve a cámara lenta. El SDK los cuenta
  en `lag_frame_counter` (unidad 04).
- **`nmi_handler` / `nmiSet`** — la función que la rutina NMI del SDK llama en
  cada VBlank, también en los frames perdidos. `nmiSet(f)` la cambia
  (unidad 04).
- **`consoleVblank`** — el `nmi_handler` por defecto: copia a la VRAM el texto
  que escribió `consoleDrawText`. La versión 4.6.0 del SDK no la declara en
  ninguna cabecera (unidad 04).
- **HBlank** — el instante entre el final de una línea de la pantalla y el
  principio de la siguiente. Es ahí donde escribe el HDMA (unidad 12).
- **VRAM** — la memoria de vídeo (64 KB). Guarda **tiles** y **tilemaps** de
  fondo, y los gráficos de los sprites. La CPU no dibuja píxeles: copia datos
  a VRAM y la PPU los muestra. Direcciones como `0x3000` o `0x6800` en el
  código son posiciones dentro de la VRAM (en hexadecimal).
- **VMAIN (`$2115`)** — registro que dice cuánto avanza la dirección de la VRAM
  tras cada escritura: `0x80` avanza 1 palabra (para filas del mapa y tiles),
  `0x81` avanza 32 (una fila del mapa entera: así se escribe una **columna**).
  El streaming de columnas del nivel usa `0x81` (unidad 07).
- **CGRAM** — la memoria de **paletas** (256 colores de 15 bits, 5 bits por
  componente RGB). `RGB8(r,g,b)` convierte un color de 0-255 al formato SNES.
- **OAM** — la tabla de **sprites** (128 entradas: posición, tile, paleta,
  prioridad). `oamSet(...)` escribe una entrada; la librería la copia a la PPU
  en cada VBlank. Además de las 128 entradas de 4 bytes hay una **tabla alta**
  de 32 bytes con 2 bits por sprite (el bit 8 de x y el tamaño): en total
  512 + 32 = 544 bytes. Ojo: el `id` que recibe `oamSet` es el número de
  entrada **× 4** (unidad 08).
- **OBSEL (`$2101`)** — registro que fija dónde empiezan en la VRAM los tiles de
  los sprites y qué pareja de tamaños (pequeño/grande) usan (unidad 08).
- **DMA / HDMA** — copia rápida de bloques de memoria hecha por hardware.
  **DMA** (`dmaCopyVram`) copia tiles/mapas a VRAM durante VBlank mucho más
  rápido que la CPU. **HDMA** escribe un valor en un registro de la PPU **en
  cada línea de la pantalla**: así se hace el cielo con degradado sin gastar
  CPU. `setModeHdmaColor` (con una tabla de un color por línea) escribe el
  **color 0 de la CGRAM** (modo 3, destino `$21`: dos escrituras a `$2121` y
  dos a `$2122`), no el color fijo `COLDATA`; además apaga todos los canales
  HDMA y espera dos `wai`.
- **Entrada de tabla HDMA** — un byte de cuenta (1..127 líneas, o `$80 | n` en
  modo repetir: un valor nuevo por línea durante n líneas) seguido de los
  bytes que van al registro. La tabla termina en un byte 0 (unidad 12).
- **Línea del haz (contadores H/V)** — la PPU dibuja la imagen de arriba
  abajo, una línea cada 63,5 µs (262 líneas por frame en NTSC; las 225..261
  son el VBlank). Leer el registro `$2137` congela los contadores y `$213D`
  devuelve en qué línea va el haz. El HUD de depuración lo usa como medidor de
  coste: si la lógica termina en la línea L, usó (37 + L) / 262 del frame.
- **`snes_vblank_count`** — contador de VBlanks del SDK (sube 1 en cada NMI).
  Comparado con `frames_en_pantalla` dice si el juego pierde frames; comparado
  con el reloj, si el emulador va a 60 por segundo.
- **Banco / LoROM** — la ROM del cartucho se ve en trozos de **32 KB** llamados
  bancos, cada uno en la mitad alta (`$8000-$FFFF`) de un banco de direcciones
  del 65816. Nuestro proyecto tiene 128 (4 MB, el máximo estándar). Un asset
  (imagen, música) no puede cruzar de un banco a otro; `data.asm` reparte cada
  uno en una sección `superfree` y el enlazador elige el banco.
- **FastROM** — modo en que la ROM se lee a 3,58 MHz en vez de 2,68 MHz
  (los registros y la WRAM van igual). Para la SNES, los bancos `$80-$FF` son
  la misma ROM que `$00-$7F` pero rápidos: con `FASTROM 1`, `hdr.asm` pone
  `.BASE $80` y todo el código vive ahí. Es también lo que permite 128 bancos:
  numerados desde `$00`, los dos últimos pisarían la WRAM (`$7E/$7F`).
- **Arranque del SDK (`tcc__start`)** — lo que pasa antes de `main`: RESET →
  `Snes_Init` → copia de `glob.data` (las variables globales con valor
  inicial, incluidas las tablas `const`) a la WRAM → `consoleInit` → `main`
  (unidad 03).

## Gráficos

- **Tile** — cuadradito de **8×8 píxeles**. Todo fondo y sprite se compone de
  tiles. Un tile de 16 colores ocupa 32 bytes ("4bpp": 4 bits por píxel). En
  general, los **bits por píxel (bpp)** fijan colores y tamaño: 2/4/8 bpp son
  4/16/256 colores y 16/32/64 bytes por tile (unidad 06).
- **Formato planar** — el tile no se guarda píxel a píxel sino por **planos de
  bits**: cada plano tiene un bit de cada píxel. En 4bpp, los planos 0-1 van en
  los bytes 0..15 y los planos 2-3 en los bytes 16..31 (unidad 06).
- **Tilemap (mapa)** — cuadrícula que dice qué tile va en cada celda de la
  pantalla (32×32 celdas = 256×256 px) y con qué paleta. Cambiar un fondo es
  cargar tiles + tilemap (`bgInitTileSet` + `bgInitMapSet`).
- **Mapa de 64×32 en páginas** — un mapa de 512 px de ancho no es una sola
  cuadrícula: son dos mapas de 32×32 seguidos, el segundo a 0x400 palabras del
  primero. Por eso `nivel_columna_vram` salta de página al pasar la columna 31
  (unidad 07).
- **Tileset** — colección de tiles pensada para combinarse (bordes, rellenos,
  esquinas). El suelo del nivel sale de un tileset *Sidescroller* de Pixellab:
  tiles de **borde** (hierba sobre tierra, con las filas de arriba vacías) y
  de **relleno** (tierra opaca), de 16×16. Es de **rejilla doble** (ver
  «Mecánicas»): lo aplica `scripts/preparar_nivel.py`.
- **Volteo de tile** — cada celda del tilemap puede mostrar su tile espejado
  en horizontal o en vertical sin gastar tiles nuevos. **Ojo:** gfx4snes solo
  aprovecha los volteos para ahorrar tiles si recibe `-F`; el Makefile se lo
  pasa a la capa cercana del nivel desde el 2026-10-22 (260 tiles; sin `-F`
  eran 281 y ninguno volteado) (unidad 06).
- **Paleta** — grupo de colores (16 en Mode 1) que un tile o sprite usa.
  Hay 8 paletas para fondos y 8 para sprites. El **color 0** de cada paleta
  es transparente en sprites y en las capas superiores.
- **Color de fondo (*backdrop*)** — la entrada 0 de la CGRAM: el color que se ve
  donde todas las capas son transparentes (unidad 06).
- **PNG indexado** — imagen que no guarda colores por píxel sino un **índice** a
  su paleta. Es lo que exige gfx4snes; el índice 0 es el transparente
  (unidades 06 y 18).
- **Cuantización (*median cut*)** — reducir una imagen a 15 colores (más el
  transparente) partiendo el espacio RGB por la mediana, sin tramado. La hacen
  los scripts de `scripts/` (unidad 18).
- **Tramado (*dithering*)** — imitar un color que falta mezclando píxeles de
  otros. Se evita: ensucia el pixel art y rompe la deduplicación de tiles
  (unidad 18).
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
  se mueven como uno solo. Tarou, en su celda de 96×64, son hasta 6 sprites de
  32×32. PVSnesLib genera la tabla de piezas con `gfx4snes -T`; el juego pone
  una entrada de la OAM por pieza (`src/juego/tarou_sprite.c`).
- **`.pic` de `gfx4snes -s 32`** — con bloques de 32×32 el archivo de tiles no
  sigue el orden de la imagen: guarda los bloques de 4 en 4 por fila de 128 px,
  así que el bloque n está en la fila n/4, columna n%4 (unidad 09).
- **Sprite dinámico** — técnica en la que todos los cuadros de animación viven
  en la ROM y solo se copia a la VRAM el cuadro que se va a mostrar. Así el
  límite de 512 tiles deja de importar. El juego usó el motor de PVSnesLib
  (`oamInitDynamicSprite`, `oamVramQueueUpdate`) hasta el 2026-10-21; su copia
  de 3 KB costaba 32 líneas del VBlank y se cambió por una propia.
- **Doble búfer (de tiles)** — dos sitios en la VRAM para el mismo sprite:
  mientras uno se muestra, el otro recibe el cuadro siguiente, repartido en
  varios VBlanks; cuando está entero, la OAM cambia de sitio. El dibujo va un
  frame por detrás de la lógica. → `src/juego/tarou_sprite.c`.
- **Plan de copias** — la lista (origen en la tira, destino en la VRAM, bytes)
  que calcula `metasprite_copias` para subir un cuadro de Tarou; las piezas
  vecinas cuyos bloques están seguidos van en un solo tramo de DMA
  (unidad 09).
- **Presupuesto del VBlank** — las 37 líneas (unos 2,3 ms) en que se puede
  escribir la VRAM. El DMA copia unos 170 bytes por línea; el SDK gasta 5
  líneas (OAM y mando). Lo que no cabe se pierde sin aviso. El HUD lo mide en
  su columna `V`. → `src/juego/dma_lista.h`, `docs/mesen-y-depuracion.md` §3.
- **Lista de DMA** — las copias a la VRAM se apuntan durante el frame (origen,
  destino, bytes, banco, modo de avance) y una rutina en ensamblador las
  dispara de un tirón en el VBlank, porque prepararlas en C con tcc816 cuesta
  más que el DMA mismo. → `src/juego/dma_ejecutar.asm`.
- **Fade (fundido) / brillo** — la PPU tiene un control de brillo de 0 (negro)
  a 15 (normal). `setBrightness(n)` lo fija; `setFadeEffectEx(FADE_IN, f)` lo
  anima en `f` pasos. **Ojo:** estas funciones bloquean el programa mientras
  dura el efecto.
- **Mosaico** — efecto de la PPU que agranda los píxeles de una capa hasta
  volverla bloques. `setMosaicEffect(MOSAIC_OUT, ...)` lo agranda (la doc del
  SDK lo tiene al revés) y `REG_MOSAIC = 0` lo apaga. El motor de escenas lo
  escribe a mano: `(tamaño << 4) | fondos`.
- **Forced blank (pantalla apagada)** — el bit 7 de `INIDISP` (`setScreenOff`).
  Con él la PPU no dibuja y la VRAM se puede escribir en cualquier momento.
  **Brillo 0 no es lo mismo**: `fade_out` deja la pantalla encendida en negro
  y lo que se copie a la VRAM fuera del VBlank se pierde (fallo del título al
  volver del nivel, 2026-10-21). `secuencia_cambiar` apaga antes de cada
  `_iniciar`.
- **Ventana** — la PPU tiene dos rangos de columnas (`WH0..WH3`, bordes
  izquierdo y derecho) que pueden recortar capas o limitar la matemática de
  color (`WOBJSEL`, `TMW`). Cambiando los bordes **en cada línea** por HDMA
  salen formas: una diagonal es `izq = base − línea`. → `src/escena/efectos.c`.
- **Matemática de color** — la PPU puede sumar o restar a cada píxel otro color:
  el de la pantalla secundaria o un **color fijo** (`COLDATA`). `CGWSEL` dice
  dónde (todo, solo dentro de la ventana...) y `CGADSUB` qué capas. Sumar un
  gris dentro de una ventana estrecha es el destello del logo Nintendo.
- **HDMA indirecto** — variante del HDMA en que la tabla no lleva los valores
  sino un **puntero** a ellos; en modo repetir el canal lee un valor nuevo por
  línea avanzando por esa memoria. Mover un efecto es cambiar el puntero (2
  bytes) en vez de reescribir la tabla. Registros `$43x0` bit 6 y `$43x7`
  (banco de los datos).
- **Motor de escenas** — una tabla de eventos (frame, duración, pista, valor
  final, paso) que mueve números en punto fijo 8.8, y una capa que lleva esos
  números a los registros en el VBlank. Cada número es una **pista** (brillo,
  mosaico, posición del destello, luz): guarda su valor 8.8, su paso, los
  frames que le quedan y su destino, y el motor la mueve sola
  (unidad 12). → `docs/diseno-motor-escenas.md`.
- **Consola de texto** — PVSnesLib trae una fuente y una capa de texto de
  32 columnas × 28 filas (celdas de 8×8). `consoleDrawText(x, y, "...")`
  escribe en la celda (x, y); la librería copia el texto a VRAM en el
  siguiente VBlank.

## Entrada y sonido

- **Mando (pad)** — `padsCurrent(0)` devuelve los botones **mantenidos** del
  mando 1 como bits; `padsDown(0)` solo los **recién pulsados** en este frame.
  Se prueba un botón con `&`: `if (padsDown(0) & KEY_START)`. A mano:
  **recién pulsado** = `(ahora ^ antes) & ahora`, **recién soltado** =
  `antes & ~ahora`; los dos duran un solo frame (unidad 05).
- **Autolectura del mando** — la consola lee los mandos sola al inicio del
  VBlank (bit 0 de `NMITIMEN`) y deja el resultado en los registros
  `$4218-$421F`; el SDK lo copia de ahí (unidad 05).
- **Máscara de botón (`KEY_*`)** — un bit por botón en un `u16`. B es el bit 15:
  un cast a `(u8)` lo pierde sin avisar (unidad 05).
- **SPC700** — el procesador de sonido: 8 bits, con 64 KB de RAM propia que la
  CPU no ve. Ejecuta el driver de sonido (unidad 17).
- **DSP (audio)** — el circuito que mezcla las 8 voces de muestras BRR y saca
  el sonido (unidad 17).
- **Driver de sonido (snesmod)** — el programa que `spcBoot()` copia al
  SPC700; obedece los mensajes que le mandan las funciones `spc*` (unidad 17).
- **Puertos APU (`$2140-$2143`)** — los 4 registros de un byte por los que se
  hablan la CPU y el SPC700 (unidad 17).
- **BRR** — formato comprimido de las muestras del DSP: bloques de 9 bytes con
  16 muestras de 4 bits (unidad 17).
- **Soundbank / módulo .it** — la música se compone en un tracker (formato
  Impulse Tracker) y `smconv` la empaqueta en un "soundbank" dentro de la ROM.
  `spcBoot()` arranca el chip de sonido; `spcLoad(MOD_X)` carga una canción;
  `spcPlay(0)` la reproduce. `smconv` genera también `soundbank.h`, con un
  `MOD_*` por canción, y el soundbank va en un banco propio (`-b 5`)
  (unidad 17).
- **`spcProcess`** — entrega al SPC700 la cola de mensajes pendientes. Se llama
  una vez por frame, antes de `WaitForVBlank` (unidad 17).
- **Región de sonido** — trozo de la RAM del SPC700 reservado con
  `spcAllocateSoundRegion` para efectos BRR que se envían por partes
  (*streaming*) (unidad 17).

## Mecánicas del juego (Revisión 1)

- **Velocidad, aceleración y frenado** — en píxeles por frame, en punto fijo.
  Cada frame: si hay una dirección pulsada, `velocidad += aceleración` hasta
  un máximo; si no, la velocidad se acerca a 0 restando el frenado. Después
  `posición += velocidad`: eso es **integrar**, sumar la velocidad a la
  posición una vez por frame (unidad 10).
- **Gravedad** — aceleración vertical constante que se suma a la velocidad
  vertical en cada frame mientras el protagonista está en el aire. La
  velocidad de caída se limita a una **velocidad terminal**.
- **Salto de altura variable** — al pulsar B se asigna una velocidad vertical
  hacia arriba; si se suelta B mientras todavía sube, esa velocidad se
  recorta y el salto queda más corto. Mantener B da el salto completo.
- **Apogeo** — el punto más alto del salto: el frame en que `vy` deja de ser
  negativa y el estado pasa de SALTANDO a CAYENDO (unidad 10).
- **Suelo** — dónde se apoya el protagonista. En la Revisión 1 era plano
  (`nivel_suelo_en(x)` devolvía una constante); desde el tutorial lo dice la
  rejilla de colisión: cualquier celda sólida o plataforma bajo uno de los dos
  «pies» de Tarou (columnas 36 y 64 de su celda). `SUELO_Y = 176` queda como
  el borde del suelo base. → `src/juego/colision.c`, `mover_y` en
  `src/juego/tarou.c`.
- **Hundir los pies (perspectiva)** — el arte de Pixellab es una vista
  lateral un poco elevada: las patas del lado lejano acaban más arriba que las
  cercanas. Si la hierba empieza justo debajo de los pies, el personaje parece
  flotar sobre una raya. Hoy se hunde 4 px en total: 2 px los pone el arte (la
  banda de hierba empieza 2 px por encima del borde de la celda) y 2 px
  `HUNDIR_PIES` en `src/juego/tarou_sprite.c`, que dibuja a Tarou 2 px más
  abajo que su posición física. Solo es dibujo: la física usa el borde exacto.
  (El antiguo `--hundir 4` de `preparar_suelo.py` ya no se usa.)
- **Celda** — el rectángulo fijo en que se dibuja cada cuadro del
  protagonista: 96×64 px (`ANCHO_CELDA` × `ALTO_CELDA`), su última fila es la
  de los pies. Era 64×64 hasta que llegó el arte real, más ancho.
- **Cámara** — cuánto se ha desplazado el nivel respecto a la pantalla. Sigue
  al protagonista y se recorta entre 0 y `ANCHO_NIVEL - ANCHO_PANTALLA`. Se
  aplica escribiendo el scroll de cada capa (`bgSetScroll`). **Recortar**
  (*clamp*) es dejar un valor en el extremo del intervalo si se sale de él
  (unidad 07).
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
- **Bloque (de colisión)** — rectángulo sólido aparte de la rejilla que puede
  quitarse (el tronco). Existe porque la rejilla vive en la ROM y no se puede
  borrar una celda (unidad 20).
- **Parche de columnas** — columnas de tiles alternativas pegadas tras el nivel
  (x 2048..2079) que se copian en lugar de las normales mientras el tronco está
  entero (unidad 20).
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
- **Frames por cuadro** — cuántos frames se muestra cada cuadro de una
  animación; es un campo de `AnimDef` en la tabla de animaciones (unidad 11).
- **`anim_terminada`** — campo de `Tarou` que pone `avanzar_animacion` cuando
  una animación sin bucle llega a su último cuadro; los estados que esperan a
  que acabe (desenvainar, atacar...) lo miran para cambiar (unidad 11).
- **Animación de espera (dormir)** — tras unos segundos sin tocar el mando, el
  protagonista se echa y duerme; cualquier botón lo despierta y, mientras se
  levanta, el mando se ignora.

## C (lo que aparece en este proyecto)

- **Bit / byte** — un bit es un 0 o un 1; 8 bits forman un byte (unidad 00).
- **Binario** — escribir un número en base 2, un bit por cifra: `0b101` = 5
  (unidad 00).
- **Hexadecimal** — base 16, con el prefijo `0x`; cada cifra son exactamente 4
  bits, por eso es la forma cómoda de leer registros y direcciones
  (`0x80` = `1000 0000`) (unidad 00).
- **Operadores de bits `& | ^ ~ << >>`** — operan bit a bit: Y, O, O exclusivo,
  negación y desplazamientos. No confundir con los lógicos `&& || !`, que
  tratan el número entero como verdadero o falso (unidad 00).
- **Máscara** — número con encendidos solo los bits que interesan, para
  probarlos (`x & m`), encenderlos (`x | m`), apagarlos (`x & ~m`) o
  alternarlos (`x ^ m`). Los botones del mando y los bits de los registros se
  manejan así (unidad 00).
- **`u8`, `u16`** — enteros sin signo de 8 bits (0-255) y 16 bits (0-65535),
  definidos por PVSnesLib. Se prefieren a `int` porque la CPU es de 16 bits y
  cada byte de RAM cuenta (hay 128 KB).
- **`s16`** — entero con signo de 16 bits (−32768 a 32767). Se usa para
  velocidades y posiciones en punto fijo, que pueden ser negativas (hacia la
  izquierda o hacia arriba).
- **Complemento a dos** — la forma de guardar negativos: −x se obtiene
  invirtiendo los bits de x y sumando 1. Por eso -1 es `0xFFFF` en un `s16`
  (unidad 01).
- **Desbordamiento** — el resultado no cabe en el tipo: se pierden los bits
  altos y el valor «da la vuelta» sin avisar (255 + 1 = 0 en un `u8`)
  (unidad 01).
- **Coma flotante** — números con decimales hechos por hardware (`float`). La
  SNES no la tiene y el proyecto la prohíbe: se usa punto fijo (unidad 01).
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
  cambian. **Ojo:** con tcc816, una tabla `const` global **no** se queda en
  la ROM: va a la WRAM (banco `$7F`), copiada por el arranque (`crt0`); solo
  las `static const` van a `.rodata`, en la ROM.
- **Preprocesador** — el paso anterior a compilar: resuelve las líneas que
  empiezan por `#` pegando y sustituyendo texto. `gcc -E` muestra lo que sale
  de él (unidad 00).
- **`#include`** — pega el contenido de otro archivo. `<snes.h>` es la
  librería; `"comun.h"` es nuestro.
- **`#define`** — constante o macro que el preprocesador reemplaza antes de
  compilar. `#define FPS 60` → cada `FPS` pasa a ser `60`.
- **`-D`** — define un nombre desde la línea del compilador
  (`-DNOMBRE=valor`), como si hubiera un `#define` al principio del archivo.
  Así se compilan `-DPRUEBAS_PC` y `-DGUION_PRUEBA` (unidad 00).
- **`#ifdef` / `#else` / `#endif`** — compila un trozo solo si un nombre está (o
  no) definido (unidad 00).
- **`#ifndef` / `#define` / `#endif` en un `.h`** — "guarda de inclusión":
  evita que el mismo header se pegue dos veces en un archivo.
- **Prototipo** — la declaración de una función: su firma terminada en `;`, sin
  cuerpo. Es lo que llevan los `.h` (unidad 00).
- **Definición vs declaración** — la definición crea la cosa (la variable o el
  cuerpo de la función) y existe una sola vez en todo el programa; la
  declaración solo anuncia que existe en algún sitio (unidad 00).
- **`enum`** — lista de nombres con valor entero (0, 1, 2...). Nuestro
  `enum Pantalla` da nombre a cada estado de la secuencia.
- **`switch` / `case`** — elige un bloque según el valor de una variable; es
  el corazón de la **máquina de estados** (`secuencia.c`).
- **Fall-through** — sin `break`, un `case` sigue ejecutando el código del
  siguiente. El juego solo lo usa con `case` vacíos apilados (varios valores,
  un mismo bloque) (unidades 00 y 02).
- **`extern`** — "esta variable/función existe en otro archivo". Así las
  pantallas leen `frames_en_pantalla`, que vive en `secuencia.c`.
- **`static`** (en una variable global o en una función) — solo se ve dentro
  de su archivo `.c`. Las piezas internas de `src/juego/tarou.c`
  (`cambiar_estado`, `acelerar`, `saltar`...) son funciones `static`; solo
  `tarou_iniciar`, `tarou_actualizar` y las de cajas se ven desde fuera.
  Dentro de una función, una **`static` local** conserva su valor entre
  llamadas (unidad 00). **Ojo:** el C estándar promete que las `static` (y las
  globales sin valor, la sección `.bss`) empiezan a 0, pero en esta ROM no:
  `crt0` intenta borrar `.bss`, pero como cada `.c` repite `SECTIONSTART_.bss`
  solo borra 10 bytes de `$7E:2000`. Hay que inicializarlas siempre.
- **`volatile`** — le dice al compilador que una variable puede cambiar "por
  fuera" (es un registro de hardware) y que no guarde su valor en caché: cada
  lectura de `REG_OPVCT` tiene que ir de verdad al chip.
- **`void`** — "sin valor": una función `void` no devuelve nada; `f(void)` no
  recibe parámetros.
- **Máquina de estados** — patrón: una variable dice en qué estado estamos; en
  cada frame se ejecuta solo el código de ese estado; al terminar se cambia la
  variable. Aquí los estados son las pantallas.
- **Transición** — el paso de un estado a otro, siempre con una causa: un
  botón, un tiempo, el fin de una animación o la física (tocar suelo, empezar
  a caer) (unidad 02).
- **Promoción entera** — antes de operar, C convierte a `int` (16 bits con
  signo en tcc816) todo lo que es más pequeño; por eso `u8 / 2` llama a
  `tcc__div` y `(u16)n / 2` es un simple `lsr` (unidad 14).
- **Relleno de struct** — bytes que el compilador añade dentro de una `struct`
  para alinear sus campos: `TramoGuion` mide 6 bytes, no 5 (unidad 14).

## Ensamblador 65816

- **Ensamblador** — el lenguaje de las instrucciones de la CPU escritas con
  nombres cortos (`lda`, `sta`, `rtl`); tcc816 traduce cada `.c` a él y
  `wla-65816` lo convierte en bytes. El juego tiene una sola rutina escrita a
  mano: `src/juego/dma_ejecutar.asm` (unidades 14 y 15).
- **Convención de llamada (tcc816)** — cómo se pasan datos entre funciones:
  parámetros en la pila de derecha a izquierda (`u8` = 1 byte, puntero = 4),
  llamada con `jsr.l` y vuelta con `rtl`; quien llama saca los parámetros y el
  resultado vuelve en `tcc__r0` (unidades 14 y 15).
- **Pseudorregistro `tcc__rN`** — palabra de la página directa (`$00..`) que
  tcc816 usa como registro de más; no se conserva entre llamadas (unidad 14).
- **Ciclo** — un paso del reloj de la CPU; cada instrucción tarda varios. Es
  la unidad con que se compara lo rápida que es una rutina (unidad 15).
- **Profiler** — herramienta de Mesen que cuenta, por función, cuántas veces
  se llamó y cuántos ciclos gastó (propios o inclusivos) (unidad 15).

## Herramientas y depuración

- **Archivo objeto (`.o`, `.obj`)** — un `.c` ya compilado pero con huecos: las
  llamadas a funciones y variables de otros archivos quedan pendientes para
  que las rellene el enlazador (unidad 00).
- **Enlazador (`ld`, `wlalink`)** — junta los archivos objeto y resuelve las
  referencias entre ellos. Si algo se declaró pero nadie lo definió, falla con
  «undefined reference» (unidad 00).
- **Regla / objetivo / dependencia (make)** — una regla del `Makefile` es
  «objetivo: dependencias» más una receta; la receta corre si el objetivo
  falta o si alguna dependencia es más nueva que él (unidad 03).
- **Regla de patrón (make)** — regla con `%` que vale para muchos archivos:
  `res/personajes/tarou/%.pic: %.png` convierte todas las tiras de Tarou con
  una sola regla (unidad 18).
- **Archivo intermedio (make)** — los `.ps` y `.asm` que `make` crea al
  encadenar reglas de patrón (`.c` → `.ps` → `.asm` → `.obj`) y que borra al
  terminar, salvo que se pidan como objetivo (unidad 03).
- **Sección** — bloque con nombre dentro de un `.obj` que el enlazador coloca
  entero en un banco (`superfree`, `.rodata_tarou_<anim>`...) (unidad 03).
- **`.log` / `.sym`** — el `.log` es la salida completa de las herramientas; el
  `.sym` lista cada etiqueta de la ROM con su dirección de 24 bits (banco +
  dirección), y es lo que usa Mesen para poner nombres (unidad 03).
- **`tccs_<archivo>.asm_<nombre>`** — así aparecen en el `.sym` las variables y
  funciones `static`, que tcc816 renombra con el archivo para que no choquen
  (unidad 13).
- **`.db :etiqueta`** — en ensamblador WLA, guarda en la ROM el **banco** que el
  enlazador dio a una etiqueta. Así sabe el DMA en qué banco está la capa
  cercana (`nivel1_cerca_col_banco`) (unidad 13).
- **Breakpoint (de ejecución, lectura o escritura)** — condición que pausa la
  emulación en Mesen en el instante exacto en que se cumple: al ejecutar una
  dirección, al leerla o al escribirla (unidad 16).
- **Event Viewer** — visor de Mesen que pinta cada escritura en un registro con
  la línea y el ciclo del frame en que ocurrió (unidad 16).
- **`consoleMesenBreakpoint` / WDM** — la instrucción WDM no hace nada en la
  consola, pero Mesen puede pararse en ella: es un breakpoint escrito en el
  código (unidad 16).
- **TDD** — desarrollo guiado por pruebas: escribir o cambiar la prueba
  primero, verla fallar y solo después cambiar el código (unidad 18).
- **Copia fija `tests/tarou_anim.h`** — copia congelada de la cabecera de
  animaciones que usan las pruebas de PC (`-I.` va primero en su `Makefile`).
  Guarda los cuadros de los antiguos marcadores, así que sus números no
  coinciden con el arte real (unidad 02).
- **Traza (x/y por cuadro)** — listado del recorrido jugado con el guion en el
  PC, cuadro a cuadro; con ella se ajustan los tramos del guion (unidad 20).
- **Exportación nativa de Pixellab** — el zip que da Pixellab: una carpeta por
  estado (`<Estado>/animations/<nombre>/<dirección>/frame_NNN.png` y
  `rotations/<dirección>.png`), 8 direcciones, lienzos de 64 a 104 px y un
  `metadata.json`. `scripts/preparar_personaje.py` la lee (unidad 19).
- **Anclar (por centro y pies)** — cómo `preparar_personaje.py` pega cada
  cuadro en la celda de 96×64: el centro del lienzo en la columna 48 y la fila
  más baja de la animación en la 63, para que el personaje no baile entre
  cuadros (unidad 19).
- **Pieza suelta (de Pixellab)** — PNG de una sola pieza (zarza, cartel,
  tronco, salida) que acompaña a la hoja del suelo (unidad 19).
