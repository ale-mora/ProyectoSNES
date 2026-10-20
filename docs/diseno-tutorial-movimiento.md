# Diseño — Nivel 1 como tutorial de movimiento

**Proyecto:** _Tarou: A Dog's Journey_ · **Etapa del plan:** 2 (Prototipo) ·
**Revisión:** 2 · **Fecha de esta versión:** 2026-10-20 · **Estado:** diseño
acordado en sesión de preguntas; fase 1 (prueba del motor de mapas) hecha.

El nivel 1 deja de ser un suelo plano de 512 px y pasa a ser un **tutorial**:
un recorrido de 8 pantallas (2048 px) en el que cada obstáculo enseña una
acción de Tarou, con carteles dibujados en el escenario, hasta una salida que
lleva al siguiente nivel. Parte de la Revisión 1 (`docs/diseno-demo-jugable.md`),
cuyas mecánicas no cambian: aquí se añade el **escenario con colisión**.

> **Vocabulario.** Los términos son los de `docs/glosario-snes.md`. Nuevos en
> este documento: *metatile*, *AABB*, *punto de control*, *plataforma de un
> solo sentido*, *streaming de columnas* (se añaden al glosario en la fase 8).

## 0. Historial

| Versión | Fecha | Qué decía |
|---|---|---|
| v1.1 | 2026-10-20 | No se usa el motor de mapas del SDK: en Tiled se pinta la **lógica** y un script la viste con el arte; el fondo se dibuja por columnas con código propio (C5, §3, §4). Celdas de 16 px; túnel y muro de 48 px, foso de 96. |
| v1 | 2026-10-20 | Diseño acordado y resultado de la prueba del motor de mapas (§5). |

## 1. Decisiones

**Del autor** (sesión del 2026-10-20):

| # | Tema | Decisión |
|---|---|---|
| 1 | Qué enseña | Todo el repertorio: un obstáculo por acción. |
| 2 | Final | Salida al siguiente nivel. |
| 3 | Cómo enseña | Carteles dibujados en el escenario, junto a cada obstáculo. |
| 4 | Fallar | Fosos y algo que hace daño, con reaparición. |
| 5 | Arte | Se reutiliza el tileset *Sidescroller* del suelo; se pide a Pixellab solo lo que falte. |
| 6 | Longitud | Unos 2048 px (8 pantallas), con el fondo dibujándose por columnas. |
| 7 | Editor | Tiled (lo instala el autor: `winget install thorbjorn.Tiled`). |

**De Claude** (el autor puede corregirlas):

| # | Tema | Decisión |
|---|---|---|
| C1 | Siguiente nivel | No existe nivel 2: una pantalla nueva `PANTALLA_PROXIMAMENTE` muestra «LEVEL 2 / COMING SOON» y vuelve al título. |
| C2 | Daño | Sin vida ni animación de herida: pinchos o foso → fundido y reaparición en el último punto de control (uno por cartel). |
| C3 | Carteles | Tablero de Pixellab (provisional: marcador generado); las letras las compone un script con la fuente de consola. |
| C4 | Rompible | Un tronco como **sprite** con su caja, sólido hasta que lo toca la caja `GOLPE` (el motor de mapas lee el mapa desde la ROM: no se pueden cambiar tiles, §5). |
| C5 | Motor | **Sin el motor de mapas del SDK** (v1.1). En Tiled el autor pinta la *lógica* (sólido, plataforma, zarza) con un tileset de 3 tiles de 16×16 y coloca objetos; `scripts/preparar_nivel.py` la **viste** con el tileset de Pixellab, que es de rejilla doble (cada celda de arte se elige según cuáles de las 4 celdas de lógica que toca son sólidas). El fondo se dibuja por columnas con un DMA propio (`src/juego/fondo_nivel.c`) y la colisión la resuelve un módulo de lógica pura sobre la rejilla generada. Tiled sigue siendo la única fuente, sin los formatos de `tmx2snes` ni la cámara del motor. |
| C6 | Pintar en Tiled | El autor eligió Tiled pensando en pintar el arte; con C5 pinta la lógica (qué es suelo, zarza o plataforma) y el script pone el arte. |

## 2. El recorrido

Dimensionado con la física real (`src/juego/tarou.h`): salto de unos 69 px y
unos 48 cuadros en el aire; caminar 1 px/cuadro, correr 2 px/cuadro; cuerpo de
52×50 px de pie y 36 px agachado. La rejilla es de celdas de **16 px** (la del
arte de Pixellab), así que las medidas son múltiplos de 16: el túnel y el muro
quedan en 48 px y el foso en 96 px.

| # | Pantalla | Cartel | Obstáculo | Qué se exige |
|---|---|---|---|---|
| 1 | 0 | `← → WALK` | Llano | — |
| 2 | 1 | `B: JUMP` | Escalón de 16 px y luego uno de 32 px | — |
| 3 | 2 | `HOLD B: JUMP HIGHER` | Muro de 48 px | un salto corto no llega |
| 4 | 3 | `DOWN: CROUCH` | Túnel de 48 px de alto y 96 px de largo | de pie no cabe; bajo el techo, soltar Abajo **no** levanta a Tarou |
| 5 | 4 | `HOLD Y: RUN` | Foso de 96 px | caminando no se cruza; corriendo sí |
| 6 | 5 | `MIND THE THORNS` | Zarzas (`T_SPIKE`) con plataformas de un solo sentido (`T_PLATE`) encima | tocar zarza = reaparecer |
| 7 | 6 | `R: SWORD  A: ATTACK` | Tronco rompible que bloquea el paso | sin espada fuera, el tronco no se rompe |
| 8 | 7 | `EXIT →` | Salida | lleva a `PANTALLA_PROXIMAMENTE` |

## 3. Tubería de datos

```
Tiled (el autor): res/nivel1/tutorial.tmj   (capa «logica» de 128x14 celdas de 16 px
     │                                       con res/nivel1/logica.png; capa «objetos»)
     ▼
scripts/preparar_nivel.py  ◄── Pixellab Sidescroller, seto del título, fuente de consola
     ├─► res/nivel1/fondo_cerca.png (2048x256) ─► gfx4snes -m ─► .pic .pal .map
     │                                              └─► scripts/mapa_a_columnas.py ─► .col
     ├─► res/nivel1/colision.bin (1792 bytes)  ─► data.asm ─► src/juego/colision.c
     └─► res/nivel1/nivel1_datos.h (objetos; la rejilla también, para el PC)
```

Tiled es la **única fuente** del nivel: el dibujo y la colisión salen del mismo
`.tmj`. La primera versión la escribe `python scripts/preparar_nivel.py --crear`;
después el autor la retoca en Tiled y vuelve a lanzar el script sin `--crear`.
El script se niega a escribir si la capa pasara de `LIMITE_TILES` (460) tiles
distintos; el nivel inicial usa 234 (gfx4snes cuenta 251).

## 4. Dibujo por columnas (sin cambiar la VRAM)

El reparto de la VRAM no cambia (v1 lo movía por el motor del SDK). La capa
cercana sigue en `VRAM_MAPA_A`, un mapa de 64×32 tiles (512 px) que se usa como
**anillo**: la columna `c` del nivel va a `VRAM_MAPA_A + nivel_columna_vram(c)`
= `((c & 32) << 5) | (c & 31)`, y el scroll de la PPU da la vuelta a los 512 px,
así que coincide. En cada VBlank, `fondo_nivel_vblank` escribe las columnas
`(camara >> 3) + 33` y `(camara >> 3) − 1`: a 2 px por cuadro como mucho, nunca
se salta una. Cada columna es un DMA de 64 bytes (canal 0) con `VMAIN = 0x81`
(avanzar 32 palabras: bajar una fila); después se deja en 0x80, que es lo que
esperan las funciones del SDK (`dmaCopyVram` fija 0x80, por eso no sirve aquí).
El banco del mapa en la ROM lo da `data.asm` (`nivel1_cerca_col_banco: .db
:nivel1_cerca_col`), porque el DMA lo necesita aparte. Verificado en Mesen el
2026-10-20: la cámara pasa de 256 (el límite viejo) a 356 y los obstáculos
entran sin cortes.

## 5. Fase 1: prueba del motor de mapas (2026-10-20)

Mini-ROM hecho con la receta de `aprendizaje/PLAN.md` §6.3 a partir de
`snes-examples/graphics/maps/tiled`, con el mapa ensanchado a **256×28 tiles**
(2048×224 px), la capa cercana en 0x6800 (SC_64x32) y una capa de texto en
BG3 con su mapa en 0x7800. Un «jugador» falso recorre el mapa a 2 px/cuadro y
`mapUpdateCamera` lo sigue. Capturas con `scripts/capturar_mesen.ps1 -Rom`.

| Pregunta | Resultado |
|---|---|
| ¿Conviven el mapa en 0x6800 y el texto en 0x7800? | **Sí.** El mapa se dibuja sin corrupción y el texto aparece encima. |
| ¿Se desplaza un mapa de 2048 px? | **Sí**, a 2 px/cuadro, sin costuras visibles (el ejemplo `slopemario` usa además 300 tiles de ancho). El `.m16` mide 14 342 bytes (256×28×2 + 6 de cabecera): cabe holgado en los bancos 14–15, hoy libres. |
| ¿Límite de tiles? | El índice de tile del mapa es de 10 bits (1024), pero nuestro presupuesto real es la zona `VRAM_FONDO_A`: **512 tiles de 4 bpp**. El `.b16`/`.t16` miden 2 bytes por tile del tileset. |
| ¿Cámara? | `x_pos` es el borde izquierdo de la cámara; los sprites se dibujan en `x_mundo − x_pos` (como LikeMario). Su zona muerta y que la cámara llegue al final se miden en la fase 2 con el HUD de depuración: el texto de la prueba salió con los caracteres desplazados (fuente por defecto del SDK) y no sirvió para leer números. |
| ¿Se pueden cambiar tiles en marcha? | **No por la API** (`map.h` no tiene ninguna función de escritura; `mapLoad` recibe el mapa en ROM). Por eso el tronco es un sprite (C4). |

**Por qué al final no se usa** (v1.1): el arte de Pixellab es de rejilla doble
(16 px, desplazada media celda) y pintarlo a mano en Tiled, en tiles de 8×8 con
sus atributos, era lento y frágil; generarlo por script exigía escribir los
formatos de `tmx2snes` (cuyo soporte de volteos se desconoce); y el motor trae
su propia cámara con zona muerta, distinta de `camara.c` y sus pruebas. Un DMA
de columnas propio son unas 40 líneas, se entiende entero y es material de la
unidad 20 de `aprendizaje/`.

**Trampa encontrada:** `tmx2snes` lee el JSON de Tiled con *cute_tiled*, que
falla («Invalid integer found during parse») si hay un **espacio después de
los dos puntos** (`"width": 256`). Tiled exporta sin espacios; un script que
escriba `.tmj` debe usar `json.dump(..., separators=(",", ":"))`.

## 6. Fases

Un commit por fase, sin push. Detalle en el plan aprobado:

1. Prueba del motor de mapas (hecha, §5).
2. Tiled (lógica) → `preparar_nivel.py` → capa de 2048 px dibujada por columnas (hecha, §4).
3. Colisión con el escenario (lógica pura, `src/juego/colision.c`, pruebas en el PC).
4. Fosos, zarzas y reaparición en puntos de control.
5. Tronco rompible (`src/objetos/`).
6. Carteles.
7. Salida y pantalla `PANTALLA_PROXIMAMENTE`.
8. Documentación (glosario, mapa de archivos, tubería, plan de desarrollo, `CLAUDE.md`, `aprendizaje/PLAN.md`).

## 7. Verificación

- Pruebas en el PC: `colision`, Tarou adaptado y un **recorrido completo**
  guionizado sobre la rejilla del nivel real que llega a la salida; y pruebas
  negativas: el foso no se cruza caminando, el muro no se salta con un salto
  corto, el túnel no se pasa de pie, el tronco bloquea hasta el golpe.
- Pruebas de los scripts con `unittest`.
- `make clean` + `make` sin avisos.
- Mesen con `scripts/capturar_mesen.ps1`, una captura por obstáculo, a 60 FPS.
- El autor juega el tutorial y retoca el mapa en Tiled.
