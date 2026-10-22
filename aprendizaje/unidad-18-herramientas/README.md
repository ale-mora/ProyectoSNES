# Unidad 18 — Herramientas: gfx4snes, scripts de Python y el Makefile

**Objetivo:** cambiar la tubería de assets del juego sin romper nada: saber
qué convierte cada herramienta, qué archivo genera cada paso, cómo se entera
`make` de que tiene que rehacerlo y cómo las pruebas de los scripts protegen
el cambio.

**Prerrequisitos:** 09 (metasprites y sprites dinámicos), 11 (animación por
estados) y 13 (memoria y bancos), y por ellas todo lo anterior: sobre todo la
03 (de `.c` a `.sfc`: compilador, `Makefile`, regla, dependencia, sección).
**Práctica:** sobre el juego real (en un clon). **Tiempo orientativo:** dos
tardes.

La práctica es la que pide el plan: **añadir una animación nueva a Tarou de
principio a fin**. La que se añade es real y arregla algo que hoy se ve mal:
con la espada fuera y quieto, Tarou usa la animación de reposo, en la que la
espada no aparece. Pixellab exportó una pose de combate (`Fight_Pose`, con la
espada en la boca) que el juego no usa. Al terminar, Tarou quieto con la
espada fuera se dibuja **en guardia**.

## La tubería en una imagen

```
res/descargas/graficos/pixellab/tarou/   (exportación nativa, NO versionada)
   │  python scripts/preparar_personaje.py tarou            ◄── pruebas: scripts/pruebas/
   ▼
res/personajes/tarou/tarou_<anim>.png   (14 tiras indexadas, versionadas)
res/personajes/tarou/tarou_anim.h       (#define TAROU_<ANIM>_CUADROS, versionado)
   │  make: regla de patrón  res/personajes/tarou/%.pic: %.png  →  gfx4snes -s 32 -T ...
   ▼
tarou_<anim>.pic  .pal  _meta.inc  .inc     (NO versionados: los rehace make)
   │                │                 │
   │ .incbin        │                 │ #include
   ▼                ▼                 ▼
data.asm (.rodata_tarou_<anim>)    src/juego/tarou_sprite.c (tarou_<anim>_metasprites)
   └──────────────► wla-65816 / tcc816 ──► wlalink ──► ProyectoSNES.sfc
```

Dos tipos de archivo generado, y la diferencia importa:

- **Salidas de los scripts de Python** (`.png` indexados, `tarou_anim.h`,
  `colision.bin`, `nivel1_datos.h`, `fondo_cerca.png`): **se versionan**. Se
  rehacen a mano, cuando cambia el arte, y `make` no sabe ejecutarlas.
- **Salidas de `gfx4snes`** (`.pic`, `.pal`, `.map`, `.inc`, `_meta.inc`,
  `.col`): **no se versionan** (`.gitignore`); `make` las rehace cuando cambia
  su `.png`, y `make clean` las borra (`cleanGfxRes`).

Tabla completa en `docs/tuberia-de-compilacion.md` §2, §3 y §8.

## Conceptos

### PNG indexado y paleta

Una imagen **indexada** (modo `P` en Pillow) no guarda colores en cada
píxel: guarda un **índice** (0..255) a una tabla de colores, la **paleta**,
que va dentro del archivo. Es justo lo que entiende la SNES (unidad 06): los
tiles guardan índices de 4 bits (0..15) y la CGRAM los colores. `gfx4snes`
exige PNG indexados (o BMP) y **respeta los índices**: el índice 0 del PNG será
el color 0 de la paleta, el **transparente** de un sprite. Por eso
`preparar_personaje.py` (`indexar`, línea 232) pone el magenta
`TRANSPARENTE` en el índice 0 y los 15 colores del perro en 1..15: el
magenta nunca se ve.

Toda la tira de un personaje y **todas sus animaciones** comparten **una**
paleta (`paleta_comun`, línea 215): el juego carga 16 colores en la paleta de
sprites 0 una vez (`tarou_sprite_cargar`: `dmaCopyCGram(&tarou_pal, ...)`) y
cambia de animación sin tocar la CGRAM. Por eso `data.asm` solo incrusta la
paleta de reposo (`tarou_pal`, en `.rodata_tarou_reposo`).

### Cuantización: median cut sin tramado

Si el arte trae más de 15 colores, hay que **cuantizar**: elegir 15 que
representen a todos y cambiar cada píxel por el más parecido. **Median cut**
es el método que usa el script (`Image.Quantize.MEDIANCUT`, línea 226): mete
todos los colores en una caja RGB, la parte por la mediana del canal más
estirado y repite hasta tener 15 cajas; cada caja da un color. **Sin
tramado** (`dither=Image.Dither.NONE`): el **tramado** (*dithering*) imita un
color que falta mezclando píxeles de dos que sí están, en damero; en pixel
art eso ensucia los bordes y además rompe la deduplicación de tiles (dos
tiles que eran iguales dejan de serlo). Con la exportación de Pixellab el
perro ya trae 15 colores o menos y el script los usa tal cual (la rama
«exactos si ya son ≤ 15»).

### `gfx4snes` y sus opciones

`gfx4snes` (del SDK, versión 2.2.0) convierte un PNG indexado en tiles
(`.pic`), paleta (`.pal`) y, según las opciones, mapa (`.map`) o tablas de
metasprite (`_meta.inc`). Las que usa el `Makefile` del juego (lista completa:
`tools/pvsneslib/devkitsnes/tools/gfx4snes -h`):

| Opción | Qué hace | Dónde la usa el juego |
|---|---|---|
| `-s N` | tamaño del bloque en píxeles: 8, 16, 32, 64 | 8 en fondos y fuentes; 32 en las tiras de Tarou; 64 en Bárbara y el lobo |
| `-o N` | cuántos colores escribe en el `.pal` | 2 en las fuentes, 16 en casi todo, 256 en las escenas de Mode 3 |
| `-u N` | colores por tile: 4 (2bpp), 16 (4bpp), 256 (8bpp) | `-u 4` solo en la fuente de BG3 |
| `-p` | escribe la paleta (`.pal`) | todas |
| `-m` | escribe el mapa (`.map`), una palabra por celda de 8×8 | fondos |
| `-e N` | número de paleta que lleva cada entrada del mapa | `-e 1` capa cercana, `-e 2` lejana |
| `-y` | mapa en **páginas de 32×32** (formato `SC_64x32`); exige alto múltiplo de 256 | capas de 512 px del título y la lejana del nivel |
| `-T` | escribe las tablas `t_metasprite` en `<nombre>_meta.inc` | tiras de Tarou |
| `-X N -Y N` | ancho y alto del metasprite (96 y 64: 3×2 piezas de 32) | tiras de Tarou |
| `-P N` | prioridad de las piezas (3: delante de todos los fondos) | tiras de Tarou |
| `-n` | tabla de HDMA con un color por línea | el cielo del título |
| `-t bmp` | la entrada es BMP | las fuentes |

**Sin `-y`, el mapa sale por filas:** la entrada de la celda `(x, y)` está en
la palabra `y·W + x`, con `W` el ancho en tiles. Con `-y`, la imagen se parte
en páginas de 32×32 celdas, una detrás de otra, que es como la PPU espera un
mapa de 64×32 (unidad 07). La capa cercana del tutorial (2080 px = 260
tiles de ancho) **no** usa `-y`: no se carga entera, se copia por columnas.

### Un paso de Python dentro del `Makefile`: `mapa_a_columnas.py`

El DMA copia **bytes seguidos**. Para traer una columna del nivel al
mapa-anillo de la VRAM (unidad 07) hacen falta sus 32 palabras, que en el
mapa por filas están separadas 260 palabras entre sí. La solución es una
regla del `Makefile` (línea 99) que ejecuta un script tras `gfx4snes`:

```make
res/nivel1/fondo_cerca.col: res/nivel1/fondo_cerca.pic
	python scripts/mapa_a_columnas.py res/nivel1/fondo_cerca.map $@ --ancho 260
```

`a_columnas` (línea 33 del script) reescribe el mismo mapa columna a columna:
la columna `x` ocupa las palabras `x·32 .. x·32 + 31`. Mismo tamaño (16 640
bytes = 260 × 32 × 2, medido) y otro orden. `data.asm` incrusta el `.col`
(`nivel1_cerca_col`) y `fondo_nivel.c` copia 64 bytes seguidos por columna.
Dos detalles de `make`: el objetivo depende del `.pic` (no del `.map`, que
sale en la misma orden de `gfx4snes`), y `$@` es el nombre del objetivo.

### Cómo se entera `make`: reglas, dependencias y lo que no ve

Una **regla** (unidad 03) dice «este objetivo se hace con esta orden a partir
de estas dependencias»; `make` la ejecuta si el objetivo no existe o alguna
dependencia es más nueva. Tres herramientas del `Makefile` del juego:

- **Regla de patrón** (línea 134): `res/personajes/tarou/%.pic:
  res/personajes/tarou/%.png`. El `%` es un comodín: una sola regla vale para
  las 14 tiras; `$<` es la dependencia (el `.png`). Con `TAROU_ANIMS` (línea
  129) y `$(foreach a,$(TAROU_ANIMS),...)` se arma la lista `TAROU_PICS`.
  Añadir una animación al `Makefile` es añadir **una palabra**.
- **`data.obj: $(ASSETS)`** (línea 142). `data.asm` hace `.incbin` de los
  `.pic`, y `make` no lee `.incbin`: sin esta línea, al regenerar un gráfico
  el `data.obj` viejo seguía en la ROM con el arte antiguo.
- **Los `.obj` que incluyen archivos generados** (líneas 149-152):
  `tarou_sprite.obj: $(TAROU_PICS)` (por los `_meta.inc`),
  `animaciones.obj: tarou_anim.h`, `pantalla_nivel1.obj: nivel1_datos.h`.

Y la trampa que esas líneas **no** arreglan: **`snes_rules` no sigue las
dependencias de los `.h` del código.** Si cambias `src/sistema/pantallas.h`,
los `.c` que lo incluyen no se recompilan y sus `.obj` conservan los valores
viejos, sin aviso (le pasó al proyecto con las duraciones de los fundidos).
Regla: **`make clean` después de tocar cualquier `.h`**, o de regenerar con
un script algo que un `.c` incluya.

### `CFLAGS` desde el entorno

`snes_rules` y el `Makefile` hacen `CFLAGS += ...`: **añaden** a lo que ya
traiga la variable. Si el entorno trae `CFLAGS=-DGUION_PRUEBA`, todos los
`.c` se compilan con `GUION_PRUEBA` definido, y `#ifdef GUION_PRUEBA` en
`src/juego/mando.c` cambia el mando por el guion del recorrido: la ROM se
juega sola. Desde PowerShell:

```
make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null
```

El `make clean` es obligatorio por la trampa anterior: cambiar una
definición no recompila los `.obj` que ya existen.

### Pruebas `unittest` de los scripts y TDD

`unittest` es el marco de pruebas de la biblioteca estándar de Python: una
clase que hereda de `unittest.TestCase`, un método `test_...` por prueba y
`self.assertEqual(a, b)` para comprobar. Las de este proyecto viven en
`scripts/pruebas/` (35 hoy: 11 de `preparar_personaje`, 11 de
`preparar_nivel` con 2 de `mapa_a_columnas`, 9 de `preparar_suelo` y 4 de
`preparar_fondo_nivel`). Cada una fabrica una entrada falsa en una carpeta
temporal (`tempfile.TemporaryDirectory`), llama al script y mira el
resultado. Por ejemplo, `exportar_pixellab` y `rotacion_pixellab` imitan la
exportación de Pixellab con rectángulos de colores.

**TDD** (*test-driven development*): primero se escribe (o se cambia) la
prueba que describe lo que se quiere, se ve **fallar** («en rojo»), y solo
entonces se cambia el código hasta que pase («en verde»). Ver el rojo
demuestra que la prueba mide de verdad el cambio: una prueba que nunca ha
fallado puede estar comprobando otra cosa. En la práctica, cambiar la prueba
antes que el script dio **4 fallos de 12** (`capturas/pruebas_en_rojo.txt`) y
el cambio del script, **36 de 36 en verde**.

```
python -m unittest scripts/pruebas/prueba_preparar_personaje.py scripts/pruebas/prueba_preparar_fondo_nivel.py scripts/pruebas/prueba_preparar_suelo.py scripts/pruebas/prueba_preparar_nivel.py
```

## Los otros scripts de la tubería

| Script | Entrada → salida | Qué decide | Pruebas |
|---|---|---|---|
| `preparar_personaje.py` | exportación de Pixellab → 14 tiras de 96×64 + `tarou_anim.h` | `MAPEOS_PIXELLAB` (de qué estado, dirección y tramo sale cada animación), `anclar` (centro del lienzo y pies en la fila 63, **sin escalar**), máximo 10 cuadros (10 × 3 KB < 32 KB, unidad 13), marcadores numerados si falta arte | `prueba_preparar_personaje.py` |
| `preparar_nivel.py` | `tutorial.tmj` (Tiled) + tileset de Pixellab → `fondo_cerca.png` (2080×256), `colision.bin` (128×14), `nivel1_datos.h` | vestir la rejilla doble, el parche del tronco, `LIMITE_TILES` | `prueba_preparar_nivel.py` (y `PruebaColumnas` para `mapa_a_columnas.py`) |
| `preparar_fondo_nivel.py` | `res/titulo/fondo_lejos.png` → `res/nivel1/fondo_lejos.png` (512 px) | borrar el logotipo y duplicar en espejo: bucle sin costura | `prueba_preparar_fondo_nivel.py` |
| `preparar_suelo.py` | el suelo plano de 512 px de la Revisión 1 | **sustituido** por `preparar_nivel.py`, que aún usa su `reducir` (median cut) | `prueba_preparar_suelo.py` |
| `mapa_a_columnas.py` | `.map` por filas → `.col` por columnas | lo llama el `Makefile`, no una persona | en `prueba_preparar_nivel.py` |

Los scripts se ejecutan **desde la raíz del repositorio** (las rutas por
defecto son relativas a ella) y necesitan Python 3 con Pillow (en este equipo,
Python 3.14 y Pillow 12.3).

## Ejercicio

`ejercicio_18.md`: la pose de guardia de principio a fin, en nueve pasos y en
un clon del repositorio. La solución está en `solucion/` (`guardia.patch` y
los archivos completos en `solucion/archivos/`, con la misma ruta que en el
repositorio) y las respuestas a las preguntas en `solucion/respuestas.md`.

## Criterio de aprobación

1. Las pruebas de los scripts pasan de **4 fallos** (después de cambiar la
   prueba, antes del script) a **36 en verde** (las 35 de siempre más la
   nueva). Referencias: `capturas/pruebas_en_rojo.txt` y
   `capturas/pruebas_en_verde.txt`.
2. `make clean` + `make` en el clon: **`No warnings or errors found.`**, y
   `tarou_guardia_til` aparece en `ProyectoSNES.sym` (en la práctica de
   referencia, en `$85:F000`). Las pruebas de PC siguen en **367 pruebas, 0
   fallos** (`capturas/compilacion_y_pruebas_pc.txt`).
3. En Mesen, en el nivel: con Tarou quieto, R (tecla W) desenvaina y, al
   terminar el gesto, Tarou queda **en guardia, con la espada en la boca**;
   al caminar vuelve la animación de caminar; R otra vez envaina y vuelve el
   reposo de siempre.

## Dónde está en el juego

| Qué | Dónde |
|---|---|
| Reglas de `gfx4snes`, `TAROU_ANIMS`, la regla de patrón, `ASSETS`, `data.obj: $(ASSETS)`, las dependencias de los `.obj`, `cleanGfxRes` | `Makefile` (líneas 53-160) |
| `CFLAGS += -Isrc` y `-Ires/personajes/tarou -Ires/nivel1` **después** del `include` | `Makefile:41-43` |
| La regla del `.col` | `Makefile:99-101` |
| Una sección por animación y `tarou_til_bancos` | `data.asm:172-253` |
| Las tablas `switch` por estado (`tarou_sprite_tabla`, `tarou_sprite_tiles`) | `src/juego/tarou_sprite.c:103` y `:124` |
| `MAPEOS_PIXELLAB`, `ANIMACIONES`, `paleta_comun`, `indexar` | `scripts/preparar_personaje.py:69`, `:82`, `:215`, `:232` |
| La tubería explicada | `docs/tuberia-de-compilacion.md` §2 (gráficos), §3 (el `Makefile`), §4 (`snes_rules`), §8 (scripts) |

## Trampas

- **Editar a mano lo generado.** `tarou_anim.h` y las tiras `.png` los
  escribe el script: lo que cambies a mano se pierde la próxima vez. Se cambia
  el script (o su tabla) y se vuelve a ejecutar.
- **`tests/tarou_anim.h` es una copia fija** (`PLAN.md` §6.6): las pruebas de
  PC no ven el `tarou_anim.h` real. En esta práctica no hace falta tocarla
  porque ningún código probado en el PC usa `TAROU_GUARDIA_CUADROS`.
- **La exportación de Pixellab no está en git.** En un clon no existe
  `res/descargas/graficos/pixellab/tarou/`: se pasa `--entrada` apuntando a la
  del repositorio principal.
- **`.h` cambiado, `.obj` viejo.** `make clean` antes de `make` siempre que
  cambie una cabecera o una definición (también `CFLAGS`).
- **Una sección no cruza un banco** (unidad 13): 10 cuadros como máximo por
  tira. Una tira de 1 cuadro ocupa 4 KB en la ROM, no 3: `gfx4snes` empaqueta
  los bloques de 32×32 en filas de 128 px y rellena la fila (4 bloques
  arriba + 4 abajo = 128 tiles × 32 bytes = 4096). La subida a la VRAM sí son
  3 KB (las 6 piezas).
- **La paleta común se recalcula entera** con cada ejecución: un color nuevo
  en una animación puede cambiar el índice de colores de todas. En esta
  práctica no pasó (las otras 13 tiras salieron idénticas byte a byte: `git
  status` solo muestra `tarou_anim.h` y la tira nueva), pero hay que mirarlo.

## Lo que viene

- **Unidad 19:** del arte al juego: qué pide Pixellab, por qué celdas de
  96×64, cómo se eligen las paletas y la perspectiva de los pies.
- **Unidad 20:** la colisión, con la rejilla que fabrica `preparar_nivel.py`.
