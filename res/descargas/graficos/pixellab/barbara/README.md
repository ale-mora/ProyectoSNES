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

- **Celda de 112×96** (2026-11-04): Bárbara mide ~70 px de alto (mayor que Tarou,
  menor que todos los ninjas) y con la katana llega a ~55 px del centro. Está en
  `PERSONAJES["barbara"]` (`scripts/preparar_personaje.py`), con los colores fijos de
  piel, pelo y acero de la katana.
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

**Personaje definitivo: «Bárbara pequeña»** (pro flash de Pixellab con el aspecto de
«Bárbara 2», 2026-11-04). Sus 23 animaciones están en el estado `Idle` (skeleton-v3, solo
el este); de qué animación y cuadros sale cada una lo dice `MAPEOS_PIXELLAB["barbara"]` en
`scripts/preparar_personaje.py`. La versión anterior (~92 px) está en `../barbara_grande_r3/`
(solo en local).
