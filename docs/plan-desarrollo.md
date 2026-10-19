# Plan de desarrollo — Tarou: A Dog's Journey

Este documento es el plan de creación y desarrollo del proyecto, organizado en
etapas que recrean el proceso profesional de desarrollo de videojuegos: de lo
más general (fundamentos, concepto) a lo más particular (contenido completo,
lanzamiento). Por cada etapa se listan los aspectos técnicos e informáticos
que hay que estudiar, con el porqué de cada uno y recursos recomendados.

**Es un documento vivo.** No es una foto congelada: se edita a mano en el
repositorio a medida que el proyecto avanza. Los ítems de estudio usan
checkboxes de Markdown (`- [ ]`); marcalos como `- [x]` a medida que los vayas
completando.

## Premisas del proyecto (confirmadas)

- **Título del juego:** _Tarou: A Dog's Journey_.
- **Meta técnica:** el juego debe poder ejecutarse en una SNES real (no solo
  en el emulador Mesen). Esto implica testeo en hardware/flashcart real como
  parte obligatoria del plan, no un extra opcional.
- **Alcance de contenido:** un juego completo multi-nivel (no una demo de un
  solo nivel).
- **Punto de partida técnico:** programador principiante — el plan incluye
  una etapa de fundamentos de programación general antes de entrar en
  contenido específico de SNES.
- **Stack:** PVSnesLib (SDK en C sobre el compilador `tcc816` y el toolchain
  `wla-65816`/`wlalink`), formato de ROM LoROM/SlowROM, emulador Mesen para
  desarrollo iterativo.
- **Distribución:** proyecto **personal**, sin venta ni distribución (decisión
  del 2026-09-23). Por eso se permite usar material original con copyright de
  la época (logo de Nintendo, logos de publisher/desarrollador) en uso
  privado; cada logo se aísla como asset reemplazable en `res/` por si la
  premisa cambia. Los ítems de distribución/legales de la Etapa 6 quedan como
  "no aplica".
- **Idioma:** los textos que se ven en pantalla (título, créditos, intro,
  "PRESS START") van en **inglés**; el código y sus comentarios, en español.

## Concepto del juego

- **Protagonistas:** Tarou, un perro, y Bárbara, su mejor amiga. Se ayudan
  mutuamente a completar cada nivel.
- **Estructura de niveles:** cada nivel es una misión/etapa que ambos
  completan juntos.
- **Mecánica de personajes:** el mismo esquema que _Donkey Kong Country_
  (Donkey Kong / Diddy Kong) — dos personajes presentes, se alterna el
  control entre ellos, cada uno con habilidades propias. Los detalles
  exactos (qué pasa si el personaje activo recibe daño, cómo se recupera al
  compañero, en qué difieren sus habilidades) quedan **pendientes de
  definir** como parte del GDD en la Etapa 1.
- **Progresión de Tarou:** el perro pasa por tres etapas de vida — cachorro,
  adulto y viejo — a lo largo del juego. Cómo se gatilla ese cambio (por
  nivel, por capítulo, por evento de historia) y qué cambia mecánicamente en
  cada etapa (habilidades, velocidad, sprite) queda **pendiente de definir**
  en el GDD.
- **Guion:** todavía no está escrito. La premisa emocional confirmada es que
  Tarou y Bárbara son mejores amigos; el resto de la narrativa se
  desarrolla progresivamente.
- **Plan de producción incremental:** el primer nivel (Etapa 3 — Vertical
  Slice de este plan) es una demo técnica conceptual que usa **únicamente a
  Tarou cachorro**, sin Bárbara y sin el mecanismo de cambio de personaje.
  Bárbara, el cambio de personaje, y las etapas de vida adulta/vieja se
  incorporan progresivamente a partir de la Etapa 4 — Producción.

## Vista general de las etapas de aprendizaje

| #   | Etapa                                                                   | Objetivo en una frase                                                                                          |
| --- | ----------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| 0   | [Fundamentos](#etapa-0--fundamentos)                                    | Aprender a programar (lógica, C) y entender cómo funciona una consola retro por dentro.                        |
| 1   | [Investigación / Preproducción](#etapa-1--investigación--preproducción) | Definir qué juego es y entender la arquitectura técnica del hardware SNES y del toolchain.                     |
| 1.5 | [Demo técnica: secuencia de arranque](#etapa-15--demo-técnica-secuencia-de-arranque) | Construir el código definitivo del arranque (logos → intro → título) y medir hasta dónde llega el hardware. |
| 2   | [Prototipo](#etapa-2--prototipo)                                        | Programar las mecánicas núcleo (movimiento, salto, colisión, cámara) para validar que el juego se siente bien. |
| 3   | [Vertical Slice](#etapa-3--vertical-slice)                              | Un nivel completo con arte y audio final, todos los sistemas integrados.                                       |
| 4   | [Producción](#etapa-4--producción)                                      | Escalar el vertical slice a todo el contenido del juego.                                                       |
| 5   | [Pulido](#etapa-5--pulido)                                              | Rendimiento, juice, corrección de bugs, balance de dificultad.                                                 |
| 6   | [Alpha/Beta/QA y Lanzamiento](#etapa-6--alphabetaqa-y-lanzamiento)      | Testing en hardware real, certificación de ROM, empaquetado y publicación.                                     |

---

## Etapa 0 — Fundamentos

**Objetivo:** adquirir las bases de programación general y de arquitectura de
computadoras necesarias para abordar el desarrollo en C sobre hardware SNES
sin quedar atascado en conceptos básicos a mitad de camino.

**Entregable(s):** ser capaz de escribir, compilar y ejecutar programas
simples en C (fuera de SNES) usando variables, condicionales, bucles,
funciones, arrays, structs y punteros con soltura; poder explicar qué es una
CPU, qué es RAM/ROM, y cómo se representa un número en binario/hexadecimal.

**Criterio de salida:** resolver un pequeño ejercicio de C (por ejemplo, un
programa que mueva un "personaje" en una grilla de texto respondiendo a
input simulado) sin ayuda externa.

### Ítems técnicos a estudiar

- [x] **Lógica de programación** (variables, condicionales, bucles) — es el
      vocabulario mínimo para leer o escribir cualquier línea de código del
      proyecto, incluido `src/main.c`.
      _Practicado con `aprendizaje/unidad-00-c-en-el-pc/solucion/ejercicio_01_logica.c`: bucle
      `for`, variable `edad`, `if`/`else if`/`else` con rangos. Depuró en el
      camino dos errores clásicos de C (`=` vs `==`, y un error de borde
      off-by-one en los límites del rango) hasta que la salida coincidió
      exactamente con lo esperado._
      Recursos: CS50 (Harvard, gratuito, `cs50.harvard.edu`); freeCodeCamp.
- [x] **Funciones y modularidad** — PVSnesLib se usa llamando a funciones del
      SDK (`consoleInitDefaultText`, `bgSetGfxPtr`, etc.); entender cómo se
      define/llama/retorna una función es imprescindible.
      _Practicado con `aprendizaje/unidad-00-c-en-el-pc/solucion/ejercicio_02_funciones.c`:
      refactorizó la lógica del Ejercicio 1 en una función `void` con
      parámetro, llamada desde un `for`; salida idéntica verificada._
      Recursos: mismo curso anterior; "The C Programming Language" (K&R).
- [ ] **Tipos de datos y arrays** — los tilemaps, paletas y sprites en SNES
      se manipulan como arrays de bytes; sin entender tipos (`u8`, `u16`) y
      arrays no se puede leer el código de LikeMario.
      Recursos: K&R, capítulos 1-5.
- [ ] **Structs** — las entidades del juego (jugador, enemigo, objeto) se
      modelan normalmente como structs en C; el patrón ya aparece en
      LikeMario.
      Recursos: K&R, capítulo 6.
- [ ] **Punteros y memoria** — el C para consolas retro se apoya fuertemente
      en punteros para acceder a direcciones de memoria específicas (VRAM,
      OAM, registros de hardware). Es el concepto más importante y más
      difícil para alguien que recién empieza.
      Recursos: K&R, capítulo 5; "Beej's Guide to C".
- [ ] **Compilación y makefiles** — el proyecto usa un `Makefile` y una
      cadena de herramientas (`816-tcc`, `wla-65816`, `wlalink`); entender a
      alto nivel qué hace un compilador/ensamblador/linker ayuda a
      diagnosticar errores de build.
      Recursos: documentación de GNU Make; el propio `Makefile` del proyecto
      como caso de estudio real.
- [ ] **Sistemas numéricos: binario y hexadecimal** — toda la programación
      de hardware (colores, direcciones de memoria, valores de registros) se
      expresa en hexadecimal/binario, no en decimal.
      _Adelantado (2026-09-23): la Etapa 1.5 escribe direcciones de VRAM y
      colores en hexadecimal en cada pantalla, así que este tema se estudia
      antes que Structs y Punteros, en paralelo con la demo._
      Recursos: serie de Ben Eater en YouTube sobre computadoras a nivel de
      hardware, muy visual para estos conceptos.
- [ ] **Arquitectura de computadoras básica** (CPU, RAM, ROM, buses, ciclos
      de reloj) — la SNES es literalmente una CPU (65816) + un chip de video
      (PPU) + un chip de audio (SPC700) comunicándose por buses; sin este
      modelo mental, "por qué hay que copiar datos a la VRAM durante VBlank"
      no tiene sentido.
      Recursos: serie "Building an 8-bit breadboard computer" de Ben Eater
      (YouTube) — no es SNES, pero enseña los conceptos de forma tangible;
      `wiki.superfamicom.org` como referencia posterior ya aplicada a SNES.
- [ ] **Control de versiones con Git** — ya está inicializado en el
      proyecto; conviene reforzar el flujo básico (commit, branch,
      `.gitignore`) antes de que el proyecto crezca en tamaño.
      Recursos: "Pro Git" (libro gratuito, `git-scm.com/book`).

---

## Etapa 1 — Investigación / Preproducción

**Objetivo:** definir con claridad qué juego se va a construir (concepto,
referencias, alcance) y entender en profundidad el hardware SNES y el
toolchain del proyecto (PVSnesLib) antes de escribir gameplay real.

**Entregable(s):** un Game Design Document (GDD) breve (2-5 páginas) para
_Tarou: A Dog's Journey_ que resuelva las preguntas de diseño abiertas —
cómo funciona exactamente el cambio de personaje Tarou/Bárbara (inspirado
en Donkey Kong Country), cómo se gatilla la progresión cachorro→adulto→viejo
y qué cambia en cada etapa, y al menos una lista preliminar de
niveles/misiones — más comprensión de la arquitectura técnica del hardware
SNES aplicada a este proyecto.

**Criterio de salida:** poder explicar sin notas qué es LoROM/SlowROM, qué
hace `hdr.asm`, qué son los 8 bancos de ROM, y tener el GDD escrito.

### Ítems técnicos a estudiar

- [ ] **Qué es un Game Design Document (GDD) y cómo estructurarlo** — en la
      industria, ningún estudio arranca a programar sin definir qué está
      construyendo; evita retrabajo enorme más adelante.
      Recursos: "The Art of Game Design: A Book of Lenses" de Jesse Schell
      (capítulos iniciales).
- [x] **Análisis de referencias de plataformers** — entender qué hace
      "sentir bien" a Super Mario World o Donkey Kong Country da un techo
      realista de qué es alcanzable en solitario.
      _Ya avanzado: investigación del pipeline de producción de Donkey Kong
      Country (Rare — 12 desarrolladores, 18 meses, render 3D→sprite —
      concluido como no replicable para un desarrollo solo) registrada en
      sesiones previas._
      Recursos: canal de YouTube "Retro Game Mechanics Explained" (análisis
      técnico de físicas de Mario).
- [ ] **Arquitectura del hardware SNES: CPU 65816** — es el procesador que
      ejecuta todo el código del juego; entender sus registros (A, X, Y),
      modos de direccionamiento y el "banking" de memoria es la base de todo
      lo que sigue.
      Recursos: `wiki.superfamicom.org` (sección CPU); "fullsnes" de nocash
      (`problemkaputt.de/fullsnes.htm`), referencia exhaustiva pero densa.
- [ ] **PPU (Picture Processing Unit)** — es el chip que dibuja gráficos; sin
      entender sus modos de BG, VRAM, CGRAM y OAM no se puede razonar sobre
      nada visual del juego.
      Recursos: `wiki.superfamicom.org` (sección PPU); ejemplos
      `graphics/Backgrounds` y `graphics/Sprites` en
      `tools/pvsneslib/snes-examples`.
- [ ] **APU / SPC700** — es el procesador de audio, separado de la CPU
      principal; entender que corre su propio programa aparte cambia cómo se
      piensa la música/sonido del juego.
      Recursos: `wiki.superfamicom.org` (sección APU); ejemplos
      `audio/music` y `audio/effectsandmusic` del SDK.
- [ ] **Formato de ROM: LoROM vs HiROM, SlowROM vs FastROM** — el proyecto ya
      está configurado como LoROM/SlowROM en el `Makefile`; entender qué
      implica esa elección (mapeo de memoria, velocidad de acceso) evita
      errores de configuración más adelante.
      Recursos: `wiki.superfamicom.org` (sección "Memory Mapping"); el
      propio `hdr.asm` generado del proyecto como caso de estudio.
- [ ] **El toolchain de PVSnesLib** — el pipeline `.c → .ps → .asm → .obj →
    .sfc` no es un compilador tradicional de un solo paso; entender qué
      hace cada herramienta (`816-tcc`, `wla-65816`, `wlalink`) ayuda a
      interpretar errores de compilación.
      Recursos: `github.com/alekmaul/pvsneslib` (wiki y documentación del
      repo); `snes_rules` dentro de `devkitsnes`.
- [x] **Estudio de la arquitectura de LikeMario** — es el ejemplo más
      cercano al género que vas a construir, y sirve de referencia de "cómo
      se organiza un proyecto real de PVSnesLib".
      _Ya avanzado: arquitectura documentada (4 capas), inventario completo
      de 33 archivos por categoría/tipo/productor, y físicas explicadas en
      sesiones previas._
      Recursos: tu propia documentación en `examples/likemario/LikeMario.md`.
- [ ] **Planificación de alcance (scope) para proyectos solo-dev** — la
      causa #1 de abandono de proyectos de juego amateur es un alcance
      irrealista; definir un MVP (producto mínimo viable) ahora evita
      frustración en la etapa de Producción.
      Recursos: charlas de GDC sobre "shipping your first indie game"
      (buscar en el canal de YouTube "GDC" / "GDC Vault").
- [ ] **Mecánica de personaje doble estilo Donkey Kong Country** (Donkey
      Kong / Diddy Kong) — es el modelo de referencia elegido para la
      dinámica Tarou/Bárbara; hay que analizar en detalle cómo DKC maneja
      el cambio de personaje, la pérdida/recuperación del compañero, y las
      habilidades diferenciadas entre ambos, para poder adaptarlo al GDD.
      Recursos: desgloses de gameplay de DKC (canal "Retro Game Mechanics
      Explained" en YouTube); tu propia investigación previa del pipeline de
      producción de DKC.
- [ ] **Diseño narrativo básico y estructura de misiones/niveles** — aunque
      el guion no está cerrado, cada nivel se organiza como una misión que
      Tarou y Bárbara completan juntos; conviene tener al menos un boceto
      de la lista de niveles/misiones en el GDD antes de entrar en
      Producción.
      Recursos: "The Art of Game Design: A Book of Lenses" (secciones de
      narrativa y estructura de niveles).

---

## Etapa 1.5 — Demo técnica: secuencia de arranque

**Objetivo:** construir el **código definitivo** de todo lo que pasa desde que
se enciende la consola hasta que el jugador presiona Start, y usarlo como demo
técnica para medir hasta dónde se puede exprimir el hardware (Mode 7, HDMA,
parallax, audio SPC) antes de tocar gameplay. Se hace en paralelo con lo que
queda de la Etapa 0.

**Diseño detallado:** `docs/diseno-secuencia-arranque.md` (spec viva, pantalla
por pantalla, con las 35 decisiones de la entrevista del 2026-09-23).

**Entregable(s):** una ROM `.sfc` que recorre en orden, sin intervención, las
seis pantallas — (1) Nintendo → (2) publisher → (3) desarrolladores → (4)
dedicatoria → (5) intro de 3 escenas → (6) título con "PRESS START" — entra en
modo attract a los 15 s, y responde a Start con la pantalla "LEVEL 1 — COMING
SOON" que vuelve al título. Más el glosario `docs/glosario-snes.md`, el script
`scripts/preparar_imagen.py` y la estructura `res/` lista para recibir assets.

**Criterio de salida:** la secuencia corre a 60 FPS en Mesen sin glitches
gráficos ni de audio, con todos los assets provisionales en su lugar, y cada
línea del código tiene su comentario explicativo.

**Estado (2026-09-24):** la secuencia completa corre en Mesen con gráficos
reales en todas las pantallas (logos, tres escenas japonesas, sprites de
Tarou y Bárbara, título con parallax y cielo HDMA). Falta el **audio** y una
pasada de **pulido visual**. Detalle y hallazgos técnicos en
`docs/diseno-secuencia-arranque.md` §9 y §10.

**Aprendido en la práctica (resumen):**

- El sistema de compilación del SDK no sigue dependencias de headers ni de los
  archivos que `data.asm` incrusta: `make clean` tras tocar un `.h`, y el
  Makefile declara `data.obj: $(ASSETS)`.
- Verificar con evidencia: se midieron los tiempos reales capturando Mesen a
  intervalos, y así se descubrió que la ROM llevaba código viejo.
- Cuando la documentación del SDK es ambigua, la fuente de verdad es su código
  en ensamblador (fundidos, mosaico, Mode 7).

**Decisiones clave:**

- Código escrito íntegramente por Claude, **cada línea explicada** en un
  comentario; conceptos de hardware en un glosario único.
- Un archivo `.c` por pantalla + `secuencia.c` con máquina de estados propia
  (`enum` + `switch`) + `main.c` mínimo. No se usa `gamestates.h` del SDK.
- Efectos a demostrar: fade y mosaico (transiciones), Mode 7 (logo de
  desarrolladores entrando con zoom y rotación), degradado HDMA y parallax de
  dos capas (título), sprite animado de Tarou cachorro (intro y título).
- Estética de referencia: _Secret of Mana_. Textos en pantalla en inglés.
- Assets: el usuario descarga a `res/descargas/` (gráficos y audio); se
  convierten con scripts Python a PNG indexado; música solo en formato `.it`.
- Ninguna pantalla se salta por ahora (`PERMITE_SALTO = 0` deja la opción).
- Rama de trabajo `demo-secuencia-arranque`, un commit por pantalla.

### Ítems técnicos a estudiar

- [x] **Máquina de estados de pantallas** (`enum` + `switch`) — es la columna
      vertebral de la secuencia y el mismo patrón que después usará el
      personaje (Etapa 2); se aprende aquí con un caso simple.
      Recursos: `src/secuencia.c` del proyecto; `systems/gastates` del SDK
      como lectura comparada.
- [x] **Modos de vídeo de la PPU: Mode 1 y Mode 7** — Mode 1 (dos capas de 16
      colores + una de 4) es la base de todas las pantallas; Mode 7 (rotación
      y zoom de una sola capa de 256 colores) se usa en el logo de
      desarrolladores.
      _Hecho: Mode 1 en logos/título, Mode 3 (256 colores) en la intro. Mode 7
      se probó y se descartó para la demo (ver spec §10)._
      Recursos: `graphics/Backgrounds/Mode1` y `Mode7` del SDK;
      `wiki.superfamicom.org` (sección "Backgrounds").
- [x] **Fades, mosaico y brillo** (`setFadeEffectEx`, `setMosaicEffect`,
      `setBrightness`) — son las transiciones entre pantallas.
      Recursos: `graphics/Effects/Fading` y `Mosaic` del SDK.
- [x] **HDMA: degradado de color por línea** — el cielo del título cambia de
      color línea a línea sin gastar CPU; es la técnica que hace "ver SNES" a
      un fondo.
      Recursos: `graphics/Effects/HDMAGradient` del SDK; `dma.h`.
- [x] **Scroll y parallax por software** (`bgSetScroll` a dos velocidades) —
      dos capas moviéndose a distinta velocidad dan profundidad; es la misma
      cámara que usará el nivel.
      Recursos: `graphics/Backgrounds/Mode1Scroll` del SDK.
- [x] **Sprites (OAM) y animación por cuadros** — Tarou cachorro en reposo en
      la intro y el título.
      Recursos: `graphics/Sprites/SimpleSprite` y `AnimatedSprite` del SDK.
- [x] **Texto por consola y efecto máquina de escribir** — dedicatoria, intro,
      créditos y "PRESS START"; contar frames para mostrar una letra a la vez.
      Recursos: `textfont.h` del SDK; `hello_world`.
- [x] **Lectura del mando en VBlank** (`padsDown`, `KEY_START`) — Start en el
      título y contador de attract; entender por qué el mando solo se actualiza
      si se llama a `WaitForVBlank()` cada frame.
      Recursos: `input/controller` del SDK.
- [x] **Pipeline de gráficos** (PNG indexado → `gfx4snes` → `.pic/.pal/.map` →
      `data.asm`) — cada asset descargado pasa por aquí; incluye el script
      Python de reducción de colores.
      Recursos: `scripts/preparar_imagen.py`; Makefiles de los ejemplos de logo
      del SDK (`thirdparty/logo/*`).
- [ ] **Pipeline de audio SPC** (`.it` → `smconv` → soundbank en banco 5;
      `.wav` → `snesbrr` → `.brr`) — jingles de logos, música de dedicatoria,
      intro y título.
      Recursos: `audio/music` y `audio/effectsandmusic` del SDK;
      `pvsneslib_snesmod.txt` (reglas de composición).
- [x] **Banking de datos en ROM** (`data.asm`, secciones `superfree` de 32 KB,
      `snesromusage`) — con seis pantallas de assets hay que repartir por
      bancos desde el principio.
      Recursos: `data.asm` de `graphics/Backgrounds/Mode3` del SDK.
- [~] **Ramas y merges en Git** — la demo se hace en la rama
      `demo-secuencia-arranque` con un commit por pantalla; primer uso real de
      ramas en el proyecto.
      Recursos: "Pro Git", capítulo 3.

---

## Etapa 2 — Prototipo

**Objetivo:** programar las mecánicas núcleo del juego (movimiento, salto,
colisión, cámara) con gráficos y audio de marcador de posición, para validar
que el gameplay se siente bien antes de invertir en arte/audio final. El
personaje del prototipo es Tarou en su etapa de cachorro únicamente — el
mecanismo de cambio de personaje con Bárbara se aborda recién en la Etapa 4.

**Entregable(s):** un `.sfc` ejecutable en Mesen donde un personaje (aunque
sea un cuadrado de color) se mueve, salta con física creíble, colisiona con
el escenario y la cámara lo sigue.

**Criterio de salida:** el prototipo es jugable de punta a punta con
mando/teclado emulado, sin crashear, a 60 FPS estables.

**Estado (2026-10-19):** Revisión 1 hecha y verificada en Mesen a 60 FPS
(`docs/diseno-demo-jugable.md`): Tarou adulto camina, corre, salta, se agacha,
usa la espada y duerme en un nivel plano de 512 px con parallax. Desde el
2026-10-19 con el **arte real de Pixellab**: el perro en celdas de 96×64 (13
animaciones; el ataque es una pose fija provisional) y un suelo de tileset
*Sidescroller* con un seto detrás. Falta la colisión con el escenario
(tileset + Tiled, ítem pendiente abajo), una animación de ataque, Sleep
mirando al este y reajustar el ritmo de desenvainar, envainar y despertar.

### Ítems técnicos a estudiar

- [x] **El loop principal y VBlank/NMI** _(Revisión 1, 2026-10-16: `main.c` + `secuencia_vblank`; `docs/hardware-snes-por-modulo.md` §8)_ — toda la lógica del juego debe
      sincronizarse con el refresco de pantalla (60 Hz en NTSC); `src/main.c`
      ya usa `WaitForVBlank()` y hay que entender por qué.
      Recursos: `wiki.superfamicom.org` (sección "NMI/VBlank");
      `systems/timer` del SDK.
- [x] **Lectura de input (joypad)** _(Revisión 1, 2026-10-16: `juego/mando.c`)_ — sin poder leer el control no hay
      gameplay; PVSnesLib expone funciones para leer el estado del joypad.
      Recursos: ejemplo `input/controller` del SDK.
- [x] **Aritmética de punto fijo (fixed-point)** _(Revisión 1, 2026-10-16: 12.4 en `sistema/puntofijo.h`, probado en `tests/`)_ — el 65816 no tiene unidad
      de punto flotante (FPU); toda velocidad/aceleración/gravedad
      fraccionaria del personaje se debe representar con enteros escalados
      (ej. formato 8.8). Es un concepto ausente en programación de alto
      nivel moderna y hay que aprenderlo específicamente para hardware
      retro.
      Recursos: artículos generales de "fixed point arithmetic for embedded
      systems"; el código de física de LikeMario como caso de estudio real.
- [x] **Física de plataformas 2D** _(Revisión 1, 2026-10-16: gravedad, salto variable, terminal en `juego/tarou.c`; sin colisión con el escenario todavía)_ (gravedad, salto, velocidad terminal) — es
      el corazón del gameplay de un plataformer; cómo se calibra la curva de
      salto define si el juego "se siente bien".
      Recursos: "Retro Game Mechanics Explained" (análisis de la física de
      Super Mario Bros/World en YouTube).
- [ ] **Detección y resolución de colisiones** (AABB, basada en tiles) — el
      personaje necesita chocar contra el piso/paredes del nivel; la
      colisión basada en tiles (en vez de pixel-perfect) es el enfoque
      estándar en plataformers 2D retro por su bajo costo de CPU.
      Recursos: artículos generales sobre "AABB collision detection" y
      "tile-based collision" (universales a juegos 2D, no específicos de
      SNES).
- [x] **Máquinas de estado (state machines) para el personaje** _(Revisión 1, 2026-10-16: 13 estados en `juego/tarou.c`)_ — el
      personaje tiene estados discretos (parado, corriendo, saltando,
      cayendo) que determinan tanto la física como la animación; modelarlo
      como una máquina de estados evita código en espagueti.
      Recursos: `systems/gastates` del SDK (game states); cualquier
      introducción general a "finite state machines" en videojuegos.
- [x] **Scroll de cámara y registros de BG** _(Revisión 1, 2026-10-16: `juego/camara.c` + `bgSetScroll` en `pantalla_nivel1.c`)_ — la cámara debe seguir al
      jugador desplazando el fondo; en SNES esto se hace escribiendo en los
      registros de scroll del PPU, no moviendo "objetos" como en un motor
      moderno.
      Recursos: `wiki.superfamicom.org` (sección "Backgrounds");
      `graphics/Backgrounds` del SDK.
- [x] **Debugging con el debugger de Mesen** _(Revisión 1, 2026-10-16: HUD de depuración con medidor de línea; `docs/mesen-y-depuracion.md`)_ — Mesen trae un debugger
      completo (breakpoints, inspección de memoria/VRAM/registros) que es la
      herramienta principal para diagnosticar bugs de física/colisión sin
      hardware real.
      Recursos: documentación/wiki oficial de Mesen; `systems/debug` y
      `systems/breakpoints` del SDK.

---

## Etapa 3 — Vertical Slice

**Objetivo:** construir el primer nivel de _Tarou: A Dog's Journey_ como
demo técnica conceptual — jugando únicamente con Tarou cachorro, sin
Bárbara ni el cambio de personaje — con arte definitivo, audio definitivo, y
todos los sistemas integrados. Es la prueba de que el pipeline completo
funciona de punta a punta, no solo el prototipo aislado.

**Entregable(s):** el primer nivel jugable de principio a fin (inicio →
meta), protagonizado solo por Tarou cachorro, con la calidad visual y
sonora que representa el estándar del juego terminado.

**Criterio de salida:** el nivel corre a 60 FPS estables en Mesen sin
glitches gráficos ni de audio, y representa fielmente el "target" de calidad
del proyecto.

### Ítems técnicos a estudiar

- [ ] **Pipeline de arte pixel** (creación de sprites y tiles) — hay que
      producir el arte real del juego (personaje, escenario, enemigos);
      herramientas como Aseprite están pensadas específicamente para pixel
      art con paletas limitadas, como exige SNES.
      Recursos: `aseprite.org` (documentación oficial); paleta de 15-bit
      color de SNES (`wiki.superfamicom.org`, sección "Palettes/CGRAM").
- [ ] **Conversión de assets con las herramientas de PVSnesLib** — el arte
      no se importa directo; hay un paso de conversión de PNG/BMP a los
      formatos binarios que la PPU entiende (`.pic`, `.pal`, `.map`), como se
      ve en los archivos de LikeMario (`mario_sprite.pic`, `tiles.pal`,
      etc.).
      Recursos: `github.com/alekmaul/pvsneslib` (documentación de
      herramientas gráficas); los propios archivos de
      `examples/likemario` como referencia.
- [ ] **Editores de tilemap (Tiled)** — LikeMario ya usa Tiled (`.tmx`) para
      diseñar el mapa del nivel (`map_1_1.tmx`); es el estándar de facto
      para diseño de niveles 2D basados en tiles.
      Recursos: `mapeditor.org` (documentación oficial de Tiled).
- [ ] **Formato de audio BRR y el SPC700** — el audio de sample (efectos de
      sonido) en SNES se codifica en un formato de compresión propio (BRR);
      sin entenderlo no se puede producir ni depurar sonido.
      Recursos: `wiki.superfamicom.org` (sección "SPC700 / Audio"); el par
      `mariojump.brr`/`.wav` en LikeMario como caso de estudio de
      conversión.
- [ ] **Música con trackers (formato .it)** — LikeMario usa `overworld.it`
      (formato Impulse Tracker); PVSnesLib convierte música de trackers a un
      formato reproducible por el SPC700.
      Recursos: documentación de OpenMPT (editor de trackers `.it`
      gratuito); `audio/music` del SDK.
- [ ] **DMA (Direct Memory Access)** — transferir tiles/paletas/sprites a
      VRAM/CGRAM es demasiado lento con CPU normal; el DMA permite copiar
      bloques grandes de datos eficientemente durante VBlank.
      Recursos: `wiki.superfamicom.org` (sección "DMA"); `graphics/Effects`
      del SDK suele mostrar casos de uso.
- [ ] **Animación de sprites (OAM, spritesheets)** — el personaje necesita
      animarse (correr, saltar); esto implica ciclar qué tile muestra cada
      entrada de OAM en sincronía con el estado del personaje.
      Recursos: `graphics/Sprites` del SDK; `mario_sprite.inc` de LikeMario
      como referencia de estructura.
- [ ] **Organización de banking de ROM** — con 8 bancos disponibles, hay que
      decidir en qué banco vive cada tipo de dato (código, gráficos, audio)
      para no quedarse sin espacio ni generar conflictos de dirección.
      Recursos: `wiki.superfamicom.org` (sección "Memory Mapping /
      Banking"); `systems/memory_mapping` del SDK.

---

## Etapa 4 — Producción

**Objetivo:** escalar el vertical slice a la totalidad del contenido del
juego: todos los niveles, todos los enemigos/objetos, y los sistemas
restantes que el vertical slice no necesitó cubrir (HUD completo, power-ups,
IA de enemigos, etc.). En esta etapa se incorporan también los elementos que
el primer nivel dejó afuera a propósito: Bárbara, el cambio de personaje
estilo Donkey Kong Country, y las etapas de vida adulta y vieja de Tarou.

**Entregable(s):** todos los niveles planeados en el GDD, implementados y
jugables de principio a fin, con todo el contenido (enemigos, objetos, HUD)
integrado.

**Criterio de salida:** el juego se puede jugar de principio a fin (todos
los niveles) sin crashes ni bloqueos, aunque todavía no esté balanceado ni
pulido.

### Ítems técnicos a estudiar

- [ ] **Implementación del cambio de personaje (Tarou/Bárbara)** — llevar a
      código la mecánica diseñada en la Etapa 1 (inspirada en Donkey Kong
      Country): gestión de dos actores simultáneos en memoria, input para
      alternar el control, habilidades diferenciadas entre ambos, y
      recuperación del compañero perdido.
      Recursos: tu propio GDD (Etapa 1) con el diseño ya resuelto;
      `systems/objects` del SDK para la gestión de múltiples entidades.
- [ ] **Progresión de edad de Tarou** (cachorro → adulto → viejo) —
      implementar el disparador definido en el GDD (por nivel, capítulo o
      evento) y los cambios mecánicos asociados a cada etapa de vida
      (sprite, habilidades, velocidad).
      Recursos: tu propio GDD (Etapa 1) con las reglas de progresión ya
      resueltas; `graphics/Sprites` del SDK para el manejo de spritesheets
      alternativos por etapa de vida.
- [ ] **Diseño e IA básica de enemigos** (patrones de movimiento, máquinas de
      estado) — escala el concepto de máquina de estados de la Etapa 2 a
      múltiples entidades simultáneas con comportamientos propios.
      Recursos: `systems/objects` del SDK (gestión de múltiples entidades);
      introducciones generales a "patrones de movimiento en juegos 2D".
- [ ] **Gestión de múltiples entidades y límites de sprites** — la SNES
      tiene un límite duro de sprites simultáneos en pantalla (32 en total,
      34 por línea); hay que gestionar activamente qué entidades existen y
      se muestran.
      Recursos: `wiki.superfamicom.org` (sección "OAM / Sprite limits");
      `systems/objects` del SDK.
- [ ] **Power-ups y objetos coleccionables** — es contenido de gameplay
      estándar en plataformers; técnicamente reutiliza colisión + máquina de
      estados ya construidas.
      Recursos: tu propio GDD (Etapa 1) como referencia de qué power-ups
      definiste.
- [ ] **HUD y renderizado de texto/UI** — mostrar vida, puntaje, tiempo
      requiere una capa de BG dedicada (o consola de texto) separada del
      gameplay.
      Recursos: `systems/scoring` y `systems/typeconsole` del SDK;
      `consoleInitDefaultText` ya usado en `src/main.c` actual.
- [ ] **Carga de niveles desde ROM (streaming de datos)** — con múltiples
      niveles, no se puede tener todo en VRAM/RAM a la vez; hay que
      cargar/descargar datos de nivel según corresponda.
      Recursos: `wiki.superfamicom.org` (sección "Memory Mapping");
      `systems/memory_mapping` del SDK.
- [ ] **Guardado de partida (SRAM)** — si el juego tiene múltiples niveles,
      probablemente quieras persistir progreso; SNES usa SRAM en el
      cartucho con batería para esto.
      Recursos: `systems/sram` del SDK.
- [ ] **Presupuesto de rendimiento (cycle budget) por frame** — con más
      entidades y niveles más complejos, hay que empezar a medir cuántos
      ciclos de CPU consume cada sistema para no caer por debajo de 60 FPS.
      Recursos: `wiki.superfamicom.org` (sección "CPU Timing"); Mesen tiene
      profiler/contador de ciclos integrado.

---

## Etapa 5 — Pulido

**Objetivo:** llevar el juego completo (de la Etapa 4) a un estado pulido:
rendimiento estable, feedback visual/sonoro satisfactorio ("juice"),
dificultad balanceada, y sin bugs conocidos.

**Entregable(s):** una build "candidata a release" sin bugs críticos
conocidos, con feedback visual/sonoro en las acciones principales del
jugador.

**Criterio de salida:** playtesting completo del juego (por vos y, si es
posible, por alguien más) sin encontrar bugs bloqueantes ni caídas de
framerate perceptibles.

### Ítems técnicos a estudiar

- [ ] **"Game feel" / juice** (screen shake, partículas, feedback) — la
      diferencia entre un prototipo funcional y un juego que "se siente
      bien" suele estar en estos detalles de feedback, no en las mecánicas
      base.
      Recursos: charla "Juice it or lose it" (Martin Jonasson & Petri
      Purho, disponible en YouTube) — el concepto aplica igual aunque no sea
      específico de SNES.
- [ ] **Profiling y optimización de rendimiento** — con todo el contenido
      integrado, es probable que aparezcan cuellos de botella que no
      existían en el vertical slice; hay que medir antes de optimizar.
      Recursos: el profiler/debugger de Mesen; foros/wiki de la comunidad
      SNES homebrew para tips de optimización específicos del 65816.
- [ ] **Balance de dificultad y playtesting** — sin testeo externo es fácil
      sobreestimar qué tan fácil/difícil es tu propio juego, porque lo
      conocés de memoria.
      Recursos: ninguno técnico específico; es un proceso (conseguir
      playtesters), no una tecnología.
- [ ] **Debugging sistemático de bugs remanentes** — esta etapa es donde se
      cierra la mayoría de bugs conocidos antes de pasar a QA formal.
      Recursos: práctica general de debugging sistemático; el debugger de
      Mesen.
- [ ] **Menús (pausa, game over)** — son parte de la experiencia completa de
      un juego "terminado" y suelen postergarse hasta esta etapa porque no
      bloquean el gameplay core. _La pantalla de título se adelantó a la
      Etapa 1.5 como parte de la demo técnica de arranque._
      Recursos: `graphics/Backgrounds` y `systems/gastates` del SDK (para
      las transiciones de estado entre menús y gameplay).

---

## Etapa 6 — Alpha/Beta/QA y Lanzamiento

**Objetivo:** validar que el juego funciona correctamente en hardware real
de SNES (no solo en Mesen), pasar por un proceso de QA formal, y
empaquetar/distribuir el resultado final.

**Entregable(s):** una ROM final validada en al menos un dispositivo de
hardware real, con un plan de distribución definido (digital y/o físico).

**Criterio de salida:** el juego corre correctamente en hardware real de
principio a fin, sin diferencias de comportamiento respecto al emulador que
afecten la jugabilidad.

### Ítems técnicos a estudiar

- [ ] **Diferencias entre emulador y hardware real** — Mesen es muy
      preciso, pero ningún emulador es 100% idéntico al hardware; timings
      límite, quirks de PPU/APU no emulados o comportamientos de hardware
      real solo aparecen al testear en consola de verdad.
      Recursos: `wiki.superfamicom.org` (secciones que documentan
      comportamientos "reales" vs supuestos); comunidades de SNES homebrew.
- [ ] **Flashcarts para testing en hardware real** (SD2SNES/FXPAK Pro,
      EverDrive) — es la forma estándar de correr una ROM homebrew en una
      SNES real sin fabricar un cartucho físico dedicado; es un paso
      intermedio obligatorio antes de pensar en cartuchos.
      Recursos: documentación oficial de FXPak Pro (`krikzz.com`) y de
      EverDrive.
- [ ] **Checksum y cabecera de ROM (compatibilidad de hardware)** — la SNES
      real (y los flashcarts) validan ciertos campos de la cabecera del
      cartucho (ya generada en `hdr.asm`); errores acá pueden hacer que la
      ROM no arranque en hardware real aunque funcione perfecto en Mesen.
      Recursos: `wiki.superfamicom.org` (sección "ROM Header"); tu propio
      `hdr.asm` generado como caso de estudio.
- [ ] **QA formal** (test plans, matrices de prueba) — a esta escala de
      proyecto conviene un mínimo de proceso de QA estructurado (qué se
      prueba, en qué dispositivos, qué se considera "bug bloqueante") en vez
      de testeo puramente ad-hoc.
      Recursos: introducciones generales a "test plans para videojuegos"
      (no específico de SNES).
- [ ] **Empaquetado físico opcional** (PCB de cartucho, carcasas) — si en
      algún momento querés un cartucho físico real (no solo flashcart), hay
      un ecosistema de fabricantes de PCBs/carcasas de la escena homebrew a
      investigar.
      Recursos: comunidad de `romhacking.net` y foros de homebrew SNES
      (buscar proveedores de PCB homebrew específicamente al llegar a esta
      etapa).
- [ ] ~~**Distribución y consideraciones legales**~~ — **no aplica** por
      premisa (proyecto personal, sin distribución; ver Premisas). Se deja el
      texto original por si la premisa cambia: publicar un juego
      homebrew de SNES (ROM digital y/o físico) tiene convenciones de la
      escena (créditos, licencias de herramientas como PVSnesLib) y
      limitaciones legales (no se puede usar la marca "Super
      Nintendo"/Nintendo comercialmente).
      Recursos: licencia del repo `github.com/alekmaul/pvsneslib`; itch.io
      como plataforma de distribución digital estándar para homebrew.
