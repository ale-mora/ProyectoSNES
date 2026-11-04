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

- **Celda de 64×96** (vertical), no 64×64. El script ya la usa
  (`PERSONAJES["barbara"]` en `scripts/preparar_personaje.py`), junto con los
  colores fijos de piel, pelo y acero de la katana.
- **Altura de la figura: unos 88–90 px**, con los pies en la última fila de la
  celda. El script no escala.
- Todo mirando a la **derecha** (el espejo del hardware hace la izquierda).
- Máximo **15 colores + transparente** con una sola paleta, sin sombra ni suelo.
- **Máximo 10 cuadros por animación** (cada cuadro lleno son unos 2,7 KB y cada
  tira va en un banco de 32 KB). Hasta **21 piezas de 16×16 no vacías** por cuadro.
- Las animaciones de ataque llevan un `<animación>.cajas.json` junto a la
  exportación: dónde golpea la katana, en qué cuadro, y la caja de daño
  (`arte-beat-em-up.md` §1.4).

## Nombres de archivo

Nombres de la Revisión 3, de `docs/pixellab-fase1.md` §1.2: `guardia`,
`caminar`, `correr`, `saltar`, `caer`, `golpe_1`, `golpe_2`, `golpe_3`,
`aereo`, `agarre`, `golpe_agarre`, `lanzamiento`, `agarrada`, `recibir`,
`derribada`, `en_suelo`, `levantarse`, `especial`, `lanzar_arma`, `recoger`,
`entrada_relevo`, `invocar`, `ko`, `victoria`, `intro_venganza`.

De qué exportación sale cada animación lo dice `MAPEOS_PIXELLAB["barbara"]` en
`scripts/preparar_personaje.py`. Por ahora solo `guardia`, `caminar` y
`golpe_3` (provisional); el resto sale como marcador hasta que llegue su arte.
