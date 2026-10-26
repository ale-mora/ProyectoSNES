# Plan de la carpeta `aprendizaje/`

**Para quién:** un programador junior que va a heredar este proyecto y tiene
que entender todo el código desplegado, de lo más simple a lo más complejo,
con el foco en programar juegos de Super Nintendo **en C con PVSnesLib**
(las unidades 14 y 15 enseñan además a leer y escribir ensamblador 65816).
**Qué es:** el plan de las unidades de estudio. Cada unidad enseña un
concepto, lo ancla en un archivo real del juego y se aprueba con una
práctica. Este documento es la especificación; desde el 2026-10-22 **las 21
unidades 00–20 están escritas** (§7) y este plan sirve para revisarlas o
reescribirlas siguiendo el formato de §2 y el procedimiento de §6. Desde el
2026-10-23 tiene además **6 fichas nuevas (21–26)** para el beat 'em up, por
escribir cuando exista su código: 27 unidades en total. Las funciones de la
librería que usa el curso están indexadas en [`pvsneslib.md`](pvsneslib.md).

**Estado del juego que enseñan (2026-10-23, rama `demo-secuencia-arranque`):**
el juego cambia de género. La **Revisión 3**, un beat 'em up de carril en
profundidad (*belt-scroll*) con Tarou y Bárbara, relevo, oleadas de ninjas y
tres apoyos, está **en diseño y sin código**
([`docs/diseno-beat-em-up.md`](../docs/diseno-beat-em-up.md)); lo primero que
se construirá es una ROM de prueba de carga (spec §9). Se quedan los motores
(secuencia de arranque, motor de escenas, lista de DMA, metasprite, fondo por
columnas, cámara, HUD de depuración, mando y guion) y se retiran la física de
plataformas, la rejilla de colisión, el tutorial y sus pruebas.

El código que enseñan las unidades 00–20 es el de la **Revisión 2**,
congelado en el tag **`plataformas-r2`** (commit `480f373`): Tarou es un
perro de Pixellab en celdas de 96×64 (metasprite dinámico de hasta 6 piezas)
con 13 animaciones; el nivel 1 es un **tutorial de movimiento** de 2048 px
(`docs/diseno-tutorial-movimiento.md`): escalones, muro, túnel, foso,
plataformas sobre zarzas, un tronco que se rompe con la espada y la salida a
«LEVEL 2 / COMING SOON»; colisión contra una rejilla de celdas de 16 px,
reaparición en puntos de control, la capa cercana dibujada en Tiled (la
lógica) y vestida por script con el tileset de Pixellab, copiada a la VRAM por
columnas; montañas en parallax y cielo por HDMA; el logo de Nintendo animado
por el motor de escenas (`src/escena/`); ROM de 4 MB LoROM FastROM. Pruebas:
367 en PC (entre ellas el recorrido completo del nivel) y 36 de scripts; una
ROM de prueba (`-DGUION_PRUEBA`) se juega sola en Mesen. **Toda práctica
sobre el juego real de las unidades 00–20 se hace en un clon fijado en ese
tag** (receta en §6.3), porque sus cifras se midieron contra él:

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\<x>
git -C E:\ProyectoSNES_trabajo\<x> checkout -q plataformas-r2
```

Las unidades 01, 02, 10, 11 y 20 son **históricas** (su práctica depende del
código que se retira; cada README lo avisa y dice qué parte del concepto se
traslada al beat 'em up). Las demás enseñan hardware y motores que se quedan.

Alcance: mecánicas, C con PVSnesLib, ensamblador 65816 (unidades 14 y 15),
audio desde C (unidad 17, sin ensamblador del SPC700) y la **integración
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
  se hacen en una rama propia. En las unidades 00–20, el clon o la rama
  parten del tag `plataformas-r2` (`git switch -c <rama> plataformas-r2`),
  no de la rama principal.

Criterio de aprobación: siempre algo **observable** (una salida en consola,
una captura de Mesen, una prueba en verde), nunca «haber leído».

## 3. Las unidades

### Unidad 00 — C en el PC: lógica, funciones, arrays, structs, punteros, bits, preprocesador, varios archivos y `enum`

- **Objetivo:** leer y escribir C de 16 bits sin miedo: variables, tipos,
  `if`/`for`/`while`, funciones, arrays, `struct`, punteros, operadores de
  bits, preprocesador, programas de varios archivos y `enum`/`switch`/`const`.
- **Ampliación del 2026-10-22 (partes 5–8):** binario, hexadecimal,
  operadores `& | ^ ~ << >>` y máscaras (los bits del mando y de la OAM);
  preprocesador (`#define` con parámetros, `#ifdef`, `-D`, `gcc -E`, guardas);
  varios archivos (`.h`/`.c`, prototipo, declaración frente a definición,
  `extern`, las tres formas de `static`, compilar frente a enlazar); `enum`,
  `switch` (y el fall-through) y tablas `const`. Las partes 3–4 pasan a usar
  `u8`/`u16` con los `typedef` de `src/sistema/tipos.h`; las 01–02 conservan
  `int` a propósito (son las soluciones del autor). Archivos nuevos:
  `demo_05_bits.c`, `demo_06_preprocesador.c` (+ `demo_06_caja.h`,
  `demo_06_expandir.c`), `demo_07/`, `demo_08_enum_switch_const.c`, sus
  ejercicios (`ejercicio_07/` es una carpeta) y soluciones, y en `capturas/`
  cuatro errores reales (guarda, enlace, `static`, `const`). Los esqueletos
  03 y 04 siguen sin resolver: el autor los tiene pendientes. La aprobación
  pasa a ser de **ocho** ejercicios. Lo que sigue es la ficha original de las
  partes 1–4.
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

- **Histórica (tag `plataformas-r2`).** El ejercicio de gravedad copia las
  constantes de salto de `tarou.h` de la Revisión 2. Se traslada: todo el
  punto fijo; la gravedad pasa a ser la del salto en z y los lanzamientos
  (spec §3).
- **Objetivo:** hacer aritmética con decimales en una CPU que no los tiene.
- **Conceptos:** (bits, binario, hex y desplazamientos: repaso de la unidad
  00, parte 5) desbordamiento (qué pasa en 255 + 1 con `u8`); signo (`s16`:
  complemento a dos; `>>` de un `s16` es aritmético en tcc816, medido);
  **punto fijo** (reservar bits para la fracción; formatos 8.8 y
  12.4); por qué las posiciones no son negativas en este proyecto; el coste
  de `/` y `%` en el 65816.
- **Archivos:** `README.md`, `demo_01_enteros.c` (desbordamiento y signo),
  `demo_01_puntofijo.c` (mueve un valor con
  velocidad 1,5 px/frame y lo imprime frame a frame), `ejercicio_01_gravedad.c`
  (simular un salto con gravedad en 12.4 e imprimir la altura máxima).
- **Código del juego:** `src/sistema/puntofijo.h`, `tests/prueba_puntofijo.c`,
  las constantes de `src/juego/tarou.h` (`VEL_SALTO`, `GRAVEDAD`).
- **Prerrequisitos:** 00. **Práctica:** PC.
- **Aprobación:** el ejercicio imprime una altura máxima de unos 69 px para
  `VEL_SALTO = -96` y `GRAVEDAD = 4` (la prueba de `tests/prueba_tarou.c`
  acepta un mínimo de `y` entre 73 y 77 partiendo de 144).

### Unidad 02 — Máquinas de estado

- **Histórica (tag `plataformas-r2`).** La práctica añade una prueba a
  `tests/prueba_tarou.c` del tutorial. Se traslada: el patrón `switch` por
  estado; lo nuevo (un luchador cambia el estado de otro) es de la unidad 23.
- **Objetivo:** modelar «qué está haciendo» algo con una variable y un `switch`.
- **Conceptos:** estado; transición; `enum` y `switch`/`case`/`break`
  (repaso: se enseñan en la unidad 00, parte 8); máquina de estados finita; el patrón
  `_iniciar` / `_actualizar`; por qué una sola cosa a la vez (y cuándo una
  variable aparte, como `espada`, es mejor que duplicar estados).
- **Archivos:** `README.md`, `demo_02_semaforo.c` (un semáforo con `enum` y
  `switch`), `ejercicio_02_perro.c` (PARADO → CAMINANDO → SALTANDO con un
  mando simulado por teclado en consola).
- **Código del juego:** `src/sistema/secuencia.c` (la máquina de pantallas),
  `src/juego/animaciones.h` (`enum Estado`), `src/juego/tarou.c`
  (`manejar_pie`, `manejar_agachado`, `manejar_espada`, `manejar_sueno`),
  `docs/diseno-demo-jugable.md` §4 (la tabla de transiciones).
- **Prerrequisitos:** 00, 01. **Práctica:** PC + sobre el juego real (añadir una
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
  generado (85 líneas, la función en las 10–68; verificado el 2026-10-22 con FastROM: carga del parámetro desde la
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
  conocido (2270 cuadros, `GUION_TUTORIAL_FIN` = 37,8 s a 60 FPS).
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
  estar en 225..261 o haber dado ya la vuelta; un valor en 225..261 es
  ambiguo (frame barato o pasado de una vuelta entera) y lo decide `F` frente a
  `VB`. 70–120 es donde acaba la lógica; con el HUD, el frame acaba en 150–200.
  La unidad trae además la variante con `nmiSet` propio (y `consoleVblank`,
  que la 4.6.0 no declara en ninguna cabecera).

### Unidad 05 — El mando

- **Objetivo:** leer los botones y separar «mantenido» de «recién pulsado».
- **Conceptos:** bits y máscaras (repaso: se enseñan en la unidad 00, parte 5);
  `padsCurrent` y `padsDown` (macros sobre `pad_keys`/`pad_keysdown`) y
  `padsUp` (función); autolectura del joypad en el VBlank; la `struct Mando` como
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
  transparente; DMA; deduplicación de tiles (por qué los 2080×256 px de la
  capa cercana del tutorial caben en 281 tiles: el arte se pega alineado a la
  rejilla de 8×8 y las letras de los carteles ocupan un tile cada una, así que
  se comparten). Los volteos solo se deduplican si `gfx4snes` recibe `-F`, y
  el `Makefile` no lo pasa: hoy hay 0 celdas volteadas (con `-F` serían 260
  tiles; medido y adoptado el 2026-10-22: la regla del `Makefile` ya lo pasa).
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

- **Histórica (tag `plataformas-r2`).** La física de plataformas y las pruebas
  del tutorial salen del juego. Se traslada: integrar, acelerar y frenar
  (andar y correr por el carril) y la gravedad del salto en z; no se trasladan
  los choques con paredes y plataformas.
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

- **Histórica (tag `plataformas-r2`).** La trampa del ejercicio es el guion
  del tutorial (`prueba_recorrido`). Se traslada: la tabla de animaciones por
  estado y `anim_terminada`; las cajas por cuadro son de la unidad 23.
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
  inicializar vale basura: `crt0` intenta borrar `.bss`, pero como
  `SECTIONSTART_.bss` se repite en cada `.c`, en esta ROM solo borra 10 bytes
  de `$7E:2000`);
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
  (pasa dirección y banco a `dma_lista_anadir`; `dma_ejecutar.asm` escribe el
  banco en `$4304`), `src/escena/efectos.c` (`efectos_destello`: dirección y
  banco del HDMA desde C con `REG_A1T6LH`, `REG_A1B6`, `REG_DASB6`). Medido el
  2026-10-22: las `const` globales van a la WRAM (`$7F`, copiadas por `crt0`);
  solo las `static const` quedan en `.rodata` de la ROM (banco `$0E`),
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
  contar los `jsr.l tcc__mul` (`TramoGuion` mide 6 bytes por el relleno, no 5: `i × 6` en cada acceso, 3 `tcc__mul` por vuelta),
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
  valor en `tcc__r0`/`tcc__r0h`, no en A); secciones `.section ".text"`;
  `extern` (no hace falta `.export`: medido el 2026-10-22);
  `REP #$20` / `SEP #$20`; bucles con `DEX`/`BNE`; escribir un registro de la
  PPU a mano (`STA $2100`); medir ciclos con el profiler de Mesen.
- **Archivos:** `README.md`, `rom/` (mini-ROM con `src/relleno.asm`: una
  rutina que rellena una zona de VRAM con un tile, llamada desde C; OJO: el
  `.asm` no puede llamarse como ningún `.c` de la ROM, `make` lo sobrescribiría;
  y con FastROM una `RAMSECTION` necesita `.BASE $00` delante y `.BASE $80`
  detrás, porque WLA-DX suma la base también a los bancos de RAM),
  `ejercicio_15.md` (reescribir `numero` del HUD en ensamblador y comparar
  ciclos; de paso, ver en el `.asm` generado por qué un bucle `*p++ = 0` en C
  son ~40 instrucciones por vuelta en una función aislada (37–40 medidas) y
  `memset` de la librería, 3; ampliación: `colision_apoyan` de
  `src/juego/colision.c` (el juego ya no llama a `colision_en`, solo las
  pruebas), la
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

### Unidad 17 — Audio desde C con PVSnesLib

Rehecha el 2026-10-22 (decisión del autor: el curso se centra en C; sin
ensamblador del SPC700). **El juego aún no aplica esta unidad.**

- **Objetivo:** poner música y efectos en una ROM desde C con el driver de
  audio del SDK (sin componer: la música es de otra sesión).
- **Conceptos:** SPC700 y DSP solo a nivel conceptual (otro procesador con
  su RAM de 64 KB; por qué el audio sigue sonando mientras la CPU hace otra
  cosa); los puertos `$2140-$2143` y la cola de mensajes de snesmod;
  soundbank y `smconv` (`soundbank.h`, `MOD_*`, banco propio); módulos `.it`;
  BRR y `snesbrr`; `spcBoot`, `spcSetBank`, `spcLoad`, `spcPlay`, `spcStop`,
  `spcPauseMusic`/`spcResumeMusic`, `spcSetModuleVolume`; dos vías de efectos
  (`spcLoadEffect`/`spcEffect` frente a BRR en streaming con
  `spcAllocateSoundRegion` + `spcSetSoundEntry` + `spcPlaySound`, la de
  LikeMario); `spcProcess` una vez por frame.
- **Archivos:** `README.md`, `rom/` (ejercicio en C con TODO 1..9: música de
  fondo con `overworld.it`, el efecto `mariojump` con A, pausa/reanudar con
  Start y volumen con arriba/abajo; texto en pantalla en inglés; base:
  `tools/pvsneslib/snes-examples/audio/effectsandmusic`), `solucion/rom/`.
- **Código del juego:** `src/audio/audio.h` (el contrato previsto, traducido
  a llamadas `spc*`), `tools/pvsneslib/snes-examples/systems/games/likemario/`
  (copia en `examples/likemario/`).
- **Prerrequisitos:** 04, 05. **Práctica:** mini-ROM.
- **Aprobación:** la mini-ROM suena en Mesen (música, efecto con A, pausa con
  Start y volumen). Pista sin oírla: con el esqueleto sin resolver, el
  enlazador descarta el soundbank (banco 5 al 100 % libre).

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

### Unidad 20 — Colisión con el escenario (histórica, tag `plataformas-r2`)

El código es el tutorial de movimiento de la Revisión 2
(`docs/diseno-tutorial-movimiento.md`), escrito el 2026-10-20 y congelado en el
tag `plataformas-r2`. No usa `src/niveles/` ni el motor de mapas del SDK.
`colision.c`, `partida.c`, `guion_tutorial.c`, `res/nivel1/*` y sus pruebas
salen del beat 'em up. Se traslada: la AABB (`se_tocan`) como base de las
cajas de golpe (unidad 23), el streaming de columnas y Tiled como editor, ahora
de zonas y oleadas (unidad 24).

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

### Unidades 21–26 — el beat 'em up (fichas nuevas del 2026-10-23)

**Estado de las seis: por escribir cuando exista el código.** Las fichas
salen de [`docs/diseno-beat-em-up.md`](../docs/diseno-beat-em-up.md) (la
spec de la Revisión 3, citada aquí como «spec §n») y de
[`docs/arte-beat-em-up.md`](../docs/arte-beat-em-up.md). Los archivos de
«Código del juego» son los **previstos** por la spec (§10): **no existen
todavía**. Por la definición de «hecho» de §6.1, una unidad se escribe
cuando su código está en la rama principal y su práctica se puede ejecutar
contra él; hasta entonces no hay demo, ejercicio ni cifras, y la
«Aprobación» queda por definir (nada de lo que dicen estas fichas está
medido). Se numeran en el orden en que la spec construye el código: primero
la prueba de carga (spec §9), después el mundo, el combate, los enemigos, los
efectos y el relevo. Al escribir cada una se relee la spec: si el código
final difiere, manda el código y se corrige la ficha.

### Unidad 21 — Presupuesto de sprites: la prueba de carga (por escribir)

- **Objetivo:** saber cuántos personajes caben en pantalla a la vez y por
  qué, midiéndolo antes de pedir el arte: el límite que manda en un
  belt-scroll.
- **Conceptos:** límite por línea (32 sprites y **34 ranuras de 8 px** por
  línea; en belt-scroll todos comparten las líneas del carril: Tarou 12
  ranuras, un ninja de 64×96 8, una sombra 4; Tarou + 3 ninjas = 36 > 34);
  qué hace la PPU cuando se pasa (se pierde un trozo, sin aviso); los **512
  tiles** de sprite (el protagonista activo usa 256 en doble búfer; un ninja
  de 64×96 a 6 piezas son 96 tiles por cuadro, así que 3 no caben: opciones
  A/B/C de spec §8, piezas de 16×16 recortadas, celda más estrecha o pose
  compartida); el **ancho de banda del VBlank** (~125 bytes por línea, ~2 KB
  libres por frame: un cuadro de enemigo por frame con turno rotatorio de
  subida); **alargar el VBlank** con *forced blank* en las últimas líneas
  (bandas de cine) por IRQ de línea (`HTIMEN`/`VTIME`) o por HDMA sobre
  `INIDISP`; el **parpadeo rotatorio** (rotar el orden de la OAM cada frame
  para que el corte cambie de sitio); sombras a 30 Hz; la ROM de prueba con
  `#ifdef` (`-DPRUEBA_CARGA`) como herramienta de medida, criterio de salida
  de spec §9.
- **Archivos (por escribir):** `README.md`; quizá una mini-ROM de las
  ranuras por línea (N sprites de 32×32 en la misma línea, con un contador)
  y un ejercicio sobre la ROM de prueba de carga del juego.
- **Código del juego (previsto, spec §8–§9; no existe):** la ROM
  `-DPRUEBA_CARGA` (ninjas placeholder de 64×96 que pasean por el carril con
  sombras), `src/juego/hud_debug.c` (filas `V a b` y `LIN`; **existe**),
  `src/juego/dma_lista.c` y `metasprite.c` (**existen**), el resultado
  escrito en la spec (v2). `docs/hardware-snes-por-modulo.md` (cifras).
- **Prerrequisitos:** 04, 08, 09, 13, 16. **Práctica:** juego real + Mesen
  (prevista).
- **Aprobación:** por definir cuando exista la ROM de prueba de carga.

### Unidad 22 — Belt-scroll: x/y/z, el carril y el orden por profundidad (por escribir)

- **Objetivo:** que un mundo 2D parezca tener fondo: posición en el carril,
  altura del salto, sombra y quién tapa a quién.
- **Conceptos:** las tres coordenadas (x avance, y profundidad en el
  **carril**, z altura; `pantalla_y = y − z`); la **sombra** siempre en y; el
  carril por columnas (`carril_arriba[col]`, `carril_abajo[col]`, deducido de
  Tiled); la regla de profundidad del golpe (`|y_a − y_b| ≤ TOLERANCIA_Y`,
  presentada aquí y usada en la 23); **orden de dibujo por y** (ordenación
  por inserción de ≤ 8 entidades: casi siempre ya vienen ordenadas, coste
  mínimo) y por qué en la SNES el **orden de la OAM** decide qué sprite tapa
  a cuál; el **bit de prioridad** del sprite frente a los tiles de BG1 con
  prioridad (en Mode 1, BG1 alto queda entre los sprites de prioridad 2 y
  los de prioridad 3: pasar detrás o delante de un poste); la gravedad de la
  unidad 10 aplicada a z; la cámara con bloqueo de zona.
- **Archivos (por escribir):** `README.md`, demo de PC (ordenación por y e
  impresión del orden de la OAM frame a frame) y ejercicio sobre el juego.
- **Código del juego (previsto, spec §3 y §10; no existe):**
  `src/juego/mundo.c/.h` (x/y/z, carril, orden por y), `src/juego/camara.c`
  (**existe**; se le añade el bloqueo de zona),
  `src/juego/protagonista_sprite.c` (sucesor de `tarou_sprite.c`), el
  `zonaN_datos.h` generado por `scripts/preparar_zona.py`.
- **Prerrequisitos:** 01, 07, 08, 21 (y 10, histórica, para la gravedad en
  z). **Práctica:** PC + juego real (prevista).
- **Aprobación:** por definir cuando exista el código.

### Unidad 23 — Combate: estados que se afectan, cajas de golpe y daño (por escribir)

- **Objetivo:** que un golpe conecte, haga daño y cambie el estado del otro,
  con la contundencia de un *Final Fight*, sin gastar el frame.
- **Conceptos:** la máquina de estados de un **luchador** (spec §5.1) y lo
  nuevo: **un luchador cambia el estado de otro** (`RECIBIR`, `DERRIBADO`,
  `AGARRADO`), resuelto fuera de los dos, en un módulo aparte; **caja de
  daño** y **caja de golpe** por cuadro de animación, en la tabla generada
  (AABB de la unidad 20 + tolerancia en y); la entrada de la tabla
  **rellena a 8 bytes** para que `a[i]` sea un desplazamiento y no
  `tcc__mul`; combo con ventana de encadenado; agarre por contacto
  sostenido; lanzamiento que golpea a otros; ataque aéreo; especial A+B que
  cuesta vida solo si conecta; invulnerabilidad con parpadeo; **parada de
  impacto** (atacante y objetivo congelados 3–4 frames mientras el mundo se
  sigue dibujando); **destello** (una paleta blanca copiada a la entrada de
  CGRAM del objetivo por la lista de DMA); comprobar cajas solo de quien
  ataca, por pares y sin multiplicar.
- **Archivos (por escribir):** `README.md`, demo de PC (dos luchadores y una
  tabla de cajas: quién golpea a quién cuadro a cuadro), ejercicio con
  pruebas en `tests/`.
- **Código del juego (previsto, spec §5 y §10; no existe):**
  `src/juego/combate.c/.h` (cajas, daño, agarre, turnos; lógica pura probada
  en el PC), `src/juego/tarou.c` (estados de combate), `src/juego/barbara.c`
  (de stub a código), `src/juego/animaciones.c` y el `<personaje>_anim.h`
  con cajas que generará `scripts/preparar_personaje.py`.
- **Prerrequisitos:** 02, 11, 20 (las tres por su concepto; el código que
  enseñan está en el tag), 22. **Práctica:** PC + juego real (prevista).
- **Aprobación:** por definir cuando exista el código.

### Unidad 24 — Enemigos: IA con turnos y oleadas desde Tiled (por escribir)

- **Objetivo:** que tres enemigos se sientan justos y la fase avance por
  zonas: IA simple, turnos de ataque, oleadas y bloqueo de cámara, todo
  descrito en Tiled.
- **Conceptos:** IA como máquina de estados por enemigo (acercarse, rodear,
  esperar, atacar; mantener distancia y cambiar de carril; avanzar y
  bloquear); el **testigo de ataque** (como mucho 2 atacan a la vez; también
  abarata la CPU); proyectiles por y (shuriken); el jefe por fases de vida;
  **variantes por paleta**; **zona** y **oleada** (la cámara se bloquea en
  `limite_zona` hasta vencerla; después «GO →»); objetos de Tiled con
  propiedades (`oleada`: `tipos`, `cantidad`, `lado`, `retardo`; `rompible`
  con `contiene`); un script que pasa Tiled a un `.h` de datos y las pruebas
  de PC que recorren los datos reales.
- **Archivos (por escribir):** `README.md`, demo de PC (una oleada simulada
  con el testigo), ejercicio sobre el juego (añadir una oleada en Tiled y
  verla pasar las pruebas y la ROM).
- **Código del juego (previsto, spec §6–§7 y §10; no existe):**
  `src/enemigos/ninja.c`, `src/enemigos/jefe.c`, `src/juego/oleadas.c`,
  `src/objetos/*`, `scripts/preparar_zona.py` (sucesor de
  `preparar_nivel.py`), los `.tmj` de las zonas y su `zonaN_datos.h`.
- **Prerrequisitos:** 05 (el guion de mando para jugar sin manos), 18, 20
  (Tiled y el script, por su concepto), 23. **Práctica:** PC + juego real
  (prevista).
- **Aprobación:** por definir cuando exista el código.

### Unidad 25 — Matemática de color, ventanas por HDMA y los efectos de los apoyos (por escribir)

- **Objetivo:** hacer con la PPU los efectos que un beat 'em up necesita:
  sombras que oscurecen, chispas translúcidas y los ataques a toda la
  pantalla de los tres apoyos, sin gastar CPU.
- **Conceptos:** qué sprites entran en la **matemática de color** (solo las
  paletas **4–7**: por eso sombras, chispas y el apoyo van ahí y los cuerpos
  opacos en 0–3); el modo es **global** para esas 4 paletas (`CGWSEL`/`CGADSUB`,
  cambiable por línea con HDMA pero no por sprite): **media suma** en el
  combate (la sombra negra oscurece lo de debajo y las chispas salen
  translúcidas) y suma con el color fijo solo en los momentos congelados; la **gata samurái**: ventanas 1+2 dibujadas por
  HDMA en diagonal + suma de color = tajos de luz, destello blanco con el
  color fijo; el **perro monje**: mosaico pulsante + ondulación por HDMA del
  scroll de BG2, y los enemigos en `LANZADO` con z alta; el **perro bruto**:
  temblor de pantalla (scroll ±2 px escrito en el VBlank) y polvo en sprites;
  congelar la acción durante el apoyo; extender el motor de escenas
  (`efectos.c`) a estos efectos; el HUD del juego en BG3 con prioridad alta.
  **Ampliación opcional, fuera de la spec:** Mode 7 en una escena (rotación y
  escala de un fondo de 8bpp, matriz por HDMA), por ejemplo para la escena
  del monasterio; solo si el autor la decide.
- **Archivos (por escribir):** `README.md`, mini-ROM (una sombra con media
  suma sobre un fondo y una diagonal de luz por HDMA), ejercicio sobre el
  juego.
- **Código del juego (previsto, spec §5.4, §8 y §10; no existe):**
  `src/juego/apoyos.c`, `src/escena/efectos.c` (**existe**; se amplía con los
  efectos de pantalla), `src/hud/hud_juego.c`,
  `src/pantallas/pantalla_monasterio.c`; `docs/arte-beat-em-up.md` §6.
- **Prerrequisitos:** 12, 21, 22. **Práctica:** mini-ROM + juego real
  (prevista).
- **Aprobación:** por definir cuando exista el código.

### Unidad 26 — Relevo y celda de VRAM compartida (por escribir)

- **Objetivo:** cambiar de protagonista en mitad de la pelea sin que la VRAM
  ni el presupuesto de sprites lo noten.
- **Conceptos:** **nunca dos protagonistas a la vez**: los dos comparten la
  celda de VRAM (Tarou 96×64 y Bárbara 64×96 son 6 piezas de 32×32: los
  mismos 3 KB por cuadro) y la paleta 0 (al relevar se copian 32 bytes de
  paleta por la lista de DMA); una tabla de metasprite por forma de celda;
  condiciones del relevo (enfriamiento, estado no bloqueado, vida del que
  entra); el **golpe de entrada** (daño pequeño y empuje a los cercanos);
  barras de vida separadas y curación lenta del que espera; entrada
  automática al caer el activo, y vida perdida si caen los dos; la segunda
  celda prestada a los cuadros del apoyo mientras el protagonista está en
  `INVOCAR`; la regla de Tarou de envainar y desenvainar según haya oleada
  (estado del juego, no botón).
- **Archivos (por escribir):** `README.md`, demo de PC (la máquina del
  relevo con sus condiciones), ejercicio sobre el juego.
- **Código del juego (previsto, spec §4 y §10; no existe):**
  `src/juego/cambio_protagonista.c` y `src/juego/barbara.c` (hoy stubs
  documentados), `src/juego/protagonista_sprite.c` (sucesor de
  `tarou_sprite.c`: celda compartida, dos paletas), `data.asm` (las tiras de
  Bárbara).
- **Prerrequisitos:** 09, 13, 23 (y 25 para el préstamo de la celda al
  apoyo). **Práctica:** PC + juego real (prevista).
- **Aprobación:** por definir cuando exista el código.

## 4. Orden y dependencias

```text
00 C ──► 01 punto fijo ──► 02 estados ──► 10 física ──► 11 animación ──► 18 herramientas ──► 19 arte
 │                                             ▲              ▲                 ▲              ▲
 └──► 03 tubería ──► 04 frame ──► 05 mando ────┘              │                 │              │
          │             │                                      │                 │              │
          │             └──► 06 fondos ──► 07 cámara           │                 │              │
          │                      │  └──────────────────────────┼─────────────────┼──────────────┘
          │                      ├──► 08 sprites ──► 09 metasprites ────────────┤
          │                      └──► 12 HDMA                                    │
          └──► 13 memoria ──► 14 asm leer ──► 15 asm escribir                    │
                   │  │      (04 + 05 ──► 17 audio desde C)                      │
                   │  └──────────────────────────────────────────────────────────┘
                   └──► 16 Mesen
(20 colisión: después de 07, 10, 13 y 18)
(01, 02, 10, 11 y 20: históricas, su código está en el tag plataformas-r2)
```

Las unidades del beat 'em up (por escribir), en el orden en que la spec
construye el código:

```text
21 presupuesto de sprites ──► 22 belt-scroll x/y/z ──► 23 combate ──► 24 enemigos y oleadas
                                     │                     │
                                     ▼                     ▼
                         25 color, ventanas y apoyos ──► 26 relevo y celda compartida

Prerrequisitos de fuera del bloque:
21 ◄── 04, 08, 09, 13, 16
22 ◄── 01, 07, 08 (+ 10, histórica: la gravedad en z)
23 ◄── 02, 11, 20 (históricas: el concepto)
24 ◄── 05, 18, 20 (Tiled y el script)
25 ◄── 12 (y 21: las paletas 4–7)
26 ◄── 09, 13
```

## 5. Cobertura de los ítems técnicos del plan de desarrollo

Las filas de las Etapas 0–2 vienen de `docs/plan-desarrollo.md`; las del
beat 'em up, de `docs/diseno-beat-em-up.md` (el plan de desarrollo se está
reescribiendo para la Revisión 3). «Histórica» = la unidad enseña el
concepto con el código del tag `plataformas-r2`.

| Ítem | Unidad |
|---|---|
| Etapa 0: C básico, tipos, funciones, arrays, binario/hexadecimal | 00 (01: desbordamiento y punto fijo) |
| Etapa 0/1: la tubería PVSnesLib, Makefile, estructura de una ROM | 03, 13 |
| Etapa 1: tiles, paletas, tilemaps, modos de vídeo, gfx4snes | 06, 18 |
| Etapa 1: sprites y OAM | 08, 09 |
| Etapa 1: HDMA, mosaico, fundidos | 12 |
| Etapa 1: audio (desde C), soundbank, smconv | 17 |
| Etapa 2: el loop principal y VBlank/NMI | 04 |
| Etapa 2: lectura de input (joypad) | 05 |
| Etapa 2: aritmética de punto fijo | 01 (histórica) |
| Etapa 2: física de plataformas 2D | 10 (histórica: el género cambió; integrar y la gravedad en z se trasladan a la 22) |
| Etapa 2: detección y resolución de colisiones (AABB, tiles) | 20 (histórica); las cajas de golpe y daño, 23 |
| Etapa 2/3: un nivel de principio a fin (inicio → meta), editor de mapas (Tiled) | 20 (histórica: el tutorial); zonas y oleadas desde Tiled, 24 |
| Etapa 2: máquinas de estado para el personaje | 02, 11 (históricas); estados que se afectan entre sí, 23 |
| Etapa 2: scroll de cámara y registros de BG | 07 (el bloqueo de zona, 22 y 24) |
| Etapa 2: debugging con el debugger de Mesen | 16 |
| Integración del arte (Pixellab → ROM) | 19 |
| Beat 'em up: presupuesto de sprites (34 ranuras por línea, 512 tiles, VBlank, bandas de cine, parpadeo rotatorio; spec §8–§9) | 21 (por escribir) |
| Beat 'em up: x/y/z, carril, sombra, orden por profundidad (OAM y bit de prioridad; spec §3) | 22 (por escribir) |
| Beat 'em up: combate (estados que se afectan, cajas de golpe y daño, parada de impacto, destello; spec §5) | 23 (por escribir) |
| Beat 'em up: IA de enemigos, turnos de ataque, oleadas y zonas desde Tiled (spec §6–§7) | 24 (por escribir) |
| Beat 'em up: matemática de color, ventanas por HDMA, efectos de los apoyos (spec §5.4, §8; Mode 7 opcional, fuera de la spec) | 25 (por escribir) |
| Beat 'em up: relevo entre protagonistas, celda de VRAM y paleta compartidas (spec §4) | 26 (por escribir) |
| Beat 'em up: doble toque para correr (lógica pura con `guion.c`; spec §2) | 05 (el guion de mando); se verá en la 22 o la 23 |
| Audio del juego (después de la R3, spec §11) | 17 (mini-ROM; el juego aún no lo aplica) |

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
   lo pida). (El 2026-10-22 las unidades 01–20 se escribieron a la vez y se
   confirmaron en un solo commit, por decisión del autor.)
6. Si la unidad usa la SNES, su `README.md` tiene la sección «La librería:
   PVSnesLib» (firma de cada función, qué hace, qué registros toca, enlace a
   la página de su cabecera en <https://alekmaul.github.io/pvsneslib/> y el
   ejemplo del SDK equivalente), y la función está en el índice común
   `aprendizaje/pvsneslib.md`. Regla: **primero la librería, después el
   registro** (`REG_xxx` desde C) cuando el juego baja a él.
7. **Qué es «el código real» desde la Revisión 3.** Para las unidades 00–20
   es el del tag `plataformas-r2`: contra él se ejecutaron sus prácticas, y
   una revisión las vuelve a ejecutar contra él, no contra la rama
   principal. Una unidad histórica (01, 02, 10, 11, 20) **no se reescribe
   para enseñar código del beat 'em up**: eso lo harán las unidades 21–26
   cuando su código exista. Si una unidad no histórica (hardware o motores
   que se quedan) se quiere reanclar en el código nuevo, se hace cuando ese
   código esté en la rama principal, volviendo a ejecutar su práctica y
   cambiando su receta de clon.

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

**Clon del juego real (unidades 00–20).** Las prácticas sobre el juego se
hacen en un clon fijado en el tag de la Revisión 2, el código contra el que
se verificaron; dentro del clon se compila con el entorno del paso 2:

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\<x>
git -C E:\ProyectoSNES_trabajo\<x> checkout -q plataformas-r2
```

(El clon queda con la cabeza separada, *detached HEAD*: para guardar cambios
en él, `git switch -c <rama>`.) Las unidades 21–26 usarán un clon de la rama
principal cuando existan.

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

Estos comandos y estas cifras son los del **tag `plataformas-r2`** (se
ejecutan en un clon fijado en él): el tutorial, su guion y sus pruebas salen
de la rama principal con la Revisión 3, y allí las cifras cambiarán.
Valores de referencia del tag (2026-10-22): **367** pruebas de PC y **36** de
scripts en verde; el título aparece hacia el segundo 47 de la captura; en la ROM de
prueba el nivel termina a los ~37,5 s. Para capturas con el guion de teclas,
**no usar el equipo durante la tanda**: con el equipo en uso el foco de Mesen
salta a su barra de menús y se pierden teclas (en la sesión del 2026-10-20 se
perdieron en todas las tandas).

### 6.6 Trampas conocidas que una unidad no debe enseñar mal

Las trampas que citan el tutorial, su guion o sus pruebas (`prueba_colision`,
`prueba_partida`, `prueba_recorrido`, `res/nivel1/`) son del código del tag
`plataformas-r2`; las de hardware y del compilador valen igual para el beat
'em up.

- **Cambio de género (2026-10-23).** Reescribir una unidad 00–20 contra la
  rama principal enseñaría ficción mientras el código del beat 'em up no
  exista, y una vez exista rompería sus cifras verificadas. Las unidades
  00–20 se revisan contra el tag (§6.1, punto 7).
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
- Indexar un array de structs de tamaño no potencia de 2 (`guion[i]`, 6 bytes con el relleno;
  `copias[i]` cuando medía 6 bytes; hoy `CopiaBloque` mide 8) hace que tcc816
  llame a `tcc__mul` en cada acceso: recorrer con un puntero o rellenar la
  struct a 8 bytes (unidades 09, 13, 14, 20). Matiz medido el 2026-10-22: con
  3 bytes (`ANIMACIONES[t->estado]`) tcc816 multiplica con `asl` + `adc`, sin
  llamada; medir el `.asm` antes de rellenar (unidad 11). Y `<< n` dentro de un bucle caliente es un bucle de n vueltas: sumar en
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
- Las `static` de C NO empiezan en 0: la WRAM arranca con basura y `crt0`
  solo borra 10 bytes de `.bss` (cuenta mal las secciones repetidas por cada
  `.c`). Inicializar siempre (unidades 00, 13).
- `setBrightness(0)` escribe `$80`: pantalla apagada (forced blank), no
  «brillo 0 encendida»; `fade_out` sí acaba en `$00` con la pantalla encendida
  (unidades 04, 12).
- Un bucle de relleno en C (`*p++ = 0`) cuesta ~40 instrucciones por vuelta (37–40 medidas en la unidad 14; ~60 en el contexto del HUD)
  en tcc816; `memset` de la librería es ensamblador (unidades 14, 15).
- El HDMA escribe en el HBlank del final de la línea, pero en una captura la
  fila r es la línea del haz r+1, así que una entrada de la tabla «en la fila
  n» cae justo en la fila n (la línea blanca de la unidad 12 sale en la 120).
  El desfase de 1 px que se veía en el logo de Nintendo era del FONDO: con
  scroll vertical 0 la fila r de pantalla enseña la fila r+1 del fondo, la
  fila 0 no se ve y en la última (223) aparece la fila 224, vacía en una
  imagen de 224 px (una raya del color de fondo). Desde el 2026-10-23 las
  pantallas de imagen usan scroll vertical −1 (`SCROLL_Y_FILA0` de
  `comun.h`); el nivel sigue en 0 (unidad 12).
- tcc816 NO promociona un `u8` en los argumentos `...`: lo empuja como 1 byte
  y `%d` lee 2, así que `consoleDrawText("%d", un_u8)` imprime basura en el
  byte alto. Castear a `(u16)` (unidades 09, 17).
- Cada `consoleDrawText` marca el mapa de texto y la NMI siguiente copia sus
  2 KB enteros: junto a una subida de tiles desborda el VBlank, y llamado cada
  frame con varios `%d` el bucle baja de 60 vueltas por segundo. Para texto que
  cambia cada frame, escribir el mapa como `hud_debug.c` (unidades 07, 09, 12).
- Un bucle en C justo después de `WaitForVBlank` se come el VBlank: el DMA que
  va detrás llega con la PPU dibujando y se pierde. En el VBlank solo se suben
  datos ya preparados (unidad 06).
- El color 0 de una paleta sacada de un PNG con color clave es magenta, y el
  color 0 de la CGRAM es el fondo de la pantalla: ponerlo con
  `setPaletteColor(0, RGB5(0,0,0))` (unidades 09, 17).
- `255 * 256` no cabe en el `int` de 16 bits de tcc816: las macros con
  constantes grandes se reparten (`* 128 ... * 2`) (unidades 01, 12). Matiz
  medido el 2026-10-22: con constantes solas el compilador pliega la cuenta
  con más bits y `RAMPA` sale igual byte a byte; desborda cuando un operando
  es una variable. El reparto sigue siendo la opción segura.

### 6.7 Orden de trabajo recomendado

Una unidad por sesión, en el orden 00 → 20 y después 21 → 26 (respetando
§4). Las unidades 00, 01 y 02 son de PC y cortas: pueden ir de dos en dos.
Antes de escribir una unidad, releer los archivos de «Código del juego» que
cita: si algo cambió desde este plan, manda el código y se corrige el plan.
Para las unidades 00–20, «el código» es el del tag `plataformas-r2` (§6.1,
punto 7): se relee en un clon fijado en él, no en la rama principal. Las
21–26 se escriben de una en una cuando su código llegue a la rama principal,
en el orden de la spec (la 21 en cuanto exista la ROM de prueba de carga).

## 7. Estado

Las unidades 00–20 se escribieron todas el 2026-10-22 (la 00, ampliada), en
un solo commit, contra el código que hoy guarda el tag `plataformas-r2`; las
21–26 (el beat 'em up) son fichas del 2026-10-23, por escribir (§3). Todo lo
que se ejecuta en el PC (gcc, pruebas, scripts) y todas las compilaciones
(mini-ROM y juego modificado, en clones) se ejecutaron de verdad y su salida
está en `capturas/`.

**Verificación en Mesen (2026-10-22/23).** `aprendizaje/tanda_mesen.ps1`
capturó las 37 ROM; la revisión encontró fallos reales en las mini-ROM de
las unidades 06 (el bucle en C se comía el VBlank), 07 (`consoleDrawText`
cada frame), 09 (VBlank desbordado por el texto, `u8` con `%d`, fondo
magenta), 12 (calendario por vueltas del bucle) y 17 (`u8` con `%d`, fondo
magenta), y una predicción equivocada en la 08. Se arreglaron y la segunda
tanda (`-Solo ...`) las dio todas por buenas, junto con las ventanas
corregidas de las 11, 18 y 19. Las capturas NO se versionan (decisión del
autor: 34 MB); la tanda las regenera en `capturas/mesen_<rom>/`. Siguen a
mano, por ser ventanas del depurador: Memory Tools (13), el breakpoint en
`camara_para` (14), el profiler (15) y los visores de los casos de la 16.
La tanda destapó además un fallo del juego: la raya de la última línea en
las pantallas de imagen (scroll vertical −1, §6.6).

| Unidad | Estado (2026-10-23) |
|---|---|
| 00 | **hecha**, ampliada a 8 partes; 27 compilaciones sin avisos y 21 capturas idénticas a la salida. Los ejercicios 03 y 04 siguen sin resolver (pendientes del autor) |
| 01 | **hecha**; PC; altura máxima 69 px medida. **Histórica**: su código está en el tag `plataformas-r2`. |
| 02 | **hecha**; PC + prueba nueva (374/374 en el clon); trampa de `tarou_anim.h` demostrada. **Histórica**: su código está en el tag `plataformas-r2`. |
| 03 | **hecha**; mini-ROM compila; experimento de la cabecera reproducido. Mesen: verificada. Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 04 | **hecha**; 3 variantes de mini-ROM + ROM de prueba con HUD. Mesen: verificada. Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 05 | **hecha**; demo de PC + mini-ROM con guion sin manos. Mesen: verificada |
| 06 | **hecha**; mini-ROM; deduplicación medida (281 tiles; 260 con `-F`). Mesen: verificada. Parte C: la capa del tutorial, del tag `plataformas-r2`. |
| 07 | **hecha**; mini-ROM + `camara_para_ancho` (375/375 en el clon). Mesen: verificada. Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 08 | **hecha**; mini-ROM + 2 variantes de fallo. Mesen: verificada |
| 09 | **hecha**; mini-ROM con el motor del SDK; `tcc__mul` medido. Mesen: verificada. Parte D: clon en el tag `plataformas-r2`. |
| 10 | **hecha**; demo de PC + parches A (6 fallos explicados) y B (367/367). Mesen: verificada. **Histórica**: su código está en el tag `plataformas-r2`. |
| 11 | **hecha**; demo de PC + parche (367/367); trampa del guion verificada. Mesen: verificada. **Histórica**: su código está en el tag `plataformas-r2`. |
| 12 | **hecha**; mini-ROM HDMA + motor de escenas en el publisher (378/378 en el clon). Mesen: verificada (línea blanca en la fila 120). Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 13 | **hecha**; PC contra la ROM real. Falta Memory Tools (a mano). Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 14 | **hecha**; `.asm` generados y anotados; sonda de divisiones. Falta el breakpoint en `camara_para` (a mano). Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 15 | **hecha**; mini-ROM + `numero` y `colision_apoyan` en asm (simulador: 0 fallos en 65 536 valores). Mesen: verificada; falta el profiler (a mano). Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 16 | **hecha**; 5 ROM con fallos provocados. Mesen: verificada; faltan las ventanas del depurador (a mano). Las ROM se rehacen en un clon en el tag `plataformas-r2`. |
| 17 | **hecha** (rehecha: audio desde C); mini-ROM compila. Mesen: verificada en pantalla; el sonido, a oído |
| 18 | **hecha**; animación «guardia» con TDD (36/36 de scripts). Mesen: verificada. Clon en `6d12030`; se compara con el tag `plataformas-r2`. |
| 19 | **hecha**; TDD (39/39 de scripts) + 4 ROM. Mesen: verificada. Práctica sobre el juego: clon en el tag `plataformas-r2`. |
| 20 | **hecha**; segundo foso + guion (367/367). Mesen: verificada. **Histórica**: su código está en el tag `plataformas-r2`. |
| 21 | **por escribir** cuando exista la ROM de prueba de carga (spec §9) |
| 22 | **por escribir** cuando exista `mundo.c` |
| 23 | **por escribir** cuando exista `combate.c` |
| 24 | **por escribir** cuando existan los enemigos y `oleadas.c` |
| 25 | **por escribir** cuando existan los apoyos (`apoyos.c` y los efectos de `efectos.c`) |
| 26 | **por escribir** cuando exista el relevo (`cambio_protagonista.c`, Bárbara jugable) |
