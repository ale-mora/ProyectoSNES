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
│   ├── juego/      la lógica del nivel: protagonista, mando, cámara, HUD
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
                      juego/mando.h, juego/hud_debug.h
juego/animaciones.c ──► tarou_anim.h   (generado en res/personajes/tarou/; en tests/, una copia fija)
juego/tarou_sprite.c ──► tarou_<anim>_meta.inc (13, generados por gfx4snes -T)
juego/tarou_sprite.c, pantallas/personajes.c ──► juego/metasprite.h ──► sistema/tipos.h   (lógica pura: contar piezas y espejar)
pantallas/personajes.c ──► tarou_reposo_estatico_meta.inc   (generado por gfx4snes -T a partir de la tira estática)
tests/*.c ──► tests/minitest.h ──► sistema/tipos.h   (+ los .h de juego/ que prueban)
```

Regla de capas: **la lógica pura** (`tarou.c`, `camara.c`, `nivel.c`,
`animaciones.c`, `metasprite.c`) incluye solo `sistema/tipos.h` y cabeceras de `juego/`; nunca
`<snes.h>`. Así se compila también en el PC. **La capa de hardware**
(`mando.c`, `tarou_sprite.c`, `hud_debug.c`, las pantallas, `comun.c`) sí
incluye `<snes.h>` y es la única que toca la PPU, la OAM y el mando.

## 2. Raíz del repositorio

| Archivo | Qué hace | Se conecta con | Hardware |
|---|---|---|---|
| `Makefile` | Define `ROMNAME`, `ROMTITLE`, `ROMSIZE 09`/`ROMBANKS 16` (512 KB), una regla `gfx4snes` por asset, la lista `ASSETS`, `bitmaps` y `data.obj: $(ASSETS)`; luego incluye `snes_rules` del SDK y añade `-Isrc -Ires/personajes/tarou` a `CFLAGS`. | Lo ejecuta `make` (desde `run.bat`, PowerShell o la tarea de VS Code). Lee `res/**`, produce `.pic/.pal/.map/_meta.inc`, `.ps/.asm/.obj`, `linkfile`, `hdr.asm`, `.sfc/.sym/.log`. Detalle regla por regla en `tuberia-de-compilacion.md`. | ninguno (describe la ROM: LoROM, SlowROM, 16 bancos) |
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
| `pantalla_nintendo.c` | (1) Logo Nintendo + "LICENSED BY NINTENDO", 3 s. | `nintendo_*` de `data.asm`; `secuencia.c`. | PPU y VRAM vía `comun.c`; mando solo si `PERMITE_SALTO` |
| `pantalla_publisher.c` | (2) Logo del publisher (Capcom del SDK, provisional), 3 s. | `publisher_*`; `secuencia.c`. | ídem |
| `pantalla_devs.c` | (3) Logo del estudio y créditos con la consola de texto, 4 s; entra de golpe (`setBrightness(15)`). | `devs_*`; `secuencia.c`. | ídem + brillo directo |
| `pantalla_dedicatoria.c` | (4) "A la memoria de Tarou" letra a letra sobre negro, fundidos lentos, 7 s. | `comun.c` (`escribir_parcial`); `secuencia.c`. | PPU vía `comun.c` |
| `pantalla_intro.c` | (5) Tres escenas de 8 s: dos en Mode 3 (256 colores) y una en Mode 1 con los sprites; texto letra a letra; sale al título con mosaico. | `escena1/2/3_*`; `personajes.h`; `secuencia.c`. | PPU: Mode 3/1, mosaico (`REG_MOSAIC`); VRAM/CGRAM (Mode 3 pisa las 256 entradas de color y la VRAM de sprites) |
| `pantalla_titulo.c` | (6) Título: capa cercana con scroll por software (parallax), capa lejana con el logotipo horneado, cielo HDMA, PRESS START parpadeando, sprites en reposo; Start → nivel 1; 15 s sin tocar nada → intro. | `cerca_*`, `lejos_*`, `hdmaGradcieloList`; `personajes.h`; `secuencia.c`. | PPU: scroll (`bgSetScroll`), prioridad BG3; HDMA canal 6; OAM vía `personajes.c`; mando |
| `pantalla_nivel1.c` | (7) El nivel 1. Por frame: `mando_leer` → `tarou_actualizar` → `camara_para` → scroll de dos capas → `tarou_sprite_dibujar` → `oamInitDynamicSpriteEndFrame` → HUD; en `_vblank`: `oamVramQueueUpdate` y DMA del HUD. START vuelve al título, SELECT alterna el HUD. Mide la línea del haz (`$2137/$213D`) para el medidor `LIN`. | `cerca_*`, `nivel1_lejos_*`, `hdmaGradcieloList`; `juego/*.h`; `secuencia.c`. | PPU: modo, scroll, BG2 on/off; HDMA; OAM/VRAM por `tarou_sprite.c`; mando por `mando.c`; contadores del haz |
| `personajes.c` / `.h` | Sprites de Tarou y Bárbara para intro y título con carga **estática**: la tira `tarou_reposo_estatico` (siempre 2 cuadros de 96×64; piezas de 32×32 según su tabla metasprite, `tile = (n/4)·64 + (n%4)·4`, **hasta** 6 por cuadro: las entradas sobrantes se esconden) en VRAM 0x0000 (tiles 0..191) y Bárbara (64×64) en la segunda tabla de tiles (256, VRAM 0x1000); `mostrar/animar/ocultar`. | `tarou_reposo_estatico_til`, `tarou_pal`, `barbara_samurai_*` de `data.asm`; `tarou_reposo_estatico_meta.inc`; `juego/metasprite.h` (`metasprite_piezas`); lo usan `pantalla_intro.c`, `pantalla_titulo.c`, `main.c`. | OAM (`oamSet`, `oamSetEx`, `oamSetXY`), VRAM (`dmaCopyVram`), CGRAM (`dmaCopyCGram`), `oamInitGfxAttr` |

## 5. `src/juego/` — la lógica del nivel

| Archivo | Qué hace | Incluye / Lo usan | Hardware |
|---|---|---|---|
| `tarou.h` / `tarou.c` | **Lógica pura** del protagonista: `struct Tarou` (posición y velocidad en 12.4, estado, espada, dirección, animación, contador de sueño), las constantes de §5 de la spec, `tarou_iniciar`, `tarou_actualizar` (transiciones → aceleración → gravedad → integrar → recortar → aterrizar → animación), `tarou_caja_cuerpo`, `tarou_caja_golpe`. 13 estados con la prioridad R > A > B > Abajo > cruceta. | `tipos.h`, `puntofijo.h`, `mando.h`, `animaciones.h`, `nivel.h`. Lo usan `pantalla_nivel1.c`, `tarou_sprite.c`, `hud_debug.c`, `tests/prueba_tarou.c`. | **ninguno** (se prueba en PC) |
| `animaciones.h` / `.c` | `enum Estado` (13), `AnimDef` y las tablas `ANIMACIONES` (cuadros de `tarou_anim.h`, frames por cuadro, bucle) y `NOMBRES_ESTADO` (texto para el HUD). | `tipos.h`, `tarou_anim.h`. Lo usan `tarou.c`, `tarou_sprite.c`, `hud_debug.c`, tests. | ninguno |
| `metasprite.h` / `.c` | **Lógica pura** de las tablas metasprite de `gfx4snes -T`: `metasprite_piezas` (cuenta hasta el terminador `0xFF80`, máximo 6, la celda de 96×64; el conversor **omite** los bloques transparentes, así que un cuadro puede tener menos de 6) y `metasprite_espejar` (copia mirando a la izquierda: `dx → 64 − dx`, bit `hflip`, terminador detrás de la última pieza). En el PC define la misma `t_metasprite` que `<snes.h>`. | `tipos.h`. Lo usan `tarou_sprite.c`, `pantallas/personajes.c`, `tests/prueba_metasprite.c`. | **ninguno** (se prueba en PC) |
| `mando.h` / `mando.c` | `struct Mando` con intenciones (izquierda, saltar_pulsado, actividad...). `mando_leer` traduce `padsCurrent/padsDown` según §3: Y corre, B salta, A ataca, R espada, SELECT HUD, START sale. Solo `mando.c` ve bits del pad. | `<snes.h>`. Lo usa `pantalla_nivel1.c`; la struct la leen `tarou.c` y los tests. | Mando: registros de autolectura `$4218/$4219` leídos por la NMI del SDK |
| `nivel.h` / `nivel.c` | `ANCHO_NIVEL 512`, `ANCHO_PANTALLA 256`, `ANCHO_CELDA 64`, `SUELO_Y 208` y `nivel_suelo_en(x)` (plano por ahora). | `tipos.h`. Lo usan `tarou.c`, `camara.c`, `pantalla_nivel1.c`, tests. | ninguno |
| `camara.h` / `camara.c` | `camara_para(x_pix)` = `x + 48 − 128` recortada a 0..256, calculada en `s16`. | `nivel.h`. Lo usa `pantalla_nivel1.c` (y de ahí `bgSetScroll`). | ninguno |
| `tarou_sprite.h` / `.c` | Capa de hardware del protagonista: motor de sprites dinámicos (`oamInitDynamicSprite`, piezas de 32×32 desde VRAM 0x0000), tablas metasprite de los 13 `_meta.inc` (incluidos **solo** aquí), espejo construido en RAM (`dx 0↔32` + `hflip`), `oamrefresh` cuando cambia el cuadro, `oamMetaDrawDyn32`. `tarou_sprite_tabla()` y `tarou_sprite_tiles()` para `personajes.c`. | `tarou.h`, `animaciones.h`, `puntofijo.h`; etiquetas `tarou_<anim>_til` y `tarou_pal` de `data.asm`. | OAM (buffer del motor), VRAM 0x0000–0x1FFF (DMA en VBlank), CGRAM paleta 0 |
| `hud_debug.h` / `.c` | HUD de depuración (SELECT): escribe 7 filas directamente en un tilemap en RAM (tile = carácter − 32, paleta 0) sin divisiones ni `consoleDrawText`, una fila por frame, y `hud_debug_vblank` las copia por DMA a `VRAM_MAPA_TEXTO`. Filas: X/VX, Y/VY, EST, ESP/DIR, CAM/F, GOLPE, LIN/VB. | `animaciones.h`, `comun.h`, `puntofijo.h`, `secuencia.h` (`frames_en_pantalla`), `tarou.h`; `diag_linea[]` de `pantalla_nivel1.c`. | PPU (BG2 on/off), VRAM por DMA |
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
| `res/personajes/tarou/tarou_reposo.png`, `tarou_caminar.png`, `tarou_correr.png`, `tarou_saltar.png`, `tarou_caer.png`, `tarou_agachado.png`, `tarou_agachado_caminar.png`, `tarou_desenvainar.png`, `tarou_envainar.png`, `tarou_atacar.png`, `tarou_echarse.png`, `tarou_dormido.png`, `tarou_despertar.png`, `tarou_reposo_estatico.png`, `tarou_anim.h` | Tiras de 96×64 por cuadro de las 13 animaciones de Tarou, una sola paleta (arte real de Pixellab desde el 2026-10-19), la tira estática de reposo (siempre 2 cuadros elegidos del reposo real, para intro y título) y el header con la cantidad de cuadros. | `scripts/preparar_personaje.py tarou` | Makefile (`-s 32 -T -X 96 -Y 64 -P 3`), `data.asm` (14 secciones), `tarou_sprite.c` (`_meta.inc`), `animaciones.c` (`tarou_anim.h`), `personajes.c` (`tarou_reposo_estatico`) |
| `res/personajes/barbara_samurai.png` | Bárbara (samurái del pack), tira de cuadros de 64×64. | `scripts/preparar_sprites.py` | Makefile (`-s 64`), `data.asm`, `personajes.c` |
| `res/personajes/lobo_samurai.png` | El lobo samurái del pack: antes era Tarou; futuro enemigo. Se convierte pero **no va en la ROM**. | `preparar_sprites.py` | Makefile (regla conservada) |
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
| `pruebas/prueba_preparar_personaje.py`, `pruebas/prueba_preparar_fondo_nivel.py` | Pruebas `unittest` de los dos scripts anteriores (`python -m unittest scripts/pruebas/...`). | — |
| `capturar_mesen.ps1` | Verificación sin manos: lanza Mesen a velocidad 100 % sin guardar ajustes, lo activa una vez, le da el foco al visor con un clic por mensaje, aparta la ventana fuera de la pantalla, "pulsa" teclas por `PostMessage` según un guion (`-Guion "desde,hasta,tecla;..."`) y guarda capturas con `PrintWindow` cada N segundos. | `ProyectoSNES.sfc` → `scripts/capturas/frame_*.png` |
| `capturas_mesen.lua`, `mando_nivel.lua` | Scripts Lua para Mesen: capturas y mando por guion. **Solo a mano** desde Debug → Script Window: MesenCE 2.2.1 no carga la ROM si recibe un `.lua` por línea de comandos y bloquea la E/S de Lua. | — |

## 9. `tests/` — pruebas de la lógica pura en el PC

| Archivo | Qué prueba |
|---|---|
| `Makefile` | `gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I. -I../src` sobre `pruebas.c`, `prueba_*.c` y los `.c` puros de `../src/juego/`. Uso: `make -C tests ; .\tests\pruebas.exe`. |
| `minitest.h` | `COMPROBAR(cond)`: cuenta y, si falla, imprime archivo, línea y condición. |
| `pruebas.c` | Llama a cada suite e imprime "N pruebas, M fallos"; devuelve 1 si hubo fallos. |
| `prueba_puntofijo.c` | `PF`, `PF_ENTERO`, signo de `s16`. |
| `prueba_camara.c` | `camara_para` en el centro y en ambos bordes; `nivel_suelo_en`; constantes del nivel. |
| `prueba_animacion.c` | Tabla `ANIMACIONES` (cuadros, frames por cuadro, bucle), `NOMBRES_ESTADO`, `struct Mando`. |
| `prueba_tarou.c` | Caminar, correr, frenar, bordes; salto variable, corte, terminal, aterrizajes, control aéreo; agacharse, espada, ataque y cajas; dormir, despertar; avance de animación. Con las demás suites, 195 comprobaciones en total. |
| `prueba_metasprite.c` | `metasprite_piezas` y `metasprite_espejar` con tablas de 6, 3 y 0 piezas y una sin terminador (el límite de 6 protege); el terminador `0xFF80` queda detrás de la última pieza. |
| `tarou_anim.h` | Copia fija del header generado, con los cuadros de los marcadores, para que las pruebas no dependan del arte. |

## 10. `docs/`, `aprendizaje/`, `examples/`, `.vscode/`, `tools/`

| Archivo o carpeta | Qué es |
|---|---|
| `docs/README.md` | Índice de la documentación. |
| `docs/diseno-demo-jugable.md` | La especificación de la Revisión 1 (Tarou adulto): mecánicas, mando, estados, física, cámara, sprite, fondo, HUD, pipeline, arquitectura, cabecera estándar, verificación. |
| `docs/diseno-secuencia-arranque.md` | La especificación de las seis pantallas de arranque y sus hallazgos técnicos verificados. |
| `docs/plan-implementacion-revision-1.md` | El plan de tareas con el que se construyó la Revisión 1 (8 fases). |
| `docs/plan-desarrollo.md` | La hoja de ruta por Etapas (0..6) del proyecto completo. |
| `docs/glosario-snes.md` | El glosario único: vocabulario del proyecto, hardware, gráficos, mecánicas, C. |
| `docs/mapa-de-archivos.md`, `tuberia-de-compilacion.md`, `mesen-y-depuracion.md`, `hardware-snes-por-modulo.md` | Esta serie. |
| `docs/screenshots/` | Capturas de referencia (secuencia de arranque y `revision-1/`). |
| `aprendizaje/` | Unidades de estudio. Hoy `etapa0/` (C en PC con gcc): `demo_01_logica.c`, `demo_02_funciones.c`, `demo_03_tipos_arrays.c` y sus `ejercicio_01_logica.c`, `ejercicio_02_funciones.c`, `ejercicio_03_tipos_arrays.c`. El plan completo de unidades está en `aprendizaje/PLAN.md`. |
| `examples/likemario/` | Copia comentada del ejemplo LikeMario del SDK (motor de mapas, objetos y sprites dinámicos) y su estudio `LikeMario.md`. |
| `.vscode/tasks.json` | Tareas "Build SNES ROM" (`Ctrl+Shift+B`, solo compila) y "Clean Project", ambas por `bash.exe` de MSYS2 con `PVSNESLIB_HOME`. |
| `.vscode/c_cpp_properties.json` | IntelliSense: rutas de inclusión (`src`, el SDK), `__snes__`, `tcc816` como compilador. |
| `tools/pvsneslib/` | El SDK: `devkitsnes/bin` (`816-tcc`, `wla-65816`, `wla-spc700`, `wlalink`), `devkitsnes/tools` (`816-opt`, `gfx4snes`, `smconv`, `snesbrr`, `tmx2snes`...), `devkitsnes/snes_rules`, `pvsneslib/include` (los `.h` de `<snes.h>`), `pvsneslib/lib` (la librería ya compilada) y `snes-examples/`. **No se edita ni se versiona.** |
| `tools/mesen/Mesen.exe` | MesenCE 2.2.1, el emulador. Su configuración vive en `Documentos/MesenCE/settings.json`. |
