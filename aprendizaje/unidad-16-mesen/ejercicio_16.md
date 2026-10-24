# Ejercicio 16 — Encontrar la causa con la herramienta adecuada

Las ROMs y sus `.sym` están en `E:\ProyectoSNES_trabajo\roms\` (tabla del
README). Las capturas de la ventana del juego se hacen con
`scripts\capturar_mesen.ps1` desde la raíz de `E:\ProyectoSNES` (sin teclas, salvo
donde se diga); las de las ventanas del depurador, a mano. **No uses el equipo
mientras corre una tanda.** Guarda cada captura con el nombre del caso.

Para cada caso escribe: síntoma, herramienta, lo que enseña, causa y arreglo.
Mira `solucion/casos_16.md` solo al terminar.

## Primero: tres prácticas de herramientas

**P1. Breakpoint de ejecución.** Abre `u16_guion_hud.sfc`, Debug → Debugger,
y pon un breakpoint de ejecución en `camara_para` (búscalo por la etiqueta o
escribe la dirección que da la demo: `$8ECFC0`). Cuando pare, mira el
parámetro en la pila y el valor que devuelve al salir (*Step Out*: el
resultado queda en `tcc__r0`, unidad 14). **Captura:** el depurador parado en
`camara_para`, con los registros; anota la `x` del HUD de ese momento y
comprueba `cámara = x + 48 − 128` (recortada a 0..1792).

**P2. Breakpoint de escritura.** En la misma ROM, breakpoint de **escritura**
en `$7E263C` (`tarou + 11`: `en_suelo`, demo `pantalla_nivel1.asm_tarou 11`).
Para cada vez que Tarou despega o aterriza. **Captura:** el depurador parado
con la instrucción que escribe (la de `aterrizar` o la de despegar en
`tarou.c`) y, en Memory Tools, el byte pasando de 0 a 1.

**P3. Event Viewer.** Con la misma ROM en el nivel, Debug → Event Viewer.
Marca las escrituras en registros de la PPU. **Captura:** el visor con una
escritura en `$2121/$2122` al final de cada franja del cielo (el HDMA del
canal 6, `setModeHdmaColor`) y el bloque de escrituras del VBlank (la lista de
DMA). Cuenta las franjas de color en las primeras 40 líneas.

## Caso 1 — Un sprite corrompido (Bárbara en el tile 192)

ROM: `u16_caso1_barbara192.sfc`. En la intro (escena 3) y en el título,
Bárbara tiene trozos de otro dibujo en la mitad de abajo de algunos cuadros.

```
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\roms\u16_caso1_barbara192.sfc -Duracion 56 -Desde 34 -Cada 1 -Espera 4
```

Herramientas: **Sprite Viewer** (la entrada 6: tamaño 64×64, tile 192) y
**Tile Viewer** (4bpp, desde la dirección de los sprites: ¿qué hay en los
tiles 256..? ¿Y en 0..63?). Calcula con la fórmula de la unidad 08 qué tiles
pide la fila de abajo de un sprite de 64×64 que empieza en el 192.

**Captura que lo aprueba:** una del título con Bárbara corrompida, y la del
Sprite Viewer con la entrada 6 seleccionada (tile 192) al lado del Tile Viewer
donde se ven los tiles de Tarou en las filas a las que «da la vuelta».
Compárala con `u16_normal.sfc` (Bárbara entera, tile 256).

## Caso 2 — Tarou parece flotar sobre el suelo

ROM: `u16_caso2_pies_flotan.sfc` (ROM de prueba: se juega sola).

```
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\roms\u16_caso2_pies_flotan.sfc -Duracion 8 -Cada 1 -Espera 3
```

La física dice que está en el suelo (con el HUD: SELECT, o compara con
`u16_guion_hud.sfc`: `Y 112`, pies en 176). Herramientas: **Sprite Viewer**
(la `y` de las piezas de abajo frente a la línea donde empieza la hierba en el
**Tilemap Viewer** de la capa cercana) y el arte (`res/nivel1/fondo_cerca.png`:
la banda de hierba empieza 2 px por encima del borde de la celda).

**Captura que lo aprueba:** dos capturas a tamaño real (o ampliadas sin
suavizado) de los pies, esta ROM y `u16_guion_hud.sfc`, en el mismo segundo:
en esta los pies quedan sobre la raya de hierba; en la buena, 4 px dentro. Y
una línea que explique por qué no es un fallo de física (perspectiva: el arte
es una vista un poco elevada; ver `HUNDIR_PIES`).

## Caso 3 — El texto del HUD tapado por el techo del túnel

ROM: `u16_caso3_hud_sin_prioridad.sfc` (se juega sola, con el HUD encendido).
Tarou cruza el túnel agachado entre los cuadros 893 y 1252 del guion (unos
15-21 s desde que empieza el nivel).

```
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\roms\u16_caso3_hud_sin_prioridad.sfc -Duracion 26 -Desde 12 -Cada 1 -Espera 3
```

Herramienta: **Tilemap Viewer**, fondo 2 (BG3, el texto): pasa el ratón por
una letra del HUD y mira el bit de **prioridad** de su entrada; compáralo con
la misma letra en `u16_guion_hud.sfc`. Y el fondo 0 (la capa cercana) en el
túnel: ¿sus tiles llevan prioridad?

**Captura que lo aprueba:** la del juego con el HUD cortado por el techo del
túnel, y la del Tilemap Viewer con la entrada de una letra del HUD
(prioridad 0) frente a la de la ROM buena (prioridad 1) en el mismo punto.

## Caso 4 — Una tecla del guion que no llega

Síntoma (real, 2026-10-20): una tanda con `-Guion "...;58,61,Right"` sobre la
ROM normal deja a Tarou quieto, y en alguna captura el menú **File** de Mesen
aparece resaltado.

No hay ROM rota: el juego no tiene la culpa. Herramientas: las propias
capturas (el menú resaltado), la salida del script (`Mesen activada: True`) y
`docs/mesen-y-depuracion.md` §5. Explica qué hace el script para mandar las
teclas (`PostMessage` a la ventana, foco en el visor) y por qué se pierden
cuando usas el equipo.

**Captura que lo aprueba:** una tanda completa con la ROM que **no necesita
teclas**, que es la salida fiable:

```
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\roms\u16_guion_hud.sfc -Duracion 46 -Cada 2 -Espera 3
```

Tarou recorre el nivel solo y la última captura enseña «LEVEL 2 / COMING SOON»
(el recorrido son 2238 cuadros, ~37,3 s). Escribe en una línea la causa del
síntoma y por qué esta tanda no puede sufrirla.

## Caso 5 — Tarou a trozos (medir, no mirar)

Lee `docs/diseno-tutorial-movimiento.md` §8 (fila «Tarou a trozos») y la fila
`V a b` del HUD en `docs/mesen-y-depuracion.md` §3. Después mide tú:

```
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\roms\u16_guion_hud.sfc -Duracion 10 -Desde 4 -Cada 0.2 -Espera 3
```

**Captura que lo aprueba:** dos capturas de esa tanda, una con `V5 7` (frame
normal) y otra con `V5 17..20` (frame en que sube medio cuadro de Tarou), y la
cuenta: ¿cuántas líneas quedan hasta 37 en el peor frame? ¿Y cuántas faltaban
el 2026-10-20 (`V5 46`)? Explica por qué ese fallo no se veía en el Sprite
Viewer (la OAM estaba bien) y sí se veía midiendo.
