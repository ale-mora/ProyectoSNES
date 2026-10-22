# Plan de la carpeta `aprendizaje/`

**Para quién:** un programador junior que va a heredar este proyecto y tiene
que entender todo el código desplegado, de lo más simple a lo más complejo,
con el foco en programar juegos de Super Nintendo en C y ensamblador.
**Qué es:** el plan de las unidades de estudio. Cada unidad enseña un
concepto, lo ancla en un archivo real del juego y se aprueba con una
práctica. **Este documento es la especificación; las unidades se escriben en
sesiones posteriores, una por sesión**, siguiendo el formato de §2 y el
procedimiento de §6. El punto de entrada para esas sesiones es
`aprendizaje/README.md`.

**Estado del juego que enseñan (2026-10-20, rama `demo-secuencia-arranque`):**
Revisión 2. Tarou es un perro de Pixellab en celdas de 96×64 (metasprite
dinámico de hasta 6 piezas) con 13 animaciones. El nivel 1 es un **tutorial
de movimiento** de 2048 px (`docs/diseno-tutorial-movimiento.md`): escalones,
muro, túnel, foso, plataformas sobre zarzas, un tronco que se rompe con la
espada y la salida a «LEVEL 2 / COMING SOON»; colisión contra una rejilla de
celdas de 16 px, reaparición en puntos de control, la capa cercana dibujada en
Tiled (la lógica) y vestida por script con el tileset de Pixellab, copiada a
la VRAM por columnas; montañas en parallax y cielo por HDMA. Pruebas: 280 en
PC (entre ellas el recorrido completo del nivel) y 35 de scripts; una ROM de
prueba (`-DGUION_PRUEBA`) se juega sola en Mesen. Lo que todavía no existe:
enemigos, audio, Bárbara jugable y el arte final de zarzas, carteles y tronco.

Alcance: mecánicas, C, ensamblador (65816 y SPC700) y la **integración
técnica** del arte (de la exportación de Pixellab a la ROM). Dibujar el arte,
diseñar niveles, componer música y escribir el guion quedan fuera.

## 1. Cómo se usa (para el alumno)

1. Las unidades se hacen **en orden**: cada una da por sabido lo anterior
   (campo «Prerrequisitos» y grafo de §4).
2. Antes de empezar una unidad se lee su `README.md` (la teoría, con todos
   los términos definidos); después se ejecuta la demo, se resuelve el
   ejercicio y se comprueba contra la solución.
3. Cada unidad termina mirando el **código real del juego** que la aplica:
   el campo «Código del juego» dice qué archivo abrir y qué buscar.
4. El glosario (`docs/glosario-snes.md`) es la referencia común: si un
   término no está definido en la unidad, está ahí.

Regla de redacción de todas las unidades: **todo concepto técnico se define
la primera vez que aparece**, en una frase, antes de usarse. Lenguaje técnico,
sin dar nada por sabido más allá de lo que enseñan las unidades anteriores.

## 2. Formato de una unidad

```
aprendizaje/unidad-NN-tema/
├── README.md         teoría: objetivo, conceptos (cada uno definido), cómo funciona,
│                     dónde está en el juego, ejercicio y criterio de aprobación
├── demo_NN_*.c       programa de ejemplo que se ejecuta y se lee (prácticas de PC)
├── ejercicio_NN_*.c  esqueleto con TODO numerados para completar (o ejercicio_NN.md
│                     cuando el ejercicio es sobre el juego real o de lectura)
├── solucion/         la solución del ejercicio (se mira después, no antes)
└── rom/              solo en unidades de hardware: una mini-ROM (receta en §6.3)
    ├── Makefile
    ├── data.asm      los assets que use (si los hay)
    ├── res/          sus imágenes de origen (si las hay)
    └── src/*.c       el programa de la mini-ROM, con la cabecera estándar
```

Tres tipos de práctica:

- **PC (gcc)**: programas de consola en C que aíslan un concepto del lenguaje
  o de la lógica (punto fijo, máquinas de estado, structs). Se compilan con
  `gcc` y se ejecutan en el PC. Es el formato de la unidad 00.
- **Mini-ROM**: un programa de SNES independiente que hace una sola cosa (un
  fondo, un sprite, leer el mando) y se abre en Mesen. Parte de un ejemplo
  del SDK (`tools/pvsneslib/snes-examples/`, ver §6.3).
- **Sobre el juego real**: cambiar una constante o una función del proyecto,
  compilar y observar el efecto en Mesen con el HUD de depuración (y, si
  aplica, con las pruebas de `tests/`). Los cambios del alumno no se
  confirman en la rama principal: se deshacen al terminar (`git restore`) o
  se hacen en una rama propia.

Criterio de aprobación: siempre algo **observable** (una salida en consola,
una captura de Mesen, una prueba en verde), nunca «haber leído».

## 3. Las unidades

### Unidad 00 — C en el PC: lógica, funciones, arrays, structs y punteros

- **Objetivo:** leer y escribir C de 16 bits sin miedo: variables, tipos,
  `if`/`for`/`while`, funciones, arrays, `struct`, punteros y `#define`.
- **Conceptos:** variable; tipo (y por qué `u8`/`u16`/`s16` en vez de
  `int`); expresión y sentencia; función (parámetros, valor devuelto, `void`);
  array (índice desde 0); `struct` (varias variables con nombre en una);
  puntero (una variable que guarda una dirección; `*`, `&`, `->`);
  `#define` y macro; `static`; `extern`; guarda de inclusión.
- **Archivos (escritos, `unidad-00-c-en-el-pc/`):** `demo_01_logica.c`,
  `demo_02_funciones.c`, `demo_03_tipos_arrays.c` y
  `ejercicio_03_tipos_arrays.c` vienen de la antigua `etapa0/` (movidos con
  `git mv`); las versiones de `ejercicio_01` y `ejercicio_02` que resolvió el
  autor están en `solucion/` (solo cambió el texto de sus `printf`, para que
  la salida coincida con la del enunciado) y en la raíz hay esqueletos con
  los TODO vacíos. Nuevos: `demo_04_structs_punteros.c` (puntero a entero,
  `typedef struct`, paso por valor frente a por puntero, `->`, parámetro de
  salida y la trampa de los paréntesis en las macros),
  `ejercicio_04_structs_punteros.c` (una `Personaje` con posición, velocidad
  y dirección; cuatro funciones que la reciben por puntero, con recorte de
  velocidad y de borde y una `Caja` de salida) y las soluciones de los
  ejercicios 03 y 04. Cada ejercicio trae su salida esperada exacta, y
  `capturas/` guarda la salida de referencia de demos y soluciones. Los
  ejemplos 01–03 usan `int` a propósito; el README explica por qué el juego
  usa `u8`/`u16`/`s16`.
- **Código del juego:** `src/juego/tarou.h` (el `typedef struct { ... } Tarou`,
  la `Caja` y las constantes `#define`), `src/juego/tarou.c` (funciones que
  reciben `Tarou *t`; `cambiar_estado` y compañía son `static`),
  `src/sistema/tipos.h` (de dónde salen `u8/u16/s16`), `src/sistema/secuencia.h`
  (`extern`).
- **Prerrequisitos:** ninguno. **Práctica:** PC.
- **Aprobación:** los cuatro ejercicios compilan con `gcc` y dan la salida
  esperada que indica cada archivo.

### Unidad 01 — Enteros de 16 bits y punto fijo

- **Objetivo:** hacer aritmética con decimales en una CPU que no los tiene.
- **Conceptos:** bit, byte, binario y hexadecimal; desbordamiento (qué pasa
  en 255 + 1 con `u8`); signo (`s16`: complemento a dos); desplazamientos
  `<<` y `>>`; **punto fijo** (reservar bits para la fracción; formatos 8.8 y
  12.4); por qué las posiciones no son negativas en este proyecto; el coste
  de `/` y `%` en el 65816.
- **Archivos:** `README.md`, `demo_01_puntofijo.c` (mueve un valor con
  velocidad 1,5 px/frame y lo imprime frame a frame), `ejercicio_01_gravedad.c`
  (simular un salto con gravedad en 12.4 e imprimir la altura máxima).
- **Código del juego:** `src/sistema/puntofijo.h`, `tests/prueba_puntofijo.c`,
  las constantes de `src/juego/tarou.h` (`VEL_SALTO`, `GRAVEDAD`).
- **Prerrequisitos:** 00. **Práctica:** PC.
- **Aprobación:** el ejercicio imprime una altura máxima de unos 69 px para
  `VEL_SALTO = -96` y `GRAVEDAD = 4` (la prueba de `tests/prueba_tarou.c`
  acepta un mínimo de `y` entre 73 y 77 partiendo de 144).

### Unidad 02 — Máquinas de estado

- **Objetivo:** modelar «qué está haciendo» algo con una variable y un `switch`.
- **Conceptos:** estado; transición; `enum` (nombres con valor entero);
  `switch`/`case`/`break`; máquina de estados finita; el patrón
  `_iniciar` / `_actualizar`; por qué una sola cosa a la vez (y cuándo una
  variable aparte, como `espada`, es mejor que duplicar estados).
- **Archivos:** `README.md`, `demo_02_semaforo.c` (un semáforo con `enum` y
  `switch`), `ejercicio_02_perro.c` (PARADO → CAMINANDO → SALTANDO con un
  mando simulado por teclado en consola).
- **Código del juego:** `src/sistema/secuencia.c` (la máquina de pantallas),
  `src/juego/animaciones.h` (`enum Estado`), `src/juego/tarou.c`
  (`manejar_pie`, `manejar_agachado`, `manejar_espada`, `manejar_sueno`),
  `docs/diseno-demo-jugable.md` §4 (la tabla de transiciones).
- **Prerrequisitos:** 00. **Práctica:** PC + sobre el juego real (añadir una
  comprobación a `tests/prueba_tarou.c` y verla pasar).
- **Aprobación:** la prueba nueva pasa en `tests/pruebas.exe` y el ejercicio
  imprime la secuencia de estados esperada.
- **Trampa a explicar:** las pruebas de PC usan `tests/tarou_anim.h`, una
  **copia fija** con los cuadros de los antiguos marcadores (reposo 4, caminar
  8...), no los del arte real (`res/personajes/tarou/tarou_anim.h`: reposo 9,
  desenvainar 9, despertar 10...). Las cuentas de frames de una prueba nueva
  se hacen con la copia fija.

### Unidad 03 — De `.c` a `.sfc`: la tubería de compilación

- **Objetivo:** saber qué hace cada herramienta y leer el ensamblador que
  genera el compilador.
- **Conceptos:** compilador, ensamblador, enlazador; código objeto y
  sección; banco de ROM (LoROM, 32 KB; FastROM: los bancos se ven en
  `$80-$FF` y el código va a 3,58 MHz, `-F` en `tcc816`); cabecera de la ROM
  (`ROMSIZE 0C` = 4 MB, `ROMBANKS 128`, `FASTROM 1`); `Makefile`,
  regla, dependencia; archivo intermedio (por qué `make` borra los `.ps` y
  `.asm` de `tcc816`); por qué `make clean` tras tocar un `.h`; el `.log` y
  el `.sym`.
- **Archivos:** `README.md` (resumen de `docs/tuberia-de-compilacion.md` con
  ejercicios), `rom/` (mini-ROM «hola mundo» a partir de
  `tools/pvsneslib/snes-examples/hello_world`, receta de §6.3),
  `ejercicio_03_leer_asm.md` (generar el ensamblador de `src/juego/camara.c`
  con la receta de §6.4, copiarlo a la unidad y anotar, línea a línea, qué
  instrucción corresponde a cada sentencia de C).
- **Código del juego:** `Makefile`, `src/juego/camara.c` y su `.asm`
  generado (77 líneas, verificado el 2026-10-22 con FastROM: carga del parámetro desde la
  pila, `adc.w #48`, `sbc.w #128`, recorte por abajo con `bvc`/`bmi`, recorte
  por arriba comparando con `#1792` (`ANCHO_NIVEL − ANCHO_PANTALLA`) y `rtl`:
  las funciones de tcc816 son «largas», se llaman con `jsl`/`jsr.l`),
  `ProyectoSNES.log`.
- **Prerrequisitos:** 00. **Práctica:** mini-ROM + lectura.
- **Aprobación:** la mini-ROM compila y muestra el texto en Mesen; las
  anotaciones del `.asm` identifican la carga del parámetro, la suma de la
  media celda (48), la resta de la media pantalla (128), los dos recortes
  (0 y 1792) y el retorno.

### Unidad 04 — El frame, el VBlank y el bucle principal

- **Objetivo:** entender el reloj del juego: 60 vueltas por segundo
  sincronizadas con la pantalla.
- **Conceptos:** frame; línea del haz; VBlank; interrupción y NMI;
  `WaitForVBlank()`; contar tiempo en frames (`SEGUNDOS(n)`); qué solo se
  puede hacer en el VBlank (escribir VRAM, OAM, CGRAM) y la excepción del
  **forced blank** (con la pantalla apagada, `setScreenOff`, la VRAM se escribe
  cuando se quiera: así reaparece Tarou); qué pasa si la lógica se pasa de
  tiempo (se pierde un VBlank); medir la velocidad real con un recorrido
  conocido (2238 cuadros = 37,3 s a 60 FPS).
- **Archivos:** `README.md`, `rom/` (mini-ROM con un contador de frames en
  pantalla y un «segundero» calculado con `SEGUNDOS`), `ejercicio_04.md`
  (hacer que el contador parpadee cada medio segundo).
- **Código del juego:** `src/sistema/main.c` (el bucle), `src/sistema/secuencia.c`
  (`secuencia_vblank`), `src/sistema/pantallas.h` (duraciones),
  `src/pantallas/pantalla_nivel1.c` (`pantalla_nivel1_vblank`: columnas,
  sprite y HUD por DMA; `reaparecer`: fundido, `setScreenOff`, reescritura,
  fundido), la fila `LIN`/`VB` del HUD y `docs/hardware-snes-por-modulo.md` §8.
- **Prerrequisitos:** 03. **Práctica:** mini-ROM + sobre el juego real
  (medir con el HUD cuánto frame usa el nivel).
- **Aprobación:** captura de Mesen con el contador y una nota con el valor
  de `LIN` medido en el nivel y su porcentaje de frame, con el HUD encendido y
  la explicación de por qué `F` puede avanzar menos que `VB` (hasta el
  2026-10-21 el HUD costaba 50–150 líneas por fila y el tutorial bajaba a ~47
  FPS; hoy cuesta 24–63 y tiene un tope de línea: ~6 frames en todo el
  recorrido). Ojo al
  leer `LIN`: la lógica empieza dentro del VBlank, así que las cifras pueden
  estar en 225..261 o haber dado ya la vuelta (70–120 en el tutorial).

### Unidad 05 — El mando

- **Objetivo:** leer los botones y separar «mantenido» de «recién pulsado».
- **Conceptos:** bits y máscaras (`&`, `|`, `<<`); `padsCurrent` y
  `padsDown`; autolectura del joypad en el VBlank; la `struct Mando` como
  traducción a intenciones; prioridad entre botones pulsados a la vez; un
  **guion de mando** (botones leídos de una tabla de tiempo): como la lógica
  solo ve la `struct Mando`, se puede jugar sin manos, en el PC o en la ROM.
- **Archivos:** `README.md`, `rom/` (mini-ROM que muestra en pantalla qué
  botones están pulsados y cuáles acaban de bajar; base:
  `tools/pvsneslib/snes-examples/input/`), `ejercicio_05.md` (añadir un
  contador de «cuántas veces se pulsó B»).
- **Código del juego:** `src/juego/mando.c/.h` (y su rama `GUION_PRUEBA`),
  `src/juego/guion.c/.h` (mantenidos y recién pulsados desde una tabla),
  `src/juego/guion_tutorial.c`, `docs/diseno-demo-jugable.md` §3 y §4
  («Prioridad el mismo frame»), el mapeo de teclado de Mesen en
  `docs/mesen-y-depuracion.md` §2.
- **Prerrequisitos:** 04. **Práctica:** mini-ROM.
- **Aprobación:** captura con los botones reflejados y el contador de B.

### Unidad 06 — Tiles, paletas, tilemaps y fondos

- **Objetivo:** cargar y mostrar un fondo entendiendo qué va a la VRAM y a la CGRAM.
- **Conceptos:** tile (8×8); bits por píxel (2bpp, 4bpp, 8bpp) y tamaño en
  bytes; paleta y color de 15 bits; tilemap (palabra por celda: tile,
  paleta, prioridad, **volteo horizontal y vertical**); modos de vídeo
  (Mode 1, Mode 3); `gfx4snes` y sus opciones; PNG indexado; índice 0
  transparente; DMA; deduplicación de tiles, volteos incluidos (por qué los
  2080×256 px de la capa cercana del tutorial caben en unos 250 tiles: el arte
  se pega alineado a la rejilla de 8×8 y las letras de los carteles ocupan un
  tile cada una, así que se comparten).
- **Archivos:** `README.md`, `rom/` (mini-ROM que carga un fondo de 16
  colores con `bgInitTileSet` + `bgInitMapSet`), `ejercicio_06.md` (cambiar
  un color de la paleta en tiempo de ejecución y cambiar un tile del mapa).
- **Código del juego:** `src/sistema/comun.c` (`fondo16_cargar`,
  `fondo256_cargar`, `fuente_2bpp`), `src/sistema/comun.h` (el reparto de la
  VRAM), `Makefile` (reglas `gfx4snes`), `data.asm`, `src/juego/hud_debug.c`
  (escribir un tilemap a mano; el **bit de prioridad** `0x2000` que pone el
  texto delante de la capa cercana), `res/nivel1/fondo_cerca.png` (la capa del
  tutorial: tierra variada con volteos de tile, que en el tilemap son gratis),
  `src/juego/fondo_nivel.c` (`fondo_nivel_cargar`: tiles y paleta con el SDK).
- **Prerrequisitos:** 03, 04. **Práctica:** mini-ROM + sobre el juego real
  (mirar los tiles y el mapa de la capa cercana en el Tilemap Viewer de Mesen
  y ver cómo cambian las columnas al avanzar).
- **Aprobación:** captura de la mini-ROM con el fondo, el color cambiado y el
  tile cambiado.

### Unidad 07 — Scroll, cámara y parallax

- **Objetivo:** mover el mundo escribiendo registros de scroll, con una
  cámara que sigue al protagonista y se detiene en los bordes.
- **Conceptos:** registro de scroll (`bgSetScroll`, doble escritura de 8
  bits); mapas de 64×32 en páginas (`-y`) y por qué el scroll da la vuelta a
  los 512 px; bucle sin costura (original + espejo: la capa lejana de 512 px se
  repite a lo largo de los 2048 del nivel); cámara como desplazamiento
  (`camara = x + 48 − 128`, 48 = media celda de 96); recorte (clamp) en `s16`
  (0..1792); parallax (capa lejana a la mitad); un nivel más ancho que el mapa
  de la VRAM: el **mapa-anillo** (la columna `c` va a la posición
  `((c & 32) << 5) | (c & 31)`; la copia por DMA se estudia en la unidad 20);
  el truco de HDMA para el cielo.
- **Archivos:** `README.md`, `rom/` (mini-ROM con dos capas que se desplazan
  a distinta velocidad con la cruceta), `ejercicio_07.md` (añadir una tercera
  capa o cambiar la relación de parallax).
- **Código del juego:** `src/juego/camara.c` + `tests/prueba_camara.c`,
  `src/juego/nivel.h/.c` (`ANCHO_NIVEL` 2048, `ANCHO_CELDA`, `SUELO_Y`,
  `nivel_columna_vram`), `src/pantallas/pantalla_nivel1.c` (paso 3),
  `src/pantallas/pantalla_titulo.c`, `scripts/preparar_fondo_nivel.py` (por
  qué 512 px y 256 de alto), `src/juego/fondo_nivel.c` (qué columnas se
  escriben en cada VBlank).
- **Prerrequisitos:** 06. **Práctica:** mini-ROM + PC (`prueba_camara.c`).
- **Aprobación:** captura con las capas desplazadas y una prueba nueva en
  `prueba_camara.c` para un ancho de nivel distinto.

### Unidad 08 — Sprites estáticos y la OAM

- **Objetivo:** poner un personaje en pantalla y animarlo cambiando de tile.
- **Conceptos:** sprite (OBJ); OAM (128 entradas: x, y, tile, paleta,
  prioridad, volteo); tamaños (`OBJ_SIZE32_L64`); el límite de 512 tiles y
  **las dos tablas de 256**: un sprite grande no puede cruzar de una a otra
  (al bajar de fila la PPU suma 16 al número de tile y da la vuelta dentro de
  su tabla); cómo empaqueta `gfx4snes -s 64` los bloques (fórmula del tile);
  paletas de sprites (CGRAM 128..255); esconder un sprite (y por qué
  `y = 240` falla).
- **Archivos:** `README.md`, `rom/` (mini-ROM con un sprite de 64×64 que
  cambia de cuadro cada 8 frames y se mueve con la cruceta; base:
  `tools/pvsneslib/snes-examples/graphics/Sprites/SimpleSprite`),
  `ejercicio_08.md` (añadir un segundo sprite con otra paleta, y provocar a
  propósito el cruce de tablas poniéndolo en el tile 192 para verlo
  corromperse en Mesen).
- **Código del juego:** `src/pantallas/personajes.c` (`tile_del_bloque64`,
  `personajes_ocultar`, el bloque «LÍMITE» de la cabecera: Tarou en 2 cuadros
  de 96×64 = 192 tiles en la primera tabla y Bárbara en la segunda, tile 256;
  con Bárbara en el 192 se corrompía, caso verificado el 2026-10-19),
  `docs/hardware-snes-por-modulo.md` §3 y §5.
- **Prerrequisitos:** 06. **Práctica:** mini-ROM.
- **Aprobación:** captura con los dos sprites, la fórmula del tile verificada
  en el Tile Viewer de Mesen y la captura del cruce de tablas.

### Unidad 09 — Metasprites y sprites dinámicos

- **Objetivo:** superar el límite de 512 tiles: un personaje grande con muchos
  cuadros cuyo cuadro actual se copia a la VRAM en cada cambio.
- **Conceptos:** metasprite (varias piezas que se mueven como una);
  `gfx4snes -T` y la tabla `t_metasprite` (`dx`, `dy`, `dtile`, atributos);
  sprites dinámicos: el motor del SDK (`oamInitDynamicSprite`,
  `oamMetaDrawDyn32`, `oamVramQueueUpdate`, el de la mini-ROM) frente al
  propio del juego (una entrada de la OAM por pieza con `oamSet`, el plan de
  copias de `metasprite_copias` y la lista de DMA); el **presupuesto del
  VBlank** (3 KB de tiles son 18 líneas de DMA de las 37) y el **doble búfer**
  (dos celdas en la VRAM, medio cuadro por VBlank, el dibujo un frame por
  detrás); espejo horizontal pieza a pieza (`dx → 64 − dx` y `hflip`, mismo
  bloque; hasta 6 piezas: `gfx4snes -T` omite las transparentes y la tabla
  termina en `metasprite_end`); cómo guarda la tira los bloques (de 4 en 4 por
  fila de 128 px, como la hoja de sprites de la VRAM); presupuesto de ROM por
  cuadro (3 KB) y por sección (10 cuadros); por qué el primer cuadro se copia
  a VRAM antes del fundido.
- **Archivos:** `README.md`, `rom/` (mini-ROM con un metasprite de 96×64 de
  dos animaciones, con espejo; base:
  `tools/pvsneslib/snes-examples/graphics/Sprites/DynamicEngineMetaSprite`),
  `ejercicio_09.md` (añadir una animación y medir en el Sprite Viewer las
  6 piezas).
- **Código del juego:** `src/juego/metasprite.c` (+ `tests/prueba_metasprite.c`,
  con `metasprite_copias`), `src/juego/tarou_sprite.c` (las dos celdas,
  `apuntar_copias`, `HUNDIR_PIES`: el dibujo va 2 px por debajo de la física),
  `src/juego/dma_lista.h`, `data.asm` (`.rodata_tarou_*`, `tarou_til_bancos`),
  `Makefile` (regla patrón de Tarou, `-X 96 -Y 64`),
  `src/pantallas/pantalla_nivel1.c` (el bucle `tarou_sprite_al_dia` en
  `_iniciar`), `docs/diseno-tutorial-movimiento.md` §7 y §8 (el fallo de Tarou
  a trozos y sus números).
- **Prerrequisitos:** 08. **Práctica:** mini-ROM + sobre el juego real.
- **Aprobación:** captura con el metasprite en ambas direcciones.

### Unidad 10 — Física de plataformas: velocidad, aceleración, gravedad y salto

- **Objetivo:** que un personaje se mueva «como en un Mario»: aceleración
  y frenado suaves, salto de altura variable, velocidad terminal, aterrizaje.
- **Conceptos:** velocidad y aceleración por frame; integrar
  (`posición += velocidad`); frenado; gravedad; apogeo; corte del salto al
  soltar el botón; velocidad terminal; control aéreo; orden de las
  operaciones dentro del frame; cajas de colisión (cuerpo y golpe) medidas
  sobre el arte; afinar constantes («a afinar») con el HUD.
- **Archivos:** `README.md`, `demo_10_salto.c` (PC: tabla de altura por
  frame para distintas `VEL_SALTO`), `ejercicio_10.md` (sobre el juego real:
  cambiar `GRAVEDAD` y `VEL_SALTO`, actualizar las pruebas y medir la altura
  en Mesen con el HUD).
- **Código del juego:** `src/juego/tarou.c` (`saltar`, `actualizar_aire`,
  `aterrizar`, el paso 2 de `tarou_actualizar` (gravedad y corte) y la parte de
  «integrar» de `mover_x`/`mover_y`; los choques se ven en la unidad 20,
  `tarou_caja_cuerpo`, `tarou_caja_golpe`), `tests/prueba_tarou.c` (salto y
  cajas, sobre un escenario plano: la fila 13 de una rejilla hecha en la
  prueba, `colision_usar`), `docs/diseno-demo-jugable.md` §5.
- **Prerrequisitos:** 01, 02, 05. **Práctica:** PC + sobre el juego real.
- **Aprobación:** `tests/pruebas.exe` en verde con las constantes nuevas y
  una captura del HUD con la nueva altura máxima (`Y` mínima). Misma trampa
  que en la unidad 02 con `tests/tarou_anim.h`. Y otra: cambiar el salto puede
  hacer imposible el muro de 48 px o el foso del tutorial; las pruebas de
  `tests/prueba_colision.c` y `prueba_recorrido.c` lo dirán (es parte del
  ejercicio explicar por qué).

### Unidad 11 — Animación por estados

- **Objetivo:** que cada estado muestre su animación con su ritmo, con bucle
  o sin él, y que el cambio de estado reinicie el cuadro.
- **Conceptos:** cuadro; frames por cuadro; bucle; animación terminada
  (`anim_terminada`) como disparador de transición; tabla de animaciones
  indexada por estado; el header generado `tarou_anim.h`; duración fija con
  división entre constantes (`DURACION_ATAQUE / cuadros`, entera y resuelta
  por el compilador).
- **Archivos:** `README.md`, `ejercicio_11.md` (sobre el juego real, con un
  problema que existe de verdad: con el arte de Pixellab, desenvainar y
  envainar (9 cuadros × 5 frames) duran unos 0,75 s cada una y despertar
  (10 × 8) unos 1,3 s; ajustar sus frames por cuadro en `ANIMACIONES`,
  ajustar las cuentas de `tests/prueba_tarou.c` y `tests/prueba_animacion.c`
  si cambian, y comprobar en Mesen que el gesto se ve natural).
- **Código del juego:** `src/juego/animaciones.c/.h`, `avanzar_animacion` en
  `src/juego/tarou.c`, `scripts/preparar_personaje.py` (de dónde salen los
  cuadros), `res/personajes/tarou/tarou_anim.h` (el real) frente a
  `tests/tarou_anim.h` (la copia fija de las pruebas).
- **Prerrequisitos:** 02, 09. **Práctica:** sobre el juego real.
- **Aprobación:** pruebas en verde y captura con el nuevo ritmo.

### Unidad 12 — HDMA y efectos de la PPU

- **Objetivo:** cambiar un registro de la PPU en cada línea sin gastar CPU,
  y animar una pantalla con una tabla de eventos en vez de con esperas.
- **Conceptos:** HDMA (tabla de valores por línea); canales; HDMA
  **indirecto** (la tabla lleva un puntero; mover el efecto es cambiar 2
  bytes) y por qué lo que se escribe «en la línea n» se ve en la n+1;
  `COLDATA`; **ventanas** (`WH0`/`WH1`, una diagonal es `izq = base − línea`);
  **matemática de color** (`CGWSEL`, `CGADSUB`: sumar un color fijo solo
  dentro de la ventana); mosaico (`(tamaño << 4) | fondos`); brillo y
  fundidos (`setFadeEffectEx` y su cuenta de frames por paso); por qué los
  fundidos bloquean; brillo 0 frente a pantalla apagada (forced blank,
  `setScreenOff`: `fade_out` deja la pantalla ENCENDIDA y las copias a VRAM
  fuera del VBlank se pierden, el fallo del título tras el nivel); el **motor
  de escenas**: tabla de eventos ordenada, pistas en punto fijo 8.8, paso
  calculado por el compilador en la macro, valor exacto en el último frame,
  separación lógica pura / registros (`docs/diseno-motor-escenas.md`).
- **Archivos:** `README.md`, `rom/` (mini-ROM con un degradado de cielo por
  HDMA y un fundido), `ejercicio_12.md` (otra tabla: cielo de atardecer;
  ampliación: una escena nueva para el logo del publisher con el motor:
  escribir la tabla, una prueba en `tests/` que compruebe su duración y que
  acaba apagada, y verla en Mesen).
- **Código del juego:** `src/escena/escena.c` (el secuenciador: `RAMPA`,
  `FIJAR`, `FIN_ESCENA`, `escena_avanzar`), `src/escena/efectos.c` (los
  registros; `efectos_destello` arma la lista de pares y la tabla HDMA
  indirecta del canal 6), `src/escena/escena_nintendo.c` (la tabla del logo:
  mosaico que se enfoca, destello, fundidos, 244 frames),
  `src/pantallas/pantalla_nintendo.c` (cómo una pantalla usa el motor:
  `_iniciar`/`_actualizar`/`_vblank`), `tests/prueba_escena.c`,
  `src/pantallas/pantalla_titulo.c` (HDMA canal 6 directo, el cielo),
  `res/titulo/cielo.bmp` + regla `-n`, `src/pantallas/pantalla_intro.c`
  (mosaico: la salida de la intro con `setMosaicEffect` que parece un fallo
  y no lo es), `src/sistema/comun.c` (`fade_in`, `fade_out`),
  `src/sistema/secuencia.c` (`setScreenOff` antes de cada `_iniciar`),
  `src/pantallas/pantalla_nivel1.c` (`reaparecer`: fundido, pantalla apagada,
  fundido).
- **Prerrequisitos:** 01, 04, 06. **Práctica:** mini-ROM + sobre el juego real.
- **Aprobación:** captura con el degradado nuevo; la escena del ejercicio
  pasa su prueba en el PC y se ve en Mesen sin dejar la ventana ni la luz
  encendidas en la pantalla siguiente.

### Unidad 13 — Memoria: VRAM, CGRAM, OAM, WRAM y ROM por bancos

- **Objetivo:** saber dónde vive cada byte y leer el `.log` y el `.sym`.
- **Conceptos:** mapa de memoria del 65816 (bancos `$7E/$7F` = WRAM; LoROM;
  FastROM: la ROM en `$80-$FF`, y por qué 128 bancos SlowROM pisarían la
  WRAM); la WRAM **no** se pone a cero al arrancar (una `static` sin
  inicializar vale basura);
  sección `superfree`; `.incbin`; etiqueta; por qué una sección no puede
  cruzar 32 KB (y por eso 10 cuadros de 96×64 por animación); dirección de
  24 bits = banco + 16 bits, y cómo darle a C el banco de una etiqueta
  (`.db :etiqueta` en `data.asm`); dónde pone tcc816 las tablas `const`
  (WRAM); leer datos directamente de la ROM con un puntero (la rejilla de
  colisión); DMA frente a copia por CPU; el reparto de VRAM de `comun.h`.
- **Archivos:** `README.md` (resumen de `docs/hardware-snes-por-modulo.md`
  con ejercicios), `ejercicio_13.md` (localizar en Memory Tools de Mesen la
  `struct Tarou` usando el `.sym` y verla cambiar al caminar; calcular cuántos
  cuadros de 96×64 más caben en la ROM: hoy quedan 3 713 537 bytes de 4 MB
  (bancos `$8F..$FF` vacíos); localizar en el `.sym` `nivel1_colision` y
  `nivel1_cerca_col` y explicar por qué hace falta `nivel1_cerca_col_banco`).
- **Código del juego:** `data.asm` (`.rodata_nivel1_cerca`, `.rodata_nivel1_banco`,
  `.rodata_nivel1_colision`), `src/sistema/comun.h`, `src/juego/fondo_nivel.c`
  (`REG_A1T0LH` y `REG_A1B0`: la dirección y el banco del DMA),
  `ProyectoSNES.log`, `ProyectoSNES.sym`.
- **Prerrequisitos:** 03, 06, 08. **Práctica:** sobre el juego real.
- **Aprobación:** captura de Memory Tools con `tarou` y la cuenta de ROM libre.

### Unidad 14 — Ensamblador 65816 I: leer

- **Objetivo:** leer el ensamblador que generan `tcc816` y el SDK.
- **Conceptos:** registros (A, X, Y, S, P, DB, DP, PB); modo de 8/16 bits
  (`REP`/`SEP`); modos de direccionamiento (inmediato, directo, absoluto,
  largo, indexado, indirecto, relativo a la pila `n,s`); la pila
  (`PHA`/`PLA`, `JSR`/`RTS`, `JSL`/`RTL`); banderas; cómo pasa tcc816 los
  parámetros y devuelve valores; cómo se ve un `switch` y un bucle en
  ensamblador; la rutina de división del runtime (`tcc__div`) y por qué el
  tipo del operando decide si se usa; `data.asm` y `hdr.asm` como
  ensamblador puro.
- **Archivos:** `README.md`, `ejercicio_14.md` (con la receta de §6.4:
  anotar `camara.asm` y `nivel.asm` (en `nivel_columna_vram`, el `<< 5` sale
  como un bucle de 5 `asl` con `ldy #5`/`dey`/`bne`); en
  `src/pantallas/personajes.asm`, comparar `tile_del_bloque64` (parámetro
  `u8`: dos llamadas `jsr.l tcc__div`) con `tile_del_bloque32` (parámetro
  `u16`: ninguna) y explicar la diferencia; y reescribir a propósito
  `mantenidos` de `src/juego/guion.c` con `guion[i]`, generar su `.asm`,
  contar los `jsr.l tcc__mul` (structs de 5 bytes: `i × 5` en cada acceso),
  explicar por qué la versión con puntero no tiene ninguno y deshacer el
  cambio).
- **Código del juego:** los `.asm` generados, `data.asm`, `hdr.asm`, las
  rutinas del SDK que se ven en el depurador (`WaitForVBlank`, el manejador
  de NMI).
- **Prerrequisitos:** 03, 13. **Práctica:** lectura + depurador de Mesen.
- **Aprobación:** el `.asm` anotado y un breakpoint en `camara_para` con los
  registros interpretados.

### Unidad 15 — Ensamblador 65816 II: escribir

- **Objetivo:** escribir una rutina en `.asm` y llamarla desde C.
- **Conceptos:** convención de llamada de tcc816 (parámetros en la pila,
  valor en A / `tcc__r0`); secciones `.section ".text"`; `.export`/`extern`;
  `REP #$20` / `SEP #$20`; bucles con `DEX`/`BNE`; escribir un registro de la
  PPU a mano (`STA $2100`); medir ciclos con el profiler de Mesen.
- **Archivos:** `README.md`, `rom/` (mini-ROM con `src/relleno.asm`: una
  rutina que rellena una zona de VRAM con un tile, llamada desde C; OJO: el
  `.asm` no puede llamarse como ningún `.c` de la ROM, `make` lo sobrescribiría;
  y con FastROM una `RAMSECTION` necesita `.BASE $00` delante y `.BASE $80`
  detrás, porque WLA-DX suma la base también a los bancos de RAM),
  `ejercicio_15.md` (reescribir `numero` del HUD en ensamblador y comparar
  ciclos; de paso, ver en el `.asm` generado por qué un bucle `*p++ = 0` en C
  son ~60 instrucciones por vuelta y `memset` de la librería, 3; ampliación: `colision_en` de `src/juego/colision.c`, la
  consulta más repetida de la física, manteniendo la versión C para el PC y
  comprobando con la ROM de prueba que el recorrido sigue igual).
- **Código del juego:** `src/juego/dma_ejecutar.asm` (**la rutina en
  ensamblador del juego**: lee una lista en la WRAM y dispara un DMA por
  entrada; `php`/`rep`/`sep`, `lda.l`/`sta.l`, `_bucle` local, `rtl`; su
  cabecera explica la convención de llamada), `src/juego/dma_lista.h` (cómo la
  ve C: `extern` de variables definidas en el `.asm`), `src/juego/hud_debug.c`
  (lo que se reescribe), `data.asm`.
- **Prerrequisitos:** 14. **Práctica:** mini-ROM + sobre el juego real.
- **Aprobación:** la mini-ROM muestra el relleno; la versión en ensamblador
  da los mismos dígitos que la de C y menos ciclos en el profiler.

### Unidad 16 — Mesen: depurador, puntos de ruptura y visores

- **Objetivo:** diagnosticar un fallo gráfico o de física sin adivinar.
- **Conceptos:** breakpoint (de ejecución, de lectura, de escritura);
  paso a paso y paso a frame; etiquetas del `.sym`; visores de tilemap,
  tiles, sprites y paletas; Event Viewer; capturas automáticas con guion y
  sus límites.
- **Archivos:** `README.md` (resumen de `docs/mesen-y-depuracion.md` con
  ejercicios), `ejercicio_16.md` (cuatro casos reales del proyecto: un sprite
  corrompido por cruzar la tabla de tiles (Bárbara en el tile 192); un
  personaje que parece flotar sobre el suelo (perspectiva: el remedio fue
  hundir los pies 4 px, hoy 2 del arte y 2 de `HUNDIR_PIES`); el texto del
  HUD tapado por el techo del túnel (bit de prioridad de BG3, visible en el
  Tilemap Viewer); y una tecla del guion que no llega (el foco de Mesen salta
  a la barra de menús si se usa el equipo durante la tanda; la salida fiable
  es la ROM de prueba con guion): encontrar la causa con la herramienta
  adecuada).
- **Código del juego:** `scripts/capturar_mesen.ps1`, la ROM de prueba
  (`-DGUION_PRUEBA`, `src/juego/guion.c`), el HUD de depuración,
  `docs/mesen-y-depuracion.md` §5 y §6 (fallos conocidos). Quinto caso, el
  mejor: Tarou a trozos al jugar (2026-10-20): se encontró MIDIENDO (líneas
  del VBlank al entrar y salir de `_vblank`), no mirando; está contado en
  `docs/diseno-tutorial-movimiento.md` §8.
- **Prerrequisitos:** 05, 08, 13. **Práctica:** sobre el juego real.
- **Aprobación:** los cuatro casos resueltos con la captura que lo demuestra.

### Unidad 17 — SPC700 y audio

- **Objetivo:** entender el segundo procesador y el driver de audio del SDK
  (sin componer: la música es de otra sesión).
- **Conceptos:** SPC700 (su RAM de 64 KB, su juego de instrucciones, su
  DSP de 8 voces); los puertos `$2140-$2143`; cómo `spcBoot` carga el driver;
  soundbank y `smconv`; módulos `.it`; BRR (`snesbrr`) para efectos;
  `spcProcess` cada frame; por qué el audio sigue sonando mientras la CPU
  hace otra cosa.
- **Archivos:** `README.md`, `rom/` (mini-ROM que reproduce
  `examples/likemario/overworld.it` y dispara `examples/likemario/mariojump.brr`
  con un botón; base: `tools/pvsneslib/snes-examples/audio/`),
  `ejercicio_17.md` (leer el ensamblador SPC700 del driver y anotar el bucle
  principal).
- **Código del juego:** `src/audio/audio.h` (el contrato previsto),
  `examples/likemario/src/LikeMario.c` (inicialización de audio),
  `examples/likemario/soundbank.asm`.
- **Prerrequisitos:** 14. **Práctica:** mini-ROM + lectura.
- **Aprobación:** la mini-ROM suena en Mesen (música y efecto).

### Unidad 18 — Herramientas: gfx4snes, scripts de Python y el Makefile

- **Objetivo:** modificar el pipeline de assets con seguridad.
- **Conceptos:** PNG indexado y paleta; cuantización (median cut, sin
  tramado); `gfx4snes` (opciones `-s -o -u -e -m -y -T -X -Y -P -n`; sin `-y`,
  el mapa sale por filas); un paso de Python dentro del `Makefile`
  (`mapa_a_columnas.py`); por qué `data.obj` depende de `ASSETS` y los `.obj`
  que incluyen archivos generados dependen de ellos (`nivel1_datos.h`); reglas
  de patrón de `make`; `CFLAGS` desde el entorno (`-DGUION_PRUEBA`); pruebas
  `unittest` de los scripts y TDD (prueba en rojo antes que el código).
- **Archivos:** `README.md`, `ejercicio_18.md` (añadir una animación nueva a
  Tarou de principio a fin: entrada en `MAPEOS_PIXELLAB` (o una tira PNG) →
  `preparar_personaje.py` → `TAROU_ANIMS` del Makefile → `data.asm` →
  `animaciones.c` → `tarou_sprite.c` → verla en Mesen).
- **Código del juego:** `scripts/preparar_personaje.py`,
  `scripts/preparar_nivel.py`, `scripts/mapa_a_columnas.py`,
  `scripts/preparar_fondo_nivel.py` (y `preparar_suelo.py`, sustituido pero
  conservado) con sus pruebas en `scripts/pruebas/`, `Makefile`, `data.asm`,
  `docs/tuberia-de-compilacion.md` §3, §5 y §8.
- **Prerrequisitos:** 09, 11, 13. **Práctica:** sobre el juego real.
- **Aprobación:** la animación nueva se ve en Mesen y las 35 pruebas de los
  scripts siguen en verde.

### Unidad 19 — Del arte al juego: Pixellab, celdas, paletas y perspectiva

- **Objetivo:** llevar arte generado fuera (Pixellab) a la SNES sin
  estropearlo, y saber qué decisiones técnicas impone el hardware.
- **Conceptos:** exportación nativa de Pixellab (un estado por carpeta, 8
  direcciones, lienzos de 76..104 px según el estado, `metadata.json`);
  mapeo de estados a animaciones del juego (tramos, inversión, pose fija);
  anclaje por centro del lienzo y pies (nunca por la esquina); por qué no se
  escala el pixel art (vecino más cercano con factor no entero = píxeles
  desiguales); la decisión de celda 96×64 y sus costes (6 piezas, 3 KB por
  cuadro, 10 cuadros por banco, 2 cuadros en la carga estática); una paleta
  de 15 colores compartida; tileset *Sidescroller* de **rejilla doble** (cada
  celda de arte se clasifica por cuáles de sus 4 esquinas tienen tierra: 16
  combinaciones; las diagonales faltan y se sustituyen); vestir una rejilla de
  lógica con él, desplazado media celda para que todo caiga en la rejilla de
  8×8; variar un relleno con volteos; el presupuesto de tiles (el script se
  niega a pasar de 460); componer capas con paleta compartida (el seto, los
  carteles con la fuente de consola); **perspectiva**: el arte es una vista
  lateral un poco elevada y por eso los pies se hunden 4 px en la banda de
  hierba (2 px del arte y 2 de `HUNDIR_PIES`).
- **Archivos:** `README.md`, `ejercicio_19.md` (tres partes sobre el juego
  real: cambiar `HUNDIR_PIES` a 0 y a 4 y comparar capturas; integrar una de
  las piezas que faltan (la zarza, el cartel o el tronco de la lista de
  `res/descargas/graficos/pixellab/suelo/README.md`) en lugar de su marcador en
  `preparar_nivel.py`, vigilando colores y tiles; añadir al mapeo una animación
  nueva de Pixellab, por ejemplo el ataque cuando exista, y verla en Mesen).
- **Código del juego:** `scripts/preparar_personaje.py` (`MAPEOS_PIXELLAB`,
  `anclar`, `cargar_pixellab`, `paleta_comun`), `scripts/preparar_nivel.py`
  (`piezas_por_esquinas`, `vestir`, `poner_seto`, `poner_cartel`,
  `tiles_distintos`), `scripts/preparar_suelo.py` (`clasificar`: la versión
  anterior, de suelo plano), `res/descargas/graficos/pixellab/tarou/README.md`
  y `res/descargas/graficos/pixellab/suelo/README.md`,
  `docs/diseno-demo-jugable.md` §2 (decisiones 3 y 16), §7 y §10,
  `docs/diseno-tutorial-movimiento.md` §3 y §7.
- **Prerrequisitos:** 06, 09, 18. **Práctica:** sobre el juego real + PC
  (pruebas `unittest`).
- **Aprobación:** las 35 pruebas de scripts en verde, `tests/pruebas.exe` en
  verde y las capturas de Mesen de cada parte del ejercicio.

### Unidad 20 — Colisión con el escenario (código listo, unidad por escribir)

Desde el 2026-10-20 el código existe (`docs/diseno-tutorial-movimiento.md`):
el tutorial de movimiento. No usa `src/niveles/` ni el motor de mapas del SDK.

- **Conceptos:** rejilla de colisión (celdas de 16 px, índice con
  desplazamientos), AABB, resolución por ejes, plataforma de un solo sentido,
  punto de control, streaming de columnas (mapa-anillo, DMA con
  `VMAIN = 0x81`, apuntado en la lista de DMA solo cuando la cámara cambia de
  columna), rejilla doble (*dual grid*), guion de mando, y el coste de las
  llamadas en tcc816 (por qué existen `colision_columna_solida` y compañía).
- **Código del juego:** `src/juego/colision.c` (la rejilla, el bloque y las
  consultas de columna y fila), `src/juego/tarou.c` (`mover_x` con el
  retroceso píxel a píxel, `mover_y` con los dos pies, `cabe_de_pie`),
  `src/juego/partida.c` (caída, zarzas, puntos de control, tronco, salida),
  `src/juego/fondo_nivel.c` (el anillo y el parche del tronco),
  `src/juego/guion.c` y `guion_tutorial.c`, `src/pantallas/pantalla_nivel1.c`
  (`reaparecer`), `src/pantallas/pantalla_proximamente.c`,
  `scripts/preparar_nivel.py`, `res/nivel1/tutorial.tmj`,
  `tests/prueba_colision.c`, `tests/prueba_partida.c`,
  `tests/prueba_recorrido.c`, `docs/diseno-tutorial-movimiento.md` (sobre todo
  §7 y la tabla de trampas de §8).
- **Demo:** en el PC, una rejilla pequeña hecha a mano y un Tarou que cae,
  aterriza, choca y se agacha bajo un techo, imprimiendo x/y por cuadro (como la
  traza con la que se ajustó el guion).
- **Ejercicio:** añadir un obstáculo al nivel en Tiled (instalarlo con
  `winget install Tiled.Tiled`; por ejemplo un segundo foso), regenerar con
  `preparar_nivel.py` (¡sin `--crear`!), `make clean` + `make`, ver fallar
  `prueba_recorrido` y ajustar `guion_tutorial.c` con una traza de x/y por
  cuadro hasta que pase; luego verlo en la ROM de prueba.
- **Prerrequisitos:** 07, 10, 13, 18. **Práctica:** PC + juego real + Mesen
  (ROM de prueba, `docs/mesen-y-depuracion.md` §5).
- **Aprobación:** `tests/pruebas.exe` en verde con el obstáculo nuevo y
  capturas de la ROM de prueba superándolo.

## 4. Orden y dependencias

```
00 C ──► 01 punto fijo ──► 02 estados ──► 10 física ──► 11 animación ──► 18 herramientas ──► 19 arte
 │                                             ▲              ▲                 ▲              ▲
 └──► 03 tubería ──► 04 frame ──► 05 mando ────┘              │                 │              │
          │             │                                      │                 │              │
          │             └──► 06 fondos ──► 07 cámara           │                 │              │
          │                      │  └──────────────────────────┼─────────────────┼──────────────┘
          │                      ├──► 08 sprites ──► 09 metasprites ────────────┤
          │                      └──► 12 HDMA                                    │
          └──► 13 memoria ──► 14 asm leer ──► 15 asm escribir                    │
                   │  │              └──► 17 SPC700                              │
                   │  └──────────────────────────────────────────────────────────┘
                   └──► 16 Mesen
(20 colisión: código listo; después de 07, 10, 13 y 18)
```

## 5. Cobertura de los ítems técnicos del plan de desarrollo

| Ítem de `docs/plan-desarrollo.md` (Etapas 0–2) | Unidad |
|---|---|
| Etapa 0: C básico, tipos, funciones, arrays, binario/hexadecimal | 00, 01 |
| Etapa 0/1: la tubería PVSnesLib, Makefile, estructura de una ROM | 03, 13 |
| Etapa 1: tiles, paletas, tilemaps, modos de vídeo, gfx4snes | 06, 18 |
| Etapa 1: sprites y OAM | 08, 09 |
| Etapa 1: HDMA, mosaico, fundidos | 12 |
| Etapa 1: audio SPC700, soundbank, smconv | 17 |
| Etapa 2: el loop principal y VBlank/NMI | 04 |
| Etapa 2: lectura de input (joypad) | 05 |
| Etapa 2: aritmética de punto fijo | 01 |
| Etapa 2: física de plataformas 2D | 10 |
| Etapa 2: detección y resolución de colisiones (AABB, tiles) | 20 (el código existe desde el tutorial, 2026-10-20) |
| Etapa 2/3: un nivel de principio a fin (inicio → meta), editor de mapas (Tiled) | 20 |
| Etapa 2: máquinas de estado para el personaje | 02, 11 |
| Etapa 2: scroll de cámara y registros de BG | 07 |
| Etapa 2: debugging con el debugger de Mesen | 16 |
| Integración del arte (Pixellab → ROM) | 19 |

## 6. Cómo se ejecuta este plan (para la sesión que escribe las unidades)

### 6.1 Definición de «hecho» de una unidad

Una unidad está hecha cuando:

1. Existe `aprendizaje/unidad-NN-tema/README.md` con: objetivo, conceptos
   (cada término definido en una frase la primera vez, con `→ glosario: X`
   cuando el término ya está en `docs/glosario-snes.md`; si no está y es del
   proyecto, se añade al glosario), cómo funciona, dónde está en el juego
   (rutas y nombres de función exactos), el ejercicio y el criterio de
   aprobación.
2. Existen la demo y el ejercicio (con TODO numerados) y la carpeta
   `solucion/` con la solución completa.
3. Si la unidad pide `rom/`, la mini-ROM **compila** (receta de §6.3) y se
   ha visto en Mesen.
4. El criterio de aprobación **se ha ejecutado de verdad** con la solución:
   la salida de consola, la prueba en verde o la captura de Mesen existen.
   Las capturas de referencia se guardan en `unidad-NN-tema/capturas/`.
5. La tabla de §7 está marcada y hay un commit (sin push salvo que el autor
   lo pida).

### 6.2 Convenciones (las del proyecto, `CLAUDE.md`)

- Todo `.c`, `.h` y `.asm` nuevo abre con la **cabecera estándar**
  (`docs/diseno-demo-jugable.md` §12): propósito, se conecta con, hardware
  que toca, variables, glosario.
- **Cada línea de código lleva su comentario en español**; documentación en
  español; el **texto que sale en pantalla, en inglés**.
- En el código para la SNES: sin `int`, `long`, `float`, `bool`; sin
  divisiones en código por frame; sin `a[i]` sobre arrays de structs de
  tamaño no potencia de 2 en bucles por frame (`tcc__mul`: recorrer con
  puntero); sin `?:` con cadenas como argumentos variádicos (error interno de
  tcc816); `PF_ENTERO` solo con valores ≥ 0.
- Los ejemplos de PC compilan con `gcc -std=c99 -Wall -Wextra` sin avisos.
- No se toca nada de `tools/` (SDK y Mesen de terceros).

### 6.3 Receta de mini-ROM (verificada el 2026-10-19)

1. Copiar a `unidad-NN-tema/rom/` el `Makefile` y la carpeta `src/` de un
   ejemplo del SDK (`tools/pvsneslib/snes-examples/hello_world` para la más
   simple; `graphics/Sprites/SimpleSprite`, `graphics/Sprites/DynamicEngineMetaSprite`,
   `input/`, `audio/`... según la unidad). Cambiar `ROMNAME` y `ROMTITLE`.
   Si hay assets, añadir sus reglas `gfx4snes` y su `data.asm` como en el
   `Makefile` del proyecto.
2. Compilar **desde PowerShell** (el Bash de Claude Code no puede ejecutar
   `wla-65816`), dentro de la carpeta `rom/`:
   ```
   $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
   ```
   Resultado esperado: `No warnings or errors found.` y un `.sfc`.
3. Abrir el `.sfc` en `tools/mesen/Mesen.exe`. Los artefactos (`.sfc`,
   `.obj`, `hdr.asm`, `linkfile`, `.sym`, `.log`) ya los ignora `.gitignore`.
4. Las mini-ROM viven fuera de `src/`, así que el `make` del proyecto no las
   compila.

### 6.4 Receta para ver el ensamblador de un `.c` del juego (verificada)

```
make src/juego/camara.asm            (desde la raíz, con el entorno de §6.3)
```

`make` deja el `.asm` cuando se pide como objetivo explícito (en una
compilación normal es un intermedio y lo borra). **Copiarlo a la unidad y
borrarlo de `src/` enseguida**: `snes_rules` compila como fuente todo
`src/*/*.asm`, y un `camara.asm` olvidado se ensamblaría junto a `camara.c`.

### 6.5 Comandos de comprobación

```
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
python -m unittest scripts/pruebas/prueba_preparar_personaje.py scripts/pruebas/prueba_preparar_fondo_nivel.py scripts/pruebas/prueba_preparar_suelo.py scripts/pruebas/prueba_preparar_nivel.py
.\scripts\capturar_mesen.ps1 -Duracion 70 -Cada 1 -Espera 4 -Guion "55,55.3,Start;57,57.3,Select;58,61,Right"
```

ROM de prueba que se juega sola (lo fiable para verificar el nivel: no
necesita teclas; sale con el mismo nombre, así que se copia aparte y se
vuelve a compilar la normal):

```
make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null
.\scripts\capturar_mesen.ps1 -Rom RUTA\tutorial_guion.sfc -Duracion 46 -Cada 2 -Espera 3 -Guion "1,1.3,Select"
```

Valores de referencia hoy: **280** pruebas de PC y **35** de scripts en
verde; el título aparece hacia el segundo 50 de la captura; en la ROM de
prueba el nivel termina a los ~37,5 s. Para capturas con el guion de teclas,
**no usar el equipo durante la tanda**: con el equipo en uso el foco de Mesen
salta a su barra de menús y se pierden teclas (en la sesión del 2026-10-20 se
perdieron en todas las tandas).

### 6.6 Trampas conocidas que una unidad no debe enseñar mal

- `tests/tarou_anim.h` es una copia fija con los cuadros de los marcadores;
  el arte real tiene otros (unidades 02, 10, 11).
- Las cifras de `LIN` pueden estar en 225..261 (la lógica empieza dentro del
  VBlank) o haber dado la vuelta. Desde el 2026-10-21 el HUD encendido cuesta
  ~6 frames en todo el recorrido; si una fila no se refresca en un frame es
  el tope `HUD_LINEA_MAX`, no un fallo (unidades 04, 16).
- `tests/prueba_colision.c`, `prueba_partida.c` y `prueba_recorrido.c` usan la
  rejilla REAL del nivel (`res/nivel1/nivel1_datos.h`): retocar el nivel en
  Tiled las rompe a propósito (unidades 10, 20).
- El efecto mosaico de salida de la intro no es un fallo (unidad 12).
- Un sprite grande no cruza la frontera de los 256 tiles (unidades 08, 16).
- Los pies quedan 4 px dentro de la hierba a propósito: desde el tutorial,
  2 px los pone el arte (la banda empieza 2 px sobre el borde de la celda) y
  2 px el dibujo (`HUNDIR_PIES` en `tarou_sprite.c`); la física usa el borde
  exacto (unidades 07, 19, 20).
- `PF_ENTERO` es solo para valores ≥ 0: una y negativa (un salto por encima de
  la pantalla) da un número enorme. Comparar en punto fijo (unidades 01, 20).
- Indexar un array de structs de tamaño no potencia de 2 (`guion[i]`, 5 bytes;
  `copias[i]`, 6 bytes) hace que tcc816 llame a `tcc__mul` en cada acceso:
  recorrer con un puntero o rellenar la struct a 8 bytes (unidades 09, 13, 14,
  20). Y `<< n` dentro de un bucle caliente es un bucle de n vueltas: sumar en
  vez de desplazar.
- Lo que se escribe en la VRAM después de la línea 37 del VBlank se pierde sin
  aviso: el HUD lo mide (`V a b`) y Tarou a trozos fue eso (unidades 04, 09, 16).
- Un `.asm` propio no puede llamarse como un `.c`: `make` lo sobrescribe con
  el intermedio del compilador (unidades 03, 15).
- Las tablas de Tiled en JSON para `tmx2snes`: sin espacio tras los dos puntos
  (unidad 20).
- `fade_out` termina con brillo 0 pero la pantalla ENCENDIDA: cargar la VRAM
  después, fuera del VBlank, pierde datos. `secuencia_cambiar` apaga la
  pantalla antes de cada `_iniciar` (unidades 04, 12).
- Con FastROM, un `.asm` a mano con `RAMSECTION` lleva `.BASE $00` delante y
  `.BASE $80` detrás; si no, sus variables «viven» en la ROM (unidades 13, 15).
- Las `static` de C NO empiezan en 0: la WRAM arranca con basura y tcc816 no
  limpia `.bss`. Inicializar siempre (unidades 00, 13).
- Un bucle de relleno en C (`*p++ = 0`) cuesta ~60 instrucciones por vuelta
  en tcc816; `memset` de la librería es ensamblador (unidades 14, 15).
- El HDMA escribe en el HBlank del final de la línea: una tabla que empieza
  «en la línea 64» se ve desde la 65 (unidad 12).
- `255 * 256` no cabe en el `int` de 16 bits de tcc816: las macros con
  constantes grandes se reparten (`* 128 ... * 2`) (unidades 01, 12).

### 6.7 Orden de trabajo recomendado

Una unidad por sesión, en el orden 00 → 20 (respetando §4). Las unidades
00, 01 y 02 son de PC y cortas: pueden ir de dos en dos. Antes de escribir
una unidad, releer los archivos de «Código del juego» que cita: si algo
cambió desde este plan, manda el código y se corrige el plan.

## 7. Estado

| Unidad | Estado |
|---|---|
| 00 | **hecha** (2026-10-19): `unidad-00-c-en-el-pc/`; las 12 fuentes compilan con `gcc -std=c99 -Wall -Wextra` sin avisos y las 4 soluciones dan exactamente su salida esperada (`capturas/`). El autor tiene pendiente resolver el ejercicio 03 (y el 04) por su cuenta |
| 01–19 | por escribir (siguiente: 01). Fichas revisadas el 2026-10-20 contra el código del tutorial |
| 12 | por escribir: el motor de escenas existe desde el 2026-10-21 (`src/escena/`); ficha en §3 actualizada |
| 20 | por escribir: el código existe desde el 2026-10-20 (el tutorial); ficha en §3 |
