# Solución del ejercicio 16

Direcciones de las ROMs de `E:\ProyectoSNES_trabajo\roms\` (compiladas el
2026-10-22). Las ROMs con fallo se rehacen con los parches de esta carpeta.

## Prácticas

- **P1.** `camara_para` en `$8E:CFC0` (`u16_guion_hud.sym`; en `u16_normal.sym`
  está en `$8E:CBC7`: cada compilación mueve las funciones). Al salir, el
  resultado es `x + 48 − 128` recortado a `0..1792` (`ANCHO_NIVEL −
  ANCHO_PANTALLA`), con `x` = la `X` del HUD (el borde izquierdo de la celda,
  no su centro). Al empezar, `X` ≈ 8: `8 + 48 − 128 < 0` → 0.
- **P2.** `tarou` en `$7E:2631`, `en_suelo` en `+11` = `$7E:263C` (en las
  cuatro ROMs del juego de esta unidad: la `.bss` no se movió). Para en las
  escrituras de `tarou.c` (`aterrizar` lo pone a 1; saltar o caer, a 0).
- **P3.** El cielo del nivel es la misma tabla del título
  (`hdmaGradcieloList`): una escritura de color 0 (`$2121` dos veces, `$2122`
  dos veces) al final de cada franja; sus primeras entradas duran 8, 2, 4, 5,
  2, 7, 6, 2, 1... líneas (`res/titulo/cielo_grad_data.as`).

## Caso 1 — Bárbara en el tile 192

- **Herramienta:** Sprite Viewer + Tile Viewer.
- **Lo que enseñan:** la entrada 6 es un sprite de 64×64 con tile 192. Su fila
  de bloques de 8 px número `r` usa los tiles `192 + 16·r .. 192 + 16·r + 7`;
  la PPU calcula esas filas **dentro de la tabla de 256 tiles** del tile base:
  a partir de la fila 4 (`192 + 64 = 256`) da la vuelta a `0, 16, 32...` en vez
  de pasar a 256: ahí están los tiles de Tarou.
- **Causa:** un sprite grande no puede cruzar la frontera entre las dos tablas
  de 256 tiles (unidad 08).
- **Arreglo (el del juego):** Bárbara en el tile 256, el principio de la
  segunda tabla (`TILE_BASE_BARBARA 256`, `VRAM_BARBARA 0x1000`,
  `src/pantallas/personajes.c`, bloque «LÍMITE»).

## Caso 2 — Los pies que flotan

- **Herramienta:** Sprite Viewer (y de las piezas de abajo) frente al Tilemap
  Viewer / el PNG de la capa cercana (dónde empieza la hierba).
- **Lo que enseñan:** la física pone los pies en el borde exacto de la celda
  (y = 176). El arte de Tarou es una vista lateral un poco elevada (las patas
  lejanas, 3 px más arriba) y la banda de hierba empieza 2 px por encima del
  borde. Dibujado 2 px más arriba (`HUNDIR_PIES (-2)`), los pies quedan justo
  sobre el filo de la hierba: parece que flota.
- **Causa:** perspectiva, no física.
- **Arreglo:** hundir los pies 4 px en la banda: 2 px los pone el arte
  (`scripts/preparar_nivel.py`) y 2 px el dibujo (`HUNDIR_PIES 2`,
  `src/juego/tarou_sprite.c:93`). La física y las pruebas siguen usando el
  borde exacto.

## Caso 3 — El HUD detrás del techo del túnel

- **Herramienta:** Tilemap Viewer (fondo 2 = BG3, y fondo 0).
- **Lo que enseña:** las entradas del HUD tienen prioridad 0. Con
  `BG3_MODE1_PRIORITY_HIGH` (`setMode` del nivel), BG3 se dibuja delante de
  todo **solo con los tiles que llevan el bit de prioridad**; con prioridad 0
  va detrás de los fondos 0 y 1. Antes del tutorial no había nada arriba en la
  capa cercana y no se notaba; el techo del túnel (filas 0..7 de la rejilla,
  columnas ~52..59) lo destapó.
- **Arreglo:** `TILE(c)` con `| 0x2000` (bit 13 de la entrada del mapa) en
  `src/juego/hud_debug.c:85`.

## Caso 4 — La tecla que no llega

- **Herramienta:** las capturas (el menú File resaltado) y la salida del
  script.
- **Causa:** el script manda las teclas con `PostMessage(WM_KEYDOWN/UP)` a la
  ventana de Mesen, que las entrega donde tenga el foco **dentro** de su
  ventana. Al principio lo pone en el visor (clic por mensaje); si el autor
  usa el equipo, el foco de Mesen salta a su barra de menús y las teclas
  navegan el menú. `--Preferences.AllowBackgroundInput=true` se fuerza, pero
  su efecto no está demostrado.
- **Salida fiable:** no usar el equipo durante la tanda y, para el nivel, la
  ROM de prueba (`-DGUION_PRUEBA`): el mando sale de `guion_tutorial.c` dentro
  de la ROM; Mesen no tiene que recibir ninguna tecla. Con `u16_guion_hud.sfc`
  la tanda llega a «LEVEL 2 / COMING SOON».

## Caso 5 — Tarou a trozos

- **Medida:** hoy `V5 7` en frames normales y `V5 17..20` cuando sube medio
  cuadro (1,5 KB): en el peor frame quedan 37 − 20 = **17 líneas** de margen.
  El 2026-10-20 era `V5 46`: **9 líneas** de más (tiles del cuadro por el motor
  del SDK, 32 líneas, + columnas, 9, + 5 del SDK).
- **Por qué no se veía mirando:** la OAM y la lista de piezas estaban bien; lo
  que fallaba era que los últimos bytes de la copia a la VRAM llegaban con la
  PPU ya dibujando, y la VRAM los descarta sin aviso. El Sprite Viewer enseña
  la OAM (correcta) y el Tile Viewer un instante cualquiera; solo midiendo la
  línea del VBlank al salir de `_vblank` se ve que se pasa de 37.
- **Arreglo:** lista de DMA en ensamblador (`dma_ejecutar.asm`), doble búfer
  en la VRAM (medio cuadro por VBlank) y HUD por filas
  (`docs/diseno-tutorial-movimiento.md` §7).
