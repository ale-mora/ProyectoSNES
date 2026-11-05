# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## How to read this file

Only what matters in every session is here: one line per rule + where the detail lives.

| Topic | File | Loaded |
| --- | --- | --- |
| What the game does today, chronicle of every feature, the test ROMs | `docs/estado/fase1-y-roms.md` | on demand (read it) |
| tcc816 / SNES traps, VBlank budget, wlalink `-d` | `.claude/rules/codigo-snes.md` | automatically when touching `src/**`, `data.asm`, `Makefile` |
| Tiled level, Pixellab MCP, asset scripts and their commands | `.claude/rules/assets.md` | automatically when touching `scripts/**`, `res/**` |
| `docs/` and the course `aprendizaje/` | `.claude/rules/docs-y-curso.md` | automatically when touching `docs/**`, `aprendizaje/**` |
| Per-file map | `docs/mapa-de-archivos.md` | on demand |

## Project overview

A Super Nintendo (SNES) homebrew game written in C, built with **PVSnesLib** (a C SDK for the SNES built on the `tcc816` compiler and the `wla-65816`/`wlalink` toolchain). Target platform: LoROM + FastROM, 128 ROM banks (4 MB, the maximum of the standard mappings; code runs in banks $80-$FF at 3.58 MHz). Output is a `.sfc` ROM run in the **Mesen** emulator.

**Genre (since 2026-10-23): a belt-scroll beat 'em up** (Final Fight style; Tarou or Bárbara chosen in a selector, 3 support characters, stage 1 = village → bamboo forest → monastery → dojo). Spec: `docs/diseno-beat-em-up.md` (Revisión 3); art list: `docs/arte-beat-em-up.md`. Stage 1 is playable end to end (summary and full chronicle: `docs/estado/fase1-y-roms.md`). The platformer (Revisiones 1–2) is frozen in the git tag **`plataformas-r2`** (`480f373`); its level 1 still compiles and is only reachable with `-DGUION_PRUEBA`. The DKC-style tag switch on L is undecided.

The vendored SDK and emulator live under `tools/` (`tools/pvsneslib/`, `tools/mesen/Mesen.exe`) and are third-party — do not edit files under `tools/`.

## Environment setup

Build tooling is not on PATH by default. Two ways this is provided:

- **One-time (persistent)**: `powershell -File tools/setup_env.ps1` sets the `PVSNESLIB_HOME` / `DEVKITSNES` user env vars and adds `tools/pvsneslib/devkitsnes/bin` and `C:\msys64\usr\bin` to the user PATH.
- **Per-run (session-local)**: `run.bat` sets `PVSNESLIB_HOME`, `DEVKITSNES`, and `PATH` inline before building — this is what to use if the persistent env vars haven't been set up.

The build requires MSYS2 (`C:\msys64`) for `make` and other POSIX tools, plus the SDK binaries in `tools/pvsneslib/devkitsnes/bin/` (`816-tcc.exe`, `wla-65816.exe`, `wla-spc700.exe`, `wlalink.exe`).

## Common commands

Build and run (from the repo root, in PowerShell or CMD; compiles, then launches the ROM in Mesen). `run.bat` always runs `make clean` first, so every graphic is reconverted and the build is slower than a plain `make`; on a compile error it prints `[ERROR]` and pauses:
```
cd E:\ProyectoSNES
.\run.bat
```

Run the already-built ROM without recompiling (check the `ProyectoSNES.sfc` timestamp first — if code changed since it was built, use `run.bat` instead):
```
& "E:\ProyectoSNES\tools\mesen\Mesen.exe" "E:\ProyectoSNES\ProyectoSNES.sfc"
```

Build only, from an MSYS2 bash shell with `PVSNESLIB_HOME` exported:
```
make
```

Clean build artifacts, ROM, and graphics/audio conversion outputs:
```
make clean
```

Build only, from PowerShell (what Claude Code should use — the sandboxed Git Bash tool cannot run `wla-65816`, which writes a temp file to the drive root):
```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
```

**`snes_rules` does not track header dependencies**: editing a `.h` (or a `#define` used by several files) does not recompile the `.c` files that include it. After any header change run `make clean` before `make`, or stale objects keep the old values silently (this bit us once with fade timings). Assets are embedded by `data.asm` via `.incbin`; the Makefile declares `data.obj: $(ASSETS)` so a regenerated image reassembles it (without that line, old graphics stayed in the ROM).

The equivalent VS Code tasks (`.vscode/tasks.json`) run these same `make` invocations through `C:\msys64\usr\bin\bash.exe`: **"Build SNES ROM"** (default build task, `Ctrl+Shift+B`; it only compiles, it does not launch Mesen) and **"Clean Project"**.

What the ROM does today (2026-11-05): boot sequence (logos and dedication can't be skipped; the 3-scene intro can, with Start or A) → title → character select → stage 1 (village → bamboo forest → monastery → dojo → STAGE CLEAR → «STAGE 2 / COMING SOON»). Details and history: `docs/estado/fase1-y-roms.md`. Keyboard mapping of Mesen on this machine: arrows, A=S, B=A, X=X, Y=Z, L=Q, R=W, Start=D, Select=E (`docs/mesen-y-depuracion.md`). Leave the PC alone while a scripted capture runs (Mesen's focus can steal the keys).

Tests: the pure game logic has a PC test suite (gcc, 2628 checks, including the scene engine, including a full playthrough of the real level driven by `src/juego/guion_tutorial.c`) and the asset scripts have `unittest` tests (80). Run before every commit that touches `src/juego/` or `scripts/`:
```
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
python -m unittest scripts/pruebas/prueba_preparar_personaje.py scripts/pruebas/prueba_preparar_fondo_nivel.py scripts/pruebas/prueba_preparar_suelo.py scripts/pruebas/prueba_preparar_nivel.py scripts/pruebas/prueba_preparar_prueba_carga.py scripts/pruebas/prueba_piezas16.py scripts/pruebas/prueba_preparar_aldea.py scripts/pruebas/prueba_preparar_objetos.py scripts/pruebas/prueba_preparar_zona.py
```
Self-playing test ROM (the reliable Mesen check for level 1: it boots straight into the level and the pad comes from the playthrough script, no keys needed; same name as the normal ROM, so copy it aside and rebuild normally afterwards):
```
make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null
.\scripts\capturar_mesen.ps1 -Rom RUTA\tutorial_guion.sfc -Duracion 46 -Cada 2 -Espera 3
```
Load-test ROM for the beat 'em up (`docs/diseno-beat-em-up.md` §9; what it runs: `docs/estado/fase1-y-roms.md`). CFLAGS changes are not tracked: `make clean` BEFORE and AFTER, or the normal ROM boots into the test:
```
make clean; $env:CFLAGS="-DPRUEBA_CARGA"; make; $env:CFLAGS=$null
.\scripts\capturar_mesen.ps1 -Rom RUTA\prueba_carga.sfc -Duracion 66 -Cada 2 -Espera 2
```
Self-playing ROM of stage 1 (boots into the phase and fights two village zones from a table in `mando.c`, HUD on; `make clean` before and after). Variants: add `-DGUION_BARBARA` (Bárbara), `-DESCENARIO_INICIAL=n` (0 village, 1 forest, 2 monastery, 3 dojo), `-DGUION_APOYO` (support bar always full, R/X pressed by the script). The capture script loses every key after an arrow key: test arrows by hand.
```
make clean; $env:CFLAGS="-DGUION_FASE1"; make; $env:CFLAGS=$null
.\scripts\capturar_mesen.ps1 -Rom RUTA\guion_fase1.sfc -Duracion 25 -Cada 1 -Espera 3
```
Stage 1 structure (scenarios and their tables, waves per scenario, text scenes, supports, opaque objects by colour windows): `docs/estado/fase1-y-roms.md` and `docs/mapa-de-archivos.md`. wlalink runs with `-d`: the ROM grows when code first references an asset only a test ROM used (`.claude/rules/codigo-snes.md`).

Mesen verification without hands (window captures every N seconds, optionally starting at `-Desde`; pad driven by a time-based key script; see `docs/mesen-y-depuracion.md` §5):
```
.\scripts\capturar_mesen.ps1 -Duracion 70 -Cada 1 -Espera 4 -Guion "55,55.3,Start;57,57.3,Select;58,61,Right"
```
Assets: the level 1 is drawn in Tiled, the art comes from Pixellab (MCP server `pixellab`; read https://api.pixellab.ai/mcp/docs first) and every asset has a generator script with versioned outputs; the commands and details are in `.claude/rules/assets.md` (loads when touching `scripts/` or `res/`).

There is no linter; `.clang-format` holds the house style.

## Architecture

Sources live in subfolders of `src/` and are included with a path from `src/` (`#include "sistema/comun.h"`; the Makefile adds `-Isrc`). Every `.c/.h/.asm` opens with the standard header block (purpose, connections, hardware touched, variables, glossary) defined in `docs/diseno-demo-jugable.md` §12. Full per-file map: `docs/mapa-de-archivos.md`.

- `src/sistema/` — `main.c` (entry point: Mode 1 + console, `secuencia_iniciar(PANTALLA_NINTENDO)`, loop `secuencia_actualizar(); WaitForVBlank(); secuencia_vblank();`), `secuencia.c/.h` (screen state machine: `_iniciar` once, `_actualizar` per frame, `_vblank` right after VBlank; `frames_en_pantalla`), `pantallas.h` (screen enum, timings in frames, `PERMITE_SALTO`), `comun.c/.h` (VRAM layout, fonts, background loaders, text helpers, fades), `tipos.h` (`u8/u16/s16` from `snes.h` on SNES or `stdint.h` with `-DPRUEBAS_PC`), `puntofijo.h` (12.4 fixed point: `PF()`, `PF_ENTERO()`).
- `src/pantallas/` — one file per screen: nintendo, publisher, devs, dedicatoria, intro (3 scenes), titulo, **nivel1** (the game: per frame `mando_leer` → `tarou_actualizar` → `partida_actualizar` → camera → sprite; the scroll registers are written in `pantalla_nivel1_vblank`, together with the DMA list, never mid-frame, because `bgSetScroll` writes immediately; respawn with the screen off), **proximamente** ("LEVEL 2 / COMING SOON" after the exit), plus `personajes.c/.h` (static sprites of Tarou and Bárbara for intro/title).
- `src/escena/` — the **scene engine** (`docs/diseno-motor-escenas.md`): `escena.c/.h` (pure logic, PC-tested: a table of `RAMPA`/`FIJAR`/`FIN_ESCENA` events, 8 bytes each with the 8.8 step computed by the compiler, moves 4 tracks: brightness, mosaic, shine position, light), `efectos.c/.h` (hardware: applies the tracks in VBlank; the shine is window 1 driven by indirect HDMA on channel 6 plus fixed-color math, moved by rewriting a 2-byte pointer; `efectos_terminar` must run before changing screen), `escena_nintendo.c/.h` (the logo's 244-frame table). Used by `pantalla_nintendo.c`, which has a `_vblank` dispatched by `secuencia_vblank`. The image screens (boot screens via `video_texto_mode1`, intro, title) use vertical scroll −1 (`SCROLL_Y_FILA0` in `comun.h`). With scroll 0 the PPU never shows row 0, and line 223 shows the empty row 224 as a colored line. The level stays at 0. `secuencia_cambiar` calls `setScreenOff()` before every `_iniciar` (fade_out leaves the screen ON at brightness 0, so VRAM loads outside VBlank were lost: the title-after-level bug).
- `src/juego/` — the level logic. **Pure logic, no `snes.h`, tested on PC**: `tarou.c/.h` (the protagonist: struct, movement constants, 13-state machine, hitboxes), `animaciones.c/.h` (`enum Estado`, animation table from the generated `tarou_anim.h`), `nivel.c/.h` (level size 2048, base ground `SUELO_Y`, `nivel_columna_vram` for the 64x32 ring map), `colision.c/.h` (the 128x14 collision grid + one removable block; fast column/row queries because every call is expensive in tcc816), `partida.c/.h` (tutorial rules: fall/thorns → respawn at the last sign, sword breaks the log, exit), `guion.c/.h` + `guion_tutorial.c/.h` (a pad read from a time table; the full playthrough), `camara.c/.h`, `metasprite.c/.h` (walk and mirror the `gfx4snes -T` tables: up to 6 pieces in a 96x64 cell, stop at the terminator; `metasprite_copias` plans the DMA chunks that upload a frame). **Hardware layer**: `mando.c/.h` (pad bits to `struct Mando`), `dma_lista.c/.h` + `dma_ejecutar.asm` (**every VRAM write of the level goes through this list**: modules append copies during the frame, the asm fires one DMA per entry in VBlank; the asm file must not share a base name with a `.c`, or `make` overwrites it), `tarou_sprite.c/.h` (96x64 metasprite drawn with plain `oamSet`, no SDK dynamic engine since 2026-10-21: two VRAM cells (tiles 0..127 / 128..255), the new frame's 3 KB go to the hidden cell over two VBlanks via `metasprite_copias`, then the OAM switches; drawn 2 px below the physics so the feet sink into the grass; PARADO with the sword out is drawn with the 1-frame `guardia` strip, a drawing and not a state: `DIBUJO_GUARDIA = ESTADO_CANTIDAD`), `fondo_nivel.c/.h` (near layer streamed by columns into the 64x32 ring: the entering columns are appended to the list when the camera crosses a column, with `VMAIN = 0x81`; plus the log patch), `hud_debug.c/.h` (debug HUD written straight into the text tilemap, one 64-byte row per frame appended to the list, rows cleared with `memset` and written through advancing pointers, and skipped when the beam is already past line 150; SELECT toggles it; its `V a b` row is the VBlank budget: lines used, limit 37).
- Beat 'em up (stage 1) modules: `src/pantallas/pantalla_selector.c`, `pantalla_fase1.c` (the phase: scenarios, waves, text scenes, supports; its `_vblank` writes scroll, palettes, windows and the DMA list); pure logic in `src/juego/` (`mundo.c` the body and its moves, `combate.c`, `oleadas.c` per-scenario tables, `estado_fase.c` pause/GAME OVER/STAGE CLEAR, `personaje.c` characters as data, `dialogo.c`, `apoyo.c`, `ambiente.c`) and `src/enemigos/ninja.c` (all enemy types and the boss), `src/objetos/objetos.c`; hardware in `protagonista_sprite.c`, `ninja_sprite.c`, `objetos_sprite.c` (colour windows), `ambiente_sprite.c`, `sprites.c` + `sprites_oam.asm`, `fondo_nivel.c` (all scenarios), `src/hud/hud_juego.c`, `hud_apoyo.c`. Per-file detail: `docs/mapa-de-archivos.md`.
- `src/audio/`, `niveles/`, `src/juego/barbara.*`, `cambio_protagonista.*`, `src/enemigos/enemigos.*`, `src/hud/hud.*` — documented **stubs** (`PENDIENTE: Etapa N`) for future subsystems; they compile and link, no logic.
- `data.asm` — every asset embedded with `.incbin` in its own `superfree` section (each must fit a 32 KB bank), including the level 1 near layer with its map stored column by column (`nivel1_cerca_col`, plus a 1-byte section with its bank, `nivel1_cerca_col_banco`, which the DMA needs), the collision grid (`nivel1_colision`) and the 14 strips of Tarou (13 states + `guardia`; `.rodata_tarou_<anim>`, at most 10 frames of 3 KB each). Labels are referenced from C as `extern char nombre;`.
- `res/` — prepared assets per screen plus `res/personajes/tarou/` (14 indexed strips, `tarou_reposo_estatico.png` (always 2 idle frames, for the static load of intro and title) and `tarou_anim.h`, generated by `scripts/preparar_personaje.py` from the native Pixellab export in `res/descargas/graficos/pixellab/tarou/`; numbered placeholders for any animation missing there) and `res/nivel1/` (`fondo_lejos.png` from `scripts/preparar_fondo_nivel.py`; `tutorial.tmj` + `logica.png`, the Tiled level, and the generated `fondo_cerca.png`, `colision.bin`, `nivel1_datos.h` from `scripts/preparar_nivel.py`; the Makefile turns the near layer's row-major map into `fondo_cerca.col` with `scripts/mapa_a_columnas.py`). gfx4snes outputs (`.pic/.pal/.map/.inc/_meta.inc/_data.as`) are git-ignored and rebuilt by `make`.
- `tests/` — PC test harness (`minitest.h`, `pruebas.c`, `prueba_*.c`, own `Makefile` with gcc and `-I../res/nivel1`) for the pure logic; `prueba_colision.c`, `prueba_partida.c` and `prueba_recorrido.c` run on the REAL level grid (`nivel1_datos.h` carries it under `PRUEBAS_PC`), so editing the level in Tiled can break them on purpose; `tests/tarou_anim.h` is a fixed copy of the generated header.
- `scripts/` — asset pipeline (`componer_escenas.py`, `preparar_personaje.py`, `preparar_fondo_nivel.py`, `preparar_sprites.py`, `preparar_imagen.py`, `generar_fuente_consola.py`, tests in `scripts/pruebas/`) and verification (`capturar_mesen.ps1`: runs Mesen at 100 % speed, drives the pad with a time-based key script and saves window captures; the Lua scripts are manual-only).
- `docs/` — `README.md` (index), design specs, plan, glossary, reference docs and `docs/estado/`. `aprendizaje/` is the course for whoever inherits the project (`aprendizaje/README.md` entry point, `aprendizaje/PLAN.md` spec; exercises on the real game in a clone, never in the main tree). Details: `.claude/rules/docs-y-curso.md`.
- `Makefile` — `ROMNAME`/`ROMTITLE`, `ROMSIZE 0C`/`ROMBANKS 128`/`FASTROM 1`, one gfx4snes rule per asset (pattern rule for the strips of Tarou with `-s 32 -T -X 96 -Y 64 -P 3`; the level 1 near layer uses `-F` so flipped tiles are deduplicated: 260 tiles instead of 281), `ASSETS`, `bitmaps`, `data.obj: $(ASSETS)` and the `.obj` dependencies on generated files (`tarou_sprite.obj`, `personajes.obj`, `animaciones.obj`), then the SDK `snes_rules`, then `CFLAGS += -Isrc -Ires/personajes/tarou`. `hdr.asm` and `linkfile` are auto-generated; do not hand-edit.
- Every line of game code carries a Spanish comment (project rule: the owner is learning C and SNES from this code). tcc816/SNES traps — no `int`/`long`/`float`/`bool`, no divisions in per-frame code, struct sizes that are powers of two in per-frame loops, no variable shifts in hot loops, no `?:` with strings in variadics, cast `u8` to `(u16)` for `%d`, `memset` not byte loops, `static` variables are NOT zero at boot, global `const` tables live in WRAM (only `static const` stays in ROM), `.BASE` around a `RAMSECTION`, `PF_ENTERO` only for values >= 0, **VBlank budget 37 lines** (VRAM writes after it are silently lost): full text in `.claude/rules/codigo-snes.md` (loads when touching `src/`, `data.asm`, `Makefile`).

## Language

Code comments and project docs are in Spanish. **On-screen game text (title, credits, intro, "PRESS START") is in English** (decision of 2026-09-23, see `docs/diseno-secuencia-arranque.md`). Keep new code consistent with that unless told otherwise.

## Dónde va cada cosa

| Contenido | Destino |
| --- | --- |
| Trampa o regla que importa al tocar cierto código | `.claude/rules/<área>.md` con `paths:` de ese código |
| Cómo se averiguó: mediciones, forenses, descartes | `docs/<AAAA-MM-DD>-<tema>.md` |
| Estado del juego, crónica de funciones, pendientes, ROMs de prueba | `docs/estado/*.md` |
| Vale en toda sesión (stack, comandos, convenciones, mapa de módulos) | aquí, en `CLAUDE.md` |

Reglas: actualizar en su sitio; una corrección reemplaza la línea vieja, no se apila; toda rule nueva lleva `paths:` (sin él se cargaría en todas las sesiones); topes: `CLAUDE.md` 40.000 bytes y cada rule 50.000 (`python scripts/revisar_presupuesto_docs.py`, también como hook de Claude Code).
