# Ejercicio 19 (juego real) — Pies, piezas y animaciones nuevas

Tres partes sobre el juego, de menos a más. Trabaja en un **clon** (`git clone
E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio`) o en una rama propia: nada de esto
se confirma en la rama principal.

**Preparar el clon.** `res/descargas/` no está en git (son descargas). Copia,
solo para leerlas, las carpetas que necesitan los scripts:

```
Copy-Item -Recurse E:\ProyectoSNES\res\descargas\graficos\pixellab\suelo\*.png  <clon>\res\descargas\graficos\pixellab\suelo\
Copy-Item -Recurse E:\ProyectoSNES\res\descargas\graficos\pixellab\tarou\*      <clon>\res\descargas\graficos\pixellab\tarou\
```

Antes de cambiar nada, comprueba que los scripts son **deterministas**: en el
clon, `python scripts/preparar_personaje.py tarou` y
`python scripts/preparar_nivel.py` deben dejar `git status` limpio (los PNG que
generan son idénticos byte a byte a los versionados; si no, algo de tu entorno
es distinto: avisa). Compilar va siempre en **PowerShell** con el entorno de la
unidad 03.

## Parte A — Hundir los pies: 0, 2 y 4

En `src/juego/tarou_sprite.c` está `#define HUNDIR_PIES 2`: el sprite se dibuja
2 px más abajo que la posición física.

1. **TODO A1 — Medir la banda de hierba.** Con la hoja de Pixellab del suelo,
   averigua en qué fila empieza la hierba de la pieza «tierra abajo, aire
   arriba» (clave 12) cuando se pega en el borde del suelo base (y 176). Pista:
   `preparar_nivel.piezas_por_esquinas(hoja)[12]` y la celda de arte se pega en
   `f * 16 − 8`. ¿Cuántos píxeles por encima del borde físico empieza?
2. **TODO A2 — Tres ROMs.** Compila la ROM de prueba (se juega sola) con
   `HUNDIR_PIES` a 0, a 2 y a 4, copiando cada `.sfc` aparte antes de la
   siguiente:

   ```
   make clean; $env:CFLAGS="-DGUION_PRUEBA"; make
   Copy-Item ProyectoSNES.sfc ..\roms\hundir2.sfc
   # cambia HUNDIR_PIES a 0 (es un .c: basta "make"), make, copia; luego a 4
   $env:CFLAGS=$null
   ```

3. **TODO A3 — Capturas.** Tarou solo está quieto mucho rato contra el tronco
   (cuadros 1757..1912 del guion, unos 29–32 s desde que arranca la ROM):

   ```
   .\scripts\capturar_mesen.ps1 -Rom ..\roms\hundir0.sfc -Duracion 33 -Desde 29 -Cada 1 -Espera 3
   ```

   (igual con 2 y 4; no uses el equipo mientras captura). Recorta los pies en
   las tres y compáralas.
4. **TODO A4 — Explicar.** ¿Cuántos píxeles quedan los pies dentro de la hierba
   en cada caso? ¿Por qué con 0 parece que flota? ¿Por qué ni la física ni
   `tests/` cambian (corre `make -C tests; .\tests\pruebas.exe`: 367 en verde
   con los tres valores)? Deja `HUNDIR_PIES` en 2.

## Parte B — Integrar una pieza que falta: la zarza

Hoy las zarzas son un **marcador** que dibuja `poner_zarzas` en
`scripts/preparar_nivel.py`. La lista de piezas que hay que pedir a Pixellab
está en `res/descargas/graficos/pixellab/suelo/README.md`: dejarlas en esa
carpeta como `zarza.png`, `cartel.png`, `tronco.png`, `salida.png`.

1. **TODO B1 — Encontrar el fallo antes de que pase.** Lee `main()` de
   `preparar_nivel.py`: ¿qué PNG toma como hoja de tiles? ¿Qué pasaría al dejar
   `zarza.png` en esa carpeta? (Respuesta: es el más reciente, se tomaría por la
   hoja y `piezas_por_esquinas` saldría con «le faltan las combinaciones...».)
2. **TODO B2 — Pruebas en rojo primero (TDD, unidad 18).** En
   `scripts/pruebas/prueba_preparar_nivel.py`, añade una clase `PruebaPiezas`
   con dos pruebas: (a) en una carpeta temporal con una hoja falsa
   (`hoja_doble`) y una `zarza.png` más reciente, `pn.hoja_de_suelo(carpeta)`
   devuelve la hoja; (b) `pn.poner_zarzas(capa, g, oscuro, claro, pieza)` con
   una pieza de 16×16 pega SUS píxeles en la celda de zarza. Córrelas: deben
   fallar (`hoja_de_suelo` no existe; `poner_zarzas` no admite la pieza).
3. **TODO B3 — El código.** En `preparar_nivel.py`: `PIEZAS = ("zarza",
   "cartel", "tronco", "salida")`; `hoja_de_suelo(carpeta)` (el PNG más
   reciente que no sea una pieza); `cargar_pieza(ruta)` (RGBA, **16×16 o
   nada**: sin escalar, avisando); `poner_zarzas(..., pieza=None)` (la pieza o
   el marcador); `preparar(..., zarza=None)`, y que `main()` use las dos.
   **Vigilar los colores:** la paleta del nivel ya está llena (15: el seto y la
   tierra); la pieza NO añade colores, `pp.indexar` cambia cada uno por el más
   cercano. Escribe `colores_ajenos(pieza, colores)` e imprime cuántos colores
   de la pieza no están en la paleta.
4. **TODO B4 — Arte.** Si no tienes aún la zarza de Pixellab, fabrica una de
   ejemplo con `solucion/crear_zarza_ejemplo.py` (dibujada a mano, NO es de
   Pixellab: no la confirmes como arte final). Regenera:
   `python scripts/preparar_nivel.py`. Anota los «tiles distintos» (antes 260)
   y el aviso de colores.
5. **TODO B5 — Comprobar.** Las pruebas de los scripts (ahora **37**, con las
   dos nuevas) en verde; `make clean; make`; `tests/pruebas.exe` en verde (367:
   la rejilla no cambia); ROM de prueba y captura del tramo de las zarzas (unos
   24–28 s):

   ```
   .\scripts\capturar_mesen.ps1 -Rom ..\roms\zarza.sfc -Duracion 29 -Desde 23 -Cada 1 -Espera 3
   ```

   Mira en la captura si los colores aproximados se ven bien. Si no, el remedio
   es pedir a Pixellab la pieza con la paleta del suelo (no ampliar la paleta:
   Mode 1 da 16 colores por paleta y la capa cercana usa una).

## Parte C — Una animación nueva de Pixellab: el ataque

`atacar` es una **pose fija** (`Attack_1/rotations/east.png`, lienzo de 64×64)
porque Pixellab no ha generado aún una animación de ataque. (Comprueba que
`Attack_1`, `Attack_2` y `Fight_Pose` son la misma imagen: no hay nada mejor en
la exportación actual.)

1. **TODO C1 — Diseñar el mapeo.** Si mañana llega una carpeta `Attack/` con
   `animations/<nombre>/east/frame_000.png...`, ¿qué línea de `MAPEOS_PIXELLAB`
   la usaría? ¿Y qué pasa con el juego HOY si la cambias y la carpeta no
   existe? (Respuesta: `cargar_pixellab` devuelve `None` y sale un marcador
   numerado de 5 cuadros: se rompe el arte del ataque.)
2. **TODO C2 — Pruebas en rojo.** En `prueba_preparar_personaje.py`, una clase
   `PruebasAtaqueNuevo` con dos pruebas, con exportaciones falsas
   (`exportar_pixellab`, `rotacion_pixellab`): sin `Attack`, atacar tiene 1
   cuadro (la pose); con `Attack` de 6 cuadros, atacar tiene 6, en orden.
3. **TODO C3 — El código.** Que una entrada de `MAPEOS_PIXELLAB` pueda ser una
   **lista de reglas por orden de preferencia** y `preparar` use la primera que
   exista: `"atacar": [("Attack", "east", 0, None, False), ("Attack_1", "east",
   None, None, False)]`. Las pruebas de scripts: **39** en verde (35 + 2 de B +
   2 de C). Con la exportación de hoy, `preparar_personaje.py tarou` debe dejar
   `git status` limpio: la pose sigue siendo la elegida.
4. **TODO C4 — Lo que vigilar cuando llegue el ataque** (responde por
   escrito): el límite de **10 cuadros** (30 KB en una sección de 32 KB);
   que el lienzo de la animación (88 o 96 px) se ancla por centro y pies igual
   que el resto, pero la pose era de 64; que la **paleta común** se recalcula
   con los cuadros de TODAS las animaciones, así que las 13 tiras cambian de
   índices (todas se regeneran y todas se revisan); que `tarou_anim.h` cambia
   (`TAROU_ATACAR_CUADROS`) y por eso hace falta `make clean`; que
   `DURACION_ATAQUE / cuadros` (unidad 11) da los frames por cuadro, entera; y
   que `tests/tarou_anim.h` es una copia fija y no cambia.

Cuando exista la animación: cópiala al clon, `preparar_personaje.py tarou`,
`make clean`, ROM de prueba y captura del golpe al tronco (cuadro 1872, unos
31–32 s).

## Criterio de aprobación

- Las pruebas de los scripts en verde (39 con las partes B y C) y
  `tests/pruebas.exe` en verde (367).
- Capturas de la parte A con los tres valores y la explicación de A4.
- Captura de la parte B con la zarza integrada y la cuenta de tiles y colores.
- Parte C: las pruebas nuevas y las respuestas de C4 (la captura, cuando
  Pixellab dé el ataque).

## Solución

`solucion/piezas_y_ataque.diff` (el cambio de los dos scripts y de sus pruebas,
tomado con `git diff` en el clon) y los archivos completos `solucion/preparar_nivel.py`,
`solucion/preparar_personaje.py`, `solucion/pruebas/prueba_preparar_nivel.py`,
`solucion/pruebas/prueba_preparar_personaje.py`; `solucion/crear_zarza_ejemplo.py`.
Salidas en `capturas/`: `unittest_rojo.txt` (las 3 pruebas nuevas que fallan
antes del código: 2 errores y 1 fallo), `unittest_verde.txt` (39 OK),
`pruebas_pc.txt` (367, 0 fallos), `preparar.txt` (lo que imprimen los scripts),
`compilacion.txt`.

Respuestas de la parte A: la hierba empieza en la fila 6 de la pieza 12, que se
pega en y 168: **y 174, 2 px por encima** del borde físico (176). Los pies
físicos acaban en la fila 175 (la última de la celda). Con `HUNDIR_PIES` 0 se
dibujan en 175: 2 px dentro de la banda; con 2, en 177: 4 px (lo elegido); con
4, en 179: 6 px, se nota que «se hunde». Ni la física ni las pruebas cambian
porque `HUNDIR_PIES` solo se suma en `tarou_sprite_dibujar`, que no está en
`tests/`.

Parte B: con la zarza de ejemplo, **245** tiles distintos (antes 260; medido:
averigua por qué baja comparando las dos `fondo_cerca.png` en la zona de las
zarzas) y «5 colores fuera de la paleta: se cambian por el más cercano».
