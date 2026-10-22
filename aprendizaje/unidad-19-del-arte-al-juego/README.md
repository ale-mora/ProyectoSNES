# Unidad 19 — Del arte al juego: Pixellab, celdas, paletas y perspectiva

**Objetivo:** llevar arte hecho fuera (Pixellab) a la SNES sin estropearlo, y
saber qué decisiones técnicas impone el hardware: tamaño de celda, piezas,
bytes por cuadro, una sola paleta de 15 colores, tiles de 8×8 alineados, el
presupuesto de tiles y la perspectiva del dibujo.

**Prerrequisitos:** 06 (tiles, paletas, tilemaps), 09 (metasprites), 18
(herramientas y TDD con `unittest`); y con ellas 00–05, 08, 11 y 13.
**Práctica:** juego real + PC (pruebas `unittest` y dos programas de Python).
**Tiempo orientativo:** tres tardes (una de teoría y demo, una por las partes A
y B del ejercicio, una para la C).

Esta unidad no dibuja nada: dibujar es del artista (aquí, Pixellab). Enseña la
**integración técnica**: qué hace cada script con el arte y por qué.

## Conceptos

### Exportación nativa de Pixellab

Pixellab es la herramienta con la que el autor genera el arte. Su exportación
de personajes (*Characters → Export*) es un `.zip` con **una carpeta por
estado** del personaje y, dentro, **8 direcciones**:

```
Idle/animations/<nombre largo>/east/frame_000.png ... frame_008.png
Idle/rotations/east.png                                (pose fija)
metadata.json                                          (el índice de todo)
```

- Un **estado** de Pixellab (`Idle`, `Walk`, `Run`, `Jump`, `Sneak`,
  `Unsheathe`, `Stand_up`, `Sleep`, `Attack_1`...) es una animación tal como la
  generó la herramienta; no coincide uno a uno con las 13 animaciones del
  juego.
- Una **pose fija** (`rotations/<dirección>.png`) es una sola imagen.
- El **lienzo** (el tamaño de cada PNG) cambia según el estado. Medido en la
  exportación de Tarou: 64×64 las poses, 76×76 `Sleep`, 88×88 `Idle`, `Walk` y
  `Jump`, 96×96 `Run` y `Sneak`, 96×84 `Unsheathe` y 104×84 `Stand_up`.
- `metadata.json` (dentro de `Tarou.zip`) dice, por estado, el tamaño base del
  personaje (64×64), la vista (`side`), las direcciones y la lista de cuadros.
  El script no lo necesita: recorre las carpetas.

La exportación vive en `res/descargas/graficos/pixellab/tarou/`, que **git
ignora** (son descargas; solo se versiona su `README.md`). Lo versionado es lo
que sale de los scripts.

### Mapeo de estados a animaciones

`MAPEOS_PIXELLAB` (`scripts/preparar_personaje.py:82`) dice de dónde sale cada
animación del juego. Cada regla es `(estado, dirección, desde, hasta, al_revés)`:

| Animación | Regla | Técnica |
|---|---|---|
| reposo, caminar, correr | `Idle` / `Walk` / `Run`, enteras | — |
| saltar / caer | `Jump` 2..4 / `Jump` 5..7 | **tramo**: un salto hacia delante partido en dos |
| agachado | `Sneak` 0..0 | un cuadro quieto de una animación |
| envainar | `Unsheathe` al revés | **inversión**: el mismo gesto hacia atrás |
| echarse | `Stand_up` 0..5 al revés | levantarse al revés = tumbarse |
| despertar | `Stand_up` 0..9 | 11 cuadros no caben (límite de 10) |
| atacar | `Attack_1`, pose fija (`desde = None`) | **provisional**: no hay animación de ataque |
| dormido | `Sleep` mirando al **sur** | Pixellab solo lo generó así |

Todo mira al **este**: la izquierda la hace el **espejo** del hardware (bit de
volteo horizontal de cada pieza, unidad 09), así que no se gastan cuadros.

### Anclar: por el centro del lienzo y por los pies

Como los lienzos miden distinto, pegar cada cuadro por su **esquina** haría
saltar al perro al cambiar de animación (de un lienzo de 88 a uno de 104 el
cuerpo se movería 8 px). `anclar` (`preparar_personaje.py:137`) lo pega:

- en x, el **centro del lienzo** en el centro de la celda: `x = (96 − ancho) // 2`;
- en y, la **fila más baja con píxeles de toda la animación** (los pies) en la
  última fila de la celda: `y = 64 − pies`.

Lo que cae fuera de la celda se recorta (el script avisa si se recorta algo
opaco). La `demo_19_arte.py` imprime la esquina de cada lienzo: `Stand_up` (104
px) empieza en x = −4, `Sleep` (76) en x = 10, la pose de 64 en x = 16.

### Por qué no se escala el pixel art

En el pixel art cada píxel es una decisión del artista. Escalar con **vecino
más cercano** (cada píxel de salida copia el de entrada más próximo) con un
factor **no entero** repite unas columnas y se come otras. La demo lo cuenta:
de 81 columnas a 64 (factor 0,79, lo que hacía falta para meter el perro de 81
px en una celda de 64) se **pierden 17 columnas**; con factor 1,5 las columnas
salen de 2, 1, 2, 1 px: píxeles desiguales. Solo un factor entero (2, 3)
conserva el dibujo. Por eso la decisión del 2026-10-19 fue **agrandar la celda**
y no achicar el perro (`docs/diseno-demo-jugable.md` §2, decisión 3).

### La celda de 96×64 y lo que cuesta

La **celda** es el rectángulo fijo donde se dibuja cada cuadro del
protagonista (unidad 09): 96×64 = 3×2 **piezas** de 32×32. Sus costes:

| Medida | Valor | De dónde sale |
|---|---|---|
| Piezas por cuadro | hasta **6** | `gfx4snes -T` omite las piezas transparentes; la demo mide 6 en todas las tiras de hoy |
| Bytes por cuadro | **3 KB** | 6 piezas × 32×32 px × 4 bpp = 6 × 512 B |
| Cuadros por animación | **10** como mucho | cada tira va en su sección de `data.asm` y una sección no cruza un banco de 32 KB: 10 × 3 KB = 30 KB |
| Carga estática (intro, título) | **2 cuadros** | 6 KB en VRAM junto a Bárbara: `tarou_reposo_estatico.png` |
| VBlank | 3 KB = 18 líneas de DMA de 37 | por eso el doble búfer y medio cuadro por VBlank (unidad 09) |

La `demo_19_arte.py` lista cada tira: `despertar` tiene 10 cuadros y llena el
93 % de su banco.

### Una paleta de 15 colores, compartida

Un sprite de 16 colores usa una paleta de la CGRAM: **índice 0 =
transparente** y 15 colores. Todo Tarou usa **una sola** paleta (la de sprites
0, CGRAM 128..143), así que `paleta_comun` (`preparar_personaje.py:215`) junta
los píxeles de **todos los cuadros de todas las animaciones** y, si hay más de
15 colores, los reduce con **median cut** (cuantización: parte el conjunto de
colores en 15 cajas y toma un representante de cada una), sin **tramado**
(*dither*: mezclar píxeles de dos colores para simular un tercero, que en pixel
art es ruido). El arte de Pixellab trae unos 100 colores y a 15 se ve bien.
`indexar` (`:232`) cambia después cada píxel por el índice del color más
cercano.

Consecuencia que hay que recordar: **añadir una animación recalcula la paleta
de las 13**, y todas las tiras cambian de índices.

### El suelo: tileset *Sidescroller* de rejilla doble

El suelo del nivel es una hoja de Pixellab (*Create Tileset → Sidescroller*,
celdas de 16×16, 384×128 px en la exportación de hoy). Es un tileset de
**rejilla doble** (*dual grid*, → glosario: Rejilla doble): cada celda de
**arte** cubre las **esquinas** de cuatro celdas de **lógica** y se elige según
cuáles de las cuatro tienen tierra. Cuatro esquinas, sí o no: **16
combinaciones**, una clave de 4 bits:

```
 TL=1 │ TR=2          lógica:  . .      clave = 0          (aire, no se dibuja)
 ─────┼─────                   # #      clave = 4 + 8 = 12 (tierra abajo: borde con hierba)
 BL=4 │ BR=8                   # #      clave = 15         (tierra entera)
```

`piezas_por_esquinas` (`scripts/preparar_nivel.py:226`) clasifica cada celda
de la hoja mirando el alfa en un punto de cada cuarto. La hoja de hoy trae
todas menos las dos **diagonales** (6 y 9: tierra en dos esquinas opuestas), que
se sustituyen por 14 y 13; si faltara otra, el script para con un error.

**Vestir** la lógica (`vestir`, `:254`): por cada **vértice** de la rejilla de
lógica se calcula la clave de sus cuatro celdas vecinas y se pega la pieza en
`(c·16 − 8, f·16 − 8)`: el arte va **desplazado media celda**. Como 8 es
múltiplo de 8, todo cae en la rejilla de tiles de la SNES y los tiles se
repiten. La tierra entera (15) se varía con **volteos** al azar (semilla fija:
el resultado es siempre el mismo); en el tilemap un volteo es un bit, **gratis**
(unidad 06). La demo imprime las claves de una rejilla pequeña.

La versión anterior, `scripts/preparar_suelo.py` (`clasificar`, `:85`), solo
sabía hacer un suelo plano: separaba la hoja en «borde» (hierba sobre tierra) y
«tierra». Se conserva, sustituida.

### El presupuesto de tiles

La capa cercana tiene sus tiles en `VRAM_FONDO_A`, donde caben **512** de 4
bpp. `tiles_distintos` (`preparar_nivel.py:365`) cuenta los tiles de 8×8
distintos de la imagen final **contando como uno un tile y sus volteos**, como
hará `gfx4snes`; si pasa de `LIMITE_TILES` = **460** (512 con margen), el
script **se niega a escribir** nada. Hoy: **260** tiles en una imagen de
2080×256 px. Lo que más ahorra: arte alineado a 8 px, piezas que se repiten y
volteos. Lo que más gasta: variedad sin rejilla (carteles, arte libre).

### Componer capas con una paleta compartida

La capa cercana es **una** paleta de Mode 1 (16 colores, `-e 1` en el
`Makefile`) para todo: tierra, hierba, seto, zarzas, carteles y tronco.
`preparar` reparte así los 15 colores: el **seto** (los arbustos del título,
`poner_seto`, `:286`) conserva los suyos; la **tierra** se reduce (median cut,
`ps.reducir`) a lo que queda. Los **carteles** (`poner_cartel`, `:335`) no
traen colores: usan los ya elegidos (el más oscuro para el marco, uno pardo de
fondo, el más claro para las letras) y sus letras son tiles de 8×8 de la fuente
de consola, alineados: la misma letra en dos carteles es el **mismo tile**. Las
**zarzas** y el **tronco** son hoy **marcadores** dibujados por el script con
esos mismos colores; la parte B del ejercicio integra una pieza de verdad.

### Perspectiva: los pies 4 px dentro de la hierba

El arte de Pixellab es una vista **lateral un poco elevada**: las patas del lado
lejano acaban más arriba que las cercanas. Con la hierba empezando justo bajo
los pies, Tarou parecía **flotar** sobre una raya (→ glosario: Hundir los pies).
El remedio es de dibujo, en dos mitades:

- **2 px del arte:** la banda de hierba de la pieza «tierra abajo» (clave 12)
  empieza en su fila 6; pegada en y 168 (176 − 8), empieza en **y 174**, 2 px
  por encima del borde del suelo base (`SUELO_Y` = 176).
- **2 px del sprite:** `HUNDIR_PIES` = 2 (`src/juego/tarou_sprite.c:93`): el
  sprite se dibuja 2 px por debajo de su posición física.

Total, **4 px**. La **física** usa el **borde exacto** de las celdas: los pies
físicos están en la última fila de la celda (y + 63 = 175, justo encima de 176)
y ni `tarou.c` ni las pruebas saben nada de esto. La parte A del ejercicio lo
comprueba cambiando `HUNDIR_PIES` a 0 y a 4.

## La librería: PVSnesLib

Esta unidad trabaja sobre todo **antes** de la ROM (Python), pero su resultado
lo consumen la herramienta y las funciones del SDK:

| Herramienta / función (firma exacta) | Qué hace / qué toca | En el juego |
|---|---|---|
| `gfx4snes` (`tools/pvsneslib/devkitsnes/tools/`) `-s 32 -o 16 -u 16 -p -T -X 96 -Y 64 -P 3` | convierte una tira en tiles de 4 bpp (`.pic`), paleta (`.pal`) y tabla de metasprite (`_meta.inc`, `-T`) con celdas de 96×64 (`-X -Y`) | regla patrón de Tarou, `Makefile:136` |
| `gfx4snes -s 8 -o 16 -u 16 -e 1 -p -m` | la capa cercana: tiles de 8×8, paleta 1, mapa (`-m`), deduplicando tiles y volteos | `Makefile:96-97` |
| `void oamInitGfxAttr(u16 address, u8 oamsize);` ([sprite.h](https://alekmaul.github.io/pvsneslib/sprite_8h.html)) | dónde están los tiles de sprites y sus tamaños (`OBJSEL`) | `tarou_sprite_cargar` |
| `void oamSet(u16 id, u16 xspr, u16 yspr, u8 priority, u8 hflip, u8 vflip, u16 gfxoffset, u8 paletteoffset);` | una entrada de la OAM: posición, prioridad, **volteo** (el espejo), tile y paleta | `tarou_sprite_dibujar`, una por pieza, con `y + HUNDIR_PIES` |
| `void oamSetEx(u16 id, u8 size, u8 hide);` | tamaño grande (32×32) y visible u oculta | ídem |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | la paleta de Tarou a la CGRAM 128..143 | `tarou_sprite_cargar` |
| `void bgInitTileSet(u8 bgNumber, u8 *tileSource, u8 *tilePalette, u8 paletteEntry, u16 tileSize, u16 paletteSize, u16 colorMode, u16 address);` ([background.h](https://alekmaul.github.io/pvsneslib/background_8h.html)) | tiles y paleta de la capa cercana | `fondo_nivel_cargar` |

Ejemplo del SDK equivalente: `tools/pvsneslib/snes-examples/graphics/Sprites/DynamicEngineMetaSprite`
(sus `Makefile` y PNG muestran `gfx4snes -T` sobre una tira).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| El mapeo de estados | `MAPEOS_PIXELLAB`, `scripts/preparar_personaje.py:82` |
| Anclar por centro y pies | `anclar`, `:137` |
| Leer la exportación nativa (tramos, inversión, pose fija) | `cargar_pixellab`, `:157` |
| Paleta común y color más cercano | `paleta_comun` `:215`, `indexar` `:232` |
| Las tiras resultantes y su cabecera | `res/personajes/tarou/tarou_*.png`, `tarou_anim.h` |
| Rejilla doble: clasificar la hoja, vestir | `piezas_por_esquinas` `scripts/preparar_nivel.py:226`, `vestir` `:254` |
| Seto, zarzas (marcador), carteles | `poner_seto` `:286`, `poner_zarzas` `:300`, `poner_cartel` `:335` |
| Presupuesto de tiles | `tiles_distintos` `:365`, `LIMITE_TILES` `:89` |
| La versión de suelo plano | `clasificar`, `scripts/preparar_suelo.py:85` |
| Hundir los pies | `HUNDIR_PIES`, `src/juego/tarou_sprite.c:93` y `:205` |
| Qué exportar de Pixellab y qué piezas faltan | `res/descargas/graficos/pixellab/tarou/README.md`, `.../suelo/README.md` |
| Las decisiones | `docs/diseno-demo-jugable.md` §2 (decisiones 3 y 16), §7, §10; `docs/diseno-tutorial-movimiento.md` §3 y §7 |
| Las pruebas | `scripts/pruebas/prueba_preparar_personaje.py`, `prueba_preparar_nivel.py` (35 en total con las otras dos suites) |

## Trampas

| Trampa | Qué pasa | Cómo se evita |
|---|---|---|
| Escalar con factor no entero | píxeles desiguales, detalles perdidos (17 de 81 columnas a 0,79) | celda más grande; nunca escalar |
| Anclar por la esquina | el perro salta entre animaciones (lienzos de 64 a 104) | centro del lienzo y pies |
| Más de 10 cuadros | la sección no cabe en un banco de 32 KB | el script se niega; tramos (`despertar` 0..9) |
| Una animación nueva | cambia la paleta de TODAS | regenerar y revisar las 13; `make clean` (cambia `tarou_anim.h`) |
| `tests/tarou_anim.h` | es una copia fija (cuadros de los marcadores), no la real | las cuentas de las pruebas se hacen con ella (unidad 02) |
| Una pieza nueva en la carpeta del suelo | `main()` toma el PNG más reciente como hoja | parte B: `hoja_de_suelo` ignora `zarza.png`... |
| Colores de una pieza fuera de la paleta | `indexar` los cambia por el más cercano sin avisar | contarlos (`colores_ajenos`) y mirarlo en Mesen |
| Más de 460 tiles | el script no escribe nada | menos variedad o arte alineado a 8 |
| Los pies dentro de la hierba | parece un fallo y no lo es | es dibujo: 2 px del arte + 2 de `HUNDIR_PIES`; la física, intacta |
| `res/descargas/` en un clon | no existe (git la ignora) | copiarla del repo, solo para leer |

## La demo

`demo_19_arte.py` hace las cuentas de esta unidad sobre los archivos reales
(las tiras versionadas de `res/personajes/tarou/`). Desde cualquier carpeta:

```
python aprendizaje\unidad-19-del-arte-al-juego\demo_19_arte.py
```

Cuatro pasos (salida completa en `capturas/salida_demo_19_arte.txt`):

1. escalar 81 → 64, 16 → 24 y 16 → 32 px: cuántas veces sale cada columna;
2. dónde cae la esquina de cada lienzo de Pixellab en la celda;
3. las 13 tiras: cuadros, piezas (6), bytes, % del banco, ancho real del perro
   (hasta 81 px agachado caminando) y la fila de los pies (63 en todas);
4. las claves de una rejilla doble pequeña.

## Ejercicio y criterio de aprobación

**Parte de PC: `ejercicio_19_cuentas.py`.** Tres `TODO` numerados: la clave de
4 bits de un vértice, la esquina de un lienzo anclado y la cuenta de tiles
distintos con volteos. Tal como está se ejecuta e imprime ceros; la salida
esperada exacta está en su cabecera y en `capturas/salida_ejercicio_19_cuentas.txt`.

**Parte sobre el juego: `ejercicio_19.md`**, tres partes:

- **A** — `HUNDIR_PIES` a 0, 2 y 4: tres ROM de prueba y sus capturas junto al
  tronco.
- **B** — integrar una pieza que falta (la zarza) en lugar de su marcador, con
  TDD, vigilando colores y tiles.
- **C** — preparar el mapeo para la animación de ataque que aún no existe: una
  lista de reglas por orden de preferencia, con TDD.

**Aprobación:**

1. `ejercicio_19_cuentas.py` da exactamente la salida esperada;
2. las pruebas de los scripts en verde (**39** con las 4 nuevas de B y C; antes
   de escribir el código, 3 de ellas en rojo) y `tests/pruebas.exe` en verde
   (**367**);
3. las capturas de Mesen de las partes A (tres alturas de pies) y B (la zarza
   integrada); la de C, cuando Pixellab dé el ataque.

La solución está en `solucion/` y las salidas en `capturas/` (ver el final de
`ejercicio_19.md`).

## Lo que viene

- **Unidad 20:** la colisión. Lo que aquí es dibujo (la hierba, los pies) allí
  es lógica: la misma rejilla de 16 px que el script viste es la que la física
  consulta.
