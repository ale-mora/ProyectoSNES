# Mapa de archivos del proyecto

**Para quién:** la persona que va a mantener este juego sin haberlo escrito.
**Qué es:** una ficha por archivo del repositorio: qué hace, con qué se conecta
(qué incluye y quién lo usa) y qué parte del hardware de la SNES toca. Los
términos técnicos están definidos en `docs/glosario-snes.md`; aquí se enlazan
con «→ glosario». Los documentos hermanos son `tuberia-de-compilacion.md`
(cómo se convierte todo esto en una ROM), `mesen-y-depuracion.md` (cómo se
ejecuta y se mira por dentro) y `hardware-snes-por-modulo.md` (qué registro de
qué chip toca cada módulo). Índice de toda la documentación: `docs/README.md`.

**Última revisión:** 2026-10-16, tras la Revisión 1 (Tarou adulto jugable).

## 1. Vista de pájaro

```
ProyectoSNES/
├── src/            el código C del juego (y nada más): lo compila tcc816
│   ├── sistema/    arranque, máquina de estados de pantallas, utilidades, tipos
│   ├── pantallas/  una pantalla por archivo: logos, intro, título, nivel 1
│   ├── juego/      la lógica del nivel: protagonista, colisión, reglas, mando, cámara, fondo por columnas, HUD
│   ├── enemigos/ objetos/ hud/ audio/ niveles/   stubs de subsistemas futuros
├── data.asm        lo que va "pegado" en la ROM: tiles, paletas, mapas, tablas HDMA
├── res/            assets preparados (PNG indexados, BMP) que convierte gfx4snes
├── scripts/        Python y PowerShell: preparan assets y verifican en Mesen
├── tests/          pruebas de la lógica pura en PC (gcc)
├── docs/           diseño, glosario, plan, este mapa
├── aprendizaje/    unidades de estudio (plan en aprendizaje/PLAN.md)
├── examples/       LikeMario, el ejemplo del SDK que sirve de referencia
├── tools/          SDK PVSnesLib y Mesen (caja negra: no se edita, no se versiona)
├── Makefile        reglas de compilación propias + include de snes_rules del SDK
├── run.bat         compila desde cero y abre Mesen
└── .vscode/        tareas de VS Code (compilar, limpiar) e IntelliSense
```

### Quién incluye a quién en `src/`

Flechas: «A → B» significa «A hace `#include` de B». Las cabeceras se incluyen
con ruta desde `src/` (el Makefile añade `-Isrc`).

```
main.c ──────────► sistema/secuencia.h ──► sistema/pantallas.h ──► <snes.h>
  │                                                  ▲
  ├──► sistema/comun.h                               │ (todas las pantalla_*.c)
  └──► pantallas/personajes.h ◄── pantalla_intro.c, pantalla_titulo.c
                 │
                 └──► juego/tarou_sprite.h ──► juego/tarou.h ──┬─► sistema/tipos.h
                                                               ├─► sistema/puntofijo.h
pantalla_nivel1.c ──► juego/nivel.h, juego/camara.h,           ├─► juego/mando.h
                      juego/tarou.h, juego/tarou_sprite.h,     └─► juego/animaciones.h
                      juego/mando.h, juego/hud_debug.h,
                      juego/fondo_nivel.h, juego/colision.h, juego/partida.h,
                      nivel1_datos.h   (generado en res/nivel1/ por preparar_nivel.py)
juego/tarou.c, juego/partida.c ──► juego/colision.h ──► sistema/tipos.h
juego/partida.c ──► juego/tarou.h, juego/nivel.h
juego/fondo_nivel.c ──► juego/nivel.h, sistema/comun.h
juego/mando.c ──► juego/guion_tutorial.h ──► juego/guion.h ──► juego/mando.h   (solo con -DGUION_PRUEBA)
juego/animaciones.c ──► tarou_anim.h   (generado en res/personajes/tarou/; en tests/, una copia fija)
juego/tarou_sprite.c ──► tarou_<anim>_meta.inc (13, generados por gfx4snes -T)
juego/tarou_sprite.c, pantallas/personajes.c ──► juego/metasprite.h ──► sistema/tipos.h   (lógica pura: contar piezas y espejar)
pantallas/personajes.c ──► tarou_reposo_estatico_meta.inc   (generado por gfx4snes -T a partir de la tira estática)
tests/*.c ──► tests/minitest.h ──► sistema/tipos.h   (+ los .h de juego/ que prueban)
```

Regla de capas: **la lógica pura** (`tarou.c`, `camara.c`, `nivel.c`,
`animaciones.c`, `metasprite.c`, `colision.c`, `partida.c`, `guion.c`,
`guion_tutorial.c`) incluye solo `sistema/tipos.h` y cabeceras de `juego/`; nunca
`<snes.h>`. Así se compila también en el PC. **La capa de hardware**
(`mando.c`, `tarou_sprite.c`, `fondo_nivel.c`, `hud_debug.c`, las pantallas, `comun.c`) sí
incluye `<snes.h>` y es la única que toca la PPU, la OAM y el mando.

## 2. Raíz del repositorio

| Archivo | Qué hace | Se conecta con | Hardware |
|---|---|---|---|
| `Makefile` | Define `ROMNAME`, `ROMTITLE`, `ROMSIZE 0C`/`ROMBANKS 128`/`FASTROM 1` (4 MB, LoROM FastROM), una regla `gfx4snes` por asset, la lista `ASSETS`, `bitmaps` y `data.obj: $(ASSETS)`; luego incluye `snes_rules` del SDK y añade `-Isrc -Ires/personajes/tarou` a `CFLAGS`. | Lo ejecuta `make` (desde `run.bat`, PowerShell o la tarea de VS Code). Lee `res/**`, produce `.pic/.pal/.map/_meta.inc`, `.ps/.asm/.obj`, `linkfile`, `hdr.asm`, `.sfc/.sym/.log`. Detalle regla por regla en `tuberia-de-compilacion.md`. | ninguno (describe la ROM: LoROM, FastROM, 128 bancos) |
| `data.asm` | Ensamblador que "pega" los binarios en la ROM con `.incbin` y les da etiquetas (`nintendo_til`, `tarou_pal`...). Una sección `superfree` por asset (≤ 32 KB cada una). | Incluye `hdr.asm`. Lee los `.pic/.pal/.map/.as` de `res/`. Lo usan todos los `.c` que declaran `extern char etiqueta;`. | ROM: bancos de 32 KB (→ glosario: Banco / LoROM) |
| `run.bat` | `make clean` + `make` + abre Mesen con la ROM; fija `PVSNESLIB_HOME`, `DEVKITSNES` y el `PATH` de la sesión. Se detiene con `[ERROR]` si falla. | Llama a `make` (MSYS2) y a `tools/mesen/Mesen.exe`. | — |
| `CLAUDE.md` | Guía para Claude Code: comandos, arquitectura, reglas del proyecto (comentarios en español, texto del juego en inglés, `make clean` tras tocar cabeceras). | La lee el agente al empezar una sesión. | — |
| `.clang-format` | Estilo de formato de C aceptado: 4 espacios, llaves Allman, `case` al nivel del `switch`, comentarios finales alineados, includes sin reordenar. | Lo usa la extensión C/C++ de VS Code al formatear. | — |
| `.gitignore` | Qué no se versiona: `tools/`, artefactos (`.sfc .sym .obj .log hdr.asm linkfile *.exe`), salidas de gfx4snes bajo `res/`, descargas de terceros, capturas de Mesen. | — | — |
| `hdr.asm` (generado) | Cabecera de la ROM: nombre, mapa de bancos, vectores de interrupción (NMI, RESET). Lo genera `snes_rules` desde `hdr.asm.in` del SDK con `ROMTITLE`, `ROMSIZE`, `ROMBANKS`. **No editar.** | Lo incluye `data.asm`; lo lee `wla-65816`. | CPU: vectores RESET/NMI; cabecera que lee la consola/el emulador |
| `linkfile` (generado) | Lista de `.obj` (los nuestros + los de la librería) que `wlalink` enlaza. | Lo escribe la regla `%.sfc` de `snes_rules`. | — |
| `ProyectoSNES.sfc` (generado) | La ROM: 524 288 bytes. Es lo que ejecuta Mesen. | — | todo |
| `ProyectoSNES.sym` / `.symfull` (generados) | Símbolos (etiqueta → dirección) en formato WLA-DX; el `.sym` está filtrado "para el depurador de Mesen" (sin `SECTIONSTART_`...). | Los carga Mesen junto a la ROM (ver `mesen-y-depuracion.md`). | — |
| `ProyectoSNES.log` (generado) | Salida completa de la compilación: órdenes, avisos, y el informe de `wlalink` con el espacio libre por banco de ROM y por zona de RAM. | Lo resume `make` al final ("COMPILATION SUMMARY"). | — |

## 3. `src/sistema/` — arranque y máquina de estados

| Archivo | Qué hace | Incluye / Lo usan | Hardware |
|---|---|---|---|
| `main.c` | Punto de entrada. Deja la PPU en Mode 1 con la consola de texto, limpia la OAM, arranca la secuencia en `PANTALLA_NINTENDO` y entra en el bucle infinito: `secuencia_actualizar()` → `WaitForVBlank()` → `secuencia_vblank()`. Una vuelta = un frame (60 por segundo). | Incluye `<snes.h>`, `sistema/secuencia.h`, `sistema/comun.h`, `pantallas/personajes.h`. Lo llama el código de arranque del SDK. | PPU (modo, brillo, encendido), OAM (`oamInit`), CPU (espera la NMI) |
| `secuencia.c` / `.h` | La máquina de estados de pantallas: `pantalla_actual` + tres `switch` (`_iniciar` al entrar, `_actualizar` por frame, `_vblank` tras el VBlank) y el contador `frames_en_pantalla`. | Incluye `sistema/pantallas.h`. Lo usan `main.c` y todas las pantallas (`secuencia_cambiar`, `frames_en_pantalla`). | ninguno |
| `pantallas.h` | El `enum Pantalla` (orden = orden de la secuencia), las duraciones en frames (`SEGUNDOS(n)`), `FRAMES_FADE`, `PERMITE_SALTO` y los prototipos `pantalla_X_iniciar/_actualizar(/_vblank)`. Tras cambiarlo: `make clean`. | Incluye `<snes.h>`. Lo usan `secuencia.*` y cada `pantalla_*.c`. | ninguno |
| `comun.c` / `.h` | Utilidades compartidas: reparto de la VRAM (`VRAM_SPRITES`, `VRAM_FONDO_A/B`, `VRAM_FUENTE`, `VRAM_MAPA_*`), `video_texto_mode1()`, fuentes 4bpp/2bpp, `fondo16_cargar()`/`fondo256_cargar()`, texto centrado y máquina de escribir, `fade_in/out`. | Lee `fuente_consola_*` y `fuente_2bpp_*` de `data.asm`. Lo usan todas las pantallas y el HUD (`VRAM_MAPA_TEXTO`, `limpiar_texto`). | PPU (modo, punteros de fondos, brillo), VRAM y CGRAM por DMA |
| `tipos.h` | `u8/u16/s8/s16` iguales en la SNES (`<snes.h>`) y en el PC (`<stdint.h>` con `-DPRUEBAS_PC`). | Lo incluye toda la lógica pura y `tests/minitest.h`. | ninguno |
| `puntofijo.h` | Punto fijo 12.4: `PF(n)` (× 16) y `PF_ENTERO(x)` (÷ 16, solo para valores no negativos). | `juego/tarou.*`, `camara.c`, `tarou_sprite.c`, `hud_debug.c`, `tests/prueba_puntofijo.c`. | ninguno (→ glosario: Punto fijo) |

## 4. `src/pantallas/` — una pantalla por archivo

Todas siguen el mismo patrón: `_iniciar()` carga gráficos y hace el fade in;
`_actualizar()` cuenta frames, lee el mando y pide el cambio de pantalla.

| Archivo | Qué hace | Incluye / Lo usan | Hardware |
|---|---|---|---|
| `pantalla_nintendo.c` | (1) Logo Nintendo + "LICENSED BY NINTENDO", 244 frames, animado por el **motor de escenas**: `_iniciar` carga el logo y prepara el destello, `_actualizar` avanza la escena y sale al terminar, `_vblank` aplica los efectos. | `nintendo_*` de `data.asm`; `escena/escena_nintendo.h`, `escena/efectos.h`; `secuencia.c`. | PPU y VRAM vía `comun.c`; brillo, mosaico, ventana, color y HDMA vía `efectos.c` |
| `pantalla_publisher.c` | (2) Logo del publisher (Capcom del SDK, provisional), 3 s. | `publisher_*`; `secuencia.c`. | ídem |
| `pantalla_devs.c` | (3) Logo del estudio y créditos con la consola de texto, 4 s; entra de golpe (`setBrightness(15)`). | `devs_*`; `secuencia.c`. | ídem + brillo directo |
| `pantalla_dedicatoria.c` | (4) "A la memoria de Tarou" letra a letra sobre negro, fundidos lentos, 7 s. | `comun.c` (`escribir_parcial`); `secuencia.c`. | PPU vía `comun.c` |
| `pantalla_intro.c` | (5) Tres escenas de 8 s: dos en Mode 3 (256 colores) y una en Mode 1 con los sprites; texto letra a letra; sale al título con mosaico. | `escena1/2/3_*`; `personajes.h`; `secuencia.c`. | PPU: Mode 3/1, mosaico (`REG_MOSAIC`); VRAM/CGRAM (Mode 3 pisa las 256 entradas de color y la VRAM de sprites) |
| `pantalla_titulo.c` | (6) Título: capa cercana con scroll por software (parallax), capa lejana con el logotipo horneado, cielo HDMA, PRESS START parpadeando, sprites en reposo; Start → nivel 1; 15 s sin tocar nada → intro. | `cerca_*`, `lejos_*`, `hdmaGradcieloList`; `personajes.h`; `secuencia.c`. | PPU: scroll (`bgSetScroll`), prioridad BG3; HDMA canal 6; OAM vía `personajes.c`; mando |
| `pantalla_nivel1.c` | (7) El nivel 1, el tutorial. Por frame: `mando_leer` → `tarou_actualizar` → `partida_actualizar` (reaparecer, tronco roto, salida) → `camara_para` → scroll de dos capas → `dma_lista_vaciar` al empezar; `tarou_sprite_dibujar` → `fondo_nivel_actualizar` → HUD (cada uno apunta sus copias en la lista); en `_vblank`: `dma_lista_ejecutar`, y la medida `V` (líneas del VBlank usadas). `reaparecer()`: fundido, pantalla apagada, Tarou al último cartel, anillo reescrito, fundido. La salida lleva a `PANTALLA_PROXIMAMENTE`; START vuelve al título, SELECT alterna el HUD. Mide la línea del haz (`$2137/$213D`) para el medidor `LIN`. | `nivel1_colision`, `nivel1_lejos_*`, `hdmaGradcieloList`; `nivel1_datos.h`; `juego/*.h`; `secuencia.c`. | PPU: modo, scroll, BG2 on/off; HDMA; OAM/VRAM por `tarou_sprite.c`; mando por `mando.c`; contadores del haz |
| `pantalla_proximamente.c` | (8) «LEVEL 2 / COMING SOON» unos segundos tras la salida del tutorial; vuelve al título. Desaparecerá cuando exista el nivel 2. | `pantallas.h` (`DURACION_PROXIMAMENTE`), `secuencia.h`, `comun.h`. | PPU por `comun.c` |
| `prueba_carga.c` / `.h` | **ROM de prueba de carga** del beat 'em up (solo con `-DPRUEBA_CARGA`; `main.c` llama a `prueba_carga_ejecutar`, que tiene su propio bucle), ya con la capa de sprites: Tarou andando en piezas de 16×16 (doble búfer, entra en el orden por profundidad) + hasta 3 ninjas de 64×96 (un búfer por hueco, una subida por VBlank), sombras con media suma; entradas fijas en la tabla de sprites; recorre 8 configuraciones de 6 s y el HUD enseña máximos de CPU (tras subidas, tabla y OAM), VBlank, frames perdidos y espera de subida (`docs/diseno-beat-em-up.md` §9.2). | `tarou_caminar_p16`, `tarou_p16_bancos`, `ninja64_*`, `ninja_pal`, `sombra_*`, `ninja_bancos` de `data.asm`; `juego/sprites.h`, `subida.h`, `p16.h`, `dma_lista.h`, `fondo_nivel.h`, `hud_debug.h`. | OAM por `sprites_oam.asm`, VRAM por la lista, CGRAM paletas de sprites 0..4, `TS`/`CGWSEL`/`CGADSUB`, HDMA canal 6, contadores del haz |
| `personajes.c` / `.h` | Sprites de Tarou y Bárbara para intro y título con carga **estática**: la tira `tarou_reposo_estatico` (siempre 2 cuadros de 96×64; piezas de 32×32 según su tabla metasprite, `tile = (n/4)·64 + (n%4)·4`, **hasta** 6 por cuadro: las entradas sobrantes se esconden) en VRAM 0x0000 (tiles 0..191) y Bárbara (64×64) en la segunda tabla de tiles (256, VRAM 0x1000); `mostrar/animar/ocultar`. | `tarou_reposo_estatico_til`, `tarou_pal`, `barbara_samurai_*` de `data.asm`; `tarou_reposo_estatico_meta.inc`; `juego/metasprite.h` (`metasprite_piezas`); lo usan `pantalla_intro.c`, `pantalla_titulo.c`, `main.c`. | OAM (`oamSet`, `oamSetEx`, `oamSetXY`), VRAM (`dmaCopyVram`), CGRAM (`dmaCopyCGram`), `oamInitGfxAttr` |

## 4b. `src/escena/` — el motor de escenas

| Archivo | Qué hace | Incluye / Lo usan | Hardware |
|---|---|---|---|
| `escena.h` / `escena.c` | **Lógica pura**: tabla de eventos (`RAMPA`, `FIJAR`, `FIN_ESCENA`; 8 bytes cada uno, paso en 8.8 calculado por el compilador) que mueve 4 pistas (brillo, mosaico, destello, luz). `escena_empezar`, `escena_avanzar` (por frame), `ESCENA_VALOR`. | `tipos.h`. Lo usan `pantalla_nintendo.c`, `tests/prueba_escena.c`. | **ninguno** (se prueba en PC) |
| `efectos.h` / `efectos.c` | Lleva las pistas a la PPU en el VBlank: `efectos_preparar(fondos)`, `efectos_destello(arriba, alto, x_min, x_max, ancho)` (calcula una vez los pares de ventana y arma la tabla HDMA indirecta), `efectos_aplicar(&escena)`, `efectos_terminar()`. | `<snes.h>`, `escena.h`. Lo usa `pantalla_nintendo.c`. | `INIDISP`, `MOSAIC`, `WOBJSEL`/`WH0`/`WH1`, `CGWSEL`/`CGADSUB`/`COLDATA`, canal 6 de HDMA (`$4360-$4367`, `$420C`) |
| `escena_nintendo.h` / `escena_nintendo.c` | La tabla del logo Nintendo (8 eventos, `ESCENA_NINTENDO_FRAMES = 244`). | `escena.h`. | ninguno |

## 5. `src/juego/` — la lógica del nivel

| Archivo | Qué hace | Incluye / Lo usan | Hardware |
|---|---|---|---|
| `tarou.h` / `tarou.c` | **Lógica pura** del protagonista: `struct Tarou` (posición y velocidad en 12.4, estado, espada, dirección, animación, contador de sueño), las constantes de §5 de la spec, `tarou_iniciar`, `tarou_actualizar(t, m)` (transiciones → aceleración → gravedad → `mover_x` contra las paredes → `mover_y`: suelo con dos pies, plataformas de un solo sentido, techo, caer por un borde → animación), `cabe_de_pie` (no se levanta bajo un techo), `tarou_caja_cuerpo`, `tarou_caja_golpe`. 13 estados con la prioridad R > A > B > Abajo > cruceta. | `tipos.h`, `puntofijo.h`, `mando.h`, `animaciones.h`, `nivel.h`, `colision.h`. Lo usan `pantalla_nivel1.c`, `tarou_sprite.c`, `hud_debug.c`, `tests/prueba_tarou.c`. | **ninguno** (se prueba en PC) |
| `animaciones.h` / `.c` | `enum Estado` (13), `AnimDef` y las tablas `ANIMACIONES` (cuadros de `tarou_anim.h`, frames por cuadro, bucle) y `NOMBRES_ESTADO` (texto para el HUD). | `tipos.h`, `tarou_anim.h`. Lo usan `tarou.c`, `tarou_sprite.c`, `hud_debug.c`, tests. | ninguno |
| `metasprite.h` / `.c` | **Lógica pura** de las tablas metasprite de `gfx4snes -T`: `metasprite_piezas` (cuenta hasta el terminador `0xFF80`, máximo 6, la celda de 96×64; el conversor **omite** los bloques transparentes, así que un cuadro puede tener menos de 6), `metasprite_espejar` (copia mirando a la izquierda: `dx → 64 − dx`, bit `hflip`, terminador detrás de la última pieza) y `metasprite_copias` (el plan de DMA para subir un cuadro: la tira guarda los bloques de 4 en 4 por fila de 128 px, como la hoja de sprites de la VRAM; bloques seguidos de una fila se copian de un tirón; 8 a 16 copias por cuadro; `CopiaBloque` mide 8 bytes a propósito). En el PC define la misma `t_metasprite` que `<snes.h>`. | `tipos.h`. Lo usan `tarou_sprite.c`, `pantallas/personajes.c`, `tests/prueba_metasprite.c`. | **ninguno** (se prueba en PC) |
| `mando.h` / `mando.c` | `struct Mando` con intenciones (izquierda, saltar_pulsado, actividad...). `mando_leer` traduce `padsCurrent/padsDown` según §3: Y corre, B salta, A ataca, R espada, SELECT HUD, START sale. Solo `mando.c` ve bits del pad. Con `-DGUION_PRUEBA` los botones del juego salen del guion del recorrido (`mando_iniciar` pone su reloj a cero). | `<snes.h>`. Lo usa `pantalla_nivel1.c`; la struct la leen `tarou.c` y los tests. | Mando: registros de autolectura `$4218/$4219` leídos por la NMI del SDK |
| `nivel.h` / `nivel.c` | `ANCHO_NIVEL 2048`, `COLUMNAS_NIVEL 256`, `ANCHO_PANTALLA 256`, `ANCHO_CELDA 96`, `ALTO_CELDA 64`, `SUELO_Y 176` (borde del suelo base) y `nivel_columna_vram(col)` (columna del nivel → sitio en el mapa-anillo de 64×32: `((col & 32) << 5) \| (col & 31)`). | `tipos.h`. Lo usan `tarou.c`, `camara.c`, `fondo_nivel.c`, `pantalla_nivel1.c`, tests. | ninguno |
| `colision.h` / `colision.c` | **Lógica pura**: la rejilla de 128×14 celdas de 16 px (0 vacío, 1 sólido, 2 plataforma, 3 zarza) y un **bloque** (el tronco). `colision_en(x, y)` (fuera del nivel: paredes a los lados, vacío arriba y abajo) y las consultas rápidas `colision_columna_solida`, `colision_fila_es`, `colision_apoyan` (una llamada para varias celdas: en tcc816 cada llamada es cara). Índice `(fila << 7) \| columna`. | `tipos.h`. Lo usan `tarou.c`, `partida.c`, `pantalla_nivel1.c` (`colision_usar(&nivel1_colision)`), tests. | **ninguno** |
| `partida.h` / `partida.c` | **Lógica pura**: las reglas del tutorial. `partida_actualizar` devuelve `PARTIDA_REAPARECER` (celda en y ≥ 192 o patas sobre una zarza), `PARTIDA_TRONCO_ROTO` (caja `GOLPE` sobre el tronco: quita el bloque) o `PARTIDA_SALIDA`, y apunta el punto de control (un cartel) más adelantado. | `tarou.h`, `colision.h`, `nivel.h`, `puntofijo.h`. Lo usan `pantalla_nivel1.c`, `tests/prueba_partida.c`, `tests/prueba_recorrido.c`. | **ninguno** |
| `fondo_nivel.h` / `.c` | Capa de hardware de la capa cercana: tiles y paleta 1, mapa de 64×32 en `VRAM_MAPA_A` usado como **anillo**; `fondo_nivel_actualizar` (cada frame) apunta en la lista de DMA las columnas que entran por los bordes cuando la cámara cambia de columna (`DMA_VMAIN_COLUMNA`, 64 bytes desde el mapa por columnas de la ROM, banco `nivel1_cerca_col_banco`); `fondo_nivel_rellenar` las 64 (pantalla apagada); el **parche** del tronco (4 columnas alternativas) y su retirada. | `nivel.h`, `dma_lista.h`, `comun.h`; `nivel1_cerca_til/_pal/_col` de `data.asm`. | VRAM y CGRAM por el SDK al cargar; las columnas, por la lista |
| `guion.h` / `guion.c` | **Lógica pura**: un mando leído de una tabla de tramos (cuadro inicial, final, botones); calcula mantenidos y recién pulsados como el mando real. Recorre la tabla con un puntero (con `guion[i]` tcc816 multiplicaba). | `mando.h`. Lo usan `mando.c` (con `-DGUION_PRUEBA`) y `tests/prueba_recorrido.c`. | ninguno |
| `guion_tutorial.h` / `.c` | El recorrido completo del tutorial: 12 tramos, llega a la salida en el cuadro 2238. | `guion.h`. Mismos usuarios. | ninguno |
| `camara.h` / `camara.c` | `camara_para(x_pix)` = `x + 48 − 128` recortada a 0..1792, calculada en `s16`. | `nivel.h`. Lo usa `pantalla_nivel1.c` (y de ahí `bgSetScroll`). | ninguno |
| `tarou_sprite.h` / `.c` | Capa de hardware del protagonista, **sin el motor dinámico del SDK** (desde el 2026-10-21): tablas metasprite de los 13 `_meta.inc` (incluidos **solo** aquí); una entrada de la OAM por pieza (`oamSet`/`oamSetEx`), en espejo `dx → 64 − dx` + `hflip` con el mismo bloque; **dos celdas en la VRAM** (tiles 0..127 y 128..255): el cuadro nuevo se apunta en la lista de DMA a medias (≤ 1,5 KB por VBlank) hacia la celda que no se muestra y, entero, la OAM pasa a él; `tarou_sprite_al_dia` para subirlo del tirón con la pantalla apagada; `HUNDIR_PIES` (el dibujo, 2 px bajo la física). `tarou_sprite_tabla()` y `tarou_sprite_tiles()` para `personajes.c`. | `tarou.h`, `animaciones.h`, `metasprite.h` (`metasprite_copias`), `dma_lista.h`, `comun.h`, `puntofijo.h`; `tarou_<anim>_til`, `tarou_pal` y `tarou_til_bancos` de `data.asm`. | OAM (buffer del SDK), CGRAM paleta 0; la VRAM, por la lista |
| `dma_lista.h` / `dma_lista.c` / `dma_ejecutar.asm` | **La lista de copias a la VRAM**: durante el frame, `dma_lista_vaciar` y `dma_lista_anadir(origen, vram, bytes, banco, vmain)` (C); en el VBlank, `dma_lista_ejecutar` (ensamblador: un DMA por entrada, ~60 ciclos cada uno; deja `VMAIN = 0x80`). 32 entradas de 8 bytes en la WRAM, definidas en el `.asm`. Se llama `dma_ejecutar.asm` porque un `dma_lista.asm` lo sobrescribiría el compilador de `dma_lista.c`. | `tipos.h`. La usan `tarou_sprite.c`, `fondo_nivel.c`, `hud_debug.c`, `pantalla_nivel1.c`. | VRAM y DMA canal 0 a mano (`$2115`, `$2116`, `$4300..$4305`, `$420B`) |
| `hud_debug.h` / `.c` | HUD de depuración (SELECT): escribe 7 filas directamente en un tilemap en RAM (tile = carácter − 32, paleta 0, bit de prioridad) sin divisiones ni `consoleDrawText`, una fila por frame, y apunta esa fila (64 bytes) en la lista de DMA (las 7 al encenderse). Filas: X/VX, Y/VY, EST, ESP/DIR, CAM/F, GOLPE, LIN/V/VB (`V`: líneas del VBlank al entrar y al salir de `_vblank`). | `animaciones.h`, `comun.h`, `puntofijo.h`, `secuencia.h` (`frames_en_pantalla`), `tarou.h`; `diag_linea[]` de `pantalla_nivel1.c`. | PPU (BG2 on/off), VRAM por DMA |
| `profundidad.h` / `.c` | **Lógica pura** del beat 'em up: `profundidad_ordenar(orden, y, n)`, inserción estable de mayor a menor y (los pies más abajo, primero en la OAM: delante). | `tipos.h`. Lo usan `sprites.c`, `tests/prueba_profundidad.c`. | **ninguno** |
| `cifras.h` | **Las cifras del combate** (spec §5.5): vida, daño, velocidades, tiempos, salto en z, rangos, jefes y puntos; las tablas por rango se leen con `cifras_vida_ninja`, `cifras_bonus_dano`, `cifras_puntos_ninja` (definidas en `combate.c`, `static const` en ROM). | `tipos.h`, `puntofijo.h`. Lo usan `mundo.c`, `combate.c`, `doble_toque.c` y sus pruebas. | **ninguno** |
| `doble_toque.h` / `.c` | **Lógica pura** del beat 'em up: correr con dos toques de la misma dirección en `VENTANA_DOBLE_TOQUE` frames (spec §2); la carrera dura hasta soltar. | `cifras.h`. Lo usa `tests/prueba_doble_toque.c` (y el luchador del prototipo). | **ninguno** |
| `mundo.h` / `.c` | **Lógica pura**: el `Cuerpo` en x/y/z (16 bytes), `mundo_colocar`, `mundo_saltar`, `mundo_mover` (recorta x e y al `Carril`, gravedad en z, avisa al aterrizar) y `mundo_y_dibujo` (y − z) (spec §3). | `puntofijo.h`, `cifras.h`. Lo usan `combate.c` y `tests/prueba_mundo.c`. | **ninguno** |
| `luchador.h` / `.c` | **Lógica pura**: Tarou en combate (spec §5.1). 24 estados `LQ_*` (incluidas la patada voladora y las dos de la entrada del relevo), con tablas de tiempos y cajas **por personaje** (`QUIEN_TAROU` / `QUIEN_BARBARA`: Bárbara, 3 golpes lentos y alcance largo en su celda de 96x96); cada uno con una fila en `fases` (duración, ventana de golpe y siguiente) y una caja en `cajas`. Combo de 4 (solo sigue si conectó y se pulsó A en la ventana), aire, ataque corriendo, especial A+B (paga solo si conecta, invulnerable), derribo, levantarse invulnerable, KO, agarre (3 zarpazos; atrás+A lanza), recibir y lanzado. Cuerpo, Golpeable y doble toque dentro de la struct (32 bytes). | `mundo.h`, `combate.h`, `doble_toque.h`, `mando.h`, `cifras.h`. Lo usa `tests/prueba_luchador.c`. | **ninguno** |
| `enemigos/ninja.h` / `.c` | **Lógica pura**: el ninja de prueba (spec §6). Vida y daño por rango (`cifras.h`), se acerca al objetivo en x y en y, ataca con el shinai (caja de golpe en sus frames activos) solo con `permiso` del llamador, en rango y fuera de enfriamiento (`COOLDOWN_NINJA` = 60). Recibir, derribo, KO. 32 bytes. | `mundo.h`, `combate.h`, `cifras.h`. Lo usa `tests/prueba_ninja.c`. | **ninguno** |
| `pantallas/prototipo.h` / `.c` | **Escena del prototipo de combate** (solo con `-DPROTOTIPO`; Etapa 2): Tarou (`luchador`) y un ninja (`enemigos/ninja`) en un carril x 48..200, con la capa de sprites de la prueba de carga, el fondo del nivel 1 y los golpes resueltos por `combate_golpear`. Pose de andar como placeholder de los dos; sin HUD de texto. Sustituye al nivel 1 en esa ROM. | `luchador.h`, `ninja.h`, `combate.h`, `sprites.h`, `subida.h`, `p16.h`, `dma_lista.h`, `fondo_nivel.h`. Lo llama `sistema/main.c`. | OAM por `sprites_dibujar`, VRAM por la lista de DMA en VBlank, CGRAM paletas 0 y 1, Mode 1 |
| `apoyos.h` / `.c` | **Lógica pura**: la barra de ataque (48 golpes, un uso, bloqueada hasta el monasterio) y los tres apoyos (spec §5.4, decisiones 47–50). Cada apoyo tiene su cronología en una tabla de 8 bytes (`defs`) y `apoyos_actualizar` devuelve los eventos de cada frame (GOLPE, LEVANTAR, CURAR, FIN). Daño a la mitad y sin derribo contra jefes; Rex cura 16. R recorre los disponibles. | `cifras.h`. Lo usa `tests/prueba_apoyos.c`. | **ninguno** |
| `cambio_protagonista.h` / `.c` | **Lógica pura**: el relevo entre Tarou y Bárbara (spec §4, decisiones 43–45). L cambia con enfriamiento de 120 frames, sin bloqueo y con el otro vivo; el que entra cae desde 80 px sobre el mismo sitio; el de fuera se cura +1 cada 120 frames hasta 48; si el activo cae KO entra el otro al instante (60 frames de protección); si caen los dos, `RELEVO_PIERDE_VIDA`. Sin la barra de ataque (va con los apoyos). Sustituye al stub. | `luchador.h`, `mando.h`, `mundo.h`, `cifras.h`. Lo usa `tests/prueba_relevo.c`. | **ninguno** |
| `combate.h` / `.c` | **Lógica pura**: cajas por cuadro de 8 bytes (`CajaCuadro`) pasadas al mundo con espejo (`combate_caja_mundo`), cruce de rectángulos, la regla `TOLERANCIA_Y`, `combate_golpear` (una vez por objetivo y ataque, vida, destello, parada de impacto en el derribo, KO), `combate_tic` y `combate_pagar_especial` (nunca mata) (spec §5). | `mundo.h`, `cifras.h`. Lo usa `tests/prueba_combate.c`. | **ninguno** |
| `sprites.h` / `sprites.c` / `sprites_oam.asm` | **La capa de sprites del beat 'em up**: tabla de entidades fijas de 16 bytes (`SpriteEnt`: x0, y0, tabla de piezas y su banco, n, tile, atributos, `borde`), `sprites_usar`, `sprites_ordenar` (por la y de los pies, con `profundidad.c`) y `sprites_dibujar` (ensamblador, UNA llamada por frame): todas las piezas a `oamMemory` con dos escrituras de 16 bits (`tile + atrib << 8` lleva el bit N solo); camino rápido de una suma por pieza si la celda está entera en pantalla; x negativa con su bit 8; fuera de pantalla y sobrantes con y = 224. ~0,5 líneas por pieza (con `oamSet`, 5,4). | `profundidad.h`; lo usa `pantallas/prueba_carga.c`. | ninguno directo (el búfer de la OAM del SDK) |
| `subida.h` / `subida.c` | **Lógica pura**: el plan de copias de un cuadro `.p16` sale del número de piezas (pares completos: 2 × 512 B; el último a medias: 2 × q·64 B; la fila de abajo, 256 palabras más allá) y `subida_tomar` lo reparte con un tope por VBlank en trozos de 512 B. `Subida` mide 64 bytes. | `tipos.h`; lo usan `prueba_carga.c`, `tests/prueba_subida.c`. | ninguno |
| `p16.h` / `p16.c` | **Lógica pura**: lee el registro de 8 bytes de un cuadro de una tira `.p16` (desplazamientos de la tabla de piezas, de la espejada y de los tiles; n). | `tipos.h`; lo usan `prueba_carga.c`, `tests/prueba_p16.c`. | ninguno |
| `dma_copias.asm` | `dma_lista_copias(copias, n, banco)`: pasa n `Copia` de una subida a la lista de DMA en un bucle de ensamblador (sin una llamada desde C por copia). | Declarada en `dma_lista.h`; la usa `prueba_carga.c`. | ninguno (prepara la lista) |
| `barbara.*`, `cambio_protagonista.*` | **Stubs** (Revisión 2 / Etapa 4): contrato previsto como prototipos comentados y una función `_pendiente()` vacía. | `tipos.h`. | ninguno |

## 6. `src/enemigos/`, `objetos/`, `hud/`, `audio/`, `niveles/` — stubs

Cada carpeta tiene `nombre.h` (cabecera estándar con `PENDIENTE: Etapa N`, el
contrato previsto comentado) y `nombre.c` (`nombre_pendiente()` vacío). Compilan
y enlazan, pero `wlalink` descarta sus secciones porque nadie las llama.

| Módulo | Para qué será | Cuándo |
|---|---|---|
| `src/enemigos/enemigos.h`, `enemigos.c` | cargar, mover, dibujar y golpear enemigos (el lobo samurái, con el motor dinámico) | Etapa 3 |
| `src/objetos/objetos.h`, `objetos.c` | ítems recogibles al tocarlos | Etapa 3 |
| `src/hud/hud.h`, `hud.c` | marcador del jugador (vida, objetos) en BG3; distinto de `hud_debug` | Etapa 3 |
| `src/audio/audio.h`, `audio.c` | `spcBoot`, soundbank, música por pantalla, efectos | sesión de audio |
| `src/niveles/niveles.h`, `niveles.c` | mapas de tiles (Tiled), scroll por columnas, colisión por tiles, como LikeMario | Etapa 2/3 |
| `src/juego/barbara.h`, `barbara.c`; `src/juego/cambio_protagonista.h`, `cambio_protagonista.c` | la segunda protagonista y el cambio entre ambos (ver §5) | Revisión 2 / Etapa 4 |

## 7. `res/` — assets preparados

Lo que hay aquí ya está **indexado** (paleta de ≤ 16 o 256 colores) y con el
tamaño exacto que necesita la SNES; `make` lo convierte con `gfx4snes`. Las
salidas (`.pic` tiles, `.pal` paleta, `.map` mapa, `.inc`/`_meta.inc` tablas,
`_data.as`) se regeneran y no se versionan.

| Carpeta / archivo | Qué es | Quién lo genera | Quién lo consume |
|---|---|---|---|
| `res/fuente/ark_consola.bmp`, `ark_consola_2bpp.bmp` | Fuente Ark Pixel 10 px: 96 glifos ASCII 32..127 en mayúsculas; versión de 4 colores para BG3. | `scripts/generar_fuente_consola.py` (desde `res/descargas/fuentes/`) | Makefile (`-s 8 -o 2 -u 16` / `-u 4`), `data.asm` (`fuente_consola_*`, `fuente_2bpp_*`), `comun.c` |
| `res/nintendo/logo_nintendo.png`, `res/publisher/logo_publisher.png`, `res/devs/logo_devs.png` | Logos de las pantallas 1–3, 256×224, 16 colores. | `scripts/componer_escenas.py` | Makefile (`-e 1 -m`), `data.asm`, `pantalla_*.c` |
| `res/intro/escena_1.png`, `escena_2.png`, `escena_3.png` | Fondos de la intro: 1 y 2 de 256 colores (Mode 3, imagen 256×120 arriba), 3 de 16 colores. | `componer_escenas.py` | Makefile, `data.asm`, `pantalla_intro.c` |
| `res/titulo/fondo_cerca.png` (512×256), `fondo_lejos.png` (256×224), `cielo.bmp` | Capas del título (cercana en bucle para scroll, lejana con el logotipo "TAROU" horneado) y el degradado del cielo (una fila de color por línea). | `componer_escenas.py` | Makefile (`-y` para la cercana, `-n` para el cielo), `data.asm`, `pantalla_titulo.c`; la cercana también la usa el nivel |
| `res/nivel1/fondo_lejos.png` (512×256) | Capa lejana del nivel: la del título sin logotipo, en espejo hasta 512 px, rellena hasta 256 de alto (`gfx4snes -y` lo exige). | `scripts/preparar_fondo_nivel.py` | Makefile (`-y -e 2`), `data.asm` (`nivel1_lejos_*`), `pantalla_nivel1.c` |
| `res/nivel1/tutorial.tmj`, `logica.png` | **El nivel**, en Tiled: capa `logica` de 128×14 celdas de 16 px pintada con los 3 tiles de `logica.png` (sólido, plataforma, zarza) y capa `objetos` (`inicio`, `cartel` con su propiedad `texto`, `tronco`, `salida`). | la primera vez, `scripts/preparar_nivel.py --crear`; después, el autor en Tiled | `preparar_nivel.py` |
| `res/nivel1/fondo_cerca.png` (2080×256) | Capa cercana del tutorial: la lógica vestida con el tileset Sidescroller de Pixellab (rejilla doble), el seto, las zarzas y los carteles (marcadores), más 32 px de **parche** a la derecha (las columnas del tronco). 15 colores + transparente, ~250 tiles. | `scripts/preparar_nivel.py` | Makefile (`gfx4snes -e 1 -m` sin `-y` y `mapa_a_columnas.py --ancho 260` → `.col`), `data.asm` (`.rodata_nivel1_cerca`), `fondo_nivel.c` |
| `res/nivel1/colision.bin`, `nivel1_datos.h` | La rejilla de colisión (1792 bytes) y la cabecera con los objetos (`NIVEL1_INICIO_X`, `NIVEL1_CONTROLES_X`, `NIVEL1_TRONCO_X/_COL`, `NIVEL1_PARCHE_COL`, `NIVEL1_SALIDA_X`; con `PRUEBAS_PC`, también la rejilla). | `scripts/preparar_nivel.py` | `data.asm` (`nivel1_colision`), `pantalla_nivel1.c`, tests |
| `res/personajes/tarou/tarou_reposo.png`, `tarou_caminar.png`, `tarou_correr.png`, `tarou_saltar.png`, `tarou_caer.png`, `tarou_agachado.png`, `tarou_agachado_caminar.png`, `tarou_desenvainar.png`, `tarou_envainar.png`, `tarou_atacar.png`, `tarou_echarse.png`, `tarou_dormido.png`, `tarou_despertar.png`, `tarou_reposo_estatico.png`, `tarou_anim.h` | Tiras de 96×64 por cuadro de las 13 animaciones de Tarou, una sola paleta (arte real de Pixellab desde el 2026-10-19), la tira estática de reposo (siempre 2 cuadros elegidos del reposo real, para intro y título) y el header con la cantidad de cuadros. | `scripts/preparar_personaje.py tarou` | Makefile (`-s 32 -T -X 96 -Y 64 -P 3`), `data.asm` (14 secciones), `tarou_sprite.c` (`_meta.inc`), `animaciones.c` (`tarou_anim.h`), `personajes.c` (`tarou_reposo_estatico`) |
| `res/personajes/barbara_samurai.png` | Bárbara (samurái del pack), tira de cuadros de 64×64. | `scripts/preparar_sprites.py` | Makefile (`-s 64`), `data.asm`, `personajes.c` |
| `res/personajes/lobo_samurai.png` | El lobo samurái del pack: antes era Tarou; futuro enemigo. Se convierte pero **no va en la ROM**. | `preparar_sprites.py` | Makefile (regla conservada) |
| `res/prueba_carga/ninja64_andar.bin`, `ninja64_ataque.bin`, `ninja_pal.bin`, `sombra.bin` | El ninja provisional de la prueba de carga (el samurái humano del pack *FULL_Samurai* ×3, recortado a la celda de 64×96) en formato `.p16`, su paleta (3 variantes + la sombra) y la sombra (2 piezas). **Versionados** (el pack no se versiona). | `scripts/preparar_prueba_carga.py` | `data.asm` (`.rodata_prueba_*`), `prueba_carga.c` |
| `res/personajes/tarou/tarou_<anim>.p16` | Las 14 tiras de Tarou en piezas de 16×16 (formato `.p16`), para la capa de sprites. Se generan con `make` y no se versionan. | `scripts/piezas16.py` (regla del Makefile) | `data.asm` (`.rodata_p16_*`, `tarou_p16_bancos`), `prueba_carga.c` |
| `res/dedicatoria/`, `musica/`, `sfx/` (`.gitkeep`) | Reservadas: la dedicatoria no tiene gráficos; música y efectos llegan en la sesión de audio. | — | — |
| `res/descargas/` | Buzón de material de terceros sin procesar; ignorado por git salvo los README. `graficos/pixellab/tarou/` y `barbara/` son las carpetas donde el autor deja las exportaciones de Pixellab (instrucciones en su README). | el autor | los scripts de `scripts/` |

## 8. `scripts/` — herramientas en Python y PowerShell

| Archivo | Qué hace | Entradas → salidas |
|---|---|---|
| `componer_escenas.py` | Compone todos los fondos de la secuencia de arranque a partir de las descargas (escala, recorta, indexa, hornea el logotipo del título). Determinista: se vuelve a ejecutar y listo. | `res/descargas/graficos/*` → `res/nintendo|publisher|devs|intro|titulo/*.png`, `cielo.bmp` |
| `generar_fuente_consola.py` | Renderiza la fuente Ark Pixel a un BMP de 96 celdas de 8×8 (ASCII 32..127, mayúsculas). | `res/descargas/fuentes/*.ttf` → `res/fuente/ark_consola*.bmp` |
| `preparar_imagen.py` | Convierte una imagen cualquiera a PNG indexado (16 o 256 colores, tamaño dado) para gfx4snes. | imagen → PNG en `res/<pantalla>/` |
| `preparar_sprites.py` | Corta una tira de cuadros, la centra en celdas (64×64 por defecto) y la reduce a 15 colores + transparente (índice 0). Es el script de Bárbara y del lobo. | tira → `res/personajes/*.png` |
| `preparar_personaje.py` | El pipeline de Tarou (y de Bárbara en la Revisión 2): lee la exportación nativa de Pixellab (con `MAPEOS_PIXELLAB`: de qué estado, dirección y tramo sale cada animación, y si va al revés; ancla sin escalar en celdas de 96×64) o tiras PNG, carpetas de cuadros o GIF, alinea con una caja común, cuantiza **todas** las animaciones a una sola paleta, genera marcadores para las que faltan, rechaza más de 15 cuadros y escribe `tarou_anim.h`. | `res/descargas/graficos/pixellab/tarou/` → `res/personajes/tarou/` |
| `preparar_fondo_nivel.py` | Borra una caja (el logotipo) de la capa lejana del título, la duplica en espejo a 512 px y la rellena hasta 256 de alto. | `res/titulo/fondo_lejos.png` → `res/nivel1/fondo_lejos.png` |
| `preparar_nivel.py` | **El nivel desde Tiled**: lee `tutorial.tmj` (lógica y objetos), viste la rejilla con el tileset de Pixellab de rejilla doble (cada celda de arte se elige por las 4 celdas de lógica de sus esquinas; desplazada 8 px, todo cae en la rejilla de 8×8), pone seto, zarzas, carteles (letras de la fuente de consola en tiles alineados, que se comparten) y el parche del tronco; se niega a escribir si pasa de 460 tiles distintos. `--crear` escribe el `.tmj` inicial (JSON sin espacios tras los dos puntos). | `tutorial.tmj` + Pixellab + seto + fuente → `fondo_cerca.png`, `colision.bin`, `nivel1_datos.h` |
| `mapa_a_columnas.py` | Reordena el mapa de gfx4snes (por filas) columna a columna, para copiar una columna con un solo DMA. Lo llama el Makefile. | `fondo_cerca.map` → `fondo_cerca.col` |
| `preparar_suelo.py` | (Sustituido por `preparar_nivel.py` en el tutorial; se conserva con sus pruebas.) Construye la capa cercana del nivel desde el tileset Sidescroller de Pixellab: encuentra solo los tiles de borde (hierba sobre tierra) y de tierra, pinta 512 px con la banda de hierba en `SUELO_Y − hundir` (4 px por defecto: los pies quedan dentro de la hierba; ancla la primera fila llena, no las briznas), varía la tierra con volteos (semilla fija) y, con `--seto`, pone detrás los arbustos del título. | Lee `res/descargas/graficos/pixellab/suelo/*.png`; escribe `res/nivel1/fondo_cerca.png`; usa `preparar_personaje.indexar`. | ninguno |
| `piezas16.py` | **El formato de sprites del beat 'em up**: tira PNG indexada → `.p16` (piezas de 16×16 sin las vacías; índice de 8 bytes por cuadro; tabla (dx, dy) y la misma espejada; tiles en orden de subida, n·128 bytes; ≤ 32 KB). `--pal` escribe además la paleta. | PNG → `.p16` |
| `preparar_prueba_carga.py` | El ninja provisional de la ROM de prueba de carga: el samurái del pack ×3 recortado a la celda de 64×96, a `.p16` con `piezas16`, su paleta y la sombra; imprime piezas por cuadro y la **predicción** de ranuras por línea de cada configuración (con Tarou en piezas de 16×16). | `res/descargas/graficos/FULL_Samurai…/Sprites/` → `res/prueba_carga/` |
| `pruebas/prueba_preparar_personaje.py`, `pruebas/prueba_preparar_fondo_nivel.py`, `pruebas/prueba_preparar_suelo.py`, `pruebas/prueba_preparar_nivel.py`, `pruebas/prueba_preparar_prueba_carga.py`, `pruebas/prueba_piezas16.py` | Pruebas `unittest` de los scripts (47 en total; `python -m unittest scripts/pruebas/...`). | — |
| `capturar_mesen.ps1` | Verificación sin manos: lanza Mesen a velocidad 100 % sin guardar ajustes, lo activa una vez, le da el foco al visor con un clic por mensaje, aparta la ventana fuera de la pantalla, "pulsa" teclas por `PostMessage` según un guion (`-Guion "desde,hasta,tecla;..."`) y guarda capturas con `PrintWindow` cada N segundos. | `ProyectoSNES.sfc` → `scripts/capturas/frame_*.png` |
| `capturas_mesen.lua`, `mando_nivel.lua` | Scripts Lua para Mesen: capturas y mando por guion. **Solo a mano** desde Debug → Script Window: MesenCE 2.2.1 no carga la ROM si recibe un `.lua` por línea de comandos y bloquea la E/S de Lua. | — |

## 9. `tests/` — pruebas de la lógica pura en el PC

| Archivo | Qué prueba |
|---|---|
| `Makefile` | `gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I. -I../src -I../res/nivel1` sobre `pruebas.c`, `prueba_*.c` y los `.c` puros de `../src/juego/`. Uso: `make -C tests ; .\tests\pruebas.exe`. |
| `minitest.h` | `COMPROBAR(cond)`: cuenta y, si falla, imprime archivo, línea y condición. |
| `pruebas.c` | Llama a cada suite e imprime "N pruebas, M fallos"; devuelve 1 si hubo fallos. |
| `prueba_puntofijo.c` | `PF`, `PF_ENTERO`, signo de `s16`. |
| `prueba_camara.c` | `camara_para` en el centro y en ambos bordes (0..1792); constantes del nivel; `nivel_columna_vram` (el anillo). |
| `prueba_animacion.c` | Tabla `ANIMACIONES` (cuadros, frames por cuadro, bucle), `NOMBRES_ESTADO`, `struct Mando`. |
| `prueba_tarou.c` | Caminar, correr, frenar, bordes; salto variable, corte, terminal, aterrizajes, control aéreo; agacharse, espada, ataque y cajas; dormir, despertar; avance de animación. Sobre un escenario plano (fila 13 sólida). Con las demás suites, 367 comprobaciones en total. |
| `prueba_colision.c` | `colision_en`, las consultas de columna y fila y el bloque; y, sobre la rejilla **real** del tutorial, que cada obstáculo exige su movimiento (escalón, muro, túnel, foso, plataforma) y que desde el inicio se llega al primer escalón. |
| `prueba_partida.c` | Reglas sobre el nivel real: foso y zarzas mandan reaparecer en el último cartel; el tronco para a Tarou, A sin espada no hace nada, con espada se rompe; la salida. |
| `prueba_escena.c` | El motor de escenas: rampas exactas, `FIJAR`, fin, evento fuera de orden; y la tabla real del logo (orden, 244 frames, sin saltos, todo apagado al final). |
| `prueba_recorrido.c` | El guion `GUION_TUTORIAL` juega el nivel real entero: llega a la salida en el cuadro 2270, sin reaparecer, tronco roto, los 8 controles pasados. |
| `prueba_metasprite.c` | `metasprite_piezas` y `metasprite_espejar` con tablas de 6, 3 y 0 piezas y una sin terminador (el límite de 6 protege); el terminador `0xFF80` queda detrás de la última pieza; `metasprite_copias` con los cuadros 0 y 1 de una tira de 8 (8 y 16 copias, orígenes y destinos exactos). |
| `tarou_anim.h` | Copia fija del header generado, con los cuadros de los marcadores, para que las pruebas no dependan del arte. |

## 10. `docs/`, `aprendizaje/`, `examples/`, `.vscode/`, `tools/`

| Archivo o carpeta | Qué es |
|---|---|
| `docs/README.md` | Índice de la documentación. |
| `docs/diseno-demo-jugable.md` | La especificación de la Revisión 1 (Tarou adulto): mecánicas, mando, estados, física, cámara, sprite, fondo, HUD, pipeline, arquitectura, cabecera estándar, verificación. |
| `docs/diseno-tutorial-movimiento.md` | La especificación de la Revisión 2: el nivel 1 como tutorial con colisión, Tiled, dibujo por columnas, reglas, guion de prueba, resultados y trampas. |
| `docs/diseno-secuencia-arranque.md` | La especificación de las seis pantallas de arranque y sus hallazgos técnicos verificados. |
| `docs/plan-implementacion-revision-1.md` | El plan de tareas con el que se construyó la Revisión 1 (8 fases). |
| `docs/plan-desarrollo.md` | La hoja de ruta por Etapas (0..6) del proyecto completo. |
| `docs/glosario-snes.md` | El glosario único: vocabulario del proyecto, hardware, gráficos, mecánicas, C. |
| `docs/mapa-de-archivos.md`, `tuberia-de-compilacion.md`, `mesen-y-depuracion.md`, `hardware-snes-por-modulo.md` | Esta serie. |
| `docs/screenshots/` | Capturas de referencia (secuencia de arranque, `revision-1/` y `tutorial/`). |
| `aprendizaje/` | Unidades de estudio. Entrada: `aprendizaje/README.md`; plan completo: `aprendizaje/PLAN.md`. Hoy existe `unidad-00-c-en-el-pc/` (C en PC con gcc): `README.md`, `demo_01..04_*.c`, `ejercicio_01..04_*.c` (esqueletos con TODO), `solucion/` y `capturas/` (salidas de referencia). |
| `examples/likemario/` | Copia comentada del ejemplo LikeMario del SDK (motor de mapas, objetos y sprites dinámicos) y su estudio `LikeMario.md`. |
| `.vscode/tasks.json` | Tareas "Build SNES ROM" (`Ctrl+Shift+B`, solo compila) y "Clean Project", ambas por `bash.exe` de MSYS2 con `PVSNESLIB_HOME`. |
| `.vscode/c_cpp_properties.json` | IntelliSense: rutas de inclusión (`src`, el SDK), `__snes__`, `tcc816` como compilador. |
| `tools/pvsneslib/` | El SDK: `devkitsnes/bin` (`816-tcc`, `wla-65816`, `wla-spc700`, `wlalink`), `devkitsnes/tools` (`816-opt`, `gfx4snes`, `smconv`, `snesbrr`, `tmx2snes`...), `devkitsnes/snes_rules`, `pvsneslib/include` (los `.h` de `<snes.h>`), `pvsneslib/lib` (la librería ya compilada) y `snes-examples/`. **No se edita ni se versiona.** |
| `tools/mesen/Mesen.exe` | MesenCE 2.2.1, el emulador. Su configuración vive en `Documentos/MesenCE/settings.json`. |
