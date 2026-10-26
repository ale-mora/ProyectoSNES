# Unidad 21 — Presupuesto de sprites: la prueba de carga

> **Unidad del beat 'em up.** Enseña sobre el código que hoy está en la rama
> `demo-secuencia-arranque` (commit `641d7c0`), no sobre el tag
> `plataformas-r2`: la capa de sprites (`sprites.c`, `sprites_oam.asm`,
> `subida.c`, `p16.c`, `dma_copias.asm`, `profundidad.c`) y la ROM de prueba de
> carga (`prueba_carga.c`) nacieron con el género nuevo
> ([`docs/diseno-beat-em-up.md`](../../docs/diseno-beat-em-up.md) §8–§9, citada
> aquí como «la spec»). La práctica sobre el juego se hace en un clon de esa
> rama, nunca en el árbol principal:
>
> ```text
> git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u21
> ```
>
> Copia esta carpeta **dentro del clon** (`E:\ProyectoSNES_trabajo\u21\aprendizaje\unidad-21-presupuesto-de-sprites\`)
> y compila la demo desde allí: así los `../../src` y `../../res` de los
> comandos son los del clon.
>
> **Qué se comprobó y qué no.** Esta unidad ejecutó de verdad: la demo y el
> ejercicio de PC (gcc), las 570 pruebas del clon (560 del juego + 10 que añade
> el ejercicio), los 11 `unittest` de `piezas16.py` y de
> `preparar_prueba_carga.py`, y la **compilación** de la ROM `-DPRUEBA_CARGA`
> (sin avisos). **No se abrió Mesen**: la verificación visual de la prueba de
> carga (las capturas, los máximos del HUD, quién desaparece cuando una línea
> se pasa) está en la spec §9.1 y §9.2. Cada cifra de esta unidad lleva su
> origen: **[PC]** la calculó este curso con los archivos reales, **[spec]** la
> midió la prueba de carga en Mesen y está escrita en la spec (aquí no se
> repitió), **[código]** se lee en el código, **[modelo]** es una cuenta
> aproximada, explicada donde aparece.

**Objetivo:** saber cuántos personajes caben en pantalla a la vez y por qué,
**midiéndolo antes de pedir el arte**. En un beat 'em up todos los luchadores
comparten la misma franja de pantalla (el carril), y la SNES tiene cuatro
presupuestos que se agotan por separado: las **ranuras de sprite por línea**,
los **tiles de sprite de la VRAM**, el **ancho de banda del VBlank** y la
**CPU** que cuesta llenar la OAM. La unidad enseña a leer cada uno, a calcularlo
en el PC con los mismos archivos que usa la ROM, y la trampa que ellos tienen
en común: **cuando se pasa uno, el hardware no avisa, se limita a no dibujar**.

**Prerrequisitos:** 04 (el frame y el VBlank), 08 (sprites y OAM), 09
(metasprites), 13 (memoria), 16 (Mesen y el HUD). Conviene haber pasado por la
14 y la 15 (ensamblador): el corazón de la capa, `sprites_oam.asm`, está en
ensamblador, y aquí solo se explica lo que hace, no cómo se escribe.
**Práctica:** PC (demo + ejercicio) + juego real en un clon (pruebas y
compilar la ROM de prueba). **Tiempo orientativo:** dos tardes.

## Conceptos

### Los cuatro presupuestos

| Presupuesto | Límite de la máquina | Quién lo gasta | Qué pasa si se pasa |
|---|---|---|---|
| **Ranuras por línea** | 34 trozos de 8 px y 32 sprites por línea | todos los personajes que comparten líneas | faltan trozos de un personaje (primero los del de delante) |
| **Tiles de sprite** | 512 tiles (16 KB) direccionables | los cuadros de cada personaje en la VRAM | no caben: hay que repartir o recortar |
| **VBlank** | 37 líneas (el SDK gasta 5) | las subidas de tiles a la VRAM por DMA | la última copia llega con la PPU dibujando y se pierde |
| **CPU** | ~262 líneas de haz por frame | la lógica, ordenar y escribir la OAM | el frame no cabe: se pierden frames |

Los cuatro se miden con la misma herramienta, la **ROM de prueba de carga**
(→ glosario: Prueba de carga): una versión del juego, compilada con
`-DPRUEBA_CARGA`, que pone la mayor carga prevista y enseña en el HUD cuánto
se gasta de cada cosa. Se hizo **antes** de pedir el arte de los ninjas a
Pixellab, porque el tamaño de un ninja (64×96) salió de ella.

### Ranura por línea

La PPU dibuja una línea de pantalla cada vez. Antes de cada línea busca los
sprites que la cruzan y, de ellos, dibuja como mucho **32 sprites** y **34
ranuras** (→ glosario: Ranura por línea). Una **ranura** es un trozo de 8 px de
ancho de un sprite en esa línea: una pieza de 16×16 gasta **2 ranuras en cada
una de sus 16 líneas**; una de 32×32, 4. Es un límite *por línea*, no por
pantalla: en un platformas casi nadie comparte línea, pero en un belt-scroll
Tarou y los enemigos pisan el mismo carril, así que **las líneas de los pies
son siempre las más cargadas**.

La cuenta es una suma por línea. Con los archivos reales (la demo, parte A y C
**[PC]**):

| Quién | Piezas de 16×16 por cuadro | Fila de 16 px más cargada | Ranuras por línea |
|---|---|---|---|
| Tarou andando (`tarou_caminar.p16`) | 14–16 | 5 piezas | **10** |
| Ninja andando (`ninja64_andar.bin`) | 18–21 | 4 piezas | **8** |
| Ninja atacando (`ninja64_ataque.bin`) | 15–19 | 4 piezas | **8** |
| Sombra (2 piezas) | 2 | 2 piezas | **4** |

Y la suma de una escena (la misma cuenta que `scripts/preparar_prueba_carga.py`,
que da exactamente los mismos números; salida en
`capturas/prediccion_preparar_prueba_carga.txt` **[PC]**):

| Escena (peor cuadro de andar) | Ranuras máx. | Líneas > 34 |
|---|---|---|
| Tarou solo | 10 | ninguna |
| Tarou + 2 ninjas, con sombras | 28 | ninguna |
| Tarou + 3 ninjas a la misma y, con sombras | **36** | 168..175 |
| Tarou + 3 ninjas repartidos en profundidad, con sombras | 34 | ninguna |
| Tarou + 3 ninjas a la misma y, **sin** sombras (ejercicio) | 34 | ninguna |

La ficha de esta unidad, escrita antes del código, hablaba de «Tarou 12
ranuras, ninja 8, sombra 4; Tarou + 3 ninjas = 36 > 34». Eso era con piezas de
32×32; con las piezas de 16×16 recortadas Tarou baja a **10**, y los 36 siguen
saliendo, de sumar línea a línea los cuerpos y las sombras. Mira las líneas:
**168..175 son las 8 líneas de los pies**, donde se apilan los cuerpos y las
sombras (la suma exacta de cada línea la imprime el script o la demo; aquí no se
desglosó).

**Qué pierde la PPU al pasarse.** Cuando una línea tiene más de 34 ranuras, la
PPU descarta los sprites de **índice más bajo de la OAM** (los que están
*delante*, ver «Orden por profundidad»), y con más de 32 sprites descarta los
de índice alto **[spec]** (§9.1, medido en Mesen y confirmado por el SNESdev
Wiki). Consecuencia directa: si el protagonista está en la entrada 0 de la OAM,
**es el primero al que le faltan trozos**. Rotar el orden de la OAM cada frame
(*parpadeo rotatorio*, el truco de muchos juegos para repartir el corte) se
estudió y se **descartó**: la OAM es a la vez el orden de profundidad (quién
tapa a quién), y rotarlo rompería la profundidad. Por eso **34 ranuras por línea
es un límite duro que se respeta con el diseño** (cuántos enemigos, su ancho y su
reparto en profundidad), y las sombras, que van al final de la OAM, sobreviven
aunque el protagonista pierda trozos **[spec]**.

> **El juego no tiene diagnóstico de esto.** El HUD de la prueba de carga no
> dice nada de las ranuras: la única manera de verlo es mirar la pantalla, o
> **predecirlo**, que es lo que hace el script y repite la demo. (La PPU tiene un
> registro de estado, `STAT77` `$213E`, con indicadores de «más de 32 sprites» y
> «más de 34 ranuras» en algún momento del frame; es dato del manual de
> hardware, **ni el juego lo lee ni esta unidad lo comprobó**.) La spec §9.2 cuenta
> un fallo que el HUD no vio y la vista sí: ninjas a trozos por guardar mal el
> banco de su tabla de piezas.

### Tiles de sprite y el reparto en pares de filas

La VRAM guarda los tiles de sprite (8×8 px, 4 bits por píxel = 32 bytes cada
uno) en una zona de **512 tiles = 16 KB** (`VRAM_SPRITES`, `comun.h`). Sirve para
todos: protagonista, enemigos, sombras. Los 512 tiles se ven como una
cuadrícula de **16 columnas × 32 filas** (la PPU numera los tiles de sprite de
16 en 16 por fila, y un sprite de 16×16 son 2×2 tiles: los de arriba están en
la fila de tiles `t` y los de abajo 16 tiles más allá). El proyecto los reparte
en **pares de filas** (→ glosario: Par de filas): dos filas de 16 tiles = 32
tiles = 1 KB, que cubren 8 piezas de 16×16 (cada una, 2 tiles de ancho). Los 16
pares son:

| Pares | Tiles | Quién | Cabe |
|---|---|---|---|
| 0–2 | 0–95 | protagonista, búfer A | 24 piezas |
| 3–5 | 96–191 | protagonista, búfer B | 24 piezas |
| 6–8 | 192–287 | enemigo 1 | 24 piezas |
| 9–11 | 288–383 | enemigo 2 | 24 piezas |
| 12–14 | 384–479 | enemigo 3 | 24 piezas |
| 15 | 480–511 | compartido: sombras (2 piezas), chispas, objetos | 8 piezas |

Es la tabla de la spec §8 y de las constantes de `prueba_carga.c`
(`TILE_TAROU_A` 0, `TILE_TAROU_B` 96, `HUECOS` {192, 288, 384}, `TILE_SOMBRA`
480) **[código]**; la demo (parte B) la imprime. Dos cosas que conviene ver:

- **El protagonista usa 192 tiles, no 256.** La ficha del plan y el principio de
  la spec §8 decían 256 (doble búfer de 2 celdas de 128); el reparto definitivo
  (v3) lo dejó en dos huecos de 96.
- **Doble búfer del protagonista.** El cuadro nuevo de Tarou sube al búfer que
  *no* se está enseñando; cuando está entero, la OAM pasa a él. Un enemigo usa
  **un solo búfer**: su cuadro nuevo se sube entero en un VBlank y la OAM cambia
  en ese mismo frame.

**La pieza de 16×16 y el formato `.p16`.** Un ninja de 64×96 sin recortar son 6
piezas de 32×32 = 96 tiles por cuadro, y 3 ninjas (288) no caben en lo que deja
Tarou (la **opción A** de la spec: recortar). El script `scripts/piezas16.py`
parte cada cuadro de una tira PNG en piezas de 16×16 y **se queda solo con las
que tienen algún píxel**. Un ninja ocupa 15–21 piezas (60–84 tiles) en vez de
96 **[PC]** (`ninja64_andar.bin`: [18, 19, 20, 21, 20, 18, 18, 19]; el ataque:
[15, 15, 17, 17, 19, 17, 15]). Un hueco de 24 piezas siempre alcanza (el máximo
medido es 21). El archivo `.p16` (formato completo en la cabecera de
`piezas16.py`) tiene, en este orden:

1. un **índice** de 8 bytes por cuadro: dónde está su tabla de piezas, dónde
   están sus tiles, cuántas piezas tiene y dónde está su tabla **espejada**
   (`p16.c` lo lee: `p16_cuadro`);
2. las **tablas de piezas**: por cuadro, un par `(dx, dy)` por pieza, y detrás
   la misma tabla espejada (`dx' = ancho − 16 − dx`), para mirar a la izquierda
   sin ningún cálculo: el ensamblador siempre suma `x0 + dx`;
3. los **tiles**, `n · 128` bytes por cuadro, **en el orden en que se suben** (ver
   siguiente apartado).

Un cuadro de `n` piezas son exactamente `n · 128` bytes (4 tiles de 32 bytes por
pieza): el ninja más grande, 21 piezas = **2688 bytes** **[PC]**.

**El tile de la pieza `k`** dentro de su hueco es `(k/8)·32 + (k%8)·2` (8 piezas
por par, 2 tiles de ancho cada una), que se calcula con desplazamientos
(`subida_tile_pieza`, `subida.c`). Un hueco puede **cruzar la frontera de los 256
tiles**: el hueco del enemigo 1 empieza en el 192, así que sus piezas 16..23
caen en el tile 256 o más (la demo imprime `16 17 18 19 20 21 22 23` **[PC]**).
Por eso el bit 8 del tile va **en cada pieza** y no en el sprite entero: es el
bit `N` del atributo de cada entrada de la OAM. `sprites_oam.asm` no lo
calcula: suma `tile + (atributos << 8)`, y el acarreo al pasar del tile 255 al
256 enciende `N` solo (lo dice su cabecera).

### Ancho de banda del VBlank y la lista de DMA

La VRAM solo se escribe en el VBlank (37 líneas, o con la pantalla apagada;
unidad 04), y lo que se sube cada frame son tiles de los cuadros nuevos. La
unidad 09 explicó la **lista de DMA** (`dma_lista.c`): durante el frame cada
módulo *apunta* sus copias (origen, destino en la VRAM, bytes, banco); en el
VBlank una rutina en ensamblador (`dma_ejecutar.asm`) dispara una copia por
entrada (unos 60 ciclos por copia, según su cabecera). Dos reglas la acompañan:

- **Subir en trozos de 512 B** (`subida.c`). El plan de copias de un cuadro sale
  del número de piezas, sin tablas: por cada **par completo**, 2 copias de 512 B
  (la fila de arriba del par y la de abajo, que en la VRAM está 256 palabras más
  allá), y por el **último par a medias** con `q` piezas, 2 copias de `q·64`
  bytes. Con 21 piezas: 4 copias de 512 + 2 de 320 = 6 copias, 2688 bytes
  **[PC]**. `subida_tomar` reparte esas copias en VBlanks sin pasarse de un
  **tope de bytes**.
- **Un tope por VBlank** (`TOPE_TAROU` = 1536 B = 3 copias de 512, en
  `prueba_carga.c`; `TOPE_SUBIDA`, con el mismo valor, en `protagonista_sprite.c`). Un cuadro de Tarou (1792–2048 B
  **[PC]**) tarda 2 VBlanks, y como cambia de cuadro cada 2 frames, **sube en
  todos los VBlanks**: es el peor caso a propósito. Un ninja sube su cuadro
  entero (`subida_tomar(&e_sub, 0xFFFF, ...)`) en **un** VBlank, y solo **un
  ninja por VBlank, por turnos** (`turno` en `subir_ninjas`).

La demo (parte A) cuenta los VBlanks de cada cuadro real: con el tope de Tarou,
**todos los cuadros de Tarou y de los ninjas necesitan 2 VBlanks; sin tope, 1**.

**Cuánto cabe: el modelo.** El HUD de la prueba de carga mide en qué línea del
VBlank acaban las copias (`V a b`). La spec §9.1 midió (Mesen): Tarou solo, el
VBlank acaba hacia la línea 17–19; añadiendo una subida de enemigo de 2 KB, en la
32; de 3 KB, en la 39. Son unas **7 líneas por KB** (≈ 146 bytes por línea;
la spec §8 usaba ~125 B/línea). Ajustando una recta por esos puntos sale

```text
línea ≈ 7,5 + 7 · (KB subidos en total)          [modelo; ±3 líneas]
```

(Con la velocidad teórica del DMA —8 ciclos maestros por byte, 1364 por línea—
saldrían ~170 B por línea; el resto lo gastan la preparación de cada copia y el
refresco de memoria. Es una cuenta mía, **no medida**.) Con ella, la demo
(parte E) calcula el peor frame: Tarou (1536 B) + el mayor cuadro de ninja
(2688 B) llega a la línea **36,4 [modelo]**, y la spec midió **36** de 37
**[spec]**. Es decir, el peor frame **tiene unos 90 bytes de margen** [modelo]:
una fila de HUD (64 B) cabe, dos no. Y dos subidas de ninja en el mismo VBlank
(modelo: línea 55; spec: 43–50) no caben nunca: de ahí el turno rotatorio.

> **La spec da tres cifras distintas para «dos subidas de ninja por VBlank»**:
> 44 (§9.1, v2, ninjas de 62 px), 50 (tabla de §9.2) y 43 (texto de §9.2).
> Cambian los ninjas y los frames elegidos; ninguna cabe en 37. No se usó ninguna
> como dato fino.

**Alargar el VBlank (bandas de cine).** La spec §8 propone apagar la pantalla
(*forced blank*, unidad 04) en las últimas ~16 líneas visibles, que quedan negras
como bandas de cine, para ganar ~16 líneas más de DMA (≈ 2 KB); se activaría con
una IRQ de línea (`HTIMEN`/`VTIME`) o con HDMA sobre `INIDISP`. Es una **opción
documentada, no implementada**: no hay en el código ninguna IRQ de línea ni HDMA
sobre `INIDISP` (se buscó). La prueba de carga mostró que con una subida de ninja
por VBlank no hace falta, y queda como reserva (→ glosario: Bandas de cine).

### El coste en CPU: `oamSet` contra la rutina en ensamblador

Llenar la OAM es lo que más CPU pide (**[spec]** §9.1 y §9.2, medidas en Mesen;
la tabla de abajo repite sus cifras, no se midió aquí):

| Cómo se escribe la OAM | Coste | Con 37 piezas (3 ninjas) |
|---|---|---|
| `oamSet` + `oamSetEx` del SDK, una vez por pieza | ~5,4 líneas por pieza | ~200 líneas: el frame no cabe (CPU 360–440) |
| una llamada a ensamblador por entidad | ~2,2 líneas por pieza | la mayor parte era la llamada, no el bucle |
| **una llamada por frame** (`sprites_dibujar`) | **~0,5 líneas por pieza** | ~44 líneas la OAM entera |

La lección es del compilador (tcc816): una llamada a una función C con 8
parámetros cuesta cientos de instrucciones, y `oamSet` tiene 8. Por eso la capa
de sprites hace así las cosas:

1. **Una tabla de entidades fija** (`sprites_ent`, `SpriteEnt` de **16 bytes**: una
   potencia de 2, así que `sprites_ent[i]` es un desplazamiento y no una
   multiplicación). Las entradas persisten de un frame a otro: cada frame solo se
   escriben `x0`, `y0` y la clave de orden de lo que se mueve; las piezas y el
   tile, **solo cuando cambia el cuadro**.
2. **Una sola llamada por frame**, `sprites_dibujar()` (`sprites_oam.asm`), que
   recorre las entidades en orden de profundidad y escribe todas sus piezas en el
   búfer de la OAM del SDK (`oamMemory`, 544 bytes: 128 entradas de 4 B + 32 B de
   tabla alta). La NMI del SDK copia ese búfer a la OAM en el VBlank.
3. **Camino rápido**: si la celda entera cae dentro de la pantalla (`borde == 0`,
   se decide una vez con `sprites_borde()`), cada pieza son **una suma de 16 bits**,
   `(dx | dy << 8) + (x0 | y0 << 8)`, y dos escrituras (la palabra de posición y la
   de tile+atributos). Sin acarreo entre bytes, porque `x0 + dx < 256`. El lazo
   `_rapida` son ~21 instrucciones por pieza (contadas a mano sobre el `.asm`).
   Si la celda puede salirse, `_lenta` comprueba cada pieza: la esconde con
   `y = 224` si queda fuera, y pone el bit 8 de `x` en la tabla alta si `x` es
   negativa (`_bit8_x`).
4. **Esconder con `y = 224`, nunca con `x = −256`**: con `x = −256` la PPU cuenta
   igualmente sus 8 px por línea en las líneas visibles, y se comería ranuras.

### Orden por profundidad

En un carril, el que tiene los pies **más abajo** en la pantalla está más cerca y
tapa a los demás. En la SNES, entre dos sprites que se solapan **gana la entrada
más baja de la OAM**; el bit de prioridad solo decide contra los *fondos*
(→ glosario: Orden por profundidad). Así que cada frame se ordenan las entidades
por la `y` de los pies, de mayor a menor, y se escriben en la OAM en ese orden.

`profundidad_ordenar` (`profundidad.c`) es una **ordenación por inserción**,
**estable**: con 8 entidades que casi nunca cambian de orden entre un frame y el
siguiente cuesta una comparación por entidad, y con la misma `y` se queda el
orden que traían (si no, dos enemigos a la misma altura se cambiarían de sitio
cada frame y parpadearían). Las sombras llevan `y = 0` en `sprites_y` para ir
**al final** del orden (detrás de todo). Lo prueban `tests/prueba_profundidad.c`.

La demo (parte D) junta los dos presupuestos: pone a Tarou y 3 ninjas a la
misma `y`, ordena con el `profundidad_ordenar` real y se ve que **Tarou queda
delante** (misma `y`, y por ser la entidad 0 gana el desempate), es decir, en las
entradas más bajas de la OAM. Aplicando la regla de la PPU **[modelo, con la
regla medida en la spec]** a una línea sobrecargada, los trozos que se pierden
son los de Tarou y luego los del primer ninja. Coincide con lo que se vio en
Mesen: «lo que desaparece es Tarou» **[spec]**.

### La ROM de prueba como herramienta de medida

`prueba_carga.c` se compila **solo** con `-DPRUEBA_CARGA` (todo el archivo está
dentro de un `#ifdef PRUEBA_CARGA`, y `main.c` llama a `prueba_carga_ejecutar()`
en lugar de arrancar la secuencia de pantallas). Es una escena fija con la mayor
carga prevista: Tarou andando y paseándose de lado a lado (pasa por delante y por
detrás de los ninjas), hasta 3 ninjas provisionales (el samurái humano del pack
*FULL_Samurai* recortado a la celda de 64×96, `res/prueba_carga/ninja64_*.bin`,
regenerables con `scripts/preparar_prueba_carga.py`), sombras con media suma de
color, y el HUD de depuración encendido. No toca el mando: recorre sola las
**configuraciones** de la tabla `CONFIGS` (8 en el código, **9 con el ejercicio**), 360 frames cada una:

| # | Ninjas | Ataca | Reparto | Sombras | Subidas/VBlank |
|---|---|---|---|---|---|
| 0 | 0 | — | — | — | 1 |
| 1 | 1 | sí | misma y | sí | 1 |
| 2 | 2 | sí | misma y | sí | 1 |
| 3 | 3 | sí | misma y | sí | 1 |
| 4 | 3 | sí | repartidos | sí | 1 |
| 5 | 3 | no | repartidos | sí | 1 |
| 6 | 3 | sí | repartidos | no | 1 |
| 7 | 3 | sí | repartidos | sí | **2** (el techo) |

(Una `Config` mide 8 bytes: otra vez una potencia de 2, `CONFIGS[n]` es un
desplazamiento.) En cada frame (`frame()`) hace, y mide después de cada paso:

```text
dma_lista_vaciar → mover a Tarou → subir_tarou (apunta sus copias)
→ avanzar los cuadros de los ninjas → subir_ninjas (como mucho cfg.subidas)
→ cpu_a = medir_cpu()            [tras las subidas]
→ mover_entidades (tabla + sprites_ordenar)
→ cpu_c = medir_cpu()            [+ la tabla de entidades, en C]
→ sprites_dibujar                [la OAM, en ensamblador]
→ cpu_b = medir_cpu()            [+ la OAM]
→ hud                            [apunta UNA fila de 64 B: la última de la lista]
WaitForVBlank → v_ini = línea → dma_lista_ejecutar → v_fin = línea
```

**Cómo mide.** `leer_linea()` congela los contadores del haz leyendo `$2137`
(`SLHV`) y lee la línea de `$213D` (`OPVCT`: primero los 8 bits bajos y luego el
bit 8). `desde_vblank(l)` la lleva a «líneas desde el VBlank en que empezó el
frame» (225 → 0, 261 → 36, 0 → 37…). `medir_cpu()` suma 262 si el contador
`snes_vblank_count` ya avanzó (otro frame empezó: el frame **no cupo**). Las
cifras del HUD (`CPU a c b  MAX`, `V v_ini v_fin  MAX`, `PERD`, `ESP`,
`BYTES`) son los máximos de cada configuración; **`PERD` = frames perdidos** =
`frames_cfg − vueltas_cfg` (VBlanks que pasaron menos vueltas del bucle).

> **Criterio de salida de la spec §9** (lo que decide si la configuración vale):
> 60 FPS, sin trozos perdidos en el caso típico (2–3 enemigos) y el VBlank por
> debajo de la línea 37 en el peor frame. Con la capa de sprites v4 la spec
> midió, con 3 ninjas de 64×96, Tarou y sombras: CPU máx. 240 de 262 y VBlank
> 36 de 37 **[spec]**. Cabe, con poco margen para la lógica del juego.

> **Trampa: las banderas del compilador no se siguen.** Cambiar `CFLAGS` no
> recompila nada, y sin `make clean` la ROM «normal» arrancaría en la prueba (o
> al revés). La receta: `make clean` **antes y después**:
> ```text
> make clean; $env:CFLAGS="-DPRUEBA_CARGA"; make; $env:CFLAGS=$null; make clean
> ```
> (el `make` del clon compila con el entorno de la receta «Build only, from
> PowerShell» de `CLAUDE.md`).

## La trampa: lo que se pasa de presupuesto se pierde en silencio

Los cuatro presupuestos fallan igual: **sin error, sin aviso, sin que el HUD
diga nada**. Hay dos mecanismos distintos de «pérdida» en la VRAM, que no se
deben confundir:

**1. La lista de DMA llena.** `dma_lista_anadir` ignora la copia si ya hay
`DMA_LISTA_MAX` = 32 (`if (dma_lista_n >= DMA_LISTA_MAX) return;`), y
`dma_copias.asm` hace lo mismo. La demo (parte F) lo ejecuta con el
`dma_lista.c` real: 35 copias apuntadas, 32 en la lista **[PC]**. Pero **no es
el que manda**: el peor frame de la prueba de carga apunta como mucho 10
entradas (Tarou hasta 3, un ninja hasta 6, 1 fila de HUD) y 16 con dos subidas de
ninja **[código]**.

**2. Pasarse del VBlank.** El DMA no sabe que el VBlank se acabó. Si
`dma_lista_ejecutar` sigue copiando cuando la PPU ya está dibujando, los
datos no llegan a la VRAM (la PPU ignora las escrituras mientras dibuja) y la
copia no da error. Es la causa del «Tarou a trozos» del 2026-10-20
(`CLAUDE.md`). **Sumar una copia más cuando el VBlank va justo hace que las
últimas copias se pierdan**: el orden de la lista manda, y las últimas son las
primeras víctimas. En la prueba de carga la fila de HUD se apunta la última
(`hud()` va tras la OAM), de modo que *es* la primera que caería fuera; la demo
(parte E) lo muestra con el modelo: con una fila de HUD acaba en la línea 36,8,
con una segunda fila en la 37,2 **[modelo]**. Recuerda que el modelo tiene ±3
líneas: no dice *cuál* sería la que se pierde, dice que **el margen del peor
frame es de unos 90 bytes y que 64 más lo agotan**.

Dos casos reales de la sesión del 2026-10-26 (relato del autor al preparar esta
unidad; **no se reprodujeron aquí**, y de la segunda no se aclaró la causa):

- Al mandar una fila de HUD en cada frame, un cuadro de Tarou salió **«en tiras
  verticales»**: parte del cuadro llegó a la VRAM y parte no.
- Una fila de tilemap propia, mandada con `dma_lista_anadir` desde un
  `static u16 fila[32]`, **no se dibujó nunca**.

Lo que sí está verificado es el **método que funciona**, el de
`hud_debug.c`: `hud_debug_empezar(fila)` deja la fila en blanco con `memset`
(la de la librería, en ensamblador) y devuelve un puntero a su primera celda;
`hud_debug_texto(p, "...")` y `hud_debug_numero(p, valor)` escriben con un
puntero que avanza (`*p++ = ...`); `hud_debug_enviar(fila)` apunta los 64 bytes
en la lista de DMA. `prueba_carga.c` lo usa en su `hud()`. Cómo trabajar sin
caer en la trampa:

1. Mide **antes y después** de añadir una copia: el HUD `V a b` (en la prueba de
   carga: `v_fin`, y su máximo) dice en qué línea del VBlank se acaba. Pasar de 37
   es escribir con la PPU dibujando.
2. Añade las copias **de una en una** y recuerda que cada 64 B son ~0,44 líneas.
3. Pon lo prescindible (el HUD de depuración) **al final de la lista**, y
   `HUD_LINEA_MAX` (`hud_debug.c`) lo retira ya en los frames cargados.
4. Si una escritura «no se dibuja nunca», sospecha primero de la **dirección de
   VRAM y el banco** (la entrada guarda el banco aparte; en la spec §9.2 los
   ninjas salieron a trozos por no guardar el banco de su tabla de piezas), y
   solo después del presupuesto.

## La librería: PVSnesLib

La capa de sprites del beat 'em up **sustituyó casi toda la librería** por
código propio (por el coste de la sección anterior). Lo que sigue usando:

| Función (firma exacta) | Qué hace / qué toca | Dónde se usa |
|---|---|---|
| `void oamInitGfxAttr(u16 address, u8 oamsize);` ([sprite.h](https://alekmaul.github.io/pvsneslib/sprite_8h.html)) | fija solo la base de los tiles de sprite y los dos tamaños (`OBSEL $2101`); `OBJ_SIZE16_L32` = 16×16 pequeño y 32×32 grande | `prueba_carga_ejecutar` |
| `void oamSetEx(u16 id, u8 size, u8 hide);` | tamaño (`OBJ_SMALL`/`OBJ_LARGE`) y visibilidad de una entrada, en la tabla alta del búfer (WRAM) | `sprites_iniciar` |
| `void oamSetXY(u16 id, u16 xspr, u16 yspr);` | solo la posición de una entrada (búfer, WRAM) | `sprites_iniciar` (todas a `y = 224`) |
| `void oamSet(u16 id, u16 xspr, u16 yspr, u8 priority, u8 hflip, u8 vflip, u16 gfxoffset, u8 paletteoffset);` | una entrada entera: **la que cuesta ~5,4 líneas por pieza** (spec §9.1); la capa nueva NO la usa, el nivel de plataformas aún sí | `tarou_sprite.c` (plataformas) |
| `void dmaCopyVram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | copia a la VRAM por el canal 0 con `VMAIN = 0x80` | `prueba_carga_ejecutar` (la sombra, con la pantalla apagada) |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` | copia a la CGRAM (paletas de sprite desde la entrada 128) | `prueba_carga_ejecutar` |
| `void setMode(u8 mode, u8 size);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | modo de vídeo y banderas (`BG3_MODE1_PRIORITY_HIGH`); `BGMODE $2105` | `prueba_carga_ejecutar` |
| `void setBrightness(u8 level);` | brillo; con 0 es *forced blank* | `prueba_carga_ejecutar` |
| `void bgSetScroll(u8 bgNumber, u16 x, u16 y);` ([background.h](https://alekmaul.github.io/pvsneslib/background_8h.html)) | scroll de un fondo, en el acto | `prueba_carga_ejecutar` |
| `void setModeHdmaColor(u8 channel, u8* hdmatable);` | degradado del color 0 de la CGRAM por HDMA (el cielo, canal 6) | `prueba_carga_ejecutar` |
| `void bgSetEnable(u8 bgNumber);` / `bgSetDisable` | enciende y apaga una capa (`TM $212C`): el HUD de BG3 | `hud_debug.c` |
| `void WaitForVBlank(void);` ([interrupt.h](https://alekmaul.github.io/pvsneslib/interrupt_8h.html)) | duerme hasta la NMI | `prueba_carga_ejecutar` |
| `snes_vblank_count` (`extern u16`) | VBlanks transcurridos, también los perdidos | `prueba_carga.c` (`medir_cpu`, `frames_cfg`) |
| `void *memset(void *b, int c, size_t n);` (`string.h`, ensamblador) | rellena memoria a 3 instrucciones por byte | `hud_debug.c` |
| `REG_TS`, `REG_CGWSEL`, `REG_CGADSUB` (`*(vuint8 *)$212D/$2130/$2131`) | matemática de color: las sombras negras se suman a la mitad con BG1+BG2 (`0x50`, solo sprites 4..7) | `prueba_carga_ejecutar` |

Los registros `$2137`/`$213D` (`SLHV`/`OPVCT`) que lee `leer_linea` no tienen
envoltorio en la librería: el juego los define como `REG_SLHV` y `REG_OPVCT`
en `prueba_carga.c`. **Primero la librería, después el registro**: aquí se baja
al registro porque la librería no tiene función para leer la línea del haz.

Ejemplos del SDK equivalentes (léelos): `snes-examples/graphics/Sprites/ObjectSize`
(los dos tamaños de `OBSEL`), `.../Sprites/MetaSprite` (un personaje de varias
piezas con `oamSet`, el método que esta unidad deja atrás),
`.../Sprites/DynamicEngineMetaSprite` (el motor dinámico del SDK, también
sustituido) y `.../graphics/Effects/Transparency` (la matemática de color). El
índice común, `aprendizaje/pvsneslib.md`.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| La ROM de prueba: `CONFIGS`, `HUECOS`, `TOPE_TAROU` | `src/pantallas/prueba_carga.c:98`, `:135`, `:125` |
| Un frame de la prueba y sus tres puntos de medida | `frame()`, `prueba_carga.c:445` |
| Medir el haz: `leer_linea`, `desde_vblank`, `medir_cpu` | `prueba_carga.c:157`, `:166`, `:176` |
| Subir a Tarou (tope 1536) y a los ninjas (turno) | `subir_tarou` `:209`, `subir_ninjas` `:240` |
| Las entidades fijas y sus sombras | `preparar_entidades` `:316`, `mover_entidades` `:360` |
| Matemática de color de las sombras | `prueba_carga_ejecutar`, `prueba_carga.c:526-528` |
| La tabla de entidades (16 B) y `sprites_borde` | `src/juego/sprites.h` (`SpriteEnt`) |
| `sprites_iniciar`, `sprites_usar`, `sprites_ordenar` | `src/juego/sprites.c:31`, `:44`, `:59` |
| **Toda la OAM en una llamada** | `src/juego/sprites_oam.asm`: `sprites_dibujar:61`, `_rapida:133`, `_lenta:163`, `_bit8_x:237` |
| El plan de copias y su reparto | `src/juego/subida.c`: `subida_empezar:35`, `subida_tomar:65`, `subida_tile_pieza:106` |
| Leer el índice de una tira | `src/juego/p16.c:21` (`p16_cuadro`) |
| La lista de DMA y su tope de 32 | `src/juego/dma_lista.h` (`DMA_LISTA_MAX`), `dma_lista.c:29` |
| Pasar copias a la lista en un bucle | `src/juego/dma_copias.asm:38` (`dma_lista_copias`) |
| Dispararlas en el VBlank | `src/juego/dma_ejecutar.asm:70` (`dma_lista_ejecutar`) |
| Orden por profundidad | `src/juego/profundidad.c:22` |
| La capa ya en el juego (no solo en la prueba) | `src/juego/protagonista_sprite.c`, `src/enemigos/ninja_sprite.c`, `src/pantallas/pantalla_fase1.c` |
| El formato `.p16` y su conversión | `scripts/piezas16.py` (`construir_p16`, `piezas_no_vacias`) |
| Los ninjas provisionales y las predicciones | `scripts/preparar_prueba_carga.py` (`predecir`, `ranuras_por_linea`) |
| Los archivos de los ninjas (versionados) | `res/prueba_carga/*.bin`; `data.asm:318-334` |
| Las pruebas de PC | `tests/prueba_subida.c`, `prueba_p16.c`, `prueba_profundidad.c` |
| Resultados medidos en Mesen | `docs/diseno-beat-em-up.md` §9.1 y §9.2 |

> **La capa ya salió de la prueba.** Los comentarios de `sprites.h` y de
> `dma_lista.h` siguen diciendo «lo usa `prueba_carga.c`», pero
> `pantalla_fase1.c` usa ya la capa: `protagonista_sprite.c` sube a Tarou con el
> mismo `subida_tomar(..., TOPE_SUBIDA, ...)` y `ninja_sprite.c` sube **un ninja
> por frame, en rotación**, igual que la prueba. Lo que midió la prueba de carga
> es la base de ese código.

## La demo

`demo_21_presupuesto.c` enlaza el **código real** (`p16.c`, `subida.c`,
`profundidad.c`, `dma_lista.c`) y lee los **archivos reales**
(`ninja64_andar.bin`, `ninja64_ataque.bin`, versionados, y `tarou_caminar.p16`,
que genera `make`: si falta, `make res/personajes/tarou/tarou_caminar.p16`).
Desde la copia de esta carpeta dentro del clon:

```text
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_21.exe demo_21_presupuesto.c ../../src/juego/p16.c ../../src/juego/subida.c ../../src/juego/profundidad.c ../../src/juego/dma_lista.c
.\demo_21.exe
```

Salida real en `capturas/salida_demo_21_presupuesto.txt`. Lo que hay que ver:

- **Parte A.** El peor cuadro del ninja (el 3 de andar) son 21 piezas = 2688 B =
  6 copias; todos los cuadros reales necesitan 2 VBlanks con el tope de Tarou
  y 1 sin tope. Tarou andando: 14–16 piezas.
- **Parte B.** Los 16 pares y las piezas 16–23 del hueco 192 que llevan el bit 8.
- **Parte C.** Las seis configuraciones del script, **idénticas** a
  `capturas/prediccion_preparar_prueba_carga.txt`: 36 ranuras con 3 ninjas a la
  misma `y` (andando), líneas 168..175. Si no coinciden, algo está mal.
- **Parte D.** El orden de dibujo (`Tarou0 ninja1 ninja2 ninja3 sombras...`) y
  las líneas-pieza perdidas por entidad (modelo).
- **Parte E.** El VBlank con el modelo y qué copia cae fuera.
- **Parte F.** La lista llena: 35 apuntadas, 32 guardadas.

(Una salvedad de colocación: el script coloca a Tarou en `SUELO_Y − 64` y su
sombra en `SUELO_Y − 8`; la ROM, en `SUELO_Y − 64 + 2` y `pies − 10`. La demo
usa la colocación del script en la parte C, para poder compararla con él, y la
de la ROM en la D. Las 8 líneas «168..175» son las del script, no las líneas
exactas de la ROM.)

Antes de ejecutarla, intenta adivinar: ¿cuántos VBlanks tarda el peor cuadro del
ninja con el tope de Tarou? ¿Y cuántas ranuras tendría la línea de los pies con
Tarou + 2 ninjas y sombras?

## Ejercicio y criterio de aprobación

Tres partes (enunciados completos en `ejercicio_21.md`):

**Parte 1 (PC): `ejercicio_21_presupuesto.c`.** Cinco `TODO`: el tile de la pieza
`k` con desplazamientos, el plan de copias de un cuadro, cuántos VBlanks tarda con
un tope, las ranuras por línea de una escena inventada, y cuántos ninjas caben
con y sin sombras. Compila sin avisos tal como está; la salida esperada exacta
está en su cabecera y en `capturas/salida_ejercicio_21_presupuesto.txt`.

**Parte 2 (clon): pruebas.** Añadir a `tests/prueba_subida.c` los cuadros reales
(21 piezas del ninja, 16 de Tarou) y comprobar que pasan: **570 pruebas, 0
fallos** (560 antes).

**Parte 3 (clon): una configuración nueva.** Predecir con la demo si «3 ninjas a
la misma `y` **sin sombras**» cabe en 34 ranuras, añadirla a `CONFIGS` de la
prueba de carga (configuración 8) y compilar la ROM `-DPRUEBA_CARGA` sin avisos.

**Aprobación:**

1. la parte 1 compila con `gcc -std=c99 -Wall -Wextra` sin avisos y su salida es
   idéntica a la esperada;
2. `tests/pruebas.exe` da **570 pruebas, 0 fallos** en el clon con la parte 2;
3. la parte 3 compila (`No warnings or errors found.`) y la predicción escrita en
   el ejercicio (34 ranuras andando, 32 atacando: cabe) coincide con la de la
   demo;
4. (a mano, por el autor) con la ROM de la parte 3 abierta en Mesen, la
   configuración 8 se ve sin trozos perdidos en la línea de los pies. **Esta
   unidad no lo comprobó** (no se abrió Mesen).

La solución está en `solucion/` y sus salidas en `capturas/`.

## Lo que esta unidad no verificó (y diferencias con la ficha del plan)

- **Mesen.** Nada visual: ni las capturas, ni los máximos del HUD, ni la
  configuración 8. Es la verificación de la spec §9.2.
- **Cifras de CPU, VMAX y frames perdidos**: de la spec, no medidas aquí. La
  recta del VBlank es un modelo de tres puntos de la spec.
- **La ficha (escrita antes del código) y lo que hay.** *No existe*: la mini-ROM
  de ranuras por línea (se sustituyó por la demo de PC); las bandas de cine
  por IRQ de línea o HDMA sobre `INIDISP`; las **sombras a 30 Hz**; el
  **parpadeo rotatorio** (se descartó con razones, ver «Ranura por línea»).
  *Difiere*: Tarou gasta 10 ranuras, no 12; el protagonista, 192 tiles, no 256;
  «ninja de 96 tiles» pasó a 15–21 piezas (60–84 tiles); el HUD de la prueba no
  tiene `LIN`, tiene sus filas propias (`CFG`, `SOMB`, `OAM`, `CPU`, `V`,
  `PERD`, `BYTES`, `T`); `metasprite.c` no lo usa la capa nueva. *Sí está*: el
  turno rotatorio de subida (`turno`), la prueba con `#ifdef`, y la lista de DMA.
- **Incoherencias del código** (no corregidas aquí): la cabecera de
  `prueba_carga.h` dice 10 configuraciones y la tabla tiene 8; `dma_lista.h` y
  `sprites.h` no citan a `protagonista_sprite.c` ni a `ninja_sprite.c` como
  usuarios.

## Lo que viene

La unidad 22 estudia el mundo x/y/z y el carril que da sentido al orden por
profundidad de aquí, y la 24 los enemigos que salen en oleadas. El presupuesto
de esta unidad es el tope que ambas tienen que respetar: **cuántos enemigos a
la vez, de qué ancho y repartidos en profundidad** se decide con estas cuentas,
no al ojo.
