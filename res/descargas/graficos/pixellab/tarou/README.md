# Pixellab → Tarou adulto (protagonista, Revisión 1)

Deja en **esta carpeta** la exportación de Pixellab. Git ignora todo lo que
pongas aquí salvo este README (son descargas, como el resto de
`res/descargas/`). Después ejecuta, desde la raíz del repositorio:

```
python scripts/preparar_personaje.py tarou
```

El script genera `res/personajes/tarou/tarou_<animación>.png` (tiras indexadas
de celdas de **96×64**, con una sola paleta), `tarou_reposo_estatico.png` y
`res/personajes/tarou/tarou_anim.h` (cuántos cuadros tiene cada animación). Si
una animación falta, pone un **marcador** numerado en su lugar, así el juego
compila igual. Después, `make` (o `run.bat`): el arte entra sin tocar código.

## Formato 1: la exportación nativa de Pixellab (la que se usa hoy)

En *Characters → Export*, Pixellab da una carpeta por estado del personaje
(`Idle/`, `Walk/`, `Run/`, `Jump/`, `Sneak/`, `Unsheathe/`, `Stand_up/`,
`Sleep/`, `Attack_1/`...) con esta forma:

```
Idle/animations/<nombre largo>/east/frame_000.png ... frame_008.png
Idle/rotations/east.png                         (pose fija, 64×64)
```

Copia esas carpetas **tal cual** aquí (los `.zip` y el `.aseprite` pueden
quedarse, se ignoran). El script reconoce el formato y decide qué estado,
qué dirección y qué cuadros forman cada animación del juego con la tabla
`MAPEOS_PIXELLAB` de `scripts/preparar_personaje.py`:

| Animación del juego | Sale de | Notas |
|---|---|---|
| reposo | Idle | |
| caminar | Walk | |
| correr | Run | |
| saltar | Jump, cuadros 2..4 | Jump es un salto hacia delante |
| caer | Jump, cuadros 5..7 | |
| agachado | Sneak, cuadro 0 | no hay pose agachada quieta |
| agachado_caminar | Sneak | |
| desenvainar | Unsheathe | |
| envainar | Unsheathe al revés | |
| atacar | Attack_1 (pose fija) | **provisional**: falta una animación de ataque |
| echarse | Stand_up, cuadros 0..5 al revés | |
| dormido | Sleep, mirando al **sur** | Pixellab solo lo generó así |
| despertar | Stand_up, cuadros 0..9 | el 11.º no cabe en un banco |

Todo mira al **este** (la izquierda la hace el espejo del hardware). El arte
**no se escala**: cada animación se centra por el centro de su lienzo y se
apoya por los pies en la última fila de la celda de 96×64.

Si generas una animación nueva en Pixellab (por ejemplo un ataque), expórtala
igual, cópiala aquí y cambia su línea en `MAPEOS_PIXELLAB`.

## Formato 2: un archivo por animación (a mano)

Con los nombres del juego (`reposo`, `caminar`... de la tabla de arriba), uno
por animación:

- **Tira horizontal PNG**: `reposo.png` con cuadros **cuadrados** uno al lado
  del otro.
- **Carpeta con cuadros sueltos**: `reposo/01.png`, `reposo/02.png`… (en orden
  alfabético).
- **GIF animado**: `reposo.gif`.

## Reglas que impone la SNES (el script avisa si no se cumplen)

- Máximo **10 cuadros** por animación: cada cuadro de 96×64 son 3 KB y cada
  animación va en un banco de ROM de 32 KB.
- El personaje cabe en **96×64** (si no, el script avisa y recorta), mirando
  **a la derecha**.
- **Máximo 15 colores + transparente** y **una sola paleta** para todo el
  personaje. El script los reduce si hace falta (el arte actual tiene ~100 y
  se ve bien a 15), pero cuantos menos use el arte, mejor.
- Fondo **transparente** (alfa 0 o 255, sin semitransparencias), sin sombra
  proyectada ni suelo.
- **Caminar en el sitio**: el cuerpo no avanza dentro del cuadro; el
  desplazamiento lo pone el código.
- Bucle limpio: el último cuadro no repite el primero.
- La intro y el título usan solo **2** cuadros del reposo (los elige el
  script): con 96×64 no caben más junto a Bárbara en la memoria de sprites.
