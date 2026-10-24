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

## Historial

| Fecha | Cambio |
|---|---|
| 2026-09-23 | Primera versión: plataformas al estilo de _Donkey Kong Country_, Etapas 0–6. |
| 2026-10-16 → 10-20 | Etapa 2 hecha como plataformas: Revisión 1 (`diseno-demo-jugable.md`) y Revisión 2, el tutorial de movimiento (`diseno-tutorial-movimiento.md`). |
| **2026-10-23** | **Cambio de género**: el juego pasa a ser un **beat 'em up de carril en profundidad** (_belt-scroll_) con Tarou, Bárbara y relevo entre ellos. Spec: [`diseno-beat-em-up.md`](diseno-beat-em-up.md) (Revisión 3); arte y trucos de hardware por pieza: [`arte-beat-em-up.md`](arte-beat-em-up.md). El código de plataformas queda congelado en el tag git **`plataformas-r2`** (commit `480f373`). Se reescriben Premisas, Concepto y las Etapas 2–6; las Etapas 0, 1 y 1.5 quedan como historia, con las frases de plataformas anotadas. |

> **Fuente de verdad.** Si este plan y `diseno-beat-em-up.md` se contradicen,
> manda la spec. Este documento solo ordena el trabajo y lo que hay que
> estudiar en cada etapa; las decisiones de diseño viven en la spec (sus
> números de decisión, «decisión N», se citan aquí tal cual).

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
- **Género (desde el 2026-10-23):** beat 'em up clásico de carril en
  profundidad (_belt-scroll_: _Final Fight_, _Streets of Rage_), **1 jugador
  con relevo** entre Tarou y Bárbara. El cooperativo a dos mandos es una meta
  futura, no un requisito (decisión 1). Hasta el 2026-10-22 fue un
  plataformas de scroll lateral (Revisiones 1–2, tag `plataformas-r2`).
- **Stack:** PVSnesLib (SDK en C sobre el compilador `tcc816` y el toolchain
  `wla-65816`/`wlalink`), formato de ROM **LoROM + FastROM, 128 bancos
  (4 MB)** (al principio fue LoROM/SlowROM de 8 bancos), emulador Mesen para
  desarrollo iterativo.
- **Reutilización:** de las Revisiones 1–2 se quedan los motores (lista de
  DMA, metasprite, fondo por columnas, cámara, motor de escenas, HUD de
  depuración); se retiran la física de plataformas, la rejilla de colisión,
  el tutorial y sus pruebas, que siguen en el tag. Se trabaja en la rama
  `demo-secuencia-arranque` (decisión 4).
- **Editor de niveles:** Tiled, para zonas y oleadas (decisión 28).
- **Arte:** Pixellab con el mismo pipeline que Tarou; placeholders numerados
  mientras tanto (decisión 22).
- **Distribución:** proyecto **personal**, sin venta ni distribución (decisión
  del 2026-09-23). Por eso se permite usar material original con copyright de
  la época (logo de Nintendo, logos de publisher/desarrollador) en uso
  privado; cada logo se aísla como asset reemplazable en `res/` por si la
  premisa cambia. Los ítems de distribución/legales de la Etapa 6 quedan como
  "no aplica".
- **Idioma:** los textos que se ven en pantalla (título, créditos, intro,
  "PRESS START") van en **inglés**; el código y sus comentarios, en español.

## Concepto del juego

El detalle completo está en [`diseno-beat-em-up.md`](diseno-beat-em-up.md);
aquí va el resumen que orienta el plan.

- **Protagonistas:** Tarou, un perro pastor (el husky es Fenrir; ver
  [`guion-historia.md`](guion-historia.md)), y Bárbara, su hermana mayor. Tarou es
  **adulto todo el juego** (decisión 9).
- **Historia (decisión 14):** Japón feudal. Un clan de **ninjas kendokas**
  humanos, liderado por el villano, roba el juguete favorito de Tarou, el
  **Señor Pimentón**, un pimiento rojo de goma con forma humana (en verdad,
  un dios antiguo). Bárbara jura venganza; Tarou quiere recuperar su juguete.
  El villano es **Nikoraemon**; el jefe de la fase 1, su lugarteniente
  **Gomasuri-dono**. Al vencerlo, Nikoraemon huye con el juguete, que es el
  gancho para la fase 2 (decisión 24). La historia completa tiene 5 fases y
  cierre en la última ([`guion-historia.md`](guion-historia.md)).
- **Perspectiva:** belt-scroll en x (avance), y (profundidad en el carril) y
  z (altura del salto), con sombra bajo los pies y orden de dibujo por y
  (decisión 2). El avance es por **zonas con oleadas**: la cámara se bloquea
  hasta vencer la oleada y después aparece «GO →» (decisión 3).
- **Relevo al estilo de _Donkey Kong Country_ (decisiones 10–11):** se juega
  con uno y el otro espera fuera; **nunca hay dos protagonistas en pantalla a
  la vez**. Botón L, golpe de entrada que empuja a los cercanos, enfriamiento
  de ~2 s. Barras de vida separadas; quien espera se cura poco a poco. Si cae
  el activo entra el otro; si caen los dos, se pierde una vida.
- **Dos estilos de lucha (decisiones 6–8):** combate a lo _Final Fight_
  (combo A-A-A con derribo al final, ataque aéreo, agarre con golpe o
  lanzamiento, especial A+B que cuesta vida). **Tarou**: katana en la boca
  y mordisco, rápido, combos largos; envaina solo fuera de combate.
  **Bárbara**: katana en la mano, lenta, mucho alcance y daño, buenos agarres.
- **Tres apoyos (decisiones 15–18):** **Lucinda**, la gata samurái (muy
  rápida, ráfaga de cortes), **Rex**, el perro monje telepático (levanta a
  todos los enemigos y los suelta) y **Fenrir**, el husky bruto (embestida que
  derriba a todos). Se
  invocan con X cuando la **barra de ataque**, compartida por el dúo y que se
  llena solo golpeando, está llena; R cambia el apoyo elegido. Lucinda se une en
  el bosque y Rex en su monasterio (fase 1, decisión 26); Fenrir, en la fase 2.
- **Enemigos (decisión 21):** ninja básico (shinai), ninja rápido (shuriken),
  kendoka pesado (agarra, bloquea) y, en la fase 1, un lugarteniente kendoka
  como jefe. Todos humanos (decisión 34).
- **Arcade (decisión 12):** 3 vidas, 3 continues, comida que cura y
  puntuación; armas arrojadizas y vida extra escondida (decisión 25).
- **Estructura de fases:** la fase 1 es aldea → bosque de bambú →
  monasterio de Rex → dojo con el jefe (decisión 23). El juego completo tiene
  **5 fases** (camino al castillo, castillo exterior, castillo interior y
  ritual final); jefes de las fases 2 a 4: Nigeashi-dono, Hankozuki-dono
  y Nebō-dono (`guion-historia.md` §2 y §6).
- **Plan de producción incremental:** la primera entrega (Etapa 3, Revisión
  3) es la **fase 1 completa**, ya con Bárbara, el relevo y los apoyos;
  antes, en la Etapa 2, una ROM de prueba de carga mide cuántos enemigos
  caben (decisiones 13 y 20). El audio va después de la Revisión 3 (decisión
  27).
- **Idea aparcada, no descartada: las etapas de vida de Tarou** (cachorro,
  adulto, viejo), que eran parte del concepto de plataformas. En la Revisión
  3 Tarou es adulto todo el juego; si la idea vuelve, se decide cuándo y cómo
  (spec §11, fuera de alcance).

## Vista general de las etapas de aprendizaje

| #   | Etapa                                                                   | Objetivo en una frase                                                                                          |
| --- | ----------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| 0   | [Fundamentos](#etapa-0--fundamentos)                                    | Aprender a programar (lógica, C) y entender cómo funciona una consola retro por dentro.                        |
| 1   | [Investigación / Preproducción](#etapa-1--investigación--preproducción) | Definir qué juego es y entender la arquitectura técnica del hardware SNES y del toolchain.                     |
| 1.5 | [Demo técnica: secuencia de arranque](#etapa-15--demo-técnica-secuencia-de-arranque) | Construir el código definitivo del arranque (logos → intro → título) y medir hasta dónde llega el hardware. |
| 2   | [Prototipo](#etapa-2--prototipo)                                        | Medir con una prueba de carga cuántos enemigos caben y programar un prototipo de combate en x/y/z. (Las Revisiones 1–2 de plataformas se hicieron aquí.) |
| 3   | [Vertical Slice](#etapa-3--vertical-slice)                              | La fase 1 completa (Revisión 3): cuatro zonas, relevo, apoyos y el jefe, todos los sistemas integrados.        |
| 4   | [Producción](#etapa-4--producción)                                      | Escalar la fase 1 a todas las fases del juego; el cooperativo a dos mandos, si se decide.                      |
| 5   | [Pulido](#etapa-5--pulido)                                              | Audio, rendimiento, juice, corrección de bugs, balance de dificultad.                                          |
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

_Nota (2026-10-23): ese GDD se escribió por partes, primero para el
plataformas (Revisiones 1–2, tag `plataformas-r2`:
`diseno-demo-jugable.md`, `diseno-tutorial-movimiento.md`) y ahora para el
beat 'em up: [`diseno-beat-em-up.md`](diseno-beat-em-up.md) resuelve el
relevo Tarou/Bárbara (§4) y la fase 1 (§7), y aparca las etapas de vida._

**Criterio de salida:** poder explicar sin notas qué es LoROM/SlowROM, qué
hace `hdr.asm`, qué son los 8 bancos de ROM, y tener el GDD escrito.
_(Hoy la ROM es LoROM + FastROM de 128 bancos.)_

### Ítems técnicos a estudiar

- [ ] **Qué es un Game Design Document (GDD) y cómo estructurarlo** — en la
      industria, ningún estudio arranca a programar sin definir qué está
      construyendo; evita retrabajo enorme más adelante.
      Recursos: "The Art of Game Design: A Book of Lenses" de Jesse Schell
      (capítulos iniciales).
- [x] **Análisis de referencias de plataformers** _(Revisión 1–2, plataformas, tag plataformas-r2; las referencias de beat 'em up se estudian en la Etapa 2)_ — entender qué hace
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
- [ ] **Formato de ROM: LoROM vs HiROM, SlowROM vs FastROM** — el proyecto
      empezó como LoROM/SlowROM y hoy es LoROM + FastROM de 128 bancos
      (`ROMSIZE 0C`, `FASTROM 1` en el `Makefile`); entender qué
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
- [x] **Estudio de la arquitectura de LikeMario** — era el ejemplo más
      cercano al género de entonces (plataformas, Revisión 1–2, tag
      plataformas-r2), y sigue sirviendo de referencia de "cómo
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
      _El diseño del relevo quedó fijado el 2026-10-23 en
      `diseno-beat-em-up.md` §4 (decisiones 10–11); su código es de la
      Etapa 3._
- [ ] **Diseño narrativo básico y estructura de misiones/niveles** — aunque
      el guion no está cerrado, cada nivel se organiza como una misión que
      Tarou y Bárbara completan juntos; conviene tener al menos un boceto
      de la lista de niveles/misiones en el GDD antes de entrar en
      Producción.
      _La premisa (el Señor Pimentón robado por el clan de ninjas kendokas)
      y la fase 1 están en `diseno-beat-em-up.md` (decisiones 14, 23 y 24);
      la lista de las 5 fases, el reparto y el final, en `guion-historia.md`
      (Rev. 3), con jefes, final y textos en inglés. Falta el arte de los jefes (§10)._
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

**Nota (2026-10-23):** con el cambio de género la secuencia de arranque se
queda, pero la **intro se rehace** (3 escenas nuevas, con placeholders en la
Revisión 3; decisión 5). Ese trabajo está en la Etapa 3. Start en el título
ya no lleva a «LEVEL 1 — COMING SOON» sino a la fase.

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
  _(Desde la Revisión 1 es Tarou adulto, con el arte de Pixellab.)_
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
      cámara que usará el nivel (y ahora las zonas del beat 'em up).
      Recursos: `graphics/Backgrounds/Mode1Scroll` del SDK.
- [x] **Sprites (OAM) y animación por cuadros** — Tarou cachorro en reposo en
      la intro y el título _(hoy, Tarou adulto)_.
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

**Objetivo:** antes de pedir arte o escribir combate de verdad, **medir**
cuánto aguanta el hardware en un beat 'em up (sprites por línea, tiles de
sprite, VBlank, CPU) y validar con un prototipo que el combate en x/y/z se
siente bien. Todo con placeholders. Spec: `diseno-beat-em-up.md` §8 y §9.

**Historia de esta etapa:** las Revisiones 1 y 2 (2026-10-16 → 10-20) se
hicieron aquí cuando el juego era un plataformas: Tarou adulto con el arte de
Pixellab en celdas de 96×64, física de salto, colisión contra una rejilla de
Tiled y un tutorial de movimiento de 2048 px (`diseno-demo-jugable.md`,
`diseno-tutorial-movimiento.md`). Ese código queda en el tag
**`plataformas-r2`**. Sus motores se reutilizan; la física de plataformas, la
rejilla de colisión, el tutorial y sus pruebas se retiran (decisión 4, spec
§10).

**Entregable(s):**

1. **La ROM de prueba de carga** (`-DPRUEBA_CARGA`, spec §9): Tarou o Bárbara
   y N ninjas placeholder de 64×96 (rectángulos numerados de colores) que se
   pasean por el carril con sombras, animando y subiendo cuadros por la lista
   de DMA, sin lógica de combate. Mide ranuras por línea, VRAM (opciones
   A/B/C de la spec §8), VBlank y CPU.
2. **El resultado escrito en la spec** (`diseno-beat-em-up.md` v2): cuántos
   enemigos a la vez, forma y tamaño de la celda del ninja (para pedirlos a
   Pixellab), sombras sí/no y a qué ritmo, turno de subida de cuadros, bandas
   de cine sí/no.
3. **Un prototipo de combate**: un protagonista y un ninja placeholder en un
   carril plano; movimiento en x/y, doble toque para correr, salto en z con
   sombra, orden de dibujo por y, combo con cajas de golpe y daño, la regla
   de profundidad (`TOLERANCIA_Y`), derribo, parada de impacto y destello. La
   lógica pura (`mundo`, `combate`, doble toque) con pruebas en el PC.

**Criterio de salida:** la configuración elegida corre a 60 FPS (HUD), sin
trozos perdidos en el caso típico (2–3 enemigos), con el VBlank por debajo de
la línea 37 en el peor frame (spec §9). El prototipo de combate se juega en
Mesen sin crashear y sus pruebas en el PC pasan.

**Estado (2026-10-23):** entregables 1 y 2 **hechos** (prueba de carga y
capa de sprites, spec §9.1 y §9.2). Entregable 3 empezado: la lógica pura
del prototipo de combate (`doble_toque.c`, `mundo.c`, `combate.c`, con las
cifras de `cifras.h`) ya tiene pruebas en el PC. El luchador de Tarou
(`luchador.c`: 24 estados para Tarou y Bárbara, combo, aire, patada voladora, especial, derribo, KO, agarre y entrada del relevo; `cambio_protagonista.c`: el relevo; `apoyos.c`: la barra de ataque y los tres apoyos) también
tiene pruebas. El ninja de prueba (`enemigos/ninja.c`) también tiene pruebas. Falta la pantalla que **sustituye al nivel 1** al pulsar Start
(decisión del autor); el código de plataformas se queda hasta la fase 1.

### Heredado de las Revisiones 1–2 (plataformas, tag plataformas-r2)

Lo que ya se aprendió y se sigue usando:

- [x] **El loop principal y VBlank/NMI** _(Revisión 1, 2026-10-16: `main.c` + `secuencia_vblank`; `docs/hardware-snes-por-modulo.md` §8)_ — toda la lógica del juego debe
      sincronizarse con el refresco de pantalla (60 Hz en NTSC); `src/main.c`
      ya usa `WaitForVBlank()` y hay que entender por qué.
      Recursos: `wiki.superfamicom.org` (sección "NMI/VBlank");
      `systems/timer` del SDK.
- [x] **Lectura de input (joypad)** _(Revisión 1, 2026-10-16: `juego/mando.c`)_ — sin poder leer el control no hay
      gameplay; PVSnesLib expone funciones para leer el estado del joypad.
      Recursos: ejemplo `input/controller` del SDK.
- [x] **Aritmética de punto fijo (fixed-point)** _(Revisión 1, 2026-10-16: 12.4 en `sistema/puntofijo.h`, probado en `tests/`)_ — el 65816 no tiene unidad
      de punto flotante (FPU); toda velocidad fraccionaria se representa con
      enteros escalados. En el beat 'em up, x, y y z siguen en 12.4.
      Recursos: artículos generales de "fixed point arithmetic for embedded
      systems"; el código de física de LikeMario como caso de estudio real.
- [x] **Física de plataformas 2D** _(Revisión 1–2, plataformas, tag plataformas-r2: gravedad, salto variable, terminal en `juego/tarou.c`)_ — se retira; lo que
      se aprendió (gravedad y salto en punto fijo) se reutiliza para el
      eje z del salto y de los lanzamientos.
      Recursos: "Retro Game Mechanics Explained" (YouTube).
- [x] **Detección y resolución de colisiones** (AABB, basada en tiles) _(Revisión 2, 2026-10-20: rejilla de 128×14 en `juego/colision.c`, en el tag plataformas-r2)_ — la
      rejilla de plataformas se retira; el carril por columnas y las cajas
      de golpe la sustituyen (ítems de abajo).
      Recursos: artículos generales sobre "AABB collision detection" y
      "tile-based collision".
- [x] **Máquinas de estado (state machines) para el personaje** _(Revisión 1, 2026-10-16: 13 estados en `juego/tarou.c`)_ — el
      mismo patrón (un `switch` por estado, una animación por estado) sirve
      para los luchadores del beat 'em up (spec §5.1).
      Recursos: `systems/gastates` del SDK; cualquier introducción general a
      "finite state machines" en videojuegos.
- [x] **Scroll de cámara y registros de BG** _(Revisión 1, 2026-10-16: `juego/camara.c`; desde la Revisión 2 el scroll se escribe en `pantalla_nivel1_vblank`)_ — la cámara se
      queda y gana el bloqueo de zona.
      Recursos: `wiki.superfamicom.org` (sección "Backgrounds");
      `graphics/Backgrounds` del SDK.
- [x] **Debugging con el debugger de Mesen** _(Revisión 1, 2026-10-16: HUD de depuración con medidor de línea; `docs/mesen-y-depuracion.md`)_ — el
      HUD de depuración (`V a b`, `LIN`) es la herramienta con la que se mide
      la prueba de carga.
      Recursos: documentación/wiki oficial de Mesen; `systems/debug` y
      `systems/breakpoints` del SDK.
- [x] **Metasprites** _(Revisión 1–2: `juego/metasprite.c`, tablas de `gfx4snes -T`, hasta 6 piezas por celda de 96×64, piezas vacías saltadas)_ — Bárbara
      (64×96) y los ninjas usan el mismo recorrido (decisión 33).
- [x] **DMA y la lista de DMA** _(Revisión 2, 2026-10-20/21: `juego/dma_lista.c` + `dma_ejecutar.asm`; doble celda de VRAM de Tarou subida en dos VBlanks)_ — toda
      escritura a la VRAM pasa por la lista, en el VBlank.
      Recursos: `wiki.superfamicom.org` (sección "DMA").
- [x] **Fondo por columnas en un mapa anillo de 64×32** _(Revisión 2: `juego/fondo_nivel.c`)_ — se
      reutiliza para las zonas.
- [x] **Niveles en Tiled y pipeline de arte de Pixellab** _(Revisión 2: `scripts/preparar_nivel.py`, `scripts/preparar_personaje.py`)_ — el
      sucesor es `scripts/preparar_zona.py` (spec §7).
- [x] **Pruebas en el PC de la lógica pura y ROM con guion** _(Revisión 2: `tests/`, `juego/guion.c`, `-DGUION_PRUEBA`)_ — las
      pruebas del tutorial salen con él; el método se queda.

### Ítems técnicos a estudiar (beat 'em up)

- [ ] **Referencias del género: _Final Fight_, _Streets of Rage_, y el cine de samuráis y ninjas de Cannon Films (los 80)** — tono de serie B, carteles de película, patada voladora, zoom al KO y entrada de los jefes (`diseno-beat-em-up.md` decisiones 40–42; `arte-beat-em-up.md` §11) — cómo
      se sienten el combo, el agarre, el derribo y los turnos de ataque; qué
      hacen cuando la pantalla se llena de sprites.
      Recursos: partidas y desgloses en YouTube; la spec §5 y §6.
- [ ] **El mundo en x/y/z** — x avance, y profundidad en el carril, z altura;
      `pantalla_y = y − z`; la sombra siempre en y. Es el modelo que hace que
      un juego 2D «parezca 3D» (spec §3). Módulo nuevo `mundo.c`, lógica
      pura probada en el PC.
- [ ] **Orden de dibujo por y y prioridad de sprites** — ordenar las
      entidades vivas (≤ 8) por y con inserción cada frame y escribir la OAM
      en ese orden; prioridad 2 o 3 contra los tiles de BG1 con el bit de
      prioridad para pasar detrás o delante de un poste (spec §3).
      Recursos: `docs/hardware-snes-por-modulo.md` (OAM y prioridades).
- [ ] **Cajas de golpe y cajas de daño por cuadro** — cada cuadro de
      animación lleva su caja de daño y, en los activos, su caja de golpe
      con daño y efecto; la regla de profundidad `TOLERANCIA_Y`; la tabla
      rellena a 8 bytes por entrada para que `a[i]` sea un desplazamiento y
      no `tcc__mul` (spec §5.2). Cómo se dibujan las cajas (Tiled o `.json`)
      queda abierto en la spec.
- [ ] **Un luchador cambia el estado de otro** — `combate.c`: el golpe que
      conecta pone al objetivo en `RECIBIR`/`DERRIBADO`, el agarre en
      `AGARRADO` (spec §5.1).
- [ ] **Doble toque para correr** — lógica pura sobre la lectura del mando,
      con `VENTANA_DOBLE_TOQUE` (~12 frames), probada con `guion.c`
      (spec §2).
- [ ] **Límite de 34 ranuras de 8 px por línea (y 32 sprites por línea)** —
      en belt-scroll todos comparten las líneas del carril; Tarou + 3 ninjas
      a la misma altura son 36 > 34. Medidas a decidir con la prueba de
      carga: recortar piezas vacías, sombras de 16 px, sombras a 30 Hz,
      rotar el orden cada frame (spec §8).
      Recursos: `docs/hardware-snes-por-modulo.md`; `wiki.superfamicom.org`
      (sección "OAM / Sprite limits").
- [ ] **512 tiles de sprite (16 KB)** — el protagonista usa 256 en doble
      búfer; 3 ninjas de 96 tiles no caben. Opciones A (piezas de 16×16
      recortadas), B (celda de 48×96) y C (2 grandes + 1 en pose residente)
      a medir (spec §8).
- [ ] **Presupuesto del VBlank** — 37 líneas, el SDK usa 5; ~125 bytes por
      línea; un cuadro de enemigo recortado (~2 KB) cabe uno por frame con un
      turno rotatorio de subida (spec §8).
- [ ] **Bandas de cine (forced blank)** — apagar la pantalla en las últimas
      ~16 líneas para alargar el VBlank ~2 KB más de DMA, con una IRQ de
      línea (`HTIMEN/VTIME`) o HDMA sobre `INIDISP`. Se decide con la prueba
      de carga (spec §8).
      Recursos: `wiki.superfamicom.org` (IRQ, `INIDISP`).
- [ ] **Presupuesto de CPU** — ~59 000 ciclos por frame; cajas solo para los
      que atacan, por pares, sin multiplicaciones; medido con `LIN`
      (spec §8).

---

## Etapa 3 — Vertical Slice

**Objetivo:** construir la **fase 1 completa** de _Tarou: A Dog's Journey_
como Revisión 3 (decisión 20): aldea → bosque de bambú → monasterio de Rex →
dojo, con Gomasuri-dono como jefe. Ya con los dos protagonistas, el relevo, los
tres apoyos, los tres tipos de enemigo, objetos, HUD del juego, pausa y la
intro rehecha. Es la prueba de que todos los sistemas del beat 'em up
funcionan juntos de punta a punta.

**Entregable(s):** una ROM donde se juega la fase 1 de principio a fin:
Start en el título → zona 1 (aldea, 3 oleadas) → zona 2 (bosque, 3 oleadas)
→ zona 3 (monasterio: escena con Rex, la barra se activa, una oleada
corta) → zona 4 (dojo: 1 oleada y el jefe) → escena del villano huyendo con
el Señor Pimentón → «STAGE CLEAR», puntuación → «STAGE 2 / COMING SOON»
(spec §7). Arte de Pixellab donde lo haya y placeholders numerados donde no
(decisión 22); la intro con placeholders. **Sin audio** (decisión 27).

**Criterio de salida:** la fase 1 se juega entera en Mesen a 60 FPS, sin
crashes ni trozos de sprite perdidos en el caso típico, con el VBlank dentro
de la línea 37; la lógica pura (mundo, combate, oleadas, IA, apoyos, doble
toque) tiene pruebas en el PC que recorren los datos reales de las zonas, y
una ROM con guion juega la fase sin manos.

### Ítems técnicos a estudiar

- [ ] **Bárbara y el relevo** — `barbara.c` y `cambio_protagonista.c` pasan
      de stub a código; los dos comparten la celda de VRAM y la paleta 0 (al
      relevar se copian 32 bytes de paleta); golpe de entrada, enfriamiento,
      curación de quien espera, caída de los dos (spec §4).
      `tarou_sprite.c` pasa a `protagonista_sprite.c`.
- [ ] **Envainar automático de Tarou** — regla del estado del juego (oleada
      activa o no), no un botón (decisión 8, spec §4).
- [ ] **Combate completo** — combo, agarre (golpe y lanzamiento, el lanzado
      golpea a otros), ataque aéreo, especial A+B que cuesta vida solo si
      conecta, invulnerabilidad al levantarse, parada de impacto, destello
      por la lista de DMA (spec §5.3).
- [ ] **IA por turnos** — como mucho 2 enemigos atacando a la vez (un
      «testigo» que se pasan); los demás rodean o esperan. Comportamientos
      del ninja básico, el rápido (proyectiles por y, cambia de carril) y el
      kendoka pesado (bloqueo frontal, agarre); el jefe por fases de vida
      (spec §6). Variantes por paleta.
- [ ] **Oleadas y zonas desde Tiled** — capa lógica de carril/pared/borde
      con celdas de 16 px; objetos `inicio`, `limite_zona`, `oleada`,
      `rompible`, `escena`, `jefe`, `salida`; `scripts/preparar_zona.py`
      genera `fondo_cerca.png` y `zonaN_datos.h` (spec §7). `oleadas.c`
      bloquea la cámara hasta vencer la oleada y muestra «GO →».
      Recursos: `mapeditor.org`; `scripts/preparar_nivel.py` como base.
- [ ] **Carril por columnas** — `carril_arriba[col]` / `carril_abajo[col]`
      deducidos por el script; una calle se estrecha, el bosque tiene el
      carril más fino (spec §3, §7).
- [ ] **Carga de zona con la pantalla apagada** — fundido con mosaico, cada
      zona con su propio juego de tiles (spec §7).
- [ ] **Paletas de sprite y matemática de color** — reparto fijo de las 8
      paletas: 0–3 cuerpos opacos; **4–7** sombras, chispas, apoyo activo y
      objetos, porque solo las paletas 4–7 entran en la matemática de color.
      El modo de color es **global** (`CGWSEL/CGADSUB`, como mucho por línea
      con HDMA): en el combate, media suma con la secundaria (las sombras
      oscurecen y el resto sale translúcido); la suma con el color fijo, solo
      en apoyos, pausa y escenas (spec §8).
      Recursos: `docs/hardware-snes-por-modulo.md` (CGRAM, `CGWSEL/CGADSUB`).
- [ ] **Apoyos y sus efectos de pantalla** — `apoyos.c` + `efectos.c`: la
      gata con **ventanas 1+2 dibujadas por HDMA** en diagonal y suma de
      color; el monje con **mosaico** pulsante y **scroll por línea** (HDMA
      sobre el scroll de BG2, ondulación); el bruto con temblor de pantalla
      (spec §5.4; detalle en `arte-beat-em-up.md` §6). Durante el apoyo la
      segunda celda de VRAM del protagonista se presta a sus cuadros.
      Recursos: el motor de escenas y la ventana del brillo del logo de
      Nintendo (`escena/efectos.c`), que ya usan HDMA indirecto.
- [ ] **HUD del juego en BG3** — vidas, barras, barra de ataque, apoyo
      elegido; prioridad alta; retratos en sprites en las líneas de arriba
      (spec §8). `hud/hud_juego.c`.
- [ ] **Objetos** — rompibles, comida que cura, armas arrojadizas (shuriken,
      kunai), vida extra escondida (decisión 25). `objetos/*`.
- [ ] **Escena del monasterio con el motor de escenas** — cuadros de texto
      en inglés; al terminar, la barra queda activa (decisión 26).
      `pantallas/pantalla_monasterio.c`.
- [ ] **Intro rehecha** — 3 escenas nuevas con placeholders (decisión 5).
- [ ] **Pausa** — Start pausa; en pausa, Select vuelve al título
      (decisión 32).
- [ ] **Mode 7 en escenas** — _decidido (2026-10-23): solo en la activación de la barra en el
      monasterio (`arte-beat-em-up.md` §5.3)._ Se dejó
      fuera de la demo de arranque (Etapa 1.5). Un sello de 128×128 gira y
      se acerca; después, mosaico y vuelta al Mode 1.
      Recursos: `graphics/Backgrounds/Mode7` del SDK.
- [ ] **Organización de banking de ROM** — con 128 bancos de 32 KB, cada
      zona, personaje y tira de animación en su sección `superfree`.
      Recursos: `data.asm` del proyecto; `wiki.superfamicom.org` (sección
      "Memory Mapping / Banking").

**Pendientes de la spec que tocan esta etapa** (spec §12): nombres del
villano, del lugarteniente y de los apoyos; el sprite del Señor Pimentón
(dejarlo en `res/descargas/graficos/` y decir su tamaño); el ataque
corriendo (propuesto, no decidido); las cifras de combate, que se ajustan
jugando.

---

## Etapa 4 — Producción

**Objetivo:** escalar la fase 1 a todas las fases del juego: más zonas,
más enemigos y jefes, el villano, y el contenido que la Revisión 3 dejó fuera
a propósito. Aquí se decide y, si se hace, se construye el **cooperativo a
dos mandos**, que hoy es una meta futura, no un requisito (decisión 1).

**Entregable(s):** todas las fases que se decidan, jugables de principio a
fin, con su contenido (enemigos, jefes, objetos, escenas) integrado.

**Criterio de salida:** el juego se puede jugar de principio a fin (todas
las fases) sin crashes ni bloqueos, aunque todavía no esté balanceado ni
pulido.

**Pendiente de decidir** (no está en la spec): cuántas fases tiene el juego,
dónde transcurre cada una, sus enemigos y jefes nuevos; si se hace el
cooperativo y cómo encaja con el relevo; si hay un mapa de viaje entre fases
(fuera de alcance de la Revisión 3, spec §11); si vuelven las etapas de vida
de Tarou (aparcadas, decisión 9).

### Ítems técnicos a estudiar

- [ ] **Producción de fases con el pipeline de Tiled** — repetir el ciclo
      `.tmj` → `preparar_zona.py` → datos → pruebas en el PC con cada fase
      nueva; cada zona con su juego de tiles.
- [ ] **Más tipos de enemigo y jefes** — escalar la IA por turnos y las
      variantes por paleta; jefes por fases de vida; el villano (que en la
      fase 1 solo huye).
- [ ] **Cooperativo a dos mandos (si se decide)** — dos protagonistas a la
      vez rompe la regla de una sola celda de VRAM y un solo presupuesto de
      sprites (spec §4, §8): habría que volver a medir con la prueba de
      carga.
      Recursos: `input/controller` del SDK (segundo mando).
- [ ] **Gestión de múltiples entidades y límites de sprites** — con más
      enemigos, proyectiles y efectos, mantener el caso típico dentro de las
      34 ranuras por línea y los 512 tiles de sprite.
- [ ] **Carga de fases desde ROM (streaming de datos)** — no todo cabe en
      VRAM/RAM; cargar y descargar datos por zona y por fase.
      Recursos: `wiki.superfamicom.org` (sección "Memory Mapping").
- [ ] **Guardado de partida (SRAM)** — _pendiente de decidir_: es un juego
      arcade de vidas y continues (decisión 12); si hay guardado entre
      fases, va en SRAM.
      Recursos: `systems/sram` del SDK.
- [ ] **Presupuesto de rendimiento (cycle budget) por frame** — con más
      entidades, medir cuántos ciclos consume cada sistema para no caer de
      60 FPS.
      Recursos: `wiki.superfamicom.org` (sección "CPU Timing"); el profiler
      de Mesen; el HUD `LIN`.
- [ ] **Idea aparcada: etapas de vida de Tarou** (cachorro, adulto, viejo)
      — solo si el autor la recupera: disparador, sprites alternativos y
      cambios mecánicos.

---

## Etapa 5 — Pulido

**Objetivo:** llevar el juego completo (de la Etapa 4) a un estado pulido:
**audio** (que se pospuso tras la Revisión 3, decisión 27), rendimiento
estable, feedback visual y sonoro satisfactorio ("juice"), dificultad
balanceada, y sin bugs conocidos.

**Entregable(s):** una build "candidata a release" con música y efectos de
sonido, sin bugs críticos conocidos y con feedback en las acciones
principales del jugador.

**Criterio de salida:** playtesting completo del juego (por vos y, si es
posible, por alguien más) sin encontrar bugs bloqueantes ni caídas de
framerate perceptibles, con audio en todas las pantallas.

### Ítems técnicos a estudiar

- [ ] **Pipeline de audio SPC** (`.it` → `smconv` → soundbank;
      `.wav` → `snesbrr` → `.brr`) — pendiente desde la Etapa 1.5: jingles
      de logos, música de arranque, título y fases.
      Recursos: `audio/music` y `audio/effectsandmusic` del SDK;
      `pvsneslib_snesmod.txt`; la unidad 17 del curso.
- [ ] **Formato de audio BRR y el SPC700** — efectos de golpe, corte,
      derribo, relevo y apoyos.
      Recursos: `wiki.superfamicom.org` (sección "SPC700 / Audio").
- [ ] **Música con trackers (formato .it)** — una pista por zona o fase, más
      el jefe.
      Recursos: documentación de OpenMPT; `audio/music` del SDK.
- [ ] **"Game feel" / juice** — en un beat 'em up es la parada de impacto,
      el destello, las chispas, el temblor de pantalla y el sonido de cada
      golpe; ajustar las cifras de combate jugando (spec §12).
      Recursos: charla "Juice it or lose it" (Martin Jonasson & Petri Purho).
- [ ] **Profiling y optimización de rendimiento** — medir antes de
      optimizar, con todo el contenido integrado.
      Recursos: el profiler/debugger de Mesen; el HUD de depuración.
- [ ] **Balance de dificultad y playtesting** — vida y daño por enemigo,
      tamaño de las oleadas, coste del especial, ritmo de la barra de ataque.
      Recursos: conseguir playtesters (es un proceso, no una tecnología).
- [ ] **Debugging sistemático de bugs remanentes** — esta etapa cierra la
      mayoría de bugs conocidos antes de pasar a QA formal.
      Recursos: el debugger de Mesen; las ROM con guion.
- [ ] **Menús y pantallas de fin (game over, continue, puntuación final)** —
      la pausa y el «STAGE CLEAR» llegan en la Etapa 3; aquí se pulen y se
      completan. _La pantalla de título se adelantó a la Etapa 1.5._
      Recursos: `systems/gastates` del SDK.
- [ ] **Arte final de la intro** — fuera de alcance de la Revisión 3
      (spec §11); sustituye a los placeholders.

---

## Etapa 6 — Alpha/Beta/QA y Lanzamiento

**Objetivo:** validar que el juego funciona correctamente en hardware real
de SNES (no solo en Mesen) y pasar por un proceso de QA formal. Por premisa
(proyecto personal), no hay distribución.

**Entregable(s):** una ROM final validada en al menos un dispositivo de
hardware real.

**Criterio de salida:** el juego corre correctamente en hardware real de
principio a fin, sin diferencias de comportamiento respecto al emulador que
afecten la jugabilidad.

### Ítems técnicos a estudiar

- [ ] **Diferencias entre emulador y hardware real** — Mesen es muy
      preciso, pero ningún emulador es 100% idéntico al hardware. En este
      género conviene revisar en consola lo que vive al límite: el VBlank
      lleno de DMA, las bandas de cine (forced blank a mitad de pantalla), las
      IRQ de línea, el HDMA de ventanas y scroll de los apoyos y el
      límite de sprites por línea.
      Recursos: `wiki.superfamicom.org`; comunidades de SNES homebrew.
- [ ] **Flashcarts para testing en hardware real** (SD2SNES/FXPAK Pro,
      EverDrive) — la forma estándar de correr una ROM homebrew en una SNES
      real; comprobar que la ROM FastROM de 4 MB arranca.
      Recursos: documentación oficial de FXPak Pro (`krikzz.com`) y de
      EverDrive.
- [ ] **Checksum y cabecera de ROM (compatibilidad de hardware)** — la SNES
      real (y los flashcarts) validan campos de la cabecera (`hdr.asm`);
      errores acá pueden hacer que la ROM no arranque en hardware real.
      Recursos: `wiki.superfamicom.org` (sección "ROM Header").
- [ ] **QA formal** (test plans, matrices de prueba) — qué se prueba, en
      qué dispositivos, qué es un bug bloqueante. Las ROM con guion y las
      pruebas en el PC son la base automática; el plan añade las partidas a
      mano (relevo en cada estado, apoyos con la pantalla llena, caída de los
      dos protagonistas, continues).
      Recursos: introducciones generales a "test plans para videojuegos".
- [ ] **Empaquetado físico opcional** (PCB de cartucho, carcasas) — si en
      algún momento querés un cartucho físico real, hay un ecosistema de
      fabricantes de la escena homebrew a investigar.
      Recursos: `romhacking.net` y foros de homebrew SNES.
- [ ] ~~**Distribución y consideraciones legales**~~ — **no aplica** por
      premisa (proyecto personal, sin distribución; ver Premisas). Se deja el
      texto original por si la premisa cambia: publicar un juego
      homebrew de SNES (ROM digital y/o físico) tiene convenciones de la
      escena (créditos, licencias de herramientas como PVSnesLib) y
      limitaciones legales (no se puede usar la marca "Super
      Nintendo"/Nintendo comercialmente).
      Recursos: licencia del repo `github.com/alekmaul/pvsneslib`; itch.io
      como plataforma de distribución digital estándar para homebrew.
