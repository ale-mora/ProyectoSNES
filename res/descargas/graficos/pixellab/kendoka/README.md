# Pixellab → kendoka (escolta del jefe de la fase 1)

Deja aquí la exportación de Pixellab del **kendoka base** (git ignora todo lo
que pongas aquí salvo este README). Prompt y cuadros:
[`docs/pixellab-fase1.md`](../../../../../docs/pixellab-fase1.md) §2.5.

Solo aparece en la pelea de Gomasuri-dono en el dojo, donde no hay ninjas, así
que usa las mismas paletas 1 a 3 en varios colores de bogu.

## Formato de la celda

- **Celda de 96×96**, como el ninja (tumbado no cabe en 64), figura de
  **74–81 px** como «Ninja 1», con los pies en la última fila.
- Mirando a la **derecha**; **15 colores + transparente**, una paleta, sin
  sombra ni suelo. Hasta **21 piezas de 16×16** por cuadro; el shinai dentro
  de la celda.

## Nombres de archivo (propuestos)

Los del ninja (`../ninja/README.md`) con un cambio: `ataque_2` es una
**patada** (4), no el shuriken. `guardia` (4), `caminar` (8), `ataque_1` (4,
shinai), `ataque_2` (4, patada), `bloqueo` (2), `agarre` (2), `lanzamiento`
(6), `recibir` (2), `derribado` (5), `en_suelo` (1), `levantarse` (5),
`agarrado` (2), `ko` (4).

El script de importación aún no conoce al kendoka: los nombres se añaden a
`MAPEOS_PIXELLAB` al importar el arte.
