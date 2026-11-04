# Pixellab → Bárbara (segunda protagonista, Revisión 3)

Deja aquí la exportación de Pixellab de Bárbara (git ignora todo lo que pongas
aquí salvo este README). La lista de animaciones, cuadros y prompts está en
[`docs/pixellab-fase1.md`](../../../../../docs/pixellab-fase1.md) §1.2; el
reparto del arte en general, en `docs/arte-beat-em-up.md` §1.2.

Cuando las animaciones estén aquí, desde la raíz del repositorio:

```
python scripts/preparar_personaje.py barbara
```

Salida prevista: `res/personajes/barbara/barbara_<animación>.png` y
`res/personajes/barbara/barbara_anim.h`. Hasta que el juego use esta carpeta,
en la secuencia de arranque Bárbara sigue siendo el sprite de pack de 64×64
(`res/personajes/barbara_samurai.png`), que **no es referencia de escala**.

## Formato de la celda (Revisión 3)

- **Celda de 144×112** (2026-11-04): con la katana mide hasta 133 px de ancho
  y 106 de alto. Está en `PERSONAJES["barbara"]` (`scripts/preparar_personaje.py`),
  junto con los colores fijos de piel, pelo y acero de la katana.
- **Altura de la figura: 90–93 px**, con los pies en la última fila de la
  celda. El script no escala.
- Todo mirando a la **derecha** (el espejo del hardware hace la izquierda).
- Máximo **15 colores + transparente** con una sola paleta, sin sombra ni suelo.
- **Máximo 10 cuadros por animación** (cada cuadro lleno son unos 2,7 KB y cada
  tira va en un banco de 32 KB). Hasta **24 piezas de 16×16 no vacías** por cuadro
  (el búfer de la protagonista), contadas con `piezas16.py --rejilla-optima`.
- Las animaciones de ataque llevan un `<animación>.cajas.json` junto a la
  exportación: dónde golpea la katana, en qué cuadro, y la caja de daño
  (`arte-beat-em-up.md` §1.4).

## Nombres de archivo

Nombres de la Revisión 3, de `docs/pixellab-fase1.md` §1.2: `guardia`,
`caminar`, `correr`, `saltar`, `caer`, `golpe_1`, `golpe_2`, `golpe_3`,
`aereo`, `agarre`, `golpe_agarre`, `lanzamiento`, `agarrada`, `recibir`,
`derribada`, `en_suelo`, `levantarse`, `especial`, `lanzar_arma`, `recoger`,
`entrada_relevo`, `invocar`, `ko`, `victoria`, `intro_venganza`.

**Personaje definitivo: «Bárbara 2» con katana.** Las 25 animaciones tienen
arte (2026-11-04). 21 se generaron con el MCP de Pixellab (skeleton-v3, solo el
este): 20 en el estado `Fighting_pose_with_a` y `especial` en
`Fighting_pose_with_a_2` (katana en llamas). `guardia` es el Idle de
`Fighting_pose_with_a` y `golpe_3`, el estado `Attack_1`. De qué estado y cuadros sale cada una lo dice
`MAPEOS_PIXELLAB["barbara"]` en `scripts/preparar_personaje.py`.
