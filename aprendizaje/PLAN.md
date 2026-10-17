# Plan de la carpeta `aprendizaje/`

**Para quién:** un programador junior que va a heredar este proyecto y tiene
que entender todo el código desplegado, de lo más simple a lo más complejo,
con el foco en programar juegos de Super Nintendo en C y ensamblador.
**Qué es:** el plan de las unidades de estudio. Cada unidad enseña un
concepto, lo ancla en un archivo real del juego y se aprueba con una
práctica. **Este documento es el plan; las unidades se escriben después,
una por sesión**, siguiendo el formato de §2.

Alcance de esta tanda: mecánicas básicas, C y ensamblador (65816 y SPC700).
Arte, diseño de niveles, música, sonido y guion se planifican en otra sesión.

## 1. Cómo se usa

1. Las unidades se hacen **en orden**: cada una da por sabido lo anterior
   (columna «Prerrequisitos»).
2. Antes de empezar una unidad se lee su `README.md` (la teoría, con todos
   los términos definidos); después se ejecuta la demo, se resuelve el
   ejercicio y se comprueba contra la solución.
3. Cada unidad termina mirando el **código real del juego** que la aplica:
   la columna «Código del juego» dice qué archivo abrir y qué buscar.
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
├── demo_NN_*.c       programa de ejemplo que se ejecuta y se lee
├── ejercicio_NN_*.c  esqueleto con TODO numerados para completar
├── solucion/         la solución del ejercicio (se mira después, no antes)
└── rom/              solo en unidades de hardware: una mini-ROM con su Makefile
    ├── Makefile      copia reducida del Makefile del proyecto
    ├── data.asm      los assets que use (si los hay)
    └── src/*.c       el programa de la mini-ROM, con la cabecera estándar
```

Tres tipos de práctica:

- **PC (gcc)**: programas de consola en C que aíslan un concepto del lenguaje
  o de la lógica (punto fijo, máquinas de estado, structs). Se compilan con
  `gcc` y se ejecutan en el PC. Es el formato de `etapa0/`.
- **Mini-ROM**: un programa de SNES independiente que hace una sola cosa (un
  fondo, un sprite, leer el mando) y se abre en Mesen. Formato de los
  ejemplos del SDK (`tools/pvsneslib/snes-examples/`).
- **Sobre el juego real**: cambiar una constante o una función del proyecto,
  compilar y observar el efecto en Mesen con el HUD de depuración (y, si
  aplica, con las pruebas de `tests/`).

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
- **Archivos:** lo que ya existe en `etapa0/` (`demo_01_logica.c`,
  `ejercicio_01_logica.c`, `demo_02_funciones.c`, `ejercicio_02_funciones.c`,
  `demo_03_tipos_arrays.c`, `ejercicio_03_tipos_arrays.c`) se mueve a esta
  unidad y se añaden `demo_04_structs_punteros.c` y
  `ejercicio_04_structs_punteros.c` (una `struct Personaje` con posición y
  velocidad, funciones que la reciben por puntero).
- **Código del juego:** `src/juego/tarou.h` (la `struct Tarou` y sus
  constantes `#define`), `src/juego/tarou.c` (funciones que reciben `Tarou *t`).
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
- **Aprobación:** el ejercicio imprime una altura máxima de 69 px para
  `VEL_SALTO = -96` y `GRAVEDAD = 4`, como calcula `tests/prueba_tarou.c`.

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

### Unidad 03 — De `.c` a `.sfc`: la tubería de compilación

- **Objetivo:** saber qué hace cada herramienta y leer el ensamblador que
  genera el compilador.
- **Conceptos:** compilador, ensamblador, enlazador; código objeto y
  sección; banco de ROM (LoROM, 32 KB); cabecera de la ROM; `Makefile`,
  regla, dependencia; por qué `make clean` tras tocar un `.h`; el `.log` y el
  `.sym`.
- **Archivos:** `README.md` (resumen de `docs/tuberia-de-compilacion.md` con
  ejercicios), `rom/` (mini-ROM «hola mundo»: imprime un texto con la
  consola del SDK), `ejercicio_03_leer_asm.md` (abrir el `.asm` que genera
  `tcc816` de `src/juego/camara.c` y anotar, línea a línea, qué instrucción
  corresponde a cada sentencia de C).
- **Código del juego:** `Makefile`, `src/juego/camara.asm` (generado),
  `ProyectoSNES.log`.
- **Prerrequisitos:** 00. **Práctica:** mini-ROM + lectura.
- **Aprobación:** la mini-ROM compila y muestra el texto en Mesen; las
  anotaciones del `.asm` identifican la carga de parámetros, la resta, el
  recorte y el `rts`.

### Unidad 04 — El frame, el VBlank y el bucle principal

- **Objetivo:** entender el reloj del juego: 60 vueltas por segundo
  sincronizadas con la pantalla.
- **Conceptos:** frame; línea del haz; VBlank; interrupción y NMI;
  `WaitForVBlank()`; contar tiempo en frames (`SEGUNDOS(n)`); qué solo se
  puede hacer en el VBlank (escribir VRAM, OAM, CGRAM) y qué pasa si la
  lógica se pasa de tiempo (se pierde un VBlank: 30 FPS).
- **Archivos:** `README.md`, `rom/` (mini-ROM con un contador de frames en
  pantalla y un «segundero» calculado con `SEGUNDOS`), `ejercicio_04.md`
  (hacer que el contador parpadee cada medio segundo).
- **Código del juego:** `src/sistema/main.c` (el bucle), `src/sistema/secuencia.c`
  (`secuencia_vblank`), `src/sistema/pantallas.h` (duraciones), la fila
  `LIN`/`VB` del HUD y `docs/hardware-snes-por-modulo.md` §8.
- **Prerrequisitos:** 03. **Práctica:** mini-ROM + sobre el juego real
  (medir con el HUD cuánto frame usa el nivel).
- **Aprobación:** captura de Mesen con el contador y una nota con el valor
  de `LIN` medido en el nivel y su porcentaje de frame.

### Unidad 05 — El mando

- **Objetivo:** leer los botones y separar «mantenido» de «recién pulsado».
- **Conceptos:** bits y máscaras (`&`, `|`, `<<`); `padsCurrent` y
  `padsDown`; autolectura del joypad en el VBlank; la `struct Mando` como
  traducción a intenciones; prioridad entre botones pulsados a la vez.
- **Archivos:** `README.md`, `rom/` (mini-ROM que muestra en pantalla qué
  botones están pulsados y cuáles acaban de bajar), `ejercicio_05.md`
  (añadir un contador de «cuántas veces se pulsó B»).
- **Código del juego:** `src/juego/mando.c/.h`, `docs/diseno-demo-jugable.md`
  §3 y §4 («Prioridad el mismo frame»).
- **Prerrequisitos:** 04. **Práctica:** mini-ROM.
- **Aprobación:** captura con los botones reflejados y el contador de B.

### Unidad 06 — Tiles, paletas, tilemaps y fondos

- **Objetivo:** cargar y mostrar un fondo entendiendo qué va a la VRAM y a la CGRAM.
- **Conceptos:** tile (8×8); bits por píxel (2bpp, 4bpp, 8bpp) y tamaño en
  bytes; paleta y color de 15 bits; tilemap (palabra por celda: tile,
  paleta, prioridad, volteo); modos de vídeo (Mode 1, Mode 3); `gfx4snes` y
  sus opciones; PNG indexado; índice 0 transparente; DMA.
- **Archivos:** `README.md`, `rom/` (mini-ROM que carga un fondo de 16
  colores con `bgInitTileSet` + `bgInitMapSet`), `ejercicio_06.md` (cambiar
  un color de la paleta en tiempo de ejecución y cambiar un tile del mapa).
- **Código del juego:** `src/sistema/comun.c` (`fondo16_cargar`,
  `fondo256_cargar`, `fuente_2bpp`), `src/sistema/comun.h` (el reparto de la
  VRAM), `Makefile` (reglas `gfx4snes`), `data.asm`, `src/juego/hud_debug.c`
  (escribir un tilemap a mano).
- **Prerrequisitos:** 03, 04. **Práctica:** mini-ROM + sobre el juego real
  (mirar los tiles y el mapa del título en el Tilemap Viewer de Mesen).
- **Aprobación:** captura de la mini-ROM con el fondo, el color cambiado y el
  tile cambiado.

### Unidad 07 — Scroll, cámara y parallax

- **Objetivo:** mover el mundo escribiendo registros de scroll, con una
  cámara que sigue al protagonista y se detiene en los bordes.
- **Conceptos:** registro de scroll (`bgSetScroll`, doble escritura de 8
  bits); mapas de 64×32 en páginas (`-y`); bucle sin costura (original +
  espejo); cámara como desplazamiento; recorte (clamp) en `s16`; parallax
  (capa lejana a la mitad); el truco de HDMA para el cielo.
- **Archivos:** `README.md`, `rom/` (mini-ROM con dos capas que se desplazan
  a distinta velocidad con la cruceta), `ejercicio_07.md` (añadir una tercera
  capa o cambiar la relación de parallax).
- **Código del juego:** `src/juego/camara.c` + `tests/prueba_camara.c`,
  `src/pantallas/pantalla_nivel1.c` (paso 3), `src/pantallas/pantalla_titulo.c`,
  `scripts/preparar_fondo_nivel.py` (por qué 512 px y 256 de alto).
- **Prerrequisitos:** 06. **Práctica:** mini-ROM + PC (`prueba_camara.c`).
- **Aprobación:** captura con las capas desplazadas y una prueba nueva en
  `prueba_camara.c` para un ancho de nivel distinto.

### Unidad 08 — Sprites estáticos y la OAM

- **Objetivo:** poner un personaje en pantalla y animarlo cambiando de tile.
- **Conceptos:** sprite (OBJ); OAM (128 entradas: x, y, tile, paleta,
  prioridad, volteo); tamaños (`OBJ_SIZE32_L64`); el límite de 512 tiles;
  cómo empaqueta `gfx4snes -s 64` los bloques (fórmula del tile); paletas de
  sprites (CGRAM 128..255); esconder un sprite (y por qué `y = 240` falla).
- **Archivos:** `README.md`, `rom/` (mini-ROM con un sprite de 64×64 que
  cambia de cuadro cada 8 frames y se mueve con la cruceta), `ejercicio_08.md`
  (añadir un segundo sprite con otra paleta).
- **Código del juego:** `src/pantallas/personajes.c` (`tile_del_bloque64`,
  `personajes_ocultar`), `docs/hardware-snes-por-modulo.md` §5.
- **Prerrequisitos:** 06. **Práctica:** mini-ROM.
- **Aprobación:** captura con los dos sprites y la fórmula del tile
  verificada en el Tile Viewer de Mesen.

### Unidad 09 — Metasprites y sprites dinámicos

- **Objetivo:** superar el límite de 512 tiles: un personaje grande con muchos
  cuadros cuyo cuadro actual se copia a la VRAM en cada cambio.
- **Conceptos:** metasprite (varias piezas que se mueven como una);
  `gfx4snes -T` y la tabla `t_metasprite` (`dx`, `dy`, `dtile`, atributos);
  motor de sprites dinámicos del SDK (`oamInitDynamicSprite`,
  `oamMetaDrawDyn32`, `oamrefresh`, `oamInitDynamicSpriteEndFrame`,
  `oamVramQueueUpdate`); espejo horizontal construido en RAM (hasta 4 piezas:
  `gfx4snes -T` omite las transparentes y la tabla termina en `metasprite_end`);
  presupuesto de ROM por cuadro (2 KB) y por sección (15 cuadros).
- **Archivos:** `README.md`, `rom/` (mini-ROM con un metasprite de 64×64 de
  dos animaciones, con espejo), `ejercicio_09.md` (añadir una animación y
  medir en el Sprite Viewer las 4 piezas).
- **Código del juego:** `src/juego/metasprite.c` (+ `tests/prueba_metasprite.c`),
  `src/juego/tarou_sprite.c`, `data.asm` (`.rodata_tarou_*`), `Makefile`
  (regla patrón de Tarou), el ejemplo del SDK
  `snes-examples/graphics/Sprites/DynamicEngineMetaSprite`.
- **Prerrequisitos:** 08. **Práctica:** mini-ROM + sobre el juego real.
- **Aprobación:** captura con el metasprite en ambas direcciones.

### Unidad 10 — Física de plataformas: velocidad, aceleración, gravedad y salto

- **Objetivo:** que un personaje se mueva «como en un Mario»: aceleración
  y frenado suaves, salto de altura variable, velocidad terminal, aterrizaje.
- **Conceptos:** velocidad y aceleración por frame; integrar
  (`posición += velocidad`); frenado; gravedad; apogeo; corte del salto al
  soltar el botón; velocidad terminal; control aéreo; orden de las
  operaciones dentro del frame; afinar constantes («a afinar») con el HUD.
- **Archivos:** `README.md`, `demo_10_salto.c` (PC: tabla de altura por
  frame para distintas `VEL_SALTO`), `ejercicio_10.md` (sobre el juego real:
  cambiar `GRAVEDAD` y `VEL_SALTO`, actualizar las pruebas y medir la altura
  en Mesen con el HUD).
- **Código del juego:** `src/juego/tarou.c` (`saltar`, `actualizar_aire`,
  `aterrizar`, pasos 2–4 de `tarou_actualizar`), `tests/prueba_tarou.c`
  (salto), `docs/diseno-demo-jugable.md` §5.
- **Prerrequisitos:** 01, 02, 05. **Práctica:** PC + sobre el juego real.
- **Aprobación:** `tests/pruebas.exe` en verde con las constantes nuevas y
  una captura del HUD con la nueva altura máxima (`Y` mínima).

### Unidad 11 — Animación por estados

- **Objetivo:** que cada estado muestre su animación con su ritmo, con bucle
  o sin él, y que el cambio de estado reinicie el cuadro.
- **Conceptos:** cuadro; frames por cuadro; bucle; animación terminada
  (`anim_terminada`) como disparador de transición; tabla de animaciones
  indexada por estado; el header generado `tarou_anim.h`.
- **Archivos:** `README.md`, `ejercicio_11.md` (sobre el juego real: cambiar
  los frames por cuadro de caminar y correr, ajustar `tests/prueba_animacion.c`
  y comprobar que las patas no «patinan» en Mesen).
- **Código del juego:** `src/juego/animaciones.c/.h`, `avanzar_animacion` en
  `src/juego/tarou.c`, `scripts/preparar_personaje.py` (de dónde salen los
  cuadros).
- **Prerrequisitos:** 02, 09. **Práctica:** sobre el juego real.
- **Aprobación:** pruebas en verde y captura con el nuevo ritmo.

### Unidad 12 — HDMA y efectos de la PPU

- **Objetivo:** cambiar un registro de la PPU en cada línea sin gastar CPU.
- **Conceptos:** HDMA (tabla de valores por línea); canales; `COLDATA`;
  mosaico; brillo y fundidos (`setFadeEffectEx` y su cuenta de frames por
  paso); por qué los fundidos bloquean.
- **Archivos:** `README.md`, `rom/` (mini-ROM con un degradado de cielo por
  HDMA y un fundido), `ejercicio_12.md` (otra tabla: cielo de atardecer).
- **Código del juego:** `src/pantallas/pantalla_titulo.c` (HDMA canal 6),
  `res/titulo/cielo.bmp` + regla `-n`, `src/pantallas/pantalla_intro.c`
  (mosaico), `src/sistema/comun.c` (`fade_in/out`).
- **Prerrequisitos:** 06. **Práctica:** mini-ROM.
- **Aprobación:** captura con el degradado nuevo.

### Unidad 13 — Memoria: VRAM, CGRAM, OAM, WRAM y ROM por bancos

- **Objetivo:** saber dónde vive cada byte y leer el `.log` y el `.sym`.
- **Conceptos:** mapa de memoria del 65816 (bancos `$7E/$7F`, LoROM);
  sección `superfree`; `.incbin`; etiqueta; por qué una sección no puede
  cruzar 32 KB; dónde pone tcc816 las tablas `const` (WRAM); DMA frente a
  copia por CPU; el reparto de VRAM de `comun.h`.
- **Archivos:** `README.md` (resumen de `docs/hardware-snes-por-modulo.md`
  con ejercicios), `ejercicio_13.md` (localizar en Memory Tools de Mesen la
  `struct Tarou` usando el `.sym` y verla cambiar al caminar; calcular cuántos
  cuadros más caben en la ROM).
- **Código del juego:** `data.asm`, `src/sistema/comun.h`, `ProyectoSNES.log`,
  `ProyectoSNES.sym`.
- **Prerrequisitos:** 03, 06, 08. **Práctica:** sobre el juego real.
- **Aprobación:** captura de Memory Tools con `tarou` y la cuenta de ROM libre.

### Unidad 14 — Ensamblador 65816 I: leer

- **Objetivo:** leer el ensamblador que generan `tcc816` y el SDK.
- **Conceptos:** registros (A, X, Y, S, P, DB, DP, PB); modo de 8/16 bits
  (`REP`/`SEP`); modos de direccionamiento (inmediato, directo, absoluto,
  largo, indexado, indirecto); la pila (`PHA`/`PLA`, `JSR`/`RTS`); banderas;
  cómo pasa tcc816 los parámetros y devuelve valores; cómo se ve un `switch`
  y un bucle en ensamblador; `data.asm` y `hdr.asm` como ensamblador puro.
- **Archivos:** `README.md`, `ejercicio_14.md` (anotar `src/juego/nivel.asm`
  y `src/juego/camara.asm` generados; localizar en el `.asm` de `tarou.c` la
  rutina de división que se evitó en el HUD).
- **Código del juego:** los `.asm` generados en `src/juego/`, `data.asm`,
  `hdr.asm`, las rutinas del SDK que se ven en el depurador
  (`WaitForVBlank`, el manejador de NMI).
- **Prerrequisitos:** 03, 13. **Práctica:** lectura + depurador de Mesen.
- **Aprobación:** el `.asm` anotado y un breakpoint en `camara_para` con los
  registros interpretados.

### Unidad 15 — Ensamblador 65816 II: escribir

- **Objetivo:** escribir una rutina en `.asm` y llamarla desde C.
- **Conceptos:** convención de llamada de tcc816 (parámetros en la pila,
  valor en A); secciones `.section ".text"`; `.export`/`extern`; `REP #$20` /
  `SEP #$20`; bucles con `DEX`/`BNE`; escribir un registro de la PPU a mano
  (`STA $2100`); medir ciclos con el profiler de Mesen.
- **Archivos:** `README.md`, `rom/` (mini-ROM con `src/relleno.asm`: una
  rutina que rellena una zona de VRAM con un tile, llamada desde C), 
  `ejercicio_15.md` (reescribir `poner_numero` del HUD en ensamblador y
  comparar ciclos).
- **Código del juego:** `src/juego/hud_debug.c` (lo que se reescribe),
  `data.asm` (ensamblador que ya existe en el proyecto).
- **Prerrequisitos:** 14. **Práctica:** mini-ROM + sobre el juego real.
- **Aprobación:** la mini-ROM muestra el relleno; la versión en ensamblador
  da los mismos dígitos que la de C y menos ciclos en el profiler.

### Unidad 16 — Mesen: depurador, puntos de ruptura y visores

- **Objetivo:** diagnosticar un fallo gráfico o de física sin adivinar.
- **Conceptos:** breakpoint (de ejecución, de lectura, de escritura);
  paso a paso y paso a frame; etiquetas del `.sym`; visores de tilemap,
  tiles, sprites y paletas; Event Viewer; capturas automáticas con guion.
- **Archivos:** `README.md` (resumen de `docs/mesen-y-depuracion.md` con
  ejercicios), `ejercicio_16.md` (tres casos: un sprite con la paleta
  equivocada, un scroll que no se mueve y una tecla que no responde:
  encontrar la causa con la herramienta adecuada).
- **Código del juego:** `scripts/capturar_mesen.ps1`, el HUD de depuración,
  `docs/mesen-y-depuracion.md` §6 (fallos conocidos).
- **Prerrequisitos:** 05, 08, 13. **Práctica:** sobre el juego real.
- **Aprobación:** los tres casos resueltos con la captura que lo demuestra.

### Unidad 17 — SPC700 y audio

- **Objetivo:** entender el segundo procesador y el driver de audio del SDK
  (sin componer: la música es de otra sesión).
- **Conceptos:** SPC700 (su RAM de 64 KB, su juego de instrucciones, su
  DSP de 8 voces); los puertos `$2140-$2143`; cómo `spcBoot` carga el driver;
  soundbank y `smconv`; módulos `.it`; BRR (`snesbrr`) para efectos;
  `spcProcess` cada frame; por qué el audio sigue sonando mientras la CPU
  hace otra cosa.
- **Archivos:** `README.md`, `rom/` (mini-ROM que reproduce `overworld.it`
  de `examples/likemario/` y dispara `mariojump.brr` con un botón),
  `ejercicio_17.md` (leer el ensamblador SPC700 del driver y anotar el bucle
  principal).
- **Código del juego:** `src/audio/audio.h` (el contrato previsto),
  `examples/likemario/src/LikeMario.c` (inicialización de audio),
  `examples/likemario/soundbank.asm`.
- **Prerrequisitos:** 14. **Práctica:** mini-ROM + lectura.
- **Aprobación:** la mini-ROM suena en Mesen (música y efecto).

### Unidad 18 — Herramientas: gfx4snes, scripts de Python y el Makefile

- **Objetivo:** modificar el pipeline de assets con seguridad.
- **Conceptos:** PNG indexado y paleta; cuantización; `gfx4snes` (opciones
  `-s -o -u -e -m -y -T -n`); por qué `data.obj` depende de `ASSETS`;
  reglas de patrón de `make`; pruebas `unittest` de los scripts.
- **Archivos:** `README.md`, `ejercicio_18.md` (añadir una animación nueva a
  Tarou de principio a fin: tira PNG → `preparar_personaje.py` → regla del
  Makefile → `data.asm` → `animaciones.c` → `tarou_sprite.c` → verla en Mesen).
- **Código del juego:** `scripts/preparar_personaje.py` y sus pruebas,
  `Makefile`, `data.asm`, `docs/tuberia-de-compilacion.md`.
- **Prerrequisitos:** 09, 11, 13. **Práctica:** sobre el juego real.
- **Aprobación:** la animación nueva se ve en Mesen y las pruebas de los
  scripts siguen en verde.

## 4. Orden y dependencias

```
00 C ──► 01 punto fijo ──► 02 estados ──► 10 física ──► 11 animación ──► 18 herramientas
 │                                             ▲              ▲
 └──► 03 tubería ──► 04 frame ──► 05 mando ────┘              │
          │             │                                      │
          │             └──► 06 fondos ──► 07 cámara           │
          │                      │                              │
          │                      ├──► 08 sprites ──► 09 metasprites ──────┘
          │                      └──► 12 HDMA
          └──► 13 memoria ──► 14 asm leer ──► 15 asm escribir
                   │                 └──► 17 SPC700
                   └──► 16 Mesen
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
| Etapa 2: detección y resolución de colisiones (AABB, tiles) | **pendiente**: se añadirá como Unidad 19 cuando exista `niveles/` (Etapa 2/3) |
| Etapa 2: máquinas de estado para el personaje | 02, 11 |
| Etapa 2: scroll de cámara y registros de BG | 07 |
| Etapa 2: debugging con el debugger de Mesen | 16 |

## 6. Cómo se ejecuta este plan

Cada unidad se escribe en una sesión: primero su `README.md` con la teoría
(revisada contra el glosario para que ningún término quede sin definir),
luego la demo y el ejercicio, luego la solución, y por último se comprueba
que el criterio de aprobación es alcanzable ejecutándolo de verdad (en el
PC o en Mesen con `scripts/capturar_mesen.ps1`). Al terminar cada unidad se
marca en esta tabla:

| Unidad | Estado |
|---|---|
| 00 | parcial (existe `etapa0/` con 01–03; falta structs y punteros) |
| 01–18 | por escribir |
