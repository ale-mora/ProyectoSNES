# Unidad 15 — Ensamblador 65816 II: escribir

**Objetivo:** escribir una rutina en un `.asm` y llamarla **desde C** como si
fuera una función más, respetando la convención de tcc816; y saber cuándo
merece la pena (casi nunca: cuando una función pequeña se llama muchísimo o va
dentro del VBlank).

**Prerrequisitos:** 14 (leer ensamblador: registros, banderas, modos de
direccionamiento, la pila, la convención de llamada de tcc816). **Práctica:**
mini-ROM + sobre el juego real. **Tiempo orientativo:** tres tardes.

El juego está escrito en C y así debe seguir. En ensamblador hay **una sola**
rutina, `src/juego/dma_ejecutar.asm`, y existe por una razón medida: preparar
16 copias por DMA desde C se salía del VBlank y Tarou se dibujaba a trozos
(2026-10-20). Esta unidad enseña a escribir rutinas así, siempre **llamadas
desde C**, y a demostrar con números que valen lo que cuestan.

## Conceptos

### La convención, vista desde el que escribe la rutina

En la unidad 14 se leyó cómo llama tcc816. Escribiendo a mano, eso se convierte
en un contrato que la rutina tiene que cumplir:

| Lo que hace C al llamar | Lo que tiene que hacer la rutina |
|---|---|
| empuja los parámetros de derecha a izquierda (u16 = 2 bytes, u8 = 1, puntero = 4) y llama con `jsr.l` (3 bytes de vuelta) | leer el primer parámetro en `4,s`, el siguiente en `4 + su tamaño,s`... (+1 a todo tras un `php`, +2 tras un `pha`) |
| está en 16 bits (m = 0, x = 0) y espera seguir así | guardar P al entrar (`php`) y devolverlo al salir (`plp`): así da igual cuántos `sep`/`rep` haga dentro |
| tiene D = `$0000` y DB = `$7E` | no tocarlos (o guardarlos con `phd`/`phb` y devolverlos) |
| saca él mismo los parámetros de la pila al volver | **no** sacarlos; dejar S como estaba y volver con **`rtl`** |
| lee el resultado en `tcc__r0` (y `tcc__r0h` si es puntero) | dejar ahí el valor devuelto |
| no espera que A, X, Y ni los `tcc__rN` sobrevivan | usarlos libremente |

La cabecera de `src/juego/dma_ejecutar.asm` lo resume así: «C llama con `jsr.l`
(dirección de 24 bits), así que se vuelve con `rtl`. No recibe parámetros; no
devuelve nada; puede usar A, X e Y a su antojo. Se guarda y restaura P».

### Secciones, etiquetas y cómo C encuentra la rutina

- **`.SECTION ".relleno_text" SUPERFREE` ... `.ENDS`**: un trozo de código con
  nombre que el enlazador coloca donde quiera (SUPERFREE: en cualquier banco;
  unidad 13). tcc816 hace una sección por función (`.camara_paratext_0x0`); el
  nombre clásico en otros compiladores es `.text`, aquí da igual mientras sea
  único.
- **Etiqueta**: `relleno_vram:` es el nombre de la dirección donde empieza la
  rutina. **wlalink une cada `jsr.l relleno_vram` de C con la etiqueta del
  mismo nombre**, esté en el `.obj` que esté. No hace falta `.export`: ni
  `dma_ejecutar.asm` ni `relleno.asm` lo usan y enlazan (comprobado en sus
  `.sym`). Las etiquetas que empiezan por `_` (`_bucle`, `_fin`) son locales a
  su archivo: no chocan con las de otros. Comprobado: una copia de la mini-ROM
  con un segundo `.asm` que también tiene `_bucle:` y `_fin:` enlaza sin avisos y
  el `.sym` lista los dos pares. (Y si C no llama a la rutina de ese segundo
  `.asm`, su sección SUPERFREE ni siquiera entra en la ROM: el enlazador descarta
  lo que nadie usa.)
- **En C**, la rutina se declara con su firma en un `.h`
  (`void relleno_vram(u16 vram, u16 palabra, u16 cuantas);`) y se llama como
  cualquier función. Para C no hay diferencia: no sabe en qué está escrita.
- **Una variable del `.asm` que C quiere ver**: se reserva en una
  `RAMSECTION` y C la declara `extern` (`extern u16 relleno_llamadas;`), como
  hacen `dma_lista_n` y `dma_lista_items` (`src/juego/dma_lista.h:49-50`).
- **Al revés, el `.asm` lee una variable de C** por su nombre
  (`lda.l mirrorINIDISP`, una del SDK). Solo si **no** es `static`: un `static`
  de tcc816 se llama `tccs_<archivo>_<nombre>` y no se ve desde otro archivo.

### `RAMSECTION` y `.BASE` con FastROM (la trampa, en vivo)

Con FastROM, `hdr.asm` deja `.BASE $80` (el código se ve en los bancos
`$80-$FF`), y WLA-DX suma esa base **también** al banco de una `RAMSECTION`:
`BANK $7E` acaba en `$FE`, que es ROM. Por eso:

```
.BASE $00
.RAMSECTION ".relleno_ram" BANK $7E SLOT 2
relleno_llamadas:  dw
.ENDS
.ifdef FASTROM
.BASE $80
.endif
```

Comprobado con la mini-ROM (`capturas/compilacion_rom.txt`): con las dos líneas,
el `.sym` dice `007e2108 relleno_llamadas`; quitando el `.BASE $00` compila
**igual de limpio** («No warnings or errors found.») y dice `00fe2108`: una
variable «en la ROM», donde escribir no hace nada. No hay aviso; solo el `.sym`
lo delata.

Y la otra trampa de la memoria (unidad 13): la WRAM arranca con basura y nadie
pone las variables a 0. `principal.c` hace `relleno_llamadas = 0;` antes de
nada.

### `REP` / `SEP` y bucles con `DEX` / `BNE`

`rep #$20` = A de 16 bits, `sep #$20` = A de 8 (unidad 14). Una rutina a mano
cambia de tamaño **solo cuando lo necesita**: los registros de la PPU de un byte
(`$2100`, `$2115`) se escriben con A de 8 bits; `$2116` (VMADD) y `$2118`
(VMDATA) son pares de registros consecutivos y con A de 16 bits un solo `sta`
escribe los dos bytes (bajo y alto, en ese orden, que es lo que piden).

El bucle típico cuenta hacia atrás en X:

```
    tax                         ; X = cuántas
_bucle:
    sta.l $2118                 ; el trabajo
    dex                         ; X - 1 (pone Z al llegar a 0)
    bne _bucle                  ; si no es 0, otra vuelta
```

Tres instrucciones por vuelta. La versión en C del mismo bucle
(`rom/src/relleno_c.c`, `relleno_vram_c`) son **30** (contadas en su `.asm`,
`capturas/relleno_c_generado.asm`): recalcular `cuantas != 0` como booleano,
construir el puntero largo a `$002118` en `tcc__r9` en cada vuelta, restar 1 a
`cuantas` en la pila...

### Escribir un registro de la PPU a mano

`sta.l $2100` escribe INIDISP (→ glosario: Forced blank): bit 7 = pantalla
apagada, bits 0-3 = brillo. **`.l` (dirección larga) es obligatorio**: el juego
tiene DB = `$7E` y un `sta $2100` corto escribiría en `$7E:2100`, la WRAM
(unidad 14). Con la pantalla apagada (forced blank) la VRAM se puede escribir en
cualquier momento, no solo en el VBlank (unidad 04): por eso `relleno_vram`
apaga, rellena y la devuelve como estaba, leyendo `mirrorINIDISP`, la copia en
la WRAM que el SDK guarda del último valor de INIDISP (si se escribiera `$0F` a
ciegas, se encendería una pantalla que C había dejado apagada).

**Una interrupción a mitad de la rutina.** El NMI del VBlank llega aunque la
pantalla esté apagada, y la CPU salta a `VBlank` (unidad 14) en mitad de
nuestro bucle. Si en ese VBlank la consola tiene texto pendiente, `consoleVblank`
reescribe VMADD y VMAIN para copiar su mapa, y nuestro relleno sigue escribiendo
**donde lo dejó la consola**. Por eso `principal.c` llama a las rutinas justo
después de un `WaitForVBlank()` (la consola ya vació lo pendiente y queda casi
un frame entero) y no llama a `consoleDrawText` hasta después. `sei` no sirve:
el NMI no se puede enmascarar con la bandera I.

### Medir ciclos: el profiler de Mesen

Un **ciclo** es un paso del reloj de la CPU; cada instrucción tarda varios
(según las tablas del 65816: `dex` 2, `bne` 3 si salta, `sta.l` de 16 bits 6).
Contar instrucciones orienta; los ciclos de verdad los da Mesen: Debug →
**Performance Profiler**, con el `.sym` cargado, lista cada función (cada
etiqueta a la que se llega con `jsl`/`jsr`) con su número de llamadas, ciclos
totales, medios, mínimos y máximos. Para comparar dos versiones de una función:
las dos ROM, el mismo recorrido (la ROM de prueba con guion lo garantiza) y los
**ciclos medios por llamada**. Ojo si la función llama a otras: el profiler
separa los ciclos **propios** (exclusivos) de los que incluyen a las que llama
(inclusivos). `colision_apoyan` en asm llama a su subrutina `_pie` con `jsr` y
puede salir en dos filas: compara siempre los **inclusivos** con los de la
versión en C. `numero` y `hud_numero` no llaman a nada y no tienen ese problema.

Estimación con las tablas para un relleno de 32 palabras: la vuelta en asm son
unos 11 ciclos (≈ 350 en total); la de C, 30 instrucciones de unos 4-6 ciclos
cada una (≈ 4 000-6 000). Es una estimación: el profiler manda (pendiente de
medir en la tanda de capturas, ver «Aprobación»).

## La librería: PVSnesLib

La librería también está escrita en ensamblador (fuentes de la 4.6.0:
<https://github.com/alekmaul/pvsneslib/tree/4.6.0/pvsneslib/source>), y sus
rutinas cumplen **la misma convención** que `relleno.asm`: por eso C las llama
sin más. `memset` es el mejor ejemplo, porque el juego la usa en vez de un bucle
de C (`src/juego/hud_debug.c`, `empezar_fila`): sus parámetros están en `4,s`
(puntero, 4 bytes), `8,s` (el valor) y `10,s` (cuántos), vuelve con `rtl` y su
bucle es `dey / sta [$00],y / bne`, 3 instrucciones por byte (leído en los bytes
de la ROM: `../unidad-14-asm-leer/capturas/rutinas_libreria.txt`).

Lo que usa la mini-ROM (firmas copiadas de las cabeceras de
`tools/pvsneslib/pvsneslib/include/snes/` y `devkitsnes/include/`):

| Función o registro | Cabecera (documentación) | Qué hace | Hardware |
|---|---|---|---|
| `void dmaClearVram(void);` | `dma.h` (<https://alekmaul.github.io/pvsneslib/dma_8h.html>) | pone toda la VRAM a 0 (por DMA, según su documentación): el mapa de BG2 queda en el tile 0 (espacio) | VRAM entera |
| `void consoleInitDefaultText(u8 palnum);` | `textfont.h` (<https://alekmaul.github.io/pvsneslib/textfont_8h.html>) | carga la fuente de la librería y prepara la consola (su NMI copia el texto) | VRAM, CGRAM |
| `void consoleDrawText(u16 x, u16 y, char *fmt, ...);` | `textfont.h` | escribe texto con formato en el búfer de la consola; se copia en el VBlank siguiente | (VRAM, en el NMI) |
| `void bgSetGfxPtr(u8 bgNumber, u16 address);` / `void bgSetMapPtr(u8 bgNumber, u16 address, u8 mapSize);` / `void bgSetDisable(u8 bgNumber);` | `background.h` (<https://alekmaul.github.io/pvsneslib/background_8h.html>) | dónde están los tiles y el mapa de cada fondo; apagar uno | `$2107-$210C`, `$212C` |
| `void setMode(u8 mode, u8 size);` / `void setScreenOn(void);` | `video.h` (<https://alekmaul.github.io/pvsneslib/video_8h.html>) | Mode 1; encender la pantalla con brillo 15 (el SDK guarda el valor escrito en `mirrorINIDISP`) | `$2105`, `$2100` |
| `extern u8 mirrorINIDISP;` | `video.h` | copia en la WRAM del último valor escrito en INIDISP | — |
| `REG_INIDISP`, `REG_VMAIN`, `REG_VMADDLH`, `REG_VMDATALH` | `video.h` | macros: `(*(vuint8 *)0x2100)`, etc. Lo que usa la versión en C | `$2100`, `$2115`, `$2116`, `$2118` |
| `void WaitForVBlank(void);` | `interrupt.h` (<https://alekmaul.github.io/pvsneslib/interrupt_8h.html>) | duerme la CPU hasta el NMI (unidad 14) | — |
| `void *memset(void *b, int c, size_t n);` | `devkitsnes/include/string.h` | rellena memoria, en ensamblador | — |

Ejemplo del SDK de partida: `tools/pvsneslib/snes-examples/hello_world/`. El
SDK trae un ejemplo de `.asm` propio junto a C:
`tools/pvsneslib/snes-examples/thirdparty/maps/mapbuffer/src/addons/`
(`mapbufferextension.c` y `mapbufferextensionA.asm`: fíjate en la `A` del
nombre, el mismo truco que `dma_ejecutar.asm` para no chocar con el `.asm`
intermedio del `.c`). El modelo de esta unidad es `src/juego/dma_ejecutar.asm`.

## La mini-ROM (`rom/`)

| Archivo | Qué es |
|---|---|
| `rom/Makefile` | el de `hello_world`, con `ROMNAME := unidad15` y `FASTROM := 1` |
| `rom/src/relleno.asm` | **la rutina en ensamblador**: `relleno_vram(vram, palabra, cuantas)` y la variable `relleno_llamadas` |
| `rom/src/relleno_c.c` | la misma rutina en C (`relleno_vram_c`) y el bucle `*p++ = 0` (`limpiar_c`) |
| `rom/src/relleno.h` | cómo las ve C |
| `rom/src/principal.c` | `main`: prepara BG1 (texto) y BG2 (franjas), llama dos veces a cada versión y escribe cuántas veces se llamó a la de asm |

El `.asm` **no puede llamarse como ningún `.c`** (`relleno_c.c` pasa por un
`relleno_c.asm` intermedio que lo sobrescribiría). No se nombra en el
`Makefile`: `snes_rules` ensambla todo `src/*.asm`.

Compilar, desde PowerShell en `rom/`:

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
```

Resultado (2026-10-22): `No warnings or errors found.` y `unidad15.sfc`; en el
`.sym`, `relleno_vram` en `$80:B029` y `relleno_llamadas` en `$7E:2108`.

**Qué se ve en Mesen:** el título `UNIT 15: ASM CALLED FROM C` arriba;
`FILLED BY C:` y debajo dos filas enteras de `#` (filas 10 y 11); `FILLED BY
ASM:` y dos filas de `=` (14 y 15); y `ASM CALLS: 2`. Las dos franjas iguales
de ancho dicen que las dos rutinas hacen lo mismo; el 2, que la variable del
`.asm` existe, está en la WRAM y C la lee. El relleno ocurre con la pantalla
apagada un instante: en el frame del arranque puede verse una banda negra.

Para leer el ensamblador de la versión en C: `make src/relleno_c.asm` dentro de
`rom/` (y sácalo de `src/` enseguida). Ya está en `capturas/relleno_c_generado.asm`.

## Dónde está en el juego

`src/juego/dma_ejecutar.asm` (112 líneas), leído con lo de esta unidad:

| Líneas | Qué |
|---|---|
| 1-48 | cabecera: por qué existe (el VBlank de 37 líneas), por qué no se llama `dma_lista.asm`, registros que toca, variables, la convención |
| 49-52 | `.include "hdr.asm"` y `.accu 16` / `.index 16` |
| 58-66 | `.BASE $00`, la `RAMSECTION` (`dma_lista_n`, `dma_lista_libre`, `dma_lista_items` de 256 bytes) y `.BASE $80` |
| 70-84 | `php`, `rep #$30`, el truco `lda.w #0` + `sep #$20` + `lda.l dma_lista_n` para leer un byte con el alto a 0, `beq _fin` si no hay copias; `tay` (Y = copias); `ldx.w #0`; `DMAP0 = $01`, `BBAD0 = $18` (una vez para todas) |
| 85-105 | `_bucle`: por cada copia lee sus 8 bytes con `lda.l dma_lista_items+n,x` y los escribe en VMAIN, A1B0, VMADD, A1T0 y DAS0 (cambiando de tamaño de A según el registro); `X += 8` (la struct `DmaItem` mide 8 bytes **a propósito**: potencia de 2, ver unidad 14); `MDMAEN = 1` dispara; `dey / bne _bucle` |
| 106-110 | VMAIN otra vez a `$80` (lo que espera el SDK), `plp`, `rtl` |

Cómo la ve C: `src/juego/dma_lista.h:39-61` (la `DmaItem` «tal como la lee
`dma_ejecutar.asm`, no cambiar el orden», los `extern` y la firma
`void dma_lista_ejecutar(void);`). La llaman `pantalla_nivel1_vblank` y, con la
pantalla apagada, `_iniciar`, `reaparecer` y `fondo_nivel_rellenar`.

Lo que se reescribe en el ejercicio: `numero` en `src/juego/hud_debug.c` (resta
potencias de 10 en vez de dividir) y, en la ampliación, `colision_apoyan` en
`src/juego/colision.c`.

## Trampas

- **Nombre del `.asm` = nombre de un `.c`** → `make` lo sobrescribe con el
  intermedio del compilador sin avisar (`dma_ejecutar.asm`, no `dma_lista.asm`).
- **`RAMSECTION` sin `.BASE $00`** con FastROM → la variable acaba en `$FE`
  (ROM); compila sin avisos. Mira el `.sym`.
- **Olvidar `php`/`plp`** → la rutina vuelve con A de 8 bits y el C que sigue se
  ejecuta mal (la CPU lee los inmediatos con otro tamaño): fallos absurdos lejos
  de la rutina.
- **`sta $2100` sin `.l`** → con DB = `$7E` escribe en la WRAM, no en la PPU.
- **Leer los parámetros con el desplazamiento de antes del `php`/`pha`** → cada
  byte que se empuja mueve todos los `n,s`.
- **Un `static` de C desde el `.asm`** → no existe con ese nombre
  (`tccs_<archivo>_...`). Quítale el `static` (ampliación).
- **Dos definiciones de la misma función** (la de C y la de asm) → el enlazador
  falla. Si la versión en C tiene que quedarse para las pruebas del PC,
  `#ifdef PRUEBAS_PC`.
- **El NMI a mitad de la rutina** puede cambiar VMADD (consola con texto
  pendiente). Llamar tras `WaitForVBlank`.
- **PowerShell y .NET**: `[IO.File]::WriteAllText("src/...")` escribe relativo a
  la carpeta en que arrancó PowerShell, no a la del `cd`. Rutas absolutas.

## Ejercicio y criterio de aprobación

`ejercicio_15.md`: completar `ejercicio_15_hud_numero.asm` (TODO 1-7) en un
clon del juego con `andamio_hud_debug.c.patch`, comprobarlo en el PC con
`comprobar_hud_numero.py`, compilar las ROM de prueba en C y en asm, y medir;
de paso, `*p++ = 0` frente a `memset`; ampliación: `colision_apoyan` en asm.

**Aprobación:**

1. La mini-ROM compila (`No warnings or errors found.`) y en Mesen muestra las
   dos franjas y `ASM CALLS: 2` (captura).
2. `comprobar_hud_numero.py` da `fallos: 0` con tu `hud_numero.asm`; en Mesen,
   la ROM `-DGUION_PRUEBA -DHUD_NUMERO_ASM` muestra `ASM 14/14` en la fila de
   `CAM` (los mismos dígitos que la versión en C para 14 valores fijos).
3. Profiler: menos ciclos medios por llamada en `hud_numero` que en `numero`
   (tabla del TODO 10).
4. (Ampliación) `367 pruebas, 0 fallos` y el recorrido de la ROM de prueba
   completo con `colision_apoyan` en asm.

La solución está en `solucion/` (`solucion_15.md`); la verificación hecha en el
clon, en `capturas/`.

## Lo que viene

- **Unidad 16:** Mesen a fondo; el profiler y los breakpoints de esta unidad son
  el principio.
- **Unidad 17:** audio desde C con PVSnesLib.
