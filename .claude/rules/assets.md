---
paths:
  - "scripts/**"
  - "res/**"
---

# Assets: Tiled, Pixellab y la tubería de scripts

> Movido verbatim desde `CLAUDE.md` el 2026-11-05 (ordenar-memoria-repo). Rutas relativas a la raíz del repo. El resto de lo que estaba en `CLAUDE.md` sigue allí; lo movido quedó así: la crónica del juego y de las ROMs en `docs/estado/fase1-y-roms.md`, las trampas del código en `.claude/rules/codigo-snes.md`, los assets en `.claude/rules/assets.md` y el detalle de `docs/` y `aprendizaje/` en `.claude/rules/docs-y-curso.md`.

Level 1 is drawn in **Tiled** as logic (solid / one-way platform / thorns on a 128x14 grid of 16 px cells, plus named objects `inicio`, `cartel` with a `texto` property, `tronco`, `salida`) in `res/nivel1/tutorial.tmj`; `python scripts/preparar_nivel.py` dresses it with the Pixellab dual-grid tileset and writes `res/nivel1/fondo_cerca.png` (2080 px: the level plus the 4 alternative columns of the log), `colision.bin` and `nivel1_datos.h` (then `make clean`). `--crear` rewrites the initial `.tmj` and discards edits.

Pixellab art can be generated from Claude Code through the `pixellab` MCP server (added 2026-11-04). Before using it, read its official documentation, https://api.pixellab.ai/mcp/docs (tools, modes, costs, downloads); the art commission per character and zone is `docs/pixellab-fase1.md`.

Asset pipeline (outputs are versioned): `python scripts/preparar_personaje.py tarou` (the native Pixellab export in `res/descargas/graficos/pixellab/tarou/`, mapped by `MAPEOS_PIXELLAB`, to 96x64 strips + `tarou_anim.h`), `python scripts/componer_escenas.py` (boot-sequence backgrounds), `python scripts/preparar_fondo_nivel.py res/titulo/fondo_lejos.png res/nivel1/fondo_lejos.png --borrar 71,22,185,76`, `python scripts/preparar_aldea.py` (the phase 1 village, above; ~20 s), `python scripts/preparar_nivel.py` (above; it takes the newest PNG of `res/descargas/graficos/pixellab/suelo/`; `preparar_suelo.py`, the flat 512 px ground, is superseded but kept).

## Al día (2026-11-05; añadido después de mover lo de arriba)

- `python scripts/preparar_zona.py [bosque monasterio dojo]`: los otros tres escenarios de la fase 1 → `res/zonas/` (cercana por columnas, lejana limitada a 496 tiles, cielo); después `make clean`.
- `preparar_aldea.py` y `preparar_zona.py` posan el «frente» (faroles, barriles, postes) al fondo del carril (`FRENTE_PIE` = 182) y sin prioridad (`FRENTE_PRIORIDAD = 0`): con prioridad tapaba a los personajes.
- `python scripts/preparar_objetos.py`: rompibles, comida, armas, astillas y chispa → `res/objetos/objetos_til.bin` y `objetos_datos.h` (`OBJ_PIEZA_*` y las cajas de lo dibujado `OBJ_CAJA_*`, que usan las ventanas de color de `objetos_sprite.c`).
- `python scripts/preparar_personaje.py <personaje>` también para `ninja_rapido` (con `--entrada res/descargas/graficos/pixellab/ninja`), `kendoka`, `gomasuri`, `lucinda`, `rex` y `fenrir`; `python scripts/preparar_personaje.py tarou --extras` escribe solo las tiras nuevas de Tarou (`golpe_4`, `levantarse`, `derribado`, `victoria`) con su paleta de siempre (la pasada normal rehace la paleta común y recolorearía todas).
- Todo frame de sprite: como mucho 24 piezas de 16x16 (`piezas16.py --rejilla-optima`); el jefe usa dos huecos (48).
- Pruebas de los scripts: las 9 de `scripts/pruebas/` (80 casos), listadas en `CLAUDE.md`.

