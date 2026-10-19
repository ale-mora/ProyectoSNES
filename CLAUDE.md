# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

A Super Nintendo (SNES) homebrew platformer written in C, built with **PVSnesLib** (a C SDK for the SNES built on the `tcc816` compiler and the `wla-65816`/`wlalink` toolchain). Target platform: LoROM, SlowROM, 16 ROM banks (512 KB). Output is a `.sfc` ROM run in the **Mesen** emulator.

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

What the current ROM does when run (Revisión 1, 2026-10-19): the boot sequence (Nintendo → publisher → devs → dedication → 3-scene intro → title with parallax; the title appears at ~50 s and loops to the intro after 15 s without input), then Start on the title enters **level 1**: Tarou (adult husky, the real Pixellab art in 96x64 cells; the attack is a fixed pose until an attack animation is generated) walks, runs (Y), jumps with variable height (B), crouches and crouch-walks (Down), unsheathes/sheathes the sword (R), attacks (A), and falls asleep after 10 s idle; the camera follows across a 512 px flat level (Pixellab ground with a hedge behind, feet 4 px into the grass band) with parallax; SELECT toggles the debug HUD (position, velocity, state, camera, hitbox, frame counter and scanline cost meter); START returns to the title. Design: `docs/diseno-demo-jugable.md`. Keyboard mapping of Mesen on this machine: arrows, A=S, B=A, X=X, Y=Z, L=Q, R=W, Start=D, Select=E (`docs/mesen-y-depuracion.md`). The capture script forces `EmulationSpeed=100` and `AllowBackgroundInput=true` on the command line; neither is saved to the author's settings. The scripted keys can still be lost (Mesen's focus jumps to its menu bar) while the author uses the PC: the reliable condition is to leave the PC alone during a run; the effect of `AllowBackgroundInput` is not proven.

Tests: the pure game logic has a PC test suite (gcc, 195 checks) and the asset scripts have `unittest` tests. Run before every commit that touches `src/juego/` or `scripts/`:
```
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
python -m unittest scripts/pruebas/prueba_preparar_personaje.py scripts/pruebas/prueba_preparar_fondo_nivel.py scripts/pruebas/prueba_preparar_suelo.py
```
Mesen verification without hands (window captures every N seconds, optionally starting at `-Desde`; pad driven by a time-based key script; see `docs/mesen-y-depuracion.md` §5):
```
.\scripts\capturar_mesen.ps1 -Duracion 70 -Cada 1 -Espera 4 -Guion "55,55.3,Start;57,57.3,Select;58,61,Right"
```
Asset pipeline (outputs are versioned): `python scripts/preparar_personaje.py tarou` (the native Pixellab export in `res/descargas/graficos/pixellab/tarou/`, mapped by `MAPEOS_PIXELLAB`, to 96x64 strips + `tarou_anim.h`), `python scripts/componer_escenas.py` (boot-sequence backgrounds), `python scripts/preparar_fondo_nivel.py res/titulo/fondo_lejos.png res/nivel1/fondo_lejos.png --borrar 71,22,185,76`, `python scripts/preparar_suelo.py --seto res/titulo/fondo_cerca.png` (level 1 ground from the Pixellab Sidescroller tileset in `res/descargas/graficos/pixellab/suelo/`, the grass band starts 4 px above `SUELO_Y` = 176 so the feet sink into it (`--hundir`), title bushes behind as a hedge; it takes the newest PNG of that folder).

There is no linter; `.clang-format` holds the house style.

## Architecture

Sources live in subfolders of `src/` and are included with a path from `src/` (`#include "sistema/comun.h"`; the Makefile adds `-Isrc`). Every `.c/.h/.asm` opens with the standard header block (purpose, connections, hardware touched, variables, glossary) defined in `docs/diseno-demo-jugable.md` §12. Full per-file map: `docs/mapa-de-archivos.md`.

- `src/sistema/` — `main.c` (entry point: Mode 1 + console, `secuencia_iniciar(PANTALLA_NINTENDO)`, loop `secuencia_actualizar(); WaitForVBlank(); secuencia_vblank();`), `secuencia.c/.h` (screen state machine: `_iniciar` once, `_actualizar` per frame, `_vblank` right after VBlank; `frames_en_pantalla`), `pantallas.h` (screen enum, timings in frames, `PERMITE_SALTO`), `comun.c/.h` (VRAM layout, fonts, background loaders, text helpers, fades), `tipos.h` (`u8/u16/s16` from `snes.h` on SNES or `stdint.h` with `-DPRUEBAS_PC`), `puntofijo.h` (12.4 fixed point: `PF()`, `PF_ENTERO()`).
- `src/pantallas/` — one file per screen: nintendo, publisher, devs, dedicatoria, intro (3 scenes), titulo, **nivel1** (the game), plus `personajes.c/.h` (static sprites of Tarou and Bárbara for intro/title).
- `src/juego/` — the level logic. **Pure logic, no `snes.h`, tested on PC**: `tarou.c/.h` (the protagonist: struct, movement constants, 13-state machine, hitboxes), `animaciones.c/.h` (`enum Estado`, animation table from the generated `tarou_anim.h`), `nivel.c/.h` (level size, flat ground), `camara.c/.h`, `metasprite.c/.h` (walk and mirror the `gfx4snes -T` tables: up to 6 pieces in a 96x64 cell, stop at the terminator). **Hardware layer**: `mando.c/.h` (pad bits to `struct Mando`), `tarou_sprite.c/.h` (64x64 metasprite with the SDK dynamic sprite engine, mirror built in RAM), `hud_debug.c/.h` (debug HUD written straight into the text tilemap and copied by DMA in VBlank; SELECT toggles it).
- `src/enemigos/`, `objetos/`, `hud/`, `audio/`, `niveles/`, `src/juego/barbara.*`, `cambio_protagonista.*` — documented **stubs** (`PENDIENTE: Etapa N`) for future subsystems; they compile and link, no logic.
- `data.asm` — every asset embedded with `.incbin` in its own `superfree` section (each must fit a 32 KB bank), including the 13 animation strips of Tarou (`.rodata_tarou_<anim>`, at most 10 frames of 3 KB each). Labels are referenced from C as `extern char nombre;`.
- `res/` — prepared assets per screen plus `res/personajes/tarou/` (13 indexed strips, `tarou_reposo_estatico.png` (always 2 idle frames, for the static load of intro and title) and `tarou_anim.h`, generated by `scripts/preparar_personaje.py` from the native Pixellab export in `res/descargas/graficos/pixellab/tarou/`; numbered placeholders for any animation missing there) and `res/nivel1/fondo_lejos.png` (`scripts/preparar_fondo_nivel.py`). gfx4snes outputs (`.pic/.pal/.map/.inc/_meta.inc/_data.as`) are git-ignored and rebuilt by `make`.
- `tests/` — PC test harness (`minitest.h`, `pruebas.c`, `prueba_*.c`, own `Makefile` with gcc) for the pure logic; `tests/tarou_anim.h` is a fixed copy of the generated header.
- `scripts/` — asset pipeline (`componer_escenas.py`, `preparar_personaje.py`, `preparar_fondo_nivel.py`, `preparar_sprites.py`, `preparar_imagen.py`, `generar_fuente_consola.py`, tests in `scripts/pruebas/`) and verification (`capturar_mesen.ps1`: runs Mesen at 100 % speed, drives the pad with a time-based key script and saves window captures; the Lua scripts are manual-only).
- `docs/` — `README.md` (index), the two design specs, the plan, the glossary and the four reference docs (file map, build pipeline, Mesen, hardware). `aprendizaje/PLAN.md` is the learning plan.
- `Makefile` — `ROMNAME`/`ROMTITLE`, `ROMSIZE 09`/`ROMBANKS 16`, one gfx4snes rule per asset (pattern rule for the strips of Tarou with `-s 32 -T -X 96 -Y 64 -P 3`), `ASSETS`, `bitmaps`, `data.obj: $(ASSETS)` and the `.obj` dependencies on generated files (`tarou_sprite.obj`, `personajes.obj`, `animaciones.obj`), then the SDK `snes_rules`, then `CFLAGS += -Isrc -Ires/personajes/tarou`. `hdr.asm` and `linkfile` are auto-generated; do not hand-edit.
- Every line of game code carries a Spanish comment (project rule: the owner is learning C and SNES from this code). Keep the pure logic free of `int`, `long`, `float`, `bool`, divisions in per-frame code (tcc816 calls a slow routine) and `?:` with strings as variadic arguments (tcc816 ICE).

## Language

Code comments and project docs are in Spanish. **On-screen game text (title, credits, intro, "PRESS START") is in English** (decision of 2026-09-23, see `docs/diseno-secuencia-arranque.md`). Keep new code consistent with that unless told otherwise.
