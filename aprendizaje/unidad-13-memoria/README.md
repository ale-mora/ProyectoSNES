# Unidad 13 — Memoria: VRAM, CGRAM, OAM, WRAM y ROM por bancos

**Objetivo:** saber dónde vive cada byte del juego (el código, los gráficos,
las variables, las tablas) y leer el `.sym` y el `.log` que deja la
compilación para comprobarlo.

**Prerrequisitos:** 03 (tubería, sección, banco, `.log`, `.sym`, FastROM), 06
(VRAM, CGRAM, DMA, tiles), 08 (OAM). **Práctica:** PC (`gcc`, sobre la ROM
real compilada en tu clon) + sobre el juego real en Mesen. **Tiempo
orientativo:** dos tardes.

Esta unidad resume `docs/hardware-snes-por-modulo.md` §1, §3, §6 y §7 con
ejercicios, y corrige con datos medidos un par de cosas que ese documento
simplifica (marcadas abajo).

## Antes de empezar: tu clon y su ROM

Todo lo de esta unidad se comprueba sobre una ROM compilada **por ti**, en tu
clon (nunca en `E:\ProyectoSNES`):

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u13
cd E:\ProyectoSNES_trabajo\u13
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
```

Deja `ProyectoSNES.sfc`, `ProyectoSNES.sym` y `ProyectoSNES.log`. Todas las
direcciones de esta unidad salen de esa compilación (2026-10-22, rama
`demo-secuencia-arranque`). **Cambian** cuando cambia el código: búscalas
siempre en tu `.sym`.

## Conceptos

### Dirección de 24 bits: banco + 16 bits

El 65816 ve **16 MB** de direcciones. Una dirección tiene **24 bits**: los 8
altos son el **banco** (`$00..$FF`) y los 16 bajos, la posición dentro del
banco (`$0000..$FFFF`, 64 KB). Se escribe `$BB:AAAA` o `$BBAAAA`:
`$86:F900` es el banco `$86`, posición `$F900`. Casi todas las instrucciones
usan solo los 16 bits bajos y toman el banco de un registro (unidad 14); por
eso, cuando algo necesita el banco (el DMA), hay que dárselo aparte.

### El mapa de memoria (LoROM + FastROM, el de este juego)

```
banco    $0000-$1FFF   $2000-$5FFF          $8000-$FFFF
$00-$3F  WRAM (8 KB,   registros: $21xx PPU,  ROM, a 2,68 MHz ("lenta")
         la misma que  $42xx/$43xx CPU, DMA
         $7E:0000)
$7E-$7F  ───────── WRAM: 128 KB (variables, pila, tablas const globales) ─────────
$80-$BF  como $00-$3F   como $00-$3F         la MISMA ROM, a 3,58 MHz ("rápida")
$C0-$FF                                      ROM (rápida)
```

- **WRAM** (*work RAM*): los 128 KB de RAM de la CPU, en los bancos `$7E` y
  `$7F` (→ glosario: Banco / LoROM).
- **LoROM**: el cartucho se ve en trozos de **32 KB**, cada uno en la mitad
  alta (`$8000-$FFFF`) de un banco. El trozo `n` del archivo `.sfc` es el
  banco `n` (o `$80 + n`).
- **FastROM**: los bancos `$80-$FF` leen la ROM más rápido (→ glosario:
  FastROM). Con `FASTROM 1` el enlazador numera los bancos desde `$80`
  (`.BASE $80` en `hdr.asm`): el código y los datos viven en `$80..$FF`.

**Por qué el juego necesita FastROM para tener 128 bancos.** 128 bancos de
32 KB son 4 MB (`ROMSIZE 0C`, `ROMBANKS 128` en el `Makefile`). Numerados desde
`$00` (SlowROM) irían de `$00` a `$7F`: los dos últimos, `$7E` y `$7F`, son la
**WRAM**. Numerados desde `$80` van de `$80` a `$FF` y no chocan con nada. La
demo de esta unidad lo imprime.

### Pasar de una dirección al byte del `.sfc`

En LoROM, el byte del archivo de una dirección de ROM `$BB:AAAA` es:

```
byte del .sfc = (BB & $7F) × $8000 + (AAAA & $7FFF)
```

(`& $7F` quita el bit «rápido»; `& $7FFF` quita el `$8000` de la mitad alta).
Ejemplo: `nivel1_colision` está en `$86:F900` → `6 × $8000 + $7900 =
0x037900`. Con esa cuenta se puede abrir la ROM en un editor hexadecimal, o
en un programa en C, y encontrar ahí los datos: es la parte A del ejercicio.

### Sección, `superfree`, `.incbin` y etiqueta

Repaso de la unidad 03 con lo que importa aquí:

- **Etiqueta**: un nombre que el ensamblador asocia a una dirección
  (`nivel1_colision:`). El C la ve con `extern char nivel1_colision;` y usa su
  dirección (`&nivel1_colision`).
- **`.incbin "archivo"`**: pega los bytes de un archivo tal cual en la ROM.
- **Sección `superfree`**: un bloque de `data.asm` que el enlazador puede
  colocar **en cualquier banco** donde quepa. Una sección entera va en **un
  solo banco**: no puede cruzar el límite de 32 KB.

Por eso una animación de Tarou tiene **como mucho 10 cuadros**: cada cuadro de
96×64 son 6 piezas de 32×32 a 4bpp = 6 × 512 = **3 072 bytes**; 10 cuadros son
30 720 bytes y caben en un banco (32 768); 11 serían 33 792 y no (`data.asm`,
secciones `.rodata_tarou_*`).

### El banco de una etiqueta: `.db :etiqueta`

Para copiar la capa cercana del nivel a la VRAM por DMA, la rutina necesita la
dirección de origen **completa**: 16 bits en `$4302` y el banco en `$4304`
(`src/juego/dma_ejecutar.asm`). El código C escribe `(u16)&nivel1_cerca_col`:
eso son los 16 bits bajos, pero el banco lo decide el enlazador al colocar la
sección superfree y C no tiene forma de pedirlo. La solución está en
`data.asm`:

```
.section ".rodata_nivel1_banco" superfree
nivel1_cerca_col_banco: .db :nivel1_cerca_col
.ends
```

`:etiqueta` es, en WLA-DX, **el banco de la etiqueta**: el enlazador escribe
ahí un byte con él. El C lo lee como una variable más (`extern u8
nivel1_cerca_col_banco;` en `src/juego/fondo_nivel.c:45`) y se lo pasa a la
lista de DMA (`dma_lista_anadir(..., nivel1_cerca_col_banco, ...)`,
`fondo_nivel.c:87-89`). En esta compilación ese byte está en `$80:FFB1` y vale
`$81`, porque `nivel1_cerca_col` quedó en `$81:B3BA`. Lo mismo hace
`tarou_til_bancos` para las 13 tiras de Tarou.

### Dónde pone tcc816 cada cosa de C

| Lo que escribes en C | Dónde acaba | Prueba en esta compilación |
|---|---|---|
| una función | ROM, sección `.text` (bancos `$80..$8E`) | `main` en `$8E:D147` |
| una variable global o `static` | WRAM, `.bss` (banco `$7E`) | `frames_en_pantalla` en `$7E:26E9` |
| una tabla `const` **global** | **WRAM** (banco `$7F`): sus valores van en la ROM y el arranque los **copia** | `ESCENA_NINTENDO` en `$7F:01FA`, `ANIMACIONES` en `$7F:0150` |
| una tabla `static const` | **ROM**, sección `.rodata` | `controles` de `pantalla_nivel1.c` en `$0E:D5B2` |

Dos matices medidos que los documentos simplifican:

- `docs/hardware-snes-por-modulo.md` §6 dice que tcc816 pone «las tablas
  `const`» en la WRAM: es verdad para las **globales** (en el `.asm` generado
  salen en una `RAMSECTION` llamada `globram.data`, con sus valores iniciales
  en `glob.data`, que el arranque copia). Las `static const` van en la ROM.
- Esa `.rodata` sale con banco `$0E` y no `$8E`: tcc816 escribe `.BASE $00`
  antes de ella (pensado para la RAM que viene detrás). `$0E:D5B2` es **la
  misma ROM** que `$8E:D5B2`, leída a la velocidad lenta. Funciona igual.

Para ver el `.asm` de un `.c` del juego: `make src/escena/escena_nintendo.asm`
en tu clon (receta de `PLAN.md` §6.4; luego **bórralo** de `src/`).

### Variables `static`: la WRAM no empieza a cero

Al encender, la WRAM tiene lo que tenga (basura). El arranque de la librería
(`crt0_snes.asm` de PVSnesLib 4.6.0, etiqueta `4.6.0` en GitHub; `tools/` solo
trae el `.obj`) hace dos cosas antes de `main`:

1. copia `glob.data` (ROM) a `globram.data` (WRAM, `$7F`): los valores de las
   tablas `const` globales (ver la tabla de arriba);
2. **intenta** poner a cero la `.bss`: un bucle `stz $2000,x` de
   `SECTIONEND_.bss − SECTIONSTART_.bss` bytes. Pero cada `.c` tiene su propia
   sección `.bss` y esas etiquetas salen **repetidas** (es el aviso
   `Label "SECTIONSTART_.bss" was defined more than once` del `.log`): la
   cuenta usa el tamaño de **una** de ellas. En esta compilación el código de
   la ROM es `ldx #10`: borra **10 bytes** (`$7E:2000..$7E:2009`) de los
   ~1 800 que ocupan las variables (byte `0x3D6` del `.sfc`, buscado con el
   patrón `A2 lo hi F0 .. CA CA 9E 00 20 D0`).

En la práctica, una `static u8 visible;` sin asignar puede valer cualquier
cosa. Regla del proyecto: **inicializar siempre** en el `_iniciar` o
`_preparar` del módulo (lo hacen `efectos_preparar`, `hud_debug_iniciar`...).

Ojo al comprobarlo en Mesen: el emulador tiene una opción del estado de la RAM
al encender (en sus ajustes de emulación). Si la pone a ceros, una `static` sin
inicializar «funciona» en tu PC y falla en una consola real. Que en Mesen
valga 0 no demuestra nada.

### `RAMSECTION` en un `.asm` escrito a mano, con FastROM

WLA-DX suma la `.BASE $80` de FastROM **también** al banco de una
`RAMSECTION`: `BANK $7E` acabaría en `$FE`, que es ROM, y las variables
«vivirían» en la ROM (el nivel salía en negro, 2026-10-21). Por eso
`src/juego/dma_ejecutar.asm:58-66` pone `.BASE $00` antes de su `RAMSECTION
".dma_lista_ram" BANK $7E SLOT 2` y vuelve a `.BASE $80` antes del código.
tcc816 hace lo mismo en cada `.asm` que genera.

### Leer de la ROM directamente con un puntero

La rejilla de colisión (1 792 bytes) no se copia a ninguna parte: la física la
lee **en la ROM**. `pantalla_nivel1_iniciar` llama a
`colision_usar(&nivel1_colision)` y `colision.c` guarda ese puntero
(`static const u8 *rejilla;`) y lee `rejilla[((y >> 4) << 7) | (x >> 4)]`
(`src/juego/colision.c:65`). En el PC, el mismo código lee un array normal
(`nivel1_datos.h` con `PRUEBAS_PC`).

### DMA frente a copia por CPU

La VRAM, la CGRAM y la OAM **no están en el mapa de la CPU**: se escriben a
través de registros (`$2118` para la VRAM). Copiar con la CPU es un bucle de
cargar y escribir; el **DMA** lo hace el hardware (~2,7 MB/s, unidad 06) y la
CPU espera parada. Por eso todo lo grande va por DMA: tiles y mapas al cargar
una pantalla (`bgInitTileSet`, `dmaCopyVram`) y, durante el nivel, la lista de
DMA en el VBlank (unidad 09).

### El reparto de la VRAM y de la CGRAM

Lo fija `src/sistema/comun.h:29-35` (en palabras de 2 bytes):

```
0x0000  tiles de sprites (512 tiles)      0x6800  mapa de texto (32x32)
0x2000  VRAM_FONDO_A: capa cercana        0x6C00  VRAM_MAPA_A: mapa 64x32 (el anillo del nivel)
0x4000  VRAM_FONDO_B: capa lejana         0x7400  VRAM_MAPA_B: mapa del fondo principal / lejana
0x6000  VRAM_FUENTE (96 tiles)            0x8000  fin (64 KB)
```

La CGRAM: paletas de fondos 0..127 (16 colores × 8), de sprites 128..255;
Tarou en la paleta de sprites 0 (128..143). La OAM: 128 entradas; Tarou usa las
0..5 (`docs/hardware-snes-por-modulo.md` §3-§5).

### El `.sym` y el `.log`

- **`.sym`** (formato *no$snes*): una línea por etiqueta, `00BBAAAA nombre`.
  `0086f900 nivel1_colision` = `$86:F900`. Las variables y funciones `static`
  de C salen con nombre largo: `tccs_<archivo .asm>_<nombre>`, por ejemplo
  `007e2631 tccs_src/pantallas/pantalla_nivel1.asm_tarou`. La copia sin filtrar
  es `.symfull`.
- **`.log`**: todo lo que dijeron las herramientas. Al final, el espacio libre
  por banco: «`ROM bank 14`» es el **trozo 14** del archivo, que la CPU ve como
  banco `$80 + 14 = $8E`. En esta compilación:

```
ROM bank 0 (10 bytes (0.03%) free)          ← $80: cabecera, vectores, código
ROM bank 1 .. 13: 0..5 bytes libres          ← $81..$8D: llenos
ROM bank 14 (10723 bytes (32.72%) free)      ← $8E: a medias
ROM bank 15 .. 127 (32768 bytes (100.00%) free)  ← $8F..$FF: 113 bancos vacíos
RAM slot 2 bank 126 (55567 bytes (96.90%) free)  ← $7E: variables (.bss)
RAM slot 3 bank 127 (64958 bytes (99.12%) free)  ← $7F: tablas const globales
ROM: 3713537 bytes (88.54%) free of total 4194304.
```

## La librería: PVSnesLib

Esta unidad no añade efectos: mira **dónde** dejan las cosas las funciones de
copia que ya usas, y las macros de registro con las que el juego programa el
DMA a mano.

| Función o macro (cabecera) | Qué hace | Hardware |
|---|---|---|
| `void dmaCopyVram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | copia `size` bytes de cualquier dirección de 24 bits (el puntero lleva el banco) a la VRAM, palabra `address`, por el canal 0 | `VMAIN/VMADD $2115-$2117`, `$4300-$4306`, `MDMAEN $420B` |
| `void dmaCopyCGram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | lo mismo hacia la CGRAM (`address` = número de color) | `CGADD $2121`, `CGDATA $2122` por DMA |
| `void dmaCopyOAram(u8 *source, u16 address, u16 size);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | lo mismo hacia la OAM (el SDK ya copia su búfer en cada NMI) | `OAMADD`, `OAMDATA` |
| `void dmaClearVram(void);` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | pone a 0 toda la VRAM | VRAM por DMA |
| `REG_DMAP0..7`, `REG_BBAD0..7`, `REG_A1T0LH..`, `REG_A1B0..`, `REG_DAS0LH..`, `REG_MDMAEN`, `REG_HDMAEN` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | los registros de cada canal como variables de C: escribir `REG_A1B6 = 0x7E;` es un `sta $4364` | `$43x0-$43xA`, `$420B`, `$420C` |
| `void setScreenOff(void);` ([video.h](https://alekmaul.github.io/pvsneslib/video_8h.html)) | forced blank: la VRAM se puede escribir fuera del VBlank | `INIDISP $2100` |

En el juego, el ejemplo de C que separa **dirección y banco** a mano es
`efectos_destello` (`src/escena/efectos.c:118-120`): `REG_A1T6LH = (u16)tabla;`
(16 bits de la tabla HDMA), `REG_A1B6 = BANCO_WRAM;` (su banco, `$7E`) y
`REG_DASB6 = BANCO_WRAM;` (`$4367`, que el SDK no define y el archivo define
él). El de la ROM es la lista de DMA: el banco viaja en cada entrada
(`dma_lista_anadir`) y `dma_ejecutar.asm` lo escribe en `$4304`.

Ejemplos del SDK: `tools/pvsneslib/snes-examples/systems/memory_mapping` (una
ROM HiROM + FastROM: otro mapa, `HIROM=1 FASTROM=1` en su `Makefile`) y
`graphics/Effects/HDMAGradient` (un asset de más de 32 KB partido en dos
secciones, `.incbin ... skip 0 read 32768` y `skip 32768`).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| Cabecera de la ROM: `ROMSIZE 0C`, `ROMBANKS 128`, `FASTROM 1` | `Makefile` (principio) y `hdr.asm` generado (`.BASE $80`) |
| Las secciones superfree; la capa cercana por columnas y su banco (`.rodata_nivel1_cerca`, `.rodata_nivel1_banco`), la rejilla (`.rodata_nivel1_colision`), las 13 tiras de Tarou y `tarou_til_bancos` | `data.asm` |
| El banco de la capa cercana, pasado a la lista de DMA | `src/juego/fondo_nivel.c:45`, `:87-89` (`apuntar_columna`) |
| Dirección + banco escritos en los registros del DMA (`$4302`, `$4304`) | `src/juego/dma_ejecutar.asm` (`_bucle`) |
| Dirección + banco de una tabla HDMA desde C | `src/escena/efectos.c:118-120` |
| `.BASE $00` / `.BASE $80` alrededor de la `RAMSECTION` | `src/juego/dma_ejecutar.asm:54-66` |
| La rejilla leída en la ROM con un puntero | `src/juego/colision.c:31-65`; `pantalla_nivel1.c` (`colision_usar`) |
| El reparto de la VRAM | `src/sistema/comun.h:29-35` |
| La `struct Tarou` (18 bytes en tcc816) y la variable `static Tarou tarou;` | `src/juego/tarou.h`, `src/pantallas/pantalla_nivel1.c:94` |
| El espacio libre y las direcciones | `ProyectoSNES.log`, `ProyectoSNES.sym` (de tu compilación) |

## Trampas

- **Las direcciones cambian** con cada compilación que mueva algo: cópialas
  siempre de tu `.sym`, nunca de un documento.
- **Una sección superfree no cruza 32 KB.** Si no cabe en ningún banco, el
  enlazador da error: partir el asset (tiles en una sección, mapa en otra).
- **`static` sin inicializar = basura** en la consola real, aunque en Mesen
  salga 0.
- **`.BASE $00` antes de una `RAMSECTION`** a mano con FastROM, y `.BASE $80`
  después.
- **Una tabla `const` global gasta WRAM** (y ROM para sus valores iniciales);
  para datos grandes, `data.asm` con `.incbin`, o `static const`.
- **`.log` bank n = CPU `$80 + n`.**
- **Indexar un array de structs de tamaño no potencia de 2** en un bucle por
  frame llama a `tcc__mul` (unidades 09 y 14): `Pista` mide 8 bytes a propósito.

## Cómo se trabaja esta unidad

### 1. La demo: qué memoria hay detrás de una dirección

```
gcc -std=c99 -Wall -Wextra -o demo_13.exe demo_13_direcciones.c
.\demo_13.exe
```

Toma 11 etiquetas reales de esta compilación y dice banco, posición, tipo de
memoria y byte del `.sfc`. Salida de referencia en
`capturas/salida_demo_13_direcciones.txt`. Antes de ejecutarla, intenta decir
de cada etiqueta si es WRAM o ROM mirando solo el banco.

### 2. El ejercicio

`ejercicio_13.md`: parte A, un programa de PC (`ejercicio_13_rom.c`, TODO 1..4)
que busca una etiqueta en el `.sym`, calcula su byte del `.sfc` y compara lo
que hay ahí con el archivo de origen; parte B, la cuenta de ROM libre; parte
C, la `struct Tarou` en Memory Tools de Mesen, cambiando mientras camina.

## Ejercicio y criterio de aprobación

**Aprobación:**

1. `ejercicio_13_rom.c` compila con `gcc -std=c99 -Wall -Wextra` sin avisos y,
   con tu ROM, imprime lo mismo que `capturas/salida_ejercicio_13_rom.txt`
   (cambiando la ruta): los 1 792 bytes de `nivel1_colision` coinciden con
   `res/nivel1/colision.bin`, los 16 640 de `nivel1_cerca_col` con
   `res/nivel1/fondo_cerca.col`, y `nivel1_cerca_col_banco` vale `$81`.
2. La cuenta de ROM libre de la parte B, razonada desde el `.log`.
3. Una captura de Memory Tools de Mesen con los 18 bytes de `tarou` y la `x`
   cambiando entre dos capturas (parte C).

## Lo que viene

- **Unidad 14:** leer el ensamblador: cómo una instrucción toma el banco de un
  registro y por qué `lda.l` lleva la dirección de 24 bits entera.
- **Unidad 16:** Mesen a fondo: breakpoints de escritura en un campo de
  `tarou`.
