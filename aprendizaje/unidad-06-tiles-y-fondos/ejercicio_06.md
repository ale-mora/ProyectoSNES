# Ejercicio 06 — Cambiar la paleta y el mapa en marcha

Tres partes. La A es la que se aprueba con una captura; la B se hace en Mesen
con el juego real; la C es una cuenta en el PC con `gfx4snes`.

## Parte A — La mini-ROM (TODO 1..4 de `rom/src/fondo.c`)

`rom/src/fondo.c` ya carga las montañas del título en el fondo 1 y el texto
en el fondo 0. Compílala tal cual (receta de abajo) y ábrela en Mesen: se ven
las montañas sobre un cielo negro, el logo TAROU y cinco líneas de texto. El
bucle principal cuenta frames y tiene tres huecos vacíos, a los 2, 4 y 6
segundos. Complétalos:

1. **TODO 1** (a los 2 s): pon el color 0 de la CGRAM en azul noche,
   `RGB5(2, 3, 10)`, con `setPaletteColor`. El cielo cambia aunque ninguna
   capa tenga un píxel de ese color: ¿por qué? (README, «El color 0»).
2. **TODO 2** (a los 2 s): pon el color `COLOR_MONTANA` (5) de la paleta
   `PALETA_MONTANAS` (2) en naranja, `RGB8(232, 120, 40)`. Calcula tú el
   número de entrada de la CGRAM (está en el README, «Paleta y color»).
   Todo lo que usa el índice 5 cambia a la vez: es el celeste claro de las
   montañas (3690 píxeles) y también el borde de arriba del bosque (1499
   píxeles en las 8 filas de abajo). Las sombras de las montañas son el 4 y
   siguen celestes. Prueba después con el 7, el
   color más usado de la imagen: cambia el bosque verde azulado de abajo, no
   las montañas. Mirar la paleta en el PNG antes de elegir índice ahorra
   sorpresas.
3. **TODO 3** (a los 4 s): copia las 4 filas del mapa donde está TAROU
   (`FILA_LOGO` = 2, `FILAS_LOGO` = 4) a las filas que empiezan en
   `FILA_CIELO` (10). Lee cada entrada de `mapa` (el mapa en la ROM) y
   escríbela en `bloque` (un array de 4 × 32 entradas en la WRAM), las cuatro
   filas seguidas, y pon `pendiente = 1`. **No llames a `dmaCopyVram` dentro
   del TODO**: el DMA ya está escrito al principio del bucle, justo después
   de `WaitForVBlank()`, y sube el bloque entero (256 bytes) en el VBlank
   siguiente. La entrada (columna c, fila r) de un mapa de 32×32 es la palabra
   `r * 32 + c` desde el principio del mapa; escribe la multiplicación como
   `<< 5`. ¿Por qué no copiar fila a fila con un DMA cada una, como parece
   natural? Lo explica el README, trampa «El bucle se come el VBlank»: esa
   versión compila, no da ningún aviso y la copia no aparece nunca.
4. **TODO 4** (a los 6 s): repite la copia **en espejo horizontal**. Hacen
   falta dos cosas: la celda de la columna `i` sale de la columna `31 - i`, y
   cada entrada lleva el bit de volteo horizontal (`BG_TIL_FLIPX`) cambiado
   con `^`. Se prepara en `bloque` y se marca `pendiente`, igual que el
   TODO 3. Antes de compilar, piensa qué se vería si hicieras solo una de las
   dos cosas (pruébalo después: es la mejor manera de entender los volteos).

Compilar (PowerShell, dentro de `rom/`):

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
& "E:\ProyectoSNES\tools\mesen\Mesen.exe" "$PWD\u06_fondo.sfc"
```

Éxito = `No warnings or errors found.` y `u06_fondo.sfc`. Lo que debe verse:

| Momento | Pantalla |
|---|---|
| 0–2 s | montañas celestes, bosque verde azulado abajo, cielo negro, TAROU y el subtítulo arriba |
| 2–4 s | cielo azul noche; montañas naranjas con sombras celestes (índice 4) y nieve blanca; el borde de arriba del bosque también naranja, el resto del bosque verde azulado igual |
| 4–6 s | un segundo TAROU en el cielo, encima de las montañas |
| desde 6 s | ese segundo TAROU en espejo: `UORAT` con cada letra al revés |

Captura sin manos (no hace falta ninguna tecla; las imágenes van a
`scripts\capturas\`):

```
cd E:\ProyectoSNES
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES\aprendizaje\unidad-06-tiles-y-fondos\rom\u06_fondo.sfc -Duracion 8 -Cada 1 -Espera 3
```

Las capturas empiezan unos 3 s después de arrancar la ROM: la captura 0
(ROM a ~3 s) ya tiene los colores cambiados; la 2 (~5 s), seguro la copia
derecha; de la 4 en adelante (~7 s y más), seguro el espejo. Las 1 y 3 caen
justo en el cambio (4 s y 6 s de ROM) y pueden salir de un lado o del otro. La fase de 0–2 s
(cielo negro, montañas celestes) no sale en la tanda: se ve al abrir la ROM
a mano.

Si te atascas, la solución completa está en `solucion/fondo.c` (cópiala
sobre `rom/src/fondo.c` para compilarla).

## Parte B — La capa cercana del juego en los visores de Mesen

Con la ROM del juego (`.\run.bat` o la ya compilada), entra en el nivel 1 y,
en Mesen, abre **Debug → Tilemap Viewer**:

1. Elige **BG1** (el fondo 0 de PVSnesLib: la capa cercana). Comprueba que el
   mapa mide **64×32** y que su dirección es `0x6C00` (`VRAM_MAPA_A`, en
   palabras; Mesen puede mostrarla en bytes: `0xD800`).
2. Pasa el ratón por una celda de tierra: Mesen muestra su número de tile, su
   paleta (**1**) y sus bits de volteo. Apunta lo que veas sobre los volteos
   (la parte C explica el resultado).
3. Camina hacia la derecha y mira cómo cambian las columnas del mapa por el
   borde derecho de la zona visible: es el anillo de la unidad 07.
4. Elige **BG3** (el texto) con el HUD encendido (SELECT): sus celdas tienen
   la **prioridad** a 1. Es el `0x2000` de `TILE(c)` en `src/juego/hud_debug.c`.
5. Abre **Debug → Tile Viewer**, dirección `0x2000` (palabras; `0x4000` en
   bytes), 4 bpp, paleta 1 de fondos: los tiles de la capa cercana. Cuenta
   cuántas filas de 16 ocupan antes de que empiece la basura o el vacío.

## Parte C — Cuántos tiles hay de verdad (PC)

La capa cercana del tutorial, `res/nivel1/fondo_cerca.png`, mide 2080×256 px:
260 × 32 = 8320 celdas de 8×8. Convierte una **copia** con tres juegos de
opciones y compara el tamaño de los `.pic` (bytes / 32 = tiles). En una
carpeta temporal, nunca en `res/` (allí `make` regenera los suyos):

```
$g="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\tools\gfx4snes.exe"
mkdir $env:TEMP\u06; cd $env:TEMP\u06
copy E:\ProyectoSNES\res\nivel1\fondo_cerca.png normal.png
copy normal.png sinreducir.png; copy normal.png volteos.png
& $g -q -s 8 -o 16 -u 16 -e 1 -p -m -i normal.png
& $g -q -R -s 8 -o 16 -u 16 -e 1 -p -m -i sinreducir.png
& $g -q -F -s 8 -o 16 -u 16 -e 1 -p -m -i volteos.png
dir *.pic
```

Responde en tu cuaderno:

1. ¿Cuántos tiles da cada conversión? ¿Cuál de las tres usa el `Makefile`
   del juego (línea de `res/nivel1/fondo_cerca.pic`)?
2. ¿Cabría la versión sin reducir en `VRAM_FONDO_A` (`src/sistema/comun.h`)?
   ¿Y en un banco de la ROM?
3. Con `-F`, la capa cercana ahorra 21 tiles y las montañas del título
   (`res/titulo/fondo_lejos.png`, pruébalo también) solo 9. ¿De dónde sale la
   diferencia? (Pista: `scripts/preparar_nivel.py`, la variable `volteos`.)
4. Con `-F`, ¿cuántas celdas del `.map` llevan algún bit de volteo? ¿Y en el
   `.map` que genera el `Makefile` del juego?
   Para contarlas (cada entrada son 2 bytes; los volteos, los bits 14 y 15):
   `python -c "import struct,sys; d=open(sys.argv[1],'rb').read(); print(sum(1 for (e,) in struct.iter_unpack('<H', d) if e & 0xC000))" volteos.map`

## Aprobación

- Captura de la mini-ROM **con tus TODO** en los tres momentos: colores
  cambiados (2–4 s), la copia (4–6 s) y la copia en espejo (desde 6 s).
- Las tres cifras de la parte C y las respuestas.
- (Parte B, recomendada) una captura del Tilemap Viewer con una celda de la
  capa cercana seleccionada.
