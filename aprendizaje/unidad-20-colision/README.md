# Unidad 20 — Colisión con el escenario

> **Lección válida, juego histórico.** Esta unidad enseña sobre el código de
> la Revisión 2 (plataformas, tag `plataformas-r2`); el juego actual es un
> beat 'em up ([`docs/diseno-beat-em-up.md`](../../docs/diseno-beat-em-up.md)).
> `colision.c`, `partida.c`, `guion_tutorial.c`, `res/nivel1/tutorial.tmj` y
> sus pruebas se retiran del juego y quedan en ese tag. La demo y el ejercicio
> se hacen en un clon fijado en él:
>
> ```text
> git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio
> git -C E:\ProyectoSNES_trabajo\mio checkout -q plataformas-r2
> ```
>
> La demo y la traza se compilan desde la copia de esta carpeta **dentro del
> clon** (`E:\ProyectoSNES_trabajo\mio\aprendizaje\unidad-20-colision\`): así
> los `../../src`, `../../tests` y `../../res/nivel1` de sus comandos son el
> código del tag y no el de la rama principal.
>
> **Qué se traslada al beat 'em up:** la AABB (`se_tocan`) es la base de las
> cajas de golpe y de daño, con una regla más: las profundidades tienen que
> estar cerca (`|y_atacante − y_objetivo| ≤ TOLERANCIA_Y`, spec §3). El
> streaming de columnas al mapa-anillo (`fondo_nivel.c`) se queda tal cual, y
> Tiled sigue siendo el editor, ahora de zonas, carril y oleadas (spec §7). La
> rejilla de plataformas y la resolución por ejes no se trasladan.

**Objetivo:** entender cómo sabe Tarou dónde hay suelo, pared, techo, plataforma
o zarza en un nivel de 2048 px, cómo se dibuja ese nivel en una VRAM donde solo
caben 512 px, y cómo se cambia el nivel sin romperlo: Tiled → script → ROM →
pruebas → guion.

**Prerrequisitos:** 07 (cámara y mapa-anillo), 10 (física), 13 (memoria), 18
(herramientas); y con ellas, todo lo anterior. **Práctica:** PC + juego real +
Mesen (la ROM de prueba que se juega sola). **Tiempo orientativo:** tres o
cuatro tardes (una para la teoría y la demo, una para el ejercicio de PC, una o
dos para el segundo foso).

El código de esta unidad existe desde el 2026-10-20: es el **tutorial de
movimiento** del nivel 1 (`docs/diseno-tutorial-movimiento.md`). La unidad 10
enseñó a integrar la velocidad (`posición += velocidad`); aquí se ve qué pasa
cuando esa posición nueva cae dentro de algo.

## Conceptos

### Rejilla de colisión

El escenario, para la física, no son tiles ni colores: es una **rejilla de
celdas de 16×16 px** en la que cada celda guarda un número que dice qué ES
(→ glosario: Colisión por celdas):

| Valor | Constante (`src/juego/colision.h`) | Qué hace |
|---|---|---|
| 0 | `COL_VACIO` | aire: se atraviesa y no sostiene |
| 1 | `COL_SOLIDO` | tierra: no se atraviesa por ningún lado |
| 2 | `COL_PLATAFORMA` | sostiene al caer encima; de lado y desde abajo se atraviesa |
| 3 | `COL_ZARZA` | se atraviesa, pero obliga a reaparecer |

El nivel mide 2048×224 px: **128 columnas × 14 filas = 1792 bytes**, un byte
por celda, fila a fila. Es `res/nivel1/colision.bin`, y `data.asm` lo mete en
la ROM con la etiqueta `nivel1_colision` (sección `.rodata_nivel1_colision`).
La física lo lee directamente de la ROM con un puntero (unidad 13): no se copia
a la WRAM.

**Del píxel a la celda, sin multiplicar ni dividir.** Como 16 y 128 son
potencias de 2, todo son desplazamientos (unidad 01: `/` y `*` llaman a una
rutina lenta en el 65816):

```
columna = x >> 4               (x / 16)
fila    = y >> 4               (y / 16)
índice  = (fila << 7) | columna   (fila * 128 + columna)
```

`colision_en(u16 x, u16 y)` (`src/juego/colision.c:49`) hace eso y además
decide qué hay **fuera** del nivel: a los lados, pared (`x >= 2048`; y también la
«x negativa», que en `u16` da la vuelta a un número enorme: una sola
comparación cubre los dos bordes); por debajo (`y >= 224`), vacío, para que se
pueda caer por un foso; por encima, vacío también.

> **Por qué 128 columnas SIEMPRE.** El `<< 7` está escrito en el código. Una
> rejilla de prueba de 30 columnas no funciona: hay que declarar las 128 y usar
> solo las primeras (así lo hacen `tests/prueba_colision.c` y la demo).

### Caja (AABB)

Un personaje no es un punto: es una **caja**, un rectángulo alineado con los
ejes (→ glosario: AABB, *axis-aligned bounding box*). La de Tarou se mide sobre
el arte, dentro de su celda de 96×64 (unidad 10): de pie, columnas 24..75 y
filas 14..63 (52×50 px); agachado, filas 28..63 (36 px de alto).

Dos cajas se solapan si **cada una empieza antes de que acabe la otra**, en x y
en y. Es `se_tocan` en `src/juego/partida.c:28`:

```c
return (u8)(ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah);
```

Cuatro comparaciones: la prueba de colisión más barata que hay. El juego la usa
para la caja de golpe de la espada contra el tronco.

### Resolución por ejes

Si Tarou se mueve en diagonal y cae dentro de una esquina, ¿se le empuja hacia
arriba o hacia atrás? La respuesta clásica es **no mover nunca en diagonal**:
primero se mueve en x y se corrige contra las paredes; después se mueve en y y
se corrige contra suelos y techos (→ glosario: Resolución por ejes). Así una
esquina nunca «engancha» al personaje. En `tarou_actualizar`
(`src/juego/tarou.c:506`) son los pasos 3 y 4: `mover_x(t)` y luego
`mover_y(t, m)`.

**En x (`mover_x`, `tarou.c:411`).** Se integra `x_fp += vx_fp`, se recorta a
los bordes del nivel y, si hay velocidad, se mira la **columna del borde
delantero** del cuerpo (la derecha si va a la derecha, la izquierda si va a la
izquierda), desde la cabeza hasta los pies. Solo cuenta lo sólido: plataformas
y zarzas se atraviesan de lado. Si toca algo, Tarou **retrocede de píxel en
píxel** hasta dejar de tocarlo:

```c
while (vueltas < CELDA_COLISION && borde_choca((u16)(x + paso), arriba, pies))
{
    x = (s16)(t->vx_fp > 0 ? x - 1 : x + 1);   /* Un píxel hacia atrás. */
    vueltas++;
}
```

Como mucho avanza 2 px por frame, así que el bucle da una o dos vueltas. Antes
se le pegaba al comienzo de la celda de 16 px, y contra el tronco (que empieza
en x 1640, no múltiplo de 16) rebotaba 8 px en bucle: es la cuarta trampa de la
tabla de `docs/diseno-tutorial-movimiento.md` §8.

**En y (`mover_y`, `tarou.c:461`).** Tres casos:

1. **En el suelo:** ¿sigue habiendo suelo? Tarou tiene dos **pies**, dos
   columnas dentro del cuerpo (`PIE_ATRAS` = 36 y `PIE_DELANTE` = 64 de su
   celda, 28 px entre sí). Se sostiene mientras **cualquiera** de los dos pise
   algo; cuando los dos están en el aire, pasa a `CAYENDO`. Con los pies tan
   juntos, un foso de 96 px exige estar 68 px en el aire: caminando no llega,
   corriendo sí.
2. **Bajando:** se recorren los **bordes de celda** (múltiplos de 16) que los
   pies han cruzado este frame, empezando por `(pies_antes + 15) & ~15`. En el
   primero que sostiene (sólido o plataforma) bajo cualquiera de los pies,
   Tarou se queda con los pies justo encima y aterriza.
3. **Subiendo:** si la fila de la cabeza toca algo sólido, se le pega debajo
   del techo y `vy = 0` (el paso 2 de `tarou_actualizar` lo pasará a `CAYENDO`).

La física usa el **borde exacto** de las celdas: los pies acaban en la fila
`y + 64`, justo encima del borde. Que en pantalla se vean 4 px dentro de la
hierba es solo dibujo (2 px los pone el arte y 2 `HUNDIR_PIES` en
`tarou_sprite.c`; unidad 19): ni la física ni las pruebas lo saben.

### Plataforma de un solo sentido

Sale gratis del caso 2: una plataforma **solo** sostiene si los pies **cruzan su
borde de arriba bajando** (→ glosario: Plataforma de un solo sentido). Desde
abajo, subiendo, la cabeza solo para en lo sólido (caso 3: la plataforma no es
sólida); de lado, `mover_x` solo mira lo sólido. Y si los pies ya estaban por
debajo de su borde cuando empezó a caer, el primer borde que se mira es el
siguiente, así que la atraviesa. `tests/prueba_colision.c`
(`prueba_plataforma`) lo comprueba: salta desde debajo, la atraviesa y se
queda encima al bajar.

### `cabe_de_pie`: el túnel

Agachado bajo un techo, soltar Abajo **no** levanta a Tarou ni le deja saltar.
`cabe_de_pie` (`tarou.c:230`) mira si la parte del cuerpo de pie que el cuerpo
agachado no ocupa (de la fila 14 a la 27 de la celda) toca algo sólido. El
túnel del tutorial mide 48 px de alto: de pie (50) no cabe y agachado (36) sí.

### El coste de las llamadas en tcc816

En el PC una llamada a función no cuesta casi nada. En la SNES, con tcc816,
cada llamada prepara la pila, copia los argumentos y los recoge: unas **150
instrucciones** por cada `colision_en`. La primera versión de la física la
llamaba unas 17 veces por frame y el juego bajó a **27 FPS** (tabla de trampas
de `docs/diseno-tutorial-movimiento.md` §8).

El arreglo: **consultas de una columna o una fila entera**, con el índice
calculado una vez y avanzando a saltos (bajar una fila es sumar 128; pasar a la
celda de al lado es sumar 1):

| Función (`colision.c`) | Pregunta | La usa |
|---|---|---|
| `colision_columna_solida(x, y0, y1)` (`:79`) | ¿algo sólido (o el bloque) en la columna de x, de y0 a y1? | `mover_x` (paredes) |
| `colision_fila_es(x0, x1, y, tipo)` (`:117`) | ¿alguna celda de la fila de y, de x0 a x1, es de `tipo`? | `cabe_de_pie`, techos, zarzas en `partida.c` |
| `colision_apoyan(xa, xb, y)` (`:144`) | ¿sostiene la celda de alguno de los dos pies? | `mover_y` (suelo) |

Fíjate también en `(y & 0xFFF0) << 3`: es lo mismo que `(y >> 4) << 7` con
menos desplazamientos (en tcc816 cada desplazamiento de un bit es una
instrucción, unidad 14).

### El bloque: algo sólido que desaparece

El tronco que la espada rompe no está en la rejilla: la rejilla vive en la ROM
y **la ROM no se puede escribir**. Es un **bloque**, un rectángulo sólido
aparte (`colision_bloque(x, y, ancho, alto)`, `colision.c:41`) que
`colision_en` y `colision_columna_solida` miran antes que la rejilla. Romperlo
es `colision_bloque(0, 0, 0, 0)` (ancho 0 = no hay). Lo pone
`partida_iniciar` y lo quita `partida_actualizar` cuando la caja de golpe lo
toca.

### Punto de control y reglas de la partida

Un **punto de control** es el sitio donde se reaparece tras caer o pincharse
(→ glosario: Punto de control). En el tutorial, cada cartel. `partida_actualizar`
(`src/juego/partida.c:49`), una vez por frame y después de `tarou_actualizar`,
comprueba en este orden:

1. **¿Cayó o se pinchó?** (gana a todo): la celda en `y_fp >= PF(Y_CAIDA)`
   (192: los pies ya en 256, fuera de la pantalla), o la fila de las patas
   sobre una zarza (`colision_fila_es(..., COL_ZARZA)`). Devuelve
   `PARTIDA_REAPARECER`.
2. **¿La espada toca el tronco?** `tarou_caja_golpe` + `se_tocan`: devuelve
   `PARTIDA_TRONCO_ROTO` y quita el bloque.
3. **Puntos de control y salida**, por el centro de la celda (`x + 48`): como
   los carteles van de izquierda a derecha, basta mirar el **siguiente** (una
   comparación por frame). Si el centro llega a `NIVEL1_SALIDA_X`, devuelve
   `PARTIDA_SALIDA`.

> **Trampa: `PF_ENTERO` con negativos.** La comparación del paso 1 se hace en
> punto fijo y con signo a propósito. Saltando desde una plataforma, la celda
> sube por encima de la pantalla (`y < 0`), y `PF_ENTERO`, que es solo para
> valores ≥ 0, convierte −5 px en 65531: «¡cayó!» en lo alto de un salto. La
> demo lo imprime. Regla: **comparar en punto fijo** (`t->y_fp >= PF(192)`),
> nunca convertir un valor que puede ser negativo.

La pantalla (`src/pantallas/pantalla_nivel1.c:184`) hace lo que toca con cada
evento: `reaparecer()` (fundido, `setScreenOff`, Tarou al último cartel, las 64
columnas de la cámara nueva reescritas, fundido; la pantalla apagada es el
*forced blank* de la unidad 04), `fondo_nivel_quitar_parche()` o
`salir(PANTALLA_PROXIMAMENTE)` («LEVEL 2 / COMING SOON», que vuelve al título,
`src/pantallas/pantalla_proximamente.c`).

### Streaming de columnas (repaso de la unidad 07, ahora con el DMA)

La capa cercana mide 2048 px y el mapa de la VRAM (64×32 tiles) solo 512: se
usa como **anillo** (unidad 07): la columna `c` del nivel va a
`VRAM_MAPA_A + nivel_columna_vram(c)` = `((c & 32) << 5) | (c & 31)`
(→ glosario: Streaming de columnas). Lo nuevo aquí es **cómo** llega cada
columna:

1. En la ROM, el mapa está guardado **columna a columna**
   (`scripts/mapa_a_columnas.py` lo reordena en el `Makefile`): las 32
   entradas de una columna son 64 bytes seguidos. Son 260 columnas: las 256 del
   nivel más 4 de «parche» (el tronco, abajo).
2. En la VRAM, en cambio, una columna son 32 palabras separadas 32 palabras
   (una fila del mapa). El registro **VMAIN** (`$2115`, `REG_VMAIN` en
   `video.h`) dice cuánto avanza la dirección de la VRAM tras cada palabra:
   `0x80` = 1 (lo normal, para tiles), `0x81` = 32 (bajar una fila: una
   columna). Con `0x81` la columna entera es **un solo DMA de 64 bytes**.
3. `fondo_nivel_actualizar(camara)` (`src/juego/fondo_nivel.c:128`), en cada
   frame: si la cámara entró en una columna de 8 px nueva, apunta en la **lista
   de DMA** (unidad 09) la columna `col + 33` (la que va a entrar por la
   derecha) y `col − 1` (por la izquierda). Solo entonces: casi siempre no
   apunta nada. A 2 px por frame como mucho, nunca se salta una.
4. En el VBlank, `dma_lista_ejecutar` (`src/juego/dma_ejecutar.asm`, unidad 15)
   dispara un DMA por entrada, con su VMAIN, y deja VMAIN en `0x80`, que es lo
   que espera el resto del SDK.

**El parche del tronco.** El mapa vive en la ROM, así que el tronco no puede
«borrarse» del dibujo. `scripts/preparar_nivel.py` dibuja aparte las 4 columnas
de tiles del tronco (x 1640..1671) **con** el tronco y las pega a la derecha del
nivel (x 2048..2079: por eso la imagen mide 2080 px). Mientras el tronco está
entero, `apuntar_columna` (`fondo_nivel.c:75`) copia esas columnas en su sitio;
al romperlo, `fondo_nivel_quitar_parche` hace que se vuelvan a copiar las de
debajo, **solo las que están a la vista** (una lejana caería en el sitio del
anillo de otra que sí se ve y la estropearía).

### Rejilla doble, en una frase

El arte del suelo es un tileset de Pixellab de **rejilla doble** (*dual grid*):
cada celda de arte de 16×16 cubre las **esquinas** de cuatro celdas de lógica y
se elige según cuáles de ellas son sólidas (16 combinaciones), con el arte
desplazado media celda (→ glosario: Rejilla doble). Para esta unidad basta
saber que **en Tiled se pinta la lógica** (sólido, plataforma, zarza) y el
script le pone el arte; la unidad 19 lo explica a fondo.

### Guion de mando

Como la física solo ve una `struct Mando` (unidad 05), el mando puede salir de
una **tabla de tiempo**: «del cuadro A al B, mantener tal botón»
(→ glosario: Guion de mando). Es `src/juego/guion.c` (`guion_mando`) y el
recorrido completo del tutorial está en `src/juego/guion_tutorial.c`: 13
tramos, llega a la salida en el cuadro `GUION_TUTORIAL_FIN` = 2270. Lo juegan:

- `tests/prueba_recorrido.c`, en el PC, con la rejilla real;
- la **ROM de prueba**, compilada con `-DGUION_PRUEBA`: `mando.c` lee el guion
  en vez del mando y arranca directa en el nivel. Si la ROM y el PC no hacen lo
  mismo, tcc816 está haciendo algo distinto que gcc.

`guion.c` recorre la tabla con un **puntero** (`t++`) y no con `guion[i]`:
`TramoGuion` mide 5 bytes, y con un índice tcc816 llamaría a `tcc__mul` (i × 5)
en cada acceso: 72 multiplicaciones por frame, medio frame (unidad 14).

## La librería: PVSnesLib

**Primero, lo que trae el SDK.** PVSnesLib 4.6.0 tiene un **motor de mapas**
y un **motor de objetos** con colisión contra el mapa. Es lo primero que se
probó para el tutorial (fase 1, `docs/diseno-tutorial-movimiento.md` §5).

Motor de mapas, [`map.h`](https://alekmaul.github.io/pvsneslib/map_8h.html)
(«WARNING! Map engine must be used on Background #0»; el mapa en VRAM `0x6800`):

| Función (firma exacta) | Qué hace |
|---|---|
| `void mapLoad(u8 *layer1map, u8 *layertiles, u8 *tilesprop);` | carga el mapa (`.m16`), los metatiles (`.t16`) y sus propiedades (`.b16`), todos en la ROM |
| `void mapUpdate(void);` | prepara, según la cámara, lo que hay que redibujar (una vez por frame) |
| `void mapVblank(void);` | lo copia a la VRAM (en el VBlank) |
| `void mapUpdateCamera(u16 xpos, u16 ypos);` | mueve la cámara para seguir un punto, con su propia zona muerta (`x_pos`, `y_pos`) |
| `u16 mapGetMetaTile(u16 xpos, u16 ypos);` | el metatile de una posición |
| `u16 mapGetMetaTilesProp(u16 xpos, u16 ypos);` | su propiedad: `T_EMPTY`, `T_SOLID` (0xFF00), `T_PLATE` (0x0008), `T_SPIKE` (0x0004), `T_LADDE`, `T_FIRES` |
| `void mapSetMapOptions(u8 optmap);` | `MAP_OPT_1WAY` (solo hacia la derecha), `MAP_OPT_BG2` |

Motor de objetos, [`object.h`](https://alekmaul.github.io/pvsneslib/object_8h.html):
`void objCollidMap(u16 objhandle);` (actualiza `tilestand`, `tileabove` y la
velocidad del objeto según el mapa), `void objCollidMapWithSlopes(u16 objhandle);`
(con rampas), `u16 objCollidObj(u16 objhandle1, u16 objhandle2);` (dos objetos,
AABB) y `void objUpdateXY(u16 objhandle);` (integra la velocidad). Cada objeto
es una `t_objs` de 64 bytes con posición en punto fijo, velocidad y caja.

El mapa sale de **Tiled** con la herramienta `tmx2snes`
(`tools/pvsneslib/devkitsnes/tools/tmx2snes.exe`; en el `Makefile` del SDK,
`$(TMXCONV) -i mapa.tmj -m tiles.map`), que escribe `.m16`, `.b16` y `.o16`
(objetos, para `objLoadObjects`).

Ejemplos del SDK que lo usan (léelos):

- `tools/pvsneslib/snes-examples/graphics/maps/tiled/` — solo el mapa:
  `tiled.c` llama a `mapLoad`, y en el bucle `mapUpdate()`, `mapUpdateCamera()`,
  `WaitForVBlank()`, `mapVblank()`. Copiado a otra carpeta compila tal cual
  (verificado: «Build finished successfully», ROM de 256 KB).
- `tools/pvsneslib/snes-examples/systems/objects/mapandobjects/` — Mario con el
  motor de objetos: `mario.c` acelera con el pad, llama a `objCollidMap(idx)` y
  luego a `objUpdateXY(idx)`; solo deja saltar si `tilestand != 0`.

Las mismas ideas que el juego, con otros nombres: `T_SOLID`, `T_PLATE` y
`T_SPIKE` son nuestros `COL_SOLIDO`, `COL_PLATAFORMA` y `COL_ZARZA`.

**Por qué el juego NO usa el motor de mapas** (decisión C5, §5 del diseño):

1. **No se puede cambiar el mapa en marcha**: `map.h` no tiene ninguna función
   de escritura y `mapLoad` lee de la ROM. El tronco rompible exige otra cosa
   (el bloque y el parche).
2. **El arte es de rejilla doble** (16 px, desplazada media celda). Pintarlo a
   mano en Tiled en tiles de 8×8 era lento y frágil; generarlo por script
   obligaba a escribir los formatos de `tmx2snes`, cuyo soporte de volteos se
   desconoce.
3. **El motor trae su propia cámara**, con zona muerta, distinta de
   `camara.c` y de sus pruebas.
4. La física de Tarou (dos pies, cuerpo que cambia al agacharse, túnel,
   retroceso píxel a píxel) ya estaba escrita y probada en el PC; el motor de
   objetos la habría sustituido por otra.

Así que el juego tiene **su propia rejilla** de 128×14 (`colision.c`, unas 170
líneas probadas en el PC) y **su propio script** de Tiled
(`preparar_nivel.py`). Tiled sigue siendo la única fuente del nivel.

**Las funciones del SDK que sí usa esta parte del juego:**

| Función (firma exacta) | Qué hace / qué toca | Dónde |
|---|---|---|
| `void bgInitTileSet(u8 bgNumber, u8 *tileSource, u8 *tilePalette, u8 paletteEntry, u16 tileSize, u16 paletteSize, u16 colorMode, u16 address);` ([background.h](https://alekmaul.github.io/pvsneslib/background_8h.html)) | tiles a la VRAM y paleta a la CGRAM, por DMA | `fondo_nivel_cargar` |
| `void bgSetMapPtr(u8 bgNumber, u16 address, u8 mapSize);` | dónde está el mapa del fondo y su tamaño (`SC_64x32`: el anillo); registro `BG1SC` | `fondo_nivel_cargar` |
| `void bgSetScroll(u8 bgNumber, u16 x, u16 y);` | el scroll (registros `BGnHOFS`, doble escritura) | `pantalla_nivel1_actualizar` |
| `void setScreenOff(void);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | *forced blank*: la VRAM se escribe cuando se quiera | `reaparecer` |
| `void dmaCopyVram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | copia a la VRAM por DMA con VMAIN = 0x80 | (no sirve para columnas) |
| `void dmaCopyVram7(u8 *source, u16 address, u16 size, u8 vrammodeinc, u16 dmacontrol);` | la misma copia, **eligiendo VMAIN** («the value of REG_VMAIN») y el control del DMA | (no la usa; ver abajo) |

**`dmaCopyVram7` y por qué el juego baja a su propia lista.** El SDK sí sabe
copiar una columna: `dmaCopyVram7` recibe el valor de VMAIN, así que con `0x81`
copiaría las 32 entradas de una columna. (Ningún ejemplo del SDK la usa y su
documentación no dice si deja VMAIN como estaba; si la usas, vuelve a ponerlo en
`0x80` tú mismo.) El juego no la usa por el **presupuesto del VBlank** (unidad
09: 37 líneas, el SDK gasta 5): cada llamada desde C son cientos de
instrucciones de preparar la pila, y en el mismo VBlank van los tiles de Tarou,
las columnas y el HUD. La lista se prepara **durante el frame** (cuando sobra
tiempo) y en el VBlank `dma_ejecutar.asm` solo escribe los registros, unos 60
ciclos por copia. Además la lista guarda el **banco** aparte
(`nivel1_cerca_col_banco: .db :nivel1_cerca_col` en `data.asm:157`, unidad
13). Con las macros de la librería, lo que hace la rutina por cada entrada es:

```c
REG_VMAIN = 0x81;                      /* +32 palabras tras cada una: una columna.  */
REG_VMADDLH = VRAM_MAPA_A + nivel_columna_vram(col);  /* Destino en la VRAM.         */
REG_DMAP0 = 0x01;                      /* Modo 1: bytes alternos a $2118 y $2119.   */
REG_BBAD0 = 0x18;                      /* Destino: VMDATA ($2118).                  */
REG_A1T0LH = origen;                   /* Dirección de la columna en su banco...    */
REG_A1B0 = banco;                      /* ...y el banco.                            */
REG_DAS0LH = 64;                       /* 32 entradas x 2 bytes.                    */
REG_MDMAEN = 0x01;                     /* Arranca el canal 0.                       */
REG_VMAIN = 0x80;                      /* Como lo espera el resto del SDK.          */
```

(Es una traducción para leerla, no código del juego: el juego lo hace en
ensamblador en `src/juego/dma_ejecutar.asm`, que la unidad 15 enseña a leer.)

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| Las constantes y las consultas rápidas | `src/juego/colision.h` |
| `colision_en`, el bloque, `colision_columna_solida`, `colision_fila_es`, `colision_apoyan` | `src/juego/colision.c:49`, `:41`, `:79`, `:117`, `:144` |
| Los dos pies (`PIE_ATRAS`, `PIE_DELANTE`) | `src/juego/tarou.c:60-61` |
| `cabe_de_pie` (el túnel) | `src/juego/tarou.c:230` |
| `mover_x` con el retroceso píxel a píxel | `src/juego/tarou.c:411` |
| `mover_y` (seguir apoyado, aterrizar cruzando bordes, techo) | `src/juego/tarou.c:461` |
| El orden x → y dentro del frame | `tarou_actualizar`, `src/juego/tarou.c:556-559` |
| `se_tocan` (AABB), caída en punto fijo, zarzas, tronco, controles, salida | `src/juego/partida.c:28`, `:49` |
| El anillo: `apuntar_columna`, `fondo_nivel_actualizar`, el parche | `src/juego/fondo_nivel.c:75`, `:128`, `:58-69` |
| `nivel_columna_vram` | `src/juego/nivel.c` |
| La lista de DMA (`DMA_VMAIN_COLUMNA` 0x81) | `src/juego/dma_lista.h:37` |
| Reaparecer con la pantalla apagada | `reaparecer`, `src/pantallas/pantalla_nivel1.c:132` |
| El frame del nivel: mando → `tarou_actualizar` → `partida_actualizar` → cámara → dibujo | `src/pantallas/pantalla_nivel1.c:184` |
| «LEVEL 2 / COMING SOON» | `src/pantallas/pantalla_proximamente.c` |
| Guion: `mantenidos` con puntero, `guion_mando` | `src/juego/guion.c:27`, `:40` |
| El recorrido (13 tramos, fin en 2270) | `src/juego/guion_tutorial.c`, `guion_tutorial.h` |
| Tiled → rejilla, arte, cabecera | `scripts/preparar_nivel.py` (`leer_tmj`, `escribir_cabecera`, `preparar`) |
| El nivel en Tiled | `res/nivel1/tutorial.tmj` (+ `logica.png`, 3 tiles) |
| Lo generado | `res/nivel1/colision.bin`, `fondo_cerca.png`, `nivel1_datos.h` |
| Las pruebas sobre la rejilla REAL | `tests/prueba_colision.c`, `prueba_partida.c`, `prueba_recorrido.c` |
| Historia y trampas | `docs/diseno-tutorial-movimiento.md` §5, §7 y §8 |

### La tubería del nivel

```
Tiled: res/nivel1/tutorial.tmj      capa «logica» 128x14 (1 sólido, 2 plataforma, 3 zarza)
   │                                capa «objetos»: inicio, cartel (texto), tronco, salida
   ▼  python scripts/preparar_nivel.py        (¡SIN --crear: eso reescribe el .tmj!)
   ├─► res/nivel1/colision.bin  (1792 B) ──► data.asm: nivel1_colision ──► colision.c
   ├─► res/nivel1/nivel1_datos.h  NIVEL1_INICIO_X, _TRONCO_X, _SALIDA_X, _CONTROLES_X...
   │                               y, con PRUEBAS_PC, la rejilla como array (para tests/)
   └─► res/nivel1/fondo_cerca.png (2080x256) ─► gfx4snes ─► mapa_a_columnas.py ─► .col
   ▼
make clean ; make      (nivel1_datos.h es un .h: sin clean, objetos viejos)
make -C tests ; .\tests\pruebas.exe
```

**`prueba_colision.c`, `prueba_partida.c` y `prueba_recorrido.c` usan la rejilla
REAL** del nivel: retocar el nivel en Tiled puede romperlas **a propósito**.
Es lo que se quiere: si alguien hace imposible un obstáculo o el guion deja de
llegar, una prueba lo dice antes que Mesen.

## Trampas

| Trampa | Qué pasa | Cómo se evita |
|---|---|---|
| `PF_ENTERO` con un valor negativo | −5 px se convierte en 65531: Tarou «moría» en lo alto de un salto | comparar en punto fijo con signo: `t->y_fp >= PF(Y_CAIDA)` |
| JSON de Tiled con espacio tras los dos puntos | `tmx2snes` (su lector, *cute_tiled*) falla: «Invalid integer found during parse» / «Cannot load map» (salida real en `capturas/tmx2snes_espacios.txt`). Nuestro script lee bien los dos, pero escribe como Tiled | al escribir un `.tmj` desde Python, `json.dump(..., separators=(",", ":"))` (`escribir_tmj`); al editarlo, con Tiled |
| Los pies, 4 px dentro de la hierba | parece un fallo de colisión y no lo es: es dibujo (2 px del arte y 2 de `HUNDIR_PIES`) | la física usa el borde exacto; no «arreglarlo» en `colision.c` |
| `preparar_nivel.py --crear` | reescribe el `.tmj` con el trazado inicial: se pierden los retoques | sin `--crear` después de la primera vez |
| Olvidar `make clean` | `nivel1_datos.h` cambia y `pantalla_nivel1.obj` se queda con los valores viejos (`snes_rules` no sigue los `.h`) | `make clean` tras regenerar el nivel |
| `colision_en` por todas partes | ~150 instrucciones por llamada: el juego a 27 FPS | consultas de columna/fila enteras |
| `guion[i]` con structs de 5 bytes | `tcc__mul` en cada acceso | recorrer con puntero |
| Rejilla de prueba de menos de 128 columnas | el `<< 7` lee fuera | declarar 128×14 siempre |
| Capa de Tiled en Base64 o comprimida | el script no la lee | `leer_tmj` lo dice: en Tiled, *Map → Map Properties → Tile Layer Format = CSV* |

## La demo

`demo_20_colision.c` usa el **código real** (`colision.c`, `tarou.c`,
`animaciones.c`) sobre una rejilla de juguete y escribe una traza, como la que
se usó para ajustar el guion. Desde esta carpeta:

```
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../tests -I../../src -o demo_20.exe demo_20_colision.c ../../src/juego/colision.c ../../src/juego/tarou.c ../../src/juego/animaciones.c
.\demo_20.exe
```

(`-I../../tests` va primero para usar la misma copia fija de `tarou_anim.h`
que las pruebas: unidad 02.) Lo que hay que ver en la salida
(`capturas/salida_demo_20_colision.txt`):

```
      columnas 0..29 (16 px cada una), filas 7..13:
  y112 ..........####................
  y144 ..........####................       techo del túnel: deja 48 px
  y160 ..................======....##       plataforma y muro (y176, y192: el muro sigue)
  y208 ##############################       suelo
```

- cuadros 0..26: cae desde `y 60` y aterriza con los pies en 208 (`PARADO`);
- cuadro 91: parado en `x 84` contra la boca del túnel (84 + 24 + 52 = 160);
- cuadros 92..324: agachado; desde x 120 (fase «3 soltar Abajo») ya no pulsa Abajo y sigue agachado
  bajo el techo; en el 325, fuera, se levanta solo (`CAMINANDO`);
- cuadros 363..399: salta, atraviesa la plataforma desde abajo y aterriza
  **encima** (pies en 160);
- cuadro 474: los dos pies fuera de la plataforma: `CAYENDO`; aterriza en el
  suelo y acaba en `x 372` contra el muro (372 + 24 + 52 = 448);
- al final, la trampa: `PF_ENTERO(-80) = 65531`.

Antes de ejecutarla, intenta adivinar en qué x se para contra el túnel y contra
el muro.

## Ejercicio y criterio de aprobación

Dos partes.

**Parte 1 (PC): `ejercicio_20_rejilla.c`.** Una rejilla propia, en pequeño, con
cuatro `TODO` numerados: `celda_en` (de píxel a celda con desplazamientos),
`se_tocan` (AABB), `mover_x` (retroceso píxel a píxel) y `aterriza`
(plataforma de un solo sentido). Compila sin avisos tal como está; la salida
esperada exacta está en su cabecera y en `capturas/salida_ejercicio_20_rejilla.txt`:

```
gcc -std=c99 -Wall -Wextra -o e20.exe ejercicio_20_rejilla.c
.\e20.exe > mia.txt
Compare-Object (Get-Content mia.txt) (Get-Content capturas\salida_ejercicio_20_rejilla.txt) -SyncWindow 0
```

**Parte 2 (juego real): `ejercicio_20.md`.** Añadir un **segundo foso** al
nivel en Tiled, regenerar, ver fallar `prueba_recorrido` y ajustar el guion con
la herramienta `traza_20.c` (una traza de x/y por cuadro del recorrido entero)
hasta que pase; luego verlo en la ROM de prueba.

**Aprobación:**

1. la parte 1 compila con `gcc -std=c99 -Wall -Wextra` sin avisos y su salida es
   idéntica a la esperada;
2. con el segundo foso, `tests/pruebas.exe` da **367 pruebas, 0 fallos**
   (antes de ajustar el guion debe dar 4 fallos, todos de `prueba_recorrido.c`:
   guarda esa salida);
3. capturas de la **ROM de prueba** (`-DGUION_PRUEBA`) en las que Tarou salta el
   foso nuevo (hacia el segundo 34 de la captura) y llega a «LEVEL 2 / COMING
   SOON».

La solución está en `solucion/` (`ejercicio_20_rejilla.c`, `foso2.diff`,
`tutorial.tmj`, `guion_tutorial.c`, `guion_tutorial.h`) y sus salidas en
`capturas/` (`pruebas_con_foso2_sin_guion.txt`, `traza_con_foso2_sin_guion.txt`,
`traza_tramo_2010_2030.txt`, `pruebas_solucion.txt`, `traza_solucion.txt`).

## Lo que viene

Con esta unidad se cierra el recorrido del plan: del C en el PC (00) a un nivel
entero que se juega solo en la SNES. Lo siguiente del proyecto son los enemigos
(`src/enemigos/`, Etapa 3): cajas que se mueven y chocan con la misma rejilla y
con Tarou, con la misma `se_tocan`.
