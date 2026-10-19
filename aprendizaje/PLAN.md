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

**Estado del juego que enseñan (2026-10-19, rama `demo-secuencia-arranque`):**
la Revisión 1 está completa con el arte real. Tarou es un perro de Pixellab en
celdas de 96×64 (metasprite dinámico de hasta 6 piezas) con 13 animaciones;
el nivel 1 tiene un suelo de Pixellab (tileset *Sidescroller*) con un seto
detrás, montañas en parallax y cielo por HDMA; pruebas: 195 en PC y 24 de
scripts. Lo que todavía no existe: colisión con el escenario, enemigos,
audio y Bárbara jugable.

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
  sección; banco de ROM (LoROM, 32 KB); cabecera de la ROM; `Makefile`,
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
  generado (74 líneas: carga del parámetro desde la pila, `adc #48`,
  `sbc #128`, recorte con `bvc`/`bmi` y `rts`), `ProyectoSNES.log`.
- **Prerrequisitos:** 00. **Práctica:** mini-ROM + lectura.
- **Aprobación:** la mini-ROM compila y muestra el texto en Mesen; las
  anotaciones del `.asm` identifican la carga del parámetro, la suma de la
  media celda (48), la resta de la media pantalla (128), el recorte y el
  retorno.

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
  de `LIN` medido en el nivel y su porcentaje de frame. Ojo al leer `LIN`:
  la primera cifra puede ser 248..254 (la lógica empieza dentro del VBlank) y
  la tercera, de la vuelta siguiente, es la que mide el final del trabajo.

### Unidad 05 — El mando

- **Objetivo:** leer los botones y separar «mantenido» de «recién pulsado».
- **Conceptos:** bits y máscaras (`&`, `|`, `<<`); `padsCurrent` y
  `padsDown`; autolectura del joypad en el VBlank; la `struct Mando` como
  traducción a intenciones; prioridad entre botones pulsados a la vez.
- **Archivos:** `README.md`, `rom/` (mini-ROM que muestra en pantalla qué
  botones están pulsados y cuáles acaban de bajar; base:
  `tools/pvsneslib/snes-examples/input/`), `ejercicio_05.md` (añadir un
  contador de «cuántas veces se pulsó B»).
- **Código del juego:** `src/juego/mando.c/.h`, `docs/diseno-demo-jugable.md`
  §3 y §4 («Prioridad el mismo frame»), el mapeo de teclado de Mesen en
  `docs/mesen-y-depuracion.md` §2.
- **Prerrequisitos:** 04. **Práctica:** mini-ROM.
- **Aprobación:** captura con los botones reflejados y el contador de B.

### Unidad 06 — Tiles, paletas, tilemaps y fondos

- **Objetivo:** cargar y mostrar un fondo entendiendo qué va a la VRAM y a la CGRAM.
- **Conceptos:** tile (8×8); bits por píxel (2bpp, 4bpp, 8bpp) y tamaño en
  bytes; paleta y color de 15 bits; tilemap (palabra por celda: tile,
  paleta, prioridad, **volteo horizontal y vertical**); modos de vídeo
  (Mode 1, Mode 3); `gfx4snes` y sus opciones; PNG indexado; índice 0
  transparente; DMA; deduplicación de tiles (por qué el suelo de 512×256
  solo ocupa unos 309 tiles).
- **Archivos:** `README.md`, `rom/` (mini-ROM que carga un fondo de 16
  colores con `bgInitTileSet` + `bgInitMapSet`), `ejercicio_06.md` (cambiar
  un color de la paleta en tiempo de ejecución y cambiar un tile del mapa).
- **Código del juego:** `src/sistema/comun.c` (`fondo16_cargar`,
  `fondo256_cargar`, `fuente_2bpp`), `src/sistema/comun.h` (el reparto de la
  VRAM), `Makefile` (reglas `gfx4snes`), `data.asm`, `src/juego/hud_debug.c`
  (escribir un tilemap a mano), `res/nivel1/fondo_cerca.png` (la capa del
  suelo: tierra variada con volteos de tile, que en el tilemap son gratis).
- **Prerrequisitos:** 03, 04. **Práctica:** mini-ROM + sobre el juego real
  (mirar los tiles y el mapa del suelo del nivel en el Tilemap Viewer de Mesen).
- **Aprobación:** captura de la mini-ROM con el fondo, el color cambiado y el
  tile cambiado.

### Unidad 07 — Scroll, cámara y parallax

- **Objetivo:** mover el mundo escribiendo registros de scroll, con una
  cámara que sigue al protagonista y se detiene en los bordes.
- **Conceptos:** registro de scroll (`bgSetScroll`, doble escritura de 8
  bits); mapas de 64×32 en páginas (`-y`); bucle sin costura (original +
  espejo); cámara como desplazamiento (`camara = x + 48 − 128`, 48 = media
  celda de 96); recorte (clamp) en `s16`; parallax (capa lejana a la mitad);
  el suelo como capa cercana (`SUELO_Y = 176`); el truco de HDMA para el cielo.
- **Archivos:** `README.md`, `rom/` (mini-ROM con dos capas que se desplazan
  a distinta velocidad con la cruceta), `ejercicio_07.md` (añadir una tercera
  capa o cambiar la relación de parallax).
- **Código del juego:** `src/juego/camara.c` + `tests/prueba_camara.c`,
  `src/juego/nivel.h` (`ANCHO_NIVEL`, `ANCHO_CELDA`, `SUELO_Y`),
  `src/pantallas/pantalla_nivel1.c` (paso 3), `src/pantallas/pantalla_titulo.c`,
  `scripts/preparar_fondo_nivel.py` (por qué 512 px y 256 de alto),
  `scripts/preparar_suelo.py` (la capa cercana del nivel).
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
  motor de sprites dinámicos del SDK (`oamInitDynamicSprite`,
  `oamMetaDrawDyn32`, `oamrefresh`, `oamInitDynamicSpriteEndFrame`,
  `oamVramQueueUpdate`); espejo horizontal construido en RAM (`dx → 64 − dx`;
  hasta 6 piezas: `gfx4snes -T` omite las transparentes y la tabla termina en
  `metasprite_end`); presupuesto de ROM por cuadro (3 KB) y por sección
  (10 cuadros); por qué el primer cuadro se copia a VRAM antes del fundido.
- **Archivos:** `README.md`, `rom/` (mini-ROM con un metasprite de 96×64 de
  dos animaciones, con espejo; base:
  `tools/pvsneslib/snes-examples/graphics/Sprites/DynamicEngineMetaSprite`),
  `ejercicio_09.md` (añadir una animación y medir en el Sprite Viewer las
  6 piezas).
- **Código del juego:** `src/juego/metasprite.c` (+ `tests/prueba_metasprite.c`),
  `src/juego/tarou_sprite.c`, `data.asm` (`.rodata_tarou_*`), `Makefile`
  (regla patrón de Tarou, `-X 96 -Y 64`), `src/pantallas/pantalla_nivel1.c`
  (`oamVramQueueUpdate` en `_iniciar`).
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
  `aterrizar`, pasos 2–4 de `tarou_actualizar`, `tarou_caja_cuerpo`,
  `tarou_caja_golpe`), `tests/prueba_tarou.c` (salto y cajas),
  `docs/diseno-demo-jugable.md` §5.
- **Prerrequisitos:** 01, 02, 05. **Práctica:** PC + sobre el juego real.
- **Aprobación:** `tests/pruebas.exe` en verde con las constantes nuevas y
  una captura del HUD con la nueva altura máxima (`Y` mínima). Misma trampa
  que en la unidad 02 con `tests/tarou_anim.h`.

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

- **Objetivo:** cambiar un registro de la PPU en cada línea sin gastar CPU.
- **Conceptos:** HDMA (tabla de valores por línea); canales; `COLDATA`;
  mosaico; brillo y fundidos (`setFadeEffectEx` y su cuenta de frames por
  paso); por qué los fundidos bloquean.
- **Archivos:** `README.md`, `rom/` (mini-ROM con un degradado de cielo por
  HDMA y un fundido), `ejercicio_12.md` (otra tabla: cielo de atardecer).
- **Código del juego:** `src/pantallas/pantalla_titulo.c` (HDMA canal 6),
  `res/titulo/cielo.bmp` + regla `-n`, `src/pantallas/pantalla_intro.c`
  (mosaico: la salida de la intro con `setMosaicEffect` que parece un fallo
  y no lo es), `src/sistema/comun.c` (`fade_in`, `fade_out`).
- **Prerrequisitos:** 06. **Práctica:** mini-ROM.
- **Aprobación:** captura con el degradado nuevo.

### Unidad 13 — Memoria: VRAM, CGRAM, OAM, WRAM y ROM por bancos

- **Objetivo:** saber dónde vive cada byte y leer el `.log` y el `.sym`.
- **Conceptos:** mapa de memoria del 65816 (bancos `$7E/$7F`, LoROM);
  sección `superfree`; `.incbin`; etiqueta; por qué una sección no puede
  cruzar 32 KB (y por eso 10 cuadros de 96×64 por animación); dónde pone
  tcc816 las tablas `const` (WRAM); DMA frente a copia por CPU; el reparto
  de VRAM de `comun.h`.
- **Archivos:** `README.md` (resumen de `docs/hardware-snes-por-modulo.md`
  con ejercicios), `ejercicio_13.md` (localizar en Memory Tools de Mesen la
  `struct Tarou` usando el `.sym` y verla cambiar al caminar; calcular cuántos
  cuadros de 96×64 más caben en la ROM: hoy quedan unos 66 KB libres).
- **Código del juego:** `data.asm`, `src/sistema/comun.h`, `ProyectoSNES.log`,
  `ProyectoSNES.sym`.
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
  anotar `camara.asm` y `nivel.asm`; en `src/pantallas/personajes.asm`,
  comparar `tile_del_bloque64` (parámetro `u8`: dos llamadas `jsr.l
  tcc__div`) con `tile_del_bloque32` (parámetro `u16`: ninguna) y explicar la
  diferencia).
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
  tiles, sprites y paletas; Event Viewer; capturas automáticas con guion y
  sus límites.
- **Archivos:** `README.md` (resumen de `docs/mesen-y-depuracion.md` con
  ejercicios), `ejercicio_16.md` (tres casos reales del proyecto: un sprite
  corrompido por cruzar la tabla de tiles (Bárbara en el tile 192), un
  personaje que parece flotar sobre el suelo (perspectiva: el remedio fue
  hundir los pies 4 px, `--hundir`), y una tecla del guion que no llega
  (el foco de Mesen salta a la barra de menús si se usa el equipo durante
  la tanda; la opción `AllowBackgroundInput` tiene un efecto no demostrado):
  encontrar la causa con la herramienta adecuada).
- **Código del juego:** `scripts/capturar_mesen.ps1`, el HUD de depuración,
  `docs/mesen-y-depuracion.md` §5 y §6 (fallos conocidos).
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
  tramado); `gfx4snes` (opciones `-s -o -u -e -m -y -T -X -Y -P -n`); por qué
  `data.obj` depende de `ASSETS` y los `.obj` que incluyen archivos generados
  dependen de ellos; reglas de patrón de `make`; pruebas `unittest` de los
  scripts y TDD (prueba en rojo antes que el código).
- **Archivos:** `README.md`, `ejercicio_18.md` (añadir una animación nueva a
  Tarou de principio a fin: entrada en `MAPEOS_PIXELLAB` (o una tira PNG) →
  `preparar_personaje.py` → `TAROU_ANIMS` del Makefile → `data.asm` →
  `animaciones.c` → `tarou_sprite.c` → verla en Mesen).
- **Código del juego:** `scripts/preparar_personaje.py`,
  `scripts/preparar_suelo.py`, `scripts/preparar_fondo_nivel.py` y sus
  pruebas en `scripts/pruebas/`, `Makefile`, `data.asm`,
  `docs/tuberia-de-compilacion.md` §3 y §8.
- **Prerrequisitos:** 09, 11, 13. **Práctica:** sobre el juego real.
- **Aprobación:** la animación nueva se ve en Mesen y las pruebas de los
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
  de 15 colores compartida; tileset *Sidescroller* (tiles de borde y de
  relleno, formatos de exportación); variar un relleno con volteos;
  componer capas con paleta compartida (el seto); **perspectiva**: el arte
  es una vista lateral un poco elevada y por eso los pies se hunden 4 px en
  la banda de hierba.
- **Archivos:** `README.md`, `ejercicio_19.md` (tres partes sobre el juego
  real: cambiar `--hundir` a 0 y a 8 y comparar capturas; regenerar el suelo
  sin `--seto`; añadir al mapeo una animación nueva de Pixellab, por ejemplo
  el ataque cuando exista, y verla en Mesen).
- **Código del juego:** `scripts/preparar_personaje.py` (`MAPEOS_PIXELLAB`,
  `anclar`, `cargar_pixellab`, `paleta_comun`), `scripts/preparar_suelo.py`
  (`clasificar`, `preparar`), `res/descargas/graficos/pixellab/tarou/README.md`
  y `res/descargas/graficos/pixellab/suelo/README.md`,
  `docs/diseno-demo-jugable.md` §2 (decisiones 3 y 16), §5, §7, §8 y §10.
- **Prerrequisitos:** 06, 09, 18. **Práctica:** sobre el juego real + PC
  (pruebas `unittest`).
- **Aprobación:** las 24 pruebas de scripts en verde y las capturas de Mesen
  de cada parte del ejercicio.

### Unidad 20 — Colisión con el escenario (pendiente)

Se escribirá cuando exista `src/niveles/` (mapas de tiles hechos con Tiled y
colisión por tiles, como LikeMario): Etapa 2/3 de `docs/plan-desarrollo.md`.
Conceptos previstos: AABB, colisión por tiles, resolución por ejes, cajas
de `tarou_caja_cuerpo`. Hasta entonces no se crea la carpeta.

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
(20 colisión: pendiente, después de 07 y 10)
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
| Etapa 2: detección y resolución de colisiones (AABB, tiles) | **20, pendiente** (cuando exista `src/niveles/`) |
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
  divisiones en código por frame; sin `?:` con cadenas como argumentos
  variádicos (error interno de tcc816).
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
python -m unittest scripts/pruebas/prueba_preparar_personaje.py scripts/pruebas/prueba_preparar_fondo_nivel.py scripts/pruebas/prueba_preparar_suelo.py
.\scripts\capturar_mesen.ps1 -Duracion 70 -Cada 1 -Espera 4 -Guion "55,55.3,Start;57,57.3,Select;58,61,Right"
```

Valores de referencia hoy: **195** pruebas de PC y **24** de scripts en
verde; el título aparece hacia el segundo 50 de la captura. Para capturas de
Mesen fiables, **no usar el equipo durante la tanda**: con el equipo en uso
el foco de Mesen salta a su barra de menús y se pierden teclas del guion.

### 6.6 Trampas conocidas que una unidad no debe enseñar mal

- `tests/tarou_anim.h` es una copia fija con los cuadros de los marcadores;
  el arte real tiene otros (unidades 02, 10, 11).
- La primera cifra de `LIN` en el HUD puede estar en 248..254 (unidad 04).
- El efecto mosaico de salida de la intro no es un fallo (unidad 12).
- Un sprite grande no cruza la frontera de los 256 tiles (unidades 08, 16).
- El suelo dibuja la hierba 4 px por encima de `SUELO_Y` a propósito
  (unidades 07, 19).

### 6.7 Orden de trabajo recomendado

Una unidad por sesión, en el orden 00 → 19 (respetando §4). Las unidades
00, 01 y 02 son de PC y cortas: pueden ir de dos en dos. Antes de escribir
una unidad, releer los archivos de «Código del juego» que cita: si algo
cambió desde este plan, manda el código y se corrige el plan.

## 7. Estado

| Unidad | Estado |
|---|---|
| 00 | **hecha** (2026-10-19): `unidad-00-c-en-el-pc/`; las 12 fuentes compilan con `gcc -std=c99 -Wall -Wextra` sin avisos y las 4 soluciones dan exactamente su salida esperada (`capturas/`). El autor tiene pendiente resolver el ejercicio 03 (y el 04) por su cuenta |
| 01–19 | por escribir (siguiente: 01) |
| 20 | pendiente de que exista `src/niveles/` |
