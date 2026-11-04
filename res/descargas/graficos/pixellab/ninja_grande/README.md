# Pixellab → ninja grande (morado, ~100 px)

Deja aquí la exportación de Pixellab (git ignora todo lo que pongas aquí salvo este
README). Es un personaje nuevo con el diseño de «Ninja 1» (pro flash, 2026-11-04) y
sus 6 animaciones en el estado Idle (plantillas skeleton-v3, solo el este): guardia,
caminar, ataque_1 (puñetazo), recibir, derribado y levantarse; el KO usa la misma caída.

Para importarlo: `python scripts/preparar_personaje.py ninja_grande` (celda de 128x112, piel
como colores fijos y el traje recoloreado: `PERSONAJES["ninja_grande"]` y
`MAPEOS_PIXELLAB["ninja_grande"]`). Ver `docs/pixellab-fase1.md` §2.
