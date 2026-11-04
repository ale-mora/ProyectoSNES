# Pixellab → ninja (enemigo común de la fase 1)

Deja aquí la exportación de Pixellab del **ninja base** (git ignora todo lo que
pongas aquí salvo este README). Animaciones, cuadros y prompts:
[`docs/pixellab-fase1.md`](../../../../../docs/pixellab-fase1.md) §2.1–§2.4.

Se pide **un solo personaje**. Los colores de traje (hasta 3 por oleada,
paletas 1 a 3) se derivan cambiando la paleta, no con otra exportación.

## Formato de la celda

- **Celda de 64×96**, figura de **88 px** con los pies en la última fila.
- Mirando a la **derecha**; **15 colores + transparente**, una paleta, sin
  sombra ni suelo.
- **15–21 piezas de 16×16 no vacías** por cuadro: la espada se queda dentro
  de la celda (§2.3).
- La faja de un color que no se use en el resto del cuerpo: es el color que
  cambia la paleta.

## Nombres de archivo (propuestos)

`guardia` (4), `caminar` (8), `ataque_1` (4, espada corta), `ataque_2` (4,
lanzar shuriken), `bloqueo` (2), `agarre` (2), `lanzamiento` (6), `recibir`
(2), `derribado` (5), `en_suelo` (1), `levantarse` (5), `agarrado` (2), `ko`
(4). Total 49 cuadros.

Para importarlo: `python scripts/preparar_personaje.py ninja`. La celda de
64×96 y los colores fijos de la piel están en `PERSONAJES["ninja"]`, y de qué
exportación sale cada animación, en `MAPEOS_PIXELLAB["ninja"]` (provisional:
faltan `bloqueo`, `agarre`, `lanzamiento` y `agarrado`, que salen como marcador).
