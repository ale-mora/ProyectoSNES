# Pixellab → ninja (enemigo común de la fase 1)

Deja aquí la exportación de Pixellab del **ninja base** (git ignora todo lo que
pongas aquí salvo este README). Animaciones, cuadros y prompts:
[`docs/pixellab-fase1.md`](../../../../../docs/pixellab-fase1.md) §2.1–§2.4.

Se pide **un solo personaje**. Los colores de traje (hasta 3 por oleada,
paletas 1 a 3) se derivan cambiando la paleta, no con otra exportación.

## Formato de la celda

- **Celda de 96×96** (no 64×96: tumbado mide hasta 88 px de ancho), figura de
  **74–81 px** con los pies en la última fila.
- Mirando a la **derecha**; **15 colores + transparente**, una paleta, sin
  sombra ni suelo.
- **15–21 piezas de 16×16 no vacías** por cuadro: la espada se queda dentro
  de la celda (§2.3).
- La faja de un color que no se use en el resto del cuerpo: es el color que
  cambia la paleta.

## Animaciones del juego (cuadros de la tira)

`guardia` (8), `caminar` (6), `ataque_1` (3, puñetazo: el ninja no lleva
espada), `ataque_2` (8, lanzar shuriken sin el cuadro de la estela),
`bloqueo` (2), `agarre` (2), `lanzamiento` (6), `recibir` (6), `derribado`
(7), `en_suelo` (1, el último cuadro de `derribado`), `levantarse` (5), `agarrado` (2), `ko` (7). Total 63.

Para importarlo: `python scripts/preparar_personaje.py ninja`. La celda de
96×96 y los colores fijos de la piel están en `PERSONAJES["ninja"]`, y de qué
exportación sale cada animación, en `MAPEOS_PIXELLAB["ninja"]`. «Ninja 1» es
el personaje definitivo. `bloqueo`, `agarre_sitio`, `lanzamiento_corto` y
`agarrado` se generaron con el MCP (2026-11-04, solo el este) dentro del estado
`Fighting_stance`; agarre y lanzamiento van «en el sitio»: no avanzan, el juego mueve al ninja.
