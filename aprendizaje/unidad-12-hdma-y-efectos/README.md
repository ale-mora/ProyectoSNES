# Unidad 12 — HDMA y efectos de la PPU

**Objetivo:** cambiar un registro de la PPU en cada línea de la pantalla sin
gastar CPU (el cielo degradado del título), usar los efectos de brillo y
mosaico de PVSnesLib sabiendo lo que cuestan, y entender el **motor de
escenas** del juego: una tabla de eventos que anima la pantalla sin esperas.

**Prerrequisitos:** 01 (punto fijo 8.8 y 12.4, el coste de `/`), 04 (frame,
VBlank, `WaitForVBlank`, forced blank), 06 (CGRAM, color 0, fondos, Mode 1).
**Práctica:** PC (`gcc`) + mini-ROM + sobre el juego real (en tu clon).
**Tiempo orientativo:** dos o tres tardes.

Orden de la unidad: primero **las funciones de la librería** (fundido,
mosaico, `setModeHdmaColor`), con la mini-ROM; después **los registros** que
hay detrás, y por qué el juego, en `src/escena/efectos.c`, baja a ellos con las
macros `REG_xxx` desde C; y al final el motor de escenas.

## Conceptos

### Registro de la PPU, y cambiarlo «a mitad de la imagen»

Un **registro** de la PPU es una dirección (`$2100`, `$2106`...) en la que la
CPU escribe un byte para cambiar cómo dibuja la PPU: el brillo, el mosaico, el
modo, el scroll... (unidad 06). La PPU dibuja la imagen **línea a línea**, de
arriba abajo (→ glosario: Línea del haz): si un registro cambia mientras dibuja,
las líneas de debajo salen con el valor nuevo. Eso es la base de todos los
efectos de esta unidad: un color de fondo distinto en cada franja es un
degradado; una ventana con los bordes distintos en cada línea es una diagonal.

Hacerlo con la CPU sería esperar a cada línea (63,5 µs) y escribir a tiempo, sin
hacer otra cosa en todo el frame. Para eso está el HDMA.

### Brillo (`INIDISP`) y forced blank: dos cosas distintas

El registro `INIDISP` (`$2100`, `REG_INIDISP`) tiene dos partes:

| Bits | Qué es | Valores |
|---|---|---|
| 0..3 | **brillo** | 0 (negro) .. 15 (normal) |
| 7 | **forced blank** (pantalla apagada) | 1 = la PPU no dibuja y la VRAM se puede escribir en cualquier momento |

Leído en el código fuente de PVSnesLib 4.6.0 (`pvsneslib/source/videos.asm`
de la etiqueta `4.6.0` en GitHub; `tools/` solo trae los `.obj`):

| Función | Escribe en `INIDISP` | Resultado |
|---|---|---|
| `setScreenOn()` | espera un VBlank y escribe `$0F` | encendida, brillo 15 |
| `setScreenOff()` | `$80` | **apagada** (forced blank) |
| `setBrightness(n)`, n = 1..15 | `n` | encendida, brillo n |
| `setBrightness(0)` | **`$80`** (caso especial: «if 0, force vblank») | **apagada** |
| `setFadeEffectEx(FADE_OUT, f)` | `$0F`, `$0E` ... `$00` | acaba en `$00`: **negro con la pantalla ENCENDIDA** |
| `setFadeEffectEx(FADE_IN, f)` | `$00` ... `$0F` | brillo 15 |

La diferencia importa: con la pantalla encendida (aunque se vea negra) la PPU
sigue dibujando, y lo que se copie a la VRAM fuera del VBlank **se pierde**. Es
el fallo del título al volver del nivel (2026-10-21): tras `fade_out` el título
cargaba los tiles de Tarou con la PPU dibujando negro y se veían los del nivel.
El arreglo está en `secuencia_cambiar` (`src/sistema/secuencia.c:50`):
`setScreenOff()` antes de cada `_iniciar` (→ glosario: Forced blank).

### Fundido (*fade*) y por qué bloquea

Un **fundido** es el brillo cambiando de 15 a 0 (salida) o de 0 a 15
(entrada) en 16 pasos. `setFadeEffectEx(modo, f)` espera `f` VBlanks antes de
escribir cada paso, así que dura **16 × f frames**: `fade_out(FRAMES_FADE)` del
juego (`FRAMES_FADE` = 2) son 32 frames (`src/sistema/comun.c:172-180`). Con
`f = 0` el contador de 8 bits da la vuelta: 256 VBlanks por paso.

Esas esperas las hace **dentro** de la función (llama a `WaitForVBlank`): el
programa no vuelve hasta que el fundido termina. Durante esos 32 frames tu
bucle no corre: ni lógica, ni mando, ni animación. La mini-ROM lo enseña con
dos contadores: `LOOP` (vueltas de nuestro bucle) se queda quieto mientras
`VBL` (`snes_vblank_count`, VBlanks reales) sube. Por eso el motor de escenas
existe (más abajo).

### Mosaico (`MOSAIC`, `$2106`)

El **mosaico** agranda los píxeles de los fondos elegidos hasta volverlos
bloques (→ glosario: Mosaico). El registro es un byte:
`(tamaño << 4) | fondos`: el tamaño (0 = nítido .. 15 = bloques de 16×16) en
los 4 bits altos y un bit por fondo en los 4 bajos (bit 0 = fondo 0...).

`setMosaicEffect(modo, fondos)` lo anima en 16 pasos de **3 VBlanks** (48
frames, también bloqueante). Leído en el fuente: `MOSAIC_OUT` hace crecer los
bloques (0 → 15) y `MOSAIC_IN` los deshace (15 → 0). La documentación del SDK
(y un comentario del propio fuente) lo dicen al revés: manda el código.

**No es un fallo:** la salida de la intro (`src/pantallas/pantalla_intro.c:152`)
llama a `setMosaicEffect(MOSAIC_OUT, ...)`: los bloques crecen hasta 16×16 y
**así se quedan** durante el `fade_out` que sigue; en negro, `REG_MOSAIC = 0`
lo apaga. Lo que se ve (la imagen deshaciéndose en cuadrados y fundiéndose) es
el efecto buscado.

### DMA y HDMA; canal

El **DMA** copia un bloque de memoria a un registro de la PPU (la VRAM, la
CGRAM...) mucho más rápido que la CPU (unidad 06). La consola tiene **8
canales** (0..7): ocho juegos de registros `$43x0..$43xA` (x = canal), cada uno
con su origen, destino y modo.

El **HDMA** (*horizontal-blank DMA*) usa los mismos canales de otra manera: en
vez de copiar un bloque de golpe, escribe **unos pocos bytes al final de cada
línea**, en el HBlank (el instante en que el haz vuelve de la derecha a la
izquierda para empezar la línea siguiente). Lo que escribe lo saca de una
**tabla** en memoria que el canal recorre solo (→ glosario: DMA / HDMA).

```
   frame:  línea 0 ─────────────────────────────── HBlank ◄─ el canal 6 escribe
           línea 1 ─────────────────────────────── HBlank ◄─ (si toca entrada nueva)
             ...                                               $2121, $2121, $2122, $2122
           línea 223 ───────────────────────────── HBlank
           VBlank (líneas 225..261); al empezar el frame siguiente, antes de la línea 0,
           el canal vuelve a leer su tabla desde el principio
```

Registros de un canal (para el canal 6, `REG_DMAP6`, `REG_BBAD6`... en `dma.h`):

| Registro | Qué guarda |
|---|---|
| `DMAP` `$43x0` | el **modo**: cuántos bytes y a qué registros (modo 1 = 2 bytes a `p`, `p+1`; modo 3 = 4 bytes a `p`, `p`, `p+1`, `p+1`) y, con el bit 6, **indirecto** |
| `BBAD` `$43x1` | el registro de la PPU de destino `p` (`$21pp`): `$21` = `CGADD`, `$26` = `WH0` |
| `A1T` `$43x2-3`, `A1B` `$43x4` | dirección de 16 bits y banco de la **tabla** |
| `DASB` `$43x7` | banco de los **datos** en modo indirecto |
| `HDMAEN` `$420C` | un bit por canal: qué canales hacen HDMA |

### La tabla HDMA

Una **tabla HDMA** es una lista de **entradas**. Cada entrada empieza por un
byte de **cuenta**:

- `1..127`: «durante estas líneas, el valor es este»: el canal escribe los bytes
  de la entrada una vez y los deja así N líneas.
- `$80 | n` (bit 7 a 1): **modo repetir**: n líneas con un valor **nuevo en
  cada una** (la entrada lleva n juegos de bytes).
- `0`: fin de la tabla: el canal se para hasta el frame siguiente.

`setModeHdmaColor` programa el canal en modo 3 con destino `$21`: cada entrada
son **5 bytes**: la cuenta y cuatro bytes que van a `$2121, $2121, $2122,
$2122` (`CGADD` = 0 dos veces y los dos bytes del color en `CGDATA`). Es decir,
cambia el **color 0 de la CGRAM**, el color de fondo que se ve donde todas las
capas son transparentes. Así empieza la tabla real del cielo del título
(`res/titulo/cielo_grad_data.as`, que genera `gfx4snes -n` desde `cielo.bmp`):

```
 .db $08,$00,$00,$A2,$3C, ...
      │   │   │   └───┴── color $3CA2 = R 2, G 5, B 15 (5 bits cada uno)
      │   └───┴────────── CGADD = 0, dos veces (el color 0)
      └────────────────── 8 líneas
```

En la mini-ROM la tabla se escribe a mano con una macro, `FRANJA(lineas, r, g,
b)` (`rom/src/cielos.h`), para ver cada byte.

**Cuándo se ve lo que escribe: líneas del haz y filas de la imagen.** Hay
que distinguir dos numeraciones:

- La **línea del haz** (*scanline*), la que cuentan el HDMA y `$213D`: 0..261.
  La línea 0 se dibuja pero **no se ve**: la imagen de 224 líneas son las
  líneas del haz **1..224**.
- La **fila de la imagen** (0..223), la de una captura de Mesen: la fila `r` es
  la línea del haz `r + 1`.

El canal escribe en el HBlank del **final** de una línea, así que una entrada
que empieza «después de n líneas» de tabla se escribe al final de la línea del
haz n y se ve desde la línea del haz **n + 1**... que es la fila **n** de la
imagen. Los dos desfases se anulan: en la imagen, la entrada cae justo donde
dice la suma de las cuentas. Medido en la parte A del ejercicio (Mesen,
2026-10-22): las entradas antes de la línea blanca suman 120 y la línea blanca
sale en la fila **120** de la captura.

¿Y el desfase de una línea que el proyecto encontró en el destello del logo
Nintendo? No era del HDMA sino del **fondo**: con el scroll vertical a 0, la
línea del haz `s` enseña la fila `s` del fondo, así que la fila `r` de la
imagen enseña la fila **`r + 1`** del fondo (la fila 0 del fondo cae en la
línea 0, la que no se ve). Una ventana por HDMA que empieza en la fila 64 de
la imagen cubre la fila 65 del fondo: para cuadrar con un dibujo del fondo
hay que empezar una línea antes. Por eso `src/escena/efectos.c:96` hace
`tabla[0] = arriba - 1` (raya gris de 1 px bajo el logo, medida en Mesen el
2026-10-21). La regla práctica: **mídelo siempre en Mesen**.

**La última línea de la pantalla.** En las capturas de la mini-ROM, la fila
223 de la imagen sale del color de la tabla del cielo, debajo del suelo de las
montañas. Es el mismo efecto: la fila 223 de la imagen enseña la fila **224**
del fondo, y una imagen de 224 px (mapa de 32×28 en un mapa de 32×32) tiene
vacías las filas 224..255: tiles transparentes, así que se ve el color de fondo,
que es el que pone el HDMA. El remedio clásico es un **scroll vertical de −1**
(`bgSetScroll(n, x, 0xFFFF)`; el registro guarda 10 bits, `$3FF`): la fila
`r` de la imagen pasa a enseñar la fila `r` del fondo, la 223 vuelve a ser
suelo y la 0 enseña la fila 0 (que antes no se veía). La mini-ROM lo aplica a
los fondos 0 y 1 a los 16 s del ciclo (o con L) y lo quita a los 22 s: compara
la última fila de la captura antes y después.

### HDMA indirecto

En el modo **indirecto** (bit 6 de `DMAP`), cada entrada de la tabla no lleva
los bytes sino un **puntero** de 2 bytes a ellos (el banco va en `DASB`). En
modo repetir, el canal lee un juego de bytes nuevo por línea **avanzando por la
memoria apuntada**. Mover el efecto entero es cambiar ese puntero: 2 bytes por
frame en vez de reescribir la tabla (→ glosario: HDMA indirecto).

### Ventanas, `COLDATA` y matemática de color

- **Ventana:** un rango de columnas `[izquierda, derecha]` (`WH0`/`WH1` para la
  ventana 1) dentro del cual la PPU puede recortar capas o aplicar un efecto.
  Si `izquierda > derecha`, la ventana está vacía. Con HDMA cambiando sus
  bordes en cada línea salen formas: si `izq = base − línea`, una diagonal
  `/` (→ glosario: Ventana).
- **`COLDATA`** (`$2132`): un **color fijo** de la PPU. Un byte con los bits
  7/6/5 diciendo a qué componentes (azul, verde, rojo) va el valor de 5 bits
  de abajo: `0xE0 | n` pone R = G = B = n (un gris).
- **Matemática de color:** la PPU puede **sumar o restar** a cada píxel otro
  color (→ glosario: Matemática de color). `CGWSEL` (`$2130`) dice **dónde**
  (`0x10`: solo dentro de la ventana de color) y con qué (el color fijo);
  `CGADSUB` (`$2131`) dice a **qué capas** y si suma o resta. `WOBJSEL`
  (`$2125`) elige qué ventana es la «ventana de color» (`0x20` = ventana 1).

Juntas: una ventana diagonal estrecha + sumar un gris solo dentro de ella = una
banda de luz que cruza el logo. Es el destello del logo Nintendo.

### Punto fijo 8.8 en una pista

Repaso de la unidad 01: un valor **8.8** guarda la parte entera en el byte alto
y la fracción en el bajo (256 = 1,0). Un brillo que sube de 0 a 15 en 16
frames suma `15 × 256 / 16 = 240` (0,94) por frame; la parte entera (`valor >>
8`) es lo que llega al registro.

### El motor de escenas: evento, pista, tabla

El **motor de escenas** (`src/escena/`, `docs/diseno-motor-escenas.md`) separa
**qué pasa y cuándo** de **cómo se dibuja** (→ glosario: Motor de escenas):

- Una **pista** (`Pista`, `src/escena/escena.h`) es un número que se mueve
  solo: valor 8.8, paso por frame, frames que le quedan y destino. Hay cuatro:
  `PISTA_BRILLO` (0..15), `PISTA_MOSAICO` (0..15), `PISTA_DESTELLO` (0..255,
  dónde está la banda) y `PISTA_LUZ` (0..31, cuánta luz suma).
- Un **evento** (`EventoEscena`, 8 bytes) dice «en el frame `cuadro`, la pista
  `pista` va hacia `hasta` en `frames` frames, sumando `paso`». Se escriben con
  tres macros: `RAMPA(cuadro, frames, pista, desde, hasta)`, `FIJAR(cuadro,
  pista, valor)` (de golpe) y `FIN_ESCENA(cuadro)` (la última).
- Una **escena** es una **tabla de eventos ordenada por frame**, que se lee
  como un guion. La del logo Nintendo (`src/escena/escena_nintendo.c:26`):

```c
    FIJAR(0, PISTA_MOSAICO, 15),             /* Arranca con bloques de 16x16...     */
    RAMPA(0, 48, PISTA_MOSAICO, 15, 0),      /* ...que se enfocan en 48 frames.     */
    RAMPA(0, 32, PISTA_BRILLO, 0, 15),       /* Fundido de entrada.                 */
    RAMPA(70, 56, PISTA_DESTELLO, 0, 255),   /* La banda cruza el logo.             */
    RAMPA(70, 14, PISTA_LUZ, 0, 14),         /* La luz se enciende al entrar...     */
    RAMPA(112, 14, PISTA_LUZ, 14, 0),        /* ...y se apaga antes de salir.       */
    RAMPA(212, 32, PISTA_BRILLO, 15, 0),     /* Fundido de salida.                  */
    FIN_ESCENA(ESCENA_NINTENDO_FRAMES),      /* 244.                                */
```

Cinco decisiones del motor que hay que entender:

1. **El paso lo calcula el compilador.** `RAMPA` es una macro y todos sus
   argumentos son constantes: la división `(hasta − desde) × 128 / frames × 2`
   se hace al compilar. En la SNES no hay ninguna división por frame (tcc816
   las hace con una rutina lenta, unidad 01).
2. **`× 128 … × 2` y no `× 256`:** el `int` de tcc816 mide **16 bits**, y
   `255 × 256 = 65280` no cabe (da −256). `255 × 128 = 32640` sí. Se pierde
   1/256 de precisión que da igual por la decisión 3.
3. **Valor exacto en el último frame:** cuando a una pista le queda 1 frame,
   `escena_avanzar` no suma el paso: pone `hasta << 8`. El brillo acaba en 15
   y no en 14,9 (`src/escena/escena.c`, paso 2).
4. **`<=` y no `==` al arrancar eventos:** un evento fuera de orden arranca
   tarde en vez de congelar la escena esperándolo (las pruebas exigen el orden).
5. **Lógica pura / registros:** `escena.c` no incluye `snes.h`: se compila
   también en el PC y tiene pruebas (`tests/prueba_escena.c`). Quien escribe
   los registros es `efectos.c`, **en el VBlank**.

Una pantalla usa el motor en sus tres momentos (unidad 04), como
`src/pantallas/pantalla_nintendo.c`:

```
_iniciar:     cargar fondos; efectos_preparar(FONDO_LOGO); efectos_destello(...);
              escena_empezar(&escena, ESCENA_NINTENDO);          (todas las pistas a 0)
_actualizar:  escena_avanzar(&escena);
              if (escena.terminada) { efectos_terminar(); secuencia_cambiar(...); }
_vblank:      efectos_aplicar(&escena);                          (los valores, a la PPU)
```

Las pistas empiezan en 0, así que la pantalla arranca **apagada**
(`setBrightness(0)` es forced blank, ver arriba): la propia escena hace el
fundido de entrada, sin bloquear nada.

## La librería: PVSnesLib

Versión 4.6.0 (la del repo). Firmas copiadas de
`tools/pvsneslib/pvsneslib/include/snes/video.h` y `dma.h`; el comportamiento
de las marcadas con † se comprobó en su fuente (`videos.asm`, `dmas.asm` de la
etiqueta 4.6.0 en GitHub), porque la documentación no lo dice o lo dice mal.

| Función (cabecera) | Qué hace | Hardware |
|---|---|---|
| `void setBrightness(u8 level);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) † | brillo 1..15; **con 0 apaga la pantalla** (`$80`) | `INIDISP $2100` |
| `void setScreenOn(void);` / `void setScreenOff(void);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) † | encender (espera un VBlank, brillo 15) / forced blank | `INIDISP` |
| `void setFadeEffect(u8 mode);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) † | fundido `FADE_IN`/`FADE_OUT`, 1 VBlank por paso (16 frames), **bloquea** | `INIDISP` |
| `void setFadeEffectEx(u8 mode, u8 framesNumber);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) † | lo mismo con `framesNumber` VBlanks por paso (16 × f frames); `FADE_OUT` acaba en `$00`, pantalla encendida | `INIDISP` |
| `void setMosaicEffect(u8 mode, u8 bgNumbers);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) † | `MOSAIC_OUT`: bloques 0 → 15; `MOSAIC_IN`: 15 → 0; 3 VBlanks por paso (48 frames), **bloquea**; `bgNumbers` = `MOSAIC_BG1..BG4` | `MOSAIC $2106` |
| `void setColorEffect(u8 colorMathA, u8 colorMathB);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | escribe los dos registros de la matemática de color | `CGWSEL $2130`, `CGADSUB $2131` |
| `void setColorIntensity(u8 colorApply, u8 intensity);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | el color fijo: componentes (`colorApply`) e intensidad 0..15 | `COLDATA $2132` |
| `void setModeHdmaColor(u8 channel, u8* hdmatable);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) † | degradado del **color 0** con una tabla de entradas de 5 bytes (modo 3, destino `$21`); apaga **todos** los canales, espera dos interrupciones (`wai`) y enciende solo el que se le pasa | canal elegido `$43x0-$43x4`, `HDMAEN $420C`, `CGADD/CGDATA` |
| `void setModeHdmaReset(u8 channels);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) † | escribe `channels` **tal cual** en `HDMAEN`: `setModeHdmaReset(0)` apaga **todos** | `HDMAEN` |
| `void setModeHdmaWindowReset(u8 channels);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) † | lo mismo y además `TMW = 0` (quita las ventanas de las capas) | `HDMAEN`, `TMW $212E` |
| `void setModeHdmaGradient(u8 maxLevels);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) † | degradado de **brillo** por franjas (canal 3; calcula la tabla con `tcc__div`) | `INIDISP` por HDMA |
| `void setModeHdmaShadeUpDown(void);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | brillo que crece hacia el centro (canal 3) | `INIDISP` por HDMA |
| `void setParallaxScrolling(u8 bgrnd);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) † | scroll horizontal distinto por franjas, tabla en `HDMATable16` (canal 3) | `BGnHOFS` por HDMA |
| `void setModeHdmaWaves(u8 channel, u8 bgrnd);` / `void setModeHdmaWavesMove(void);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | ondas (HDMA indirecto) y su animación por frame | `BGnHOFS` por HDMA |
| `void setModeHdmaWindow(u8 channels, u8 bgrnd, u8 bgrndmask, u8* hdmatableL, u8* hdmatableR);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | ventana con forma: bordes izquierdo y derecho por línea (2 canales) | `WH0/WH1`, `W12SEL`, `TMW` |

Ojo con los canales: `setModeHdmaGradient`, `setModeHdmaShadeUpDown` y
`setParallaxScrolling` usan **los tres el canal 3** (no pueden ir a la vez), y
`setModeHdmaColor` apaga todo canal que no sea el suyo.

Ejemplos del SDK (`tools/pvsneslib/snes-examples/graphics/Effects/`):
`GradientColors` (la base de la mini-ROM: `setModeHdmaColor` con una tabla de
`gfx4snes -n`), `HDMAGradient` (`setModeHdmaGradient` en Mode 3), `Fading`
(`setFadeEffect`/`setFadeEffectEx`), `Mosaic` (`setMosaicEffect`),
`ParallaxScrolling`, `Window`, `WindowCircular`, `Waves`, `Transparency`,
`TransparentWindow`.

## Cómo funciona: del SDK al registro

Con la librería se hacen el cielo del título y del nivel
(`setModeHdmaColor(HDMA_CHANNEL6, hdmaGradcieloList)` en
`pantalla_titulo.c:84` y `pantalla_nivel1.c:180`; `setModeHdmaReset(0)` al
salir) y los fundidos de casi todas las pantallas (`fade_in`/`fade_out`).

El logo Nintendo, en cambio, baja a los registros en `src/escena/efectos.c`.
Por qué:

1. **Nada bloquea.** `setFadeEffectEx` y `setMosaicEffect` se comen 32-48
   frames enteros. El motor mueve brillo, mosaico y luz **a la vez**, cada uno
   a su ritmo, escribiendo un byte por registro y frame en el VBlank.
2. **La librería no tiene el efecto.** No hay ninguna función para una ventana
   cuyos bordes salgan de una lista **indirecta** que se mueve cambiando 2
   bytes. Hay que programar el canal a mano: `REG_DMAP6 = 0x41` (indirecto,
   modo 1: dos bytes, a `WH0` y `WH1`), `REG_BBAD6 = 0x26`, la tabla en
   `REG_A1T6LH`/`REG_A1B6` y el banco de los datos en `$4367`, que el SDK **no
   define**: `efectos.c:47` lo define él, `#define REG_DASB6 (*(vuint8 *)0x4367)`.
3. **Es C igualmente.** Todo con las macros `REG_xxx` de `dma.h`, `video.h` y
   `background.h` (`REG_WOBJSEL`, `REG_CGWSEL`, `REG_CGADSUB`, `REG_COLDATA`,
   `REG_MOSAIC`, `REG_HDMAEN`...): una asignación en C es un `sta` en
   ensamblador. Para el brillo sí usa la librería: `setBrightness`.

`efectos_destello` (`efectos.c:63`) calcula **una vez** una lista de 256 + 64
pares (izquierda, derecha); el par `j` vale `izq = 255 + x_min − ancho − j`,
recortado al rectángulo del logo, o el par vacío (255, 0) fuera. La tabla del
canal 6 tiene tres entradas: `arriba − 1` líneas con el par vacío, `0x80 |
alto` líneas en modo repetir (un par por línea, desde el puntero que se mueve)
y 1 línea vacía, y el 0 final:

```
 tabla:  [arriba-1][ptr → par vacío]  [0x80|alto][ptr → pares + 2 + 2·(255 − destello)]  [1][ptr → vacío]  [0]
                                                   └── efectos_aplicar solo reescribe estos 2 bytes
```

Como la línea `y` del rectángulo lee el par `inicio + y`, cada línea queda 1 px
a la izquierda de la de arriba: una banda `/`. `efectos_aplicar` (`efectos.c:125`)
pone el brillo (`setBrightness`), el mosaico (`REG_MOSAIC = (tamaño << 4) |
fondos`), la luz (`REG_COLDATA = 0xE0 | luz`) y el puntero. `efectos_terminar`
(`efectos.c:142`) apaga HDMA, matemática de color, ventana y mosaico: **se
llama antes de cambiar de pantalla**, porque el título usa el mismo canal 6 para
su cielo y heredaría la ventana y la luz.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| El secuenciador: `escena_empezar`, `escena_avanzar` (los tres pasos) | `src/escena/escena.c:27`, `:44` |
| `RAMPA` (`* 128 / frames * 2`), `FIJAR`, `FIN_ESCENA`, `Pista` de 8 bytes, `ESCENA_VALOR` | `src/escena/escena.h:79-111` |
| La tabla del logo (244 frames = 32 + 180 + 32) | `src/escena/escena_nintendo.c:26`, `escena_nintendo.h` |
| Los registros: `efectos_preparar`, `efectos_destello` (pares + tabla indirecta del canal 6), `efectos_aplicar`, `efectos_terminar` | `src/escena/efectos.c:56`, `:63`, `:125`, `:142` |
| La pantalla que usa el motor: `_iniciar`/`_actualizar`/`_vblank`, rectángulo del logo (`LOGO_ARRIBA` 64, `LOGO_ALTO` 59) | `src/pantallas/pantalla_nintendo.c:42-91` |
| `secuencia_vblank` despacha el `_vblank` del logo | `src/sistema/secuencia.c` (final del archivo) |
| Las pruebas del motor y de la tabla real (orden, duración, sin saltos, todo apagado al final) | `tests/prueba_escena.c` |
| HDMA del cielo con la librería, canal 6 directo; `setModeHdmaReset(0)` al salir | `src/pantallas/pantalla_titulo.c:52`, `:84`; `pantalla_nivel1.c:125`, `:180` |
| La tabla del cielo y su regla `gfx4snes -n -t bmp` | `res/titulo/cielo.bmp`, `res/titulo/cielo_grad_data.as`, `Makefile:118` |
| El mosaico de salida de la intro (no es un fallo) | `src/pantallas/pantalla_intro.c:149-154` |
| `fade_in`, `fade_out` (16 × f frames, bloquean) | `src/sistema/comun.c:167-180` |
| `setScreenOff` antes de cada `_iniciar` | `src/sistema/secuencia.c:44-50` |
| `reaparecer`: fundido, pantalla apagada (reescribe 4 KB fuera del VBlank), fundido | `src/pantallas/pantalla_nivel1.c:132-151` |

## Trampas

- **Brillo 0 no es pantalla apagada... salvo con `setBrightness(0)`.** El
  final de `fade_out` deja `INIDISP = $00` (negro, encendida); `setBrightness(0)`
  y `setScreenOff()` escriben `$80` (apagada). Tras un `fade_out`, cargar la
  VRAM fuera del VBlank pierde datos.
- **Los fundidos y el mosaico de la librería bloquean** 32 y 48 frames.
- **El HDMA escribe al final de la línea**: una entrada que empieza tras n
  líneas de tabla se ve desde la línea del haz n + 1, que es la **fila n** de
  la imagen. El desfase de una línea aparece al cuadrar el HDMA con un
  **fondo** con scroll vertical 0 (la fila r de la imagen enseña la fila r + 1
  del fondo). Mídelo en Mesen (lo hace la parte A del ejercicio).
- **Con scroll vertical 0, la última fila de la imagen es la fila 224 del
  fondo**: en una imagen de 224 px está vacía y se ve el color de fondo (el del
  HDMA, si lo hay). Remedio: scroll vertical −1 (`0xFFFF`).
- **Un ciclo contado en vueltas del bucle no es un reloj.** La primera versión
  de la mini-ROM contaba los segundos en vueltas (60 = 1 s), pero un
  `consoleDrawText` con dos `%d` en cada vuelta tardaba más de un frame: el
  bucle iba a **30 Hz** (medido en las capturas: entre dos capturas a 3,5 s,
  `VBL` subió 188 y `LOOP` 94) y además los efectos bloqueantes no cuentan
  vueltas. Todo salía tarde (el mosaico «de los 4 s» salió hacia `VBL` 360..450, ~6..7,5 s). Para tiempos
  de verdad, `snes_vblank_count`; y el texto con números, no en cada frame.
- **El mosaico de salida de la intro no es un fallo.**
- **`255 * 256` no cabe** en el `int` de 16 bits de tcc816: por eso `RAMPA`
  reparte la cuenta (`* 128 … * 2`). En gcc (`int` de 32 bits) no se nota: la
  demo lo imita con `int16_t`.
- **`efectos_terminar()` antes de cambiar de pantalla**: si no, la siguiente
  hereda ventana, luz y mosaico (el título usa el mismo canal 6).
- **`setModeHdmaColor` apaga los demás canales**, y `setModeHdmaReset(n)`
  escribe `n` en `HDMAEN` (no «resetea el canal n»): `setModeHdmaReset(0)`
  apaga todos.
- **El `desde` de una `RAMPA` tiene que ser el valor que la pista tenga en ese
  frame**: si no, la rampa va con otra velocidad y al final da un salto. La
  prueba «sin saltos» de `tests/prueba_escena.c` lo detecta.
- Las `static` de `efectos.c` se inicializan en `efectos_preparar`: la WRAM no
  empieza a cero (unidad 13).

## Cómo se trabaja esta unidad

### 1. La demo de PC: el motor, frame a frame

`demo_12_escena.c` enlaza el `escena.c` **real** del juego con una tabla
pequeña y escribe cada frame el valor 8.8 de dos pistas. Desde esta carpeta:

```
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_12.exe demo_12_escena.c ../../src/escena/escena.c
.\demo_12.exe
```

Salida de referencia en `capturas/salida_demo_12_escena.txt`. Fíjate en:

```
1. evento: frame  0, 16 frames, pista 0, hasta 15, paso   240 (0.938 por frame)
2.     0 |        240       0 |        3200      12
2.    15 |       3840      15 |           0       0   <- ultimo frame de una rampa: exacto
4. 255 * 256 en 16 bits = -256 (deberia ser 65280)
```

- El frame 0 ya muestra brillo 240 (0,94, entero 0) y mosaico 3200 (12,5): en
  el mismo frame se aplicó el `FIJAR` a 15 y la rampa dio su primer paso de
  −2,5.
- `15 × 128 / 16 × 2 = 240`; `16 × 240 = 3840 = 15 × 256`: aquí la cuenta es
  exacta. En la rampa de salida, 3 frames de 15 a 0: −1280 por frame y el
  tercero se pone a 0 exacto.

### 2. La mini-ROM: cielo por HDMA, fundido y mosaico

`rom/` parte de `snes-examples/graphics/Effects/GradientColors` (receta de
`PLAN.md` §6.3). Muestra las montañas del título (su cielo es transparente:
color 0) con el cielo degradado por el canal 6 detrás, y se anima **sola**, sin
teclas, en un ciclo de 22 s contado con `snes_vblank_count` (frames de verdad:
un efecto que bloquea no retrasa los siguientes). Los segundos son desde el
arranque: `VBL / 60`, el número que se ve en pantalla (la línea `LOOP n  VBL
m` se reescribe cada 16 vueltas). También responde al mando:

| Segundo (VBL) | Tecla | Qué se ve | Función |
|---|---|---|---|
| 0 | — | cielo de día degradado; `SKY TABLE: DAY`, `HDMA CH6: ON`, `BG VSCROLL: 0` (filas 10..12, bajo el logo); la fila 223 de la imagen, del color del cielo | `setModeHdmaColor` |
| 6 (360) | A | fundido a negro y vuelta, ~1,1 s (64 frames: `VBL` sube 64 más que `LOOP`) | `setFadeEffectEx(FADE_OUT/IN, 2)` |
| 9 (540) | B | mosaico que crece y se deshace en las montañas (no en el texto), ~1,6 s (96 frames: 16 tamaños × 3 VBlanks, dos veces) | `setMosaicEffect(MOSAIC_OUT/IN, MOSAIC_BG2)` |
| 13 (780) | X | cambia a la tabla del atardecer, `SKY TABLE: SUNSET` (en el esqueleto: cielo negro; en la solución, morado → naranja, la línea blanca en la fila 120 y naranja → amarillo) | `setModeHdmaColor` |
| 16 (960) | L | scroll vertical −1 en los fondos 0 y 1, `BG VSCROLL: -1`: el texto y las montañas bajan 1 px y la fila 223 pasa a ser suelo | `bgSetScroll(n, 0, 0xFFFF)` |
| 19 (1140) | Y | apaga el HDMA, `HDMA CH6: OFF`: el cielo queda **de un solo color**, el último que escribió el canal (el amarillo de la franja de abajo; en el esqueleto, negro) | `setModeHdmaReset(0)` |
| 22 (1320) | — | cielo de día, HDMA encendido, scroll 0; el ciclo vuelve a empezar (los siguientes, 22 s después de cada vuelta) | `setModeHdmaColor`, `bgSetScroll` |

Ojo: `MOSAIC_OUT` hace **crecer** los bloques (tamaño 0 → 15) y `MOSAIC_IN`
los deshace, al revés de lo que dice el comentario de `video.h` («IN = normal
to mosaic»). Comprobado en el código de la librería desensamblado de la ROM
(`setMosaicEffect`: con modo 1 escribe `$2106` con 0, `$10`, `$20`... cada 3
VBlanks).

Para capturarla sin teclas: la primera captura llega con el juego ya en marcha
(en la tanda del 2026-10-22, la primera mostraba `VBL 266`, unos 4,4 s), así
que con `-Duracion 22 -Cada 0.5` se ven el fundido (captura ~1,6..2,7 s), el
mosaico (~4,6..6,2), el atardecer (~8,6), el scroll −1 (~11,6), el cielo de un
color (~14,6) y la vuelta al día (~17,6). Lee siempre el `VBL` de la captura:
es el reloj.

Compilar desde `rom/` (PowerShell):

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
```

Resultado: `No warnings or errors found.` y `u12_cielo.sfc` (resumen en
`capturas/compilacion_rom.txt`). Ábrelo en Mesen. Después busca `cielo_dia` en
`u12_cielo.sym`: está en `$7F:0000`, en la **WRAM**. tcc816 deja las tablas
`const` **globales** en la WRAM (se copian al arrancar; unidad 13). Al HDMA le
da igual de dónde lea.

### 3. El ejercicio

`ejercicio_12.md`: parte A, el cielo de atardecer en la mini-ROM (con la línea
blanca de 1 px para medir el desfase del HDMA); parte B (ampliación), una
escena del motor para el logo del publisher, con su prueba en `tests/`.

## Ejercicio y criterio de aprobación

**Aprobación** (detalle en `ejercicio_12.md`):

1. La demo compila con `gcc -std=c99 -Wall -Wextra` sin avisos y su salida es
   idéntica a `capturas/salida_demo_12_escena.txt`.
2. **Parte A:** la mini-ROM con tu `cielos.c` compila sin avisos y una captura
   de Mesen enseña el cielo de atardecer (morado → naranja, con la línea
   blanca), con la fila de píxel medida en la que cae la línea blanca.
3. **Parte B (ampliación):** en tu clon, `tests/pruebas.exe` en verde con la
   prueba nueva (con la solución: **378 pruebas, 0 fallos**, 367 + 11), el
   juego compila (`No warnings or errors found.`), y en Mesen el logo del
   publisher se enfoca al entrar, se pixela al salir y la pantalla de los
   desarrolladores aparece **sin mosaico** (sin ventana ni luz heredadas), con
   el título todavía hacia el segundo 47.

## Lo que viene

- **Unidad 13:** dónde vive cada byte: por qué `cielo_dia` y
  `ESCENA_NINTENDO` están en el banco `$7F`, y por qué el DMA necesita el banco
  de una etiqueta aparte.
- **Unidad 16:** Mesen por dentro: el Event Viewer enseña cada escritura del
  HDMA del cielo línea a línea.
