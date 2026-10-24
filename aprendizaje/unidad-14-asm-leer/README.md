# Unidad 14 — Ensamblador 65816 I: leer

**Objetivo:** leer el ensamblador que genera `tcc816` a partir del C del juego (y
el de las rutinas de PVSnesLib que se ven en el depurador) lo bastante bien como
para **explicar por qué un trozo de C es lento** y cómo escribirlo para que no lo
sea.

**Prerrequisitos:** 03 (la tubería: `.c` → `.ps` → `.asm` → `.obj` → `.sfc`, y
`camara.asm` anotado por sentencias) y 13 (memoria: bancos, WRAM, `.sym`,
`RAMSECTION` con `.BASE`). **Práctica:** lectura + depurador de Mesen.
**Tiempo orientativo:** dos o tres tardes.

Este curso enseña a programar la SNES **en C**. Las unidades 14 y 15 son la
excepción obligatoria, y por un motivo práctico: el C de este proyecto tiene
reglas raras («nada de `a[i]` sobre structs de 5 bytes», «no dividas un `u8`»,
«`memset` y no un bucle»), y todas salen de mirar el ensamblador. Quien no sabe
leerlo tiene que creérselas; quien sabe, las comprueba en cinco minutos. Aquí no
se escribe ensamblador (eso es la unidad 15): se **lee** lo que escribe el
compilador.

## Antes de empezar: dónde se mira

Todo lo de esta unidad se genera en un **clon** del juego, nunca en
`E:\ProyectoSNES\src` (un `.asm` olvidado en `src/` se ensamblaría junto a su
`.c`; PLAN §6.4):

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mi_u14
git -C E:\ProyectoSNES_trabajo\mi_u14 checkout -q plataformas-r2
cd E:\ProyectoSNES_trabajo\mi_u14
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
make                                  # una vez, entera (genera los gráficos)
make src/juego/camara.asm             # el .asm de UN .c; muévelo fuera de src/ enseguida
```

El clon se fija en el tag `plataformas-r2` (la Revisión 2): parte del
ejercicio lee el guion del tutorial y `res/nivel1/nivel1_datos.h`, que salen
del juego actual (un beat 'em up, `docs/diseno-beat-em-up.md`), y los `.asm`
de referencia se generaron con ese código.

En `asm_generado/` están ya generados (2026-10-22) los cuatro que usa el
ejercicio: `camara.asm` (85 líneas), `nivel.asm` (43), `personajes.asm` (894) y
`guion.asm` (516), más una **sonda**, `divisiones.c` / `divisiones.asm`: diez
funciones de una línea para ver aislado qué hace tcc816 con cada tipo.

## Conceptos

### Registro, instrucción y modo de la CPU

Un **registro** es una casilla de memoria dentro de la CPU (unidad 03). El
65816 tiene estos, y todos aparecen en el panel de registros de Mesen:

| Registro | Bits | Para qué | En el código de tcc816 |
|---|---|---|---|
| **A** (acumulador) | 8 o 16 | donde se hacen las cuentas: `lda` carga, `adc` suma, `sbc` resta, `sta` guarda | casi todo pasa por A |
| **X**, **Y** (índices) | 8 o 16 | contadores e índices (`lda tabla,x`) | X guarda booleanos de comparaciones; Y cuenta vueltas de desplazamientos |
| **S** (pila) | 16 | apunta al primer byte **libre** de la pila | parámetros y variables locales: `5,s` |
| **P** (estado) | 8 | las **banderas** (abajo) | `php` / `plp` lo guardan y lo recuperan |
| **D** (página directa) | 16 | base de las direcciones cortas (`.b`) | `$0000`: los `tcc__rN` están en `$00..$3F` |
| **DB** (banco de datos) | 8 | banco que completa las direcciones de 16 bits (`.w`) | `$7E`: las variables de C |
| **PB** (banco de programa) | 8 | banco del código que se ejecuta | `$80..$8F` con FastROM |
| **PC** (contador de programa) | 16 | la instrucción siguiente | |

(→ glosario: Banco / LoROM, FastROM)

Un **opcode** es el byte que codifica una instrucción: `lda 4,s` es `a3 04`
(dos bytes), `rtl` es `6b`. El ensamblador (`wla-65816`) traduce el texto a esos
bytes; Mesen hace lo contrario (desensamblar) en su depurador. En
`capturas/rutinas_libreria.txt` está `memset` leído byte a byte de la ROM.

### Banderas

**P** guarda 8 bits que las instrucciones ponen o miran. Las que salen en esta
unidad:

| Bandera | Se pone a 1 cuando... | La mira |
|---|---|---|
| **N** (negativo) | el bit más alto del resultado es 1 (bit 15 en 16 bits) | `bmi` (salta si 1), `bpl` (si 0) |
| **Z** (cero) | el resultado es 0 | `beq` (si 1), `bne` (si 0) |
| **C** (acarreo) | una suma se pasa de `$FFFF`; en una resta o `cmp`, cuando **no** hubo préstamo (A ≥ operando, sin signo) | `bcs` (si 1), `bcc` (si 0); `adc`/`sbc` la usan como entrada: por eso `clc` antes de sumar y `sec` antes de restar |
| **V** (desbordamiento) | una suma o resta **con signo** se sale de −32768..32767 | `bvc` (si 0), `bvs` (si 1) |
| **m** | A es de 8 bits | (la cambian `rep`/`sep`) |
| **x** | X e Y son de 8 bits | |
| **e** | modo emulación del 6502 (solo al arrancar; el juego va en nativo, e = 0) | |

### Modo de 8 o 16 bits: `REP` y `SEP`

El 65816 puede trabajar con A de 8 o de 16 bits **según la bandera m**, y con
X/Y según **x**. `rep #$20` pone m a 0 (A de 16 bits), `sep #$20` a 1 (A de 8);
`#$10` es x y `#$30` las dos. **tcc816 vive en 16 bits** y baja a 8 solo un
momento para leer o escribir un byte. De ahí un patrón que verás cien veces,
leer un `u8`:

```
lda.w #0        ; A = 0 entero (el byte alto también)
sep #$20        ; A de 8 bits
lda 3 + 1,s     ; carga SOLO el byte bajo (el u8)
rep #$20        ; A de 16 bits: A = $00xx
```

Las líneas `.accu 16` / `.index 16` del principio de cada `.asm` no cambian la
CPU: le dicen al **ensamblador** de qué tamaño son los números inmediatos
(`lda #5` son 2 o 3 bytes según m). Si la CPU y el ensamblador no están de
acuerdo, la CPU lee mal el resto del programa: por eso las rutinas escritas a
mano guardan y devuelven P (`php`/`plp`, unidad 15).

### Modos de direccionamiento

El **modo de direccionamiento** es cómo dice la instrucción dónde está el dato.
Todos estos salen en los `.asm` del juego:

| Modo | Se escribe | Significa | Ejemplo real |
|---|---|---|---|
| inmediato | `#n` | el número n | `adc.w #48` (`camara.asm`) |
| directo | `.b dir` | página directa: `D + dir` (banco 0) | `sta.b tcc__r0` |
| absoluto | `.w dir` | `DB:dir` (16 bits + el banco de DB) | `lda.w tccs_..._turno` (variable en `$7E`) |
| largo | `.l dir` | dirección completa de 24 bits | `sta.l $4300`, `lda.l mirrorINIDISP` |
| indexado | `dir,x` / `dir,y` | `dir + X` | `lda.l dma_lista_items+7,x` |
| indirecto largo | `[dir]` / `[dir],y` | en `dir` hay un **puntero** de 3 bytes; leer donde apunta (+Y) | `lda.b [tcc__r0]`, `sta [$00],y` (`memset`) |
| relativo a la pila | `n,s` | `S + n` | `lda 3 + 1,s` (un parámetro) |
| relativo (saltos) | `bne etiqueta` | a −128..+127 bytes de aquí; `brl` llega a ±32 KB | `bne -`, `brl __local_0` |

**Por qué DB importa:** el juego deja **DB = $7E** (el banco de la WRAM), y tcc816
lee y escribe las variables de C con direcciones de 16 bits (`.w`), que la CPU
completa con DB. Todo lo que **no** está en el banco $7E se tiene que alcanzar
con dirección larga: los registros de la PPU y del DMA (banco `$00`), la ROM.
Por eso `src/juego/dma_ejecutar.asm` escribe `sta.l $4300` y no `sta $4300`, y
por eso tcc816 traduce `REG_VMDATALH = v` a «puntero `$00:2118` en `tcc__r9` y
`sta.b [tcc__r9]`» (`asm_generado/divisiones.asm`, `escribir_vram`).

### La pila: `PHA`/`PLA`, `JSR`/`RTS`, `JSL`/`RTL`

La **pila** es una zona de la WRAM que crece **hacia abajo**; S apunta al primer
byte libre, así que lo último que se metió está en `1,s`.

- `pha` mete A (2 bytes en 16 bits) y `pla` lo saca; `pea #n` mete un número;
  `pei (dir)` mete los 2 bytes que hay en `dir`; `php`/`plp`, el registro P.
- `jsr` llama a una subrutina **del mismo banco** (mete 2 bytes de vuelta) y
  `rts` vuelve. `jsl` (en la sintaxis de WLA, `jsr.l`) llama a **cualquier
  banco** (mete 3: banco + dirección) y `rtl` vuelve. **Todas las funciones de
  tcc816 se llaman con `jsr.l` y terminan en `rtl`**, porque el enlazador puede
  ponerlas en cualquier banco (`SUPERFREE`).
- `tsa` / `tas` copian S a A y A a S: así reserva tcc816 las **variables
  locales**, restando a S (`tsa / sec / sbc #2 / tas`) y devolviéndolo al final.

### Cómo llama tcc816 a una función (la convención de llamada)

La **convención de llamada** es el acuerdo entre quien llama y quien es llamado
sobre dónde van los parámetros y el resultado. La de tcc816, comprobada en los
`.asm` del juego:

1. **Los parámetros van en la pila, empujados de derecha a izquierda** (el
   último primero), así que el primero queda más cerca: `4,s` al entrar.
2. Cada `u16`/`s16` ocupa 2 bytes; cada **`u8`, 1 byte**; cada **puntero, 4**
   (16 bits bajos y una palabra con el banco). En `hud_debug.asm`, apuntar una
   fila empuja `banco` y `vmain` (dos `u8`) con un solo `pea.w (128 * 256 + 126)`.
3. `jsr.l` añade la vuelta (3 bytes). Dentro, el parámetro `k` está en
   `3 + locales + 1 + desplazamiento,s`: es la forma que escribe tcc816 siempre.
4. **Quien llama saca los parámetros** al volver: `tsa / clc / adc #6 / tas`
   (6 = los bytes que empujó; con tres `u16`).
5. **El valor devuelto va en `tcc__r0`** (16 bits; un puntero, en `tcc__r0` y
   `tcc__r0h`). No en A: quien llama lo lee de `tcc__r0`
   (`guion_mando` hace `sep #$20 / lda.b tcc__r0` tras llamar a `mantenidos`).
6. `tcc__r0`, `tcc__r1`... `tcc__r10` son **pseudorregistros**: palabras de la
   página directa (`tcc__r0` en `$00`, `tcc__r0h` en `$02`, `tcc__r1` en `$04`,
   `tcc__r9` en `$18`, `tcc__r10` en `$1C`, según el `.sym`) que hacen de
   registros extra. Son de usar y tirar: quien llama no espera que sobrevivan a
   una llamada (lo que necesita lo guarda en su pila).
7. Una función `static` se llama `tccs_{WLA_FILENAME}_nombre` (en el `.sym`,
   `tccs_src/juego/hud_debug.asm_numero`): el prefijo evita choques entre
   archivos y hace que **no se vea desde otro `.asm`**.

### `if`, comparaciones, bucles y `switch`

- **Comparación con signo** (`s16 < 0`): `sec / sbc / bvc + / eor #$8000 / + /
  bmi`. Restar pone N, pero si la resta se desbordó (V = 1) N miente y
  `eor #$8000` le da la vuelta. Es la firma de un `if` con `s16`.
- **Comparación que calcula un booleano**: `ldx #1 / ... / dex / stx.b tcc__r5 /
  txa / bne`. tcc816 calcula el resultado de la comparación en X (1 o 0) y
  luego pregunta por X. Muchas instrucciones para un `if`.
- **Bucle**: una etiqueta arriba, la condición calculada como antes y un
  `jmp.w`/`bra` de vuelta. Una vuelta de `for (; n != 0; n--) *p++ = 0;` son
  **37 a 40 instrucciones** (`divisiones.asm`, `llenar_c` y `llenar_c16`;
  `limpiar_c` en la unidad 15): copiar el puntero de 4 bytes, sumarle, guardarlo,
  restar 1 a `n` con `adc.w #65535`...
- **Desplazar**: hasta 4 bits, instrucciones seguidas (`lsr a` x4); desde 5, un
  bucle `ldy.w #n` / `asl a` / `dey` / `bne -` (`nivel.asm`, el `<< 5`).
- **`switch`**: una **cadena de comparaciones** (`cmp #0`, `cmp #1`...), no una
  tabla de saltos. El caso 7 de `hud_debug_dibujar` cuesta 8 comparaciones.
- **`>>` de un `s16`** (o de un `u8`, que C convierte en `int` con signo):
  `cmp #$8000 / ror a` por cada bit. `PF_ENTERO` son cuatro de esas parejas.

### Multiplicar y dividir: `tcc__mul`, `tcc__div`, `tcc__udiv`

El 65816 **no tiene** instrucciones de multiplicar ni de dividir. tcc816 llama a
**rutinas del runtime** (las rutinas de apoyo que el compilador necesita y que
trae la librería, `libtcc.obj`):

| Rutina | La usa tcc816 para | Entrada / salida (leído en los `.asm`) |
|---|---|---|
| `tcc__mul` | `a * b` que no es potencia de 2 | factores en `tcc__r9` y `tcc__r10`, resultado en A |
| `tcc__div` | `/` y `%` **con signo** | dividendo en X, divisor en A; cociente en `tcc__r9`, resto en X |
| `tcc__udiv` | `/` y `%` **sin signo** con divisor variable | ídem |

Y el **tipo** del operando decide cuál sale. La sonda `divisiones.asm`, medida:

| C | Sale |
|---|---|
| `u16 / 2` | `lsr` (un desplazamiento) |
| `u8 / 2` | **`jsr.l tcc__div`** |
| `(u16)u8 / 2` | `lsr` |
| `s16 / 2` | `jsr.l tcc__div` |
| `u16 / u16` | `jsr.l tcc__udiv` |

¿Por qué `u8 / 2` es caro? Por la **promoción entera** de C: antes de operar,
todo tipo más pequeño que `int` se convierte a `int`, y en tcc816 `int` es de 16
bits **con signo**. Una división con signo redondea hacia el cero (`-3 / 2` es
`-1`) y un desplazamiento no (`-3 >> 1` es `-2`), así que el compilador no puede
cambiarla por un `lsr`: llama a la rutina. Que un `u8` nunca sea negativo no le
importa: mira el tipo (→ glosario: `u8`, `u16`; `s16`). Es el caso real de `tile_del_bloque64` (ejercicio, TODO 3).

Lo mismo con las structs: `guion[i]` es «la dirección de `guion` + `i ×
sizeof(TramoGuion)`», y si el tamaño no es potencia de 2 eso es un `tcc__mul`
**en cada acceso**. `sizeof(TramoGuion)` es **6** para tcc816 (dos `u16` y un
`u8` = 5, más un byte de **relleno** para que cada `u16` del array empiece en
dirección par); el comentario de `guion.c` dice 5. Recorrer con un puntero que
avanza (`t++`, un `adc.w #6` por vuelta) no multiplica nada (TODO 4).

### `data.asm` y `hdr.asm`: ensamblador puro

No todo el ensamblador del proyecto viene de C:

- **`data.asm`** (raíz) es ensamblador escrito a mano y **sin instrucciones**:
  solo **directivas** (órdenes al ensamblador, no a la CPU). `.section
  ".rodata_fuente" superfree` abre una sección; `fuente_consola_til: .incbin
  "res/fuente/ark_consola.pic"` pega un archivo binario y le pone nombre;
  `nivel1_cerca_col_banco: .db :nivel1_cerca_col` (`data.asm:157`) guarda un byte
  con el **banco** de otra etiqueta (unidad 13).
- **`hdr.asm`** lo genera `snes_rules` desde el `Makefile` (no se edita): el
  mapa de memoria (`.MEMORYMAP`, `SLOT`), `.ROMBANKS 128`, la cabecera
  (`.SNESHEADER`: nombre, `FASTROM`, `LOROM`, `ROMSIZE $0C`), los **vectores**
  de interrupción (`NMI VBlank`, → glosario: VBlank / NMI: cuando llega el VBlank la CPU salta a la
  etiqueta `VBlank` de la librería; `RESET tcc__start`) y, con FastROM,
  `.BASE $80`. Todo `.asm` del proyecto empieza con `.include "hdr.asm"`.

## La librería: PVSnesLib

PVSnesLib (4.6.0, la de `tools/pvsneslib/`) está **escrita en ensamblador**: los
`.c` del juego llaman a rutinas de `pvsneslib/lib/LoROM_FastROM/*.obj`, ya
ensambladas. El repositorio del proyecto no trae sus fuentes; están en GitHub,
en la etiqueta de la misma versión:
<https://github.com/alekmaul/pvsneslib/tree/4.6.0/pvsneslib/source>. Para que C
pueda llamarlas, cada rutina de la librería sigue **la misma convención de
tcc816** (parámetros en la pila desde `4,s`, `rtl`, resultado en `tcc__r0`). Lo
que se ve de ella en esta unidad:

| Función (firma de la cabecera) | Cabecera / fuente | Qué hace por dentro | Toca |
|---|---|---|---|
| `void WaitForVBlank(void);` | `interrupt.h` → <https://alekmaul.github.io/pvsneslib/interrupt_8h.html>; fuente `vblank.asm` | `php / sep #$20 / pha / lda #1 / sta.l vblank_flag / wai / lda.l vblank_flag / bne` (vuelve a `wai`) `/ pla / plp / rtl`: **duerme la CPU** (`wai`) hasta la próxima interrupción y repite mientras el NMI no haya borrado la bandera | `vblank_flag` (`$00:0030`) |
| `VBlank` (no se llama desde C: es el **vector NMI** de `hdr.asm`) | `vblank.asm` | con FastROM, `jml FVBlank`; guarda A, X, Y, D y DB; pone **D = `$0100`** (sus propios pseudorregistros, para no pisar los del programa interrumpido); si `vblank_flag` está a 1 copia la OAM por DMA (canal 7, 544 bytes); llama a la rutina de `nmiSet` con `jsl`; incrementa `snes_vblank_count`; lee los mandos; `rti` | `$2102`, `$4370..$4375`, `$420B`, `$4218..` |
| `void nmiSet(void (*vblankRoutine)(void));` | `interrupt.h` | guarda el puntero de la rutina que `VBlank` llamará en cada NMI | `nmi_handler` |
| `void *memset(void *b, int c, size_t n);` | `devkitsnes/include/string.h` (no está en `snes/*.h`); fuente `libc.asm` | `lda 4,s` / `sta $00` ... `tay` / `sep #$20` y el bucle **`dey / sta [$00],y / bne`**: 3 instrucciones por byte (leído en los bytes de la ROM, `capturas/rutinas_libreria.txt`) | memoria |
| `tcc__mul`, `tcc__div`, `tcc__udiv` (sin cabecera: las llama el compilador) | fuente `libtcc.asm` | multiplicar: desplazar y sumar; dividir: resta con restauración (la de con signo cambia los signos y llama a la de sin signo) | — |

En el `.sym`, `memset` (`$81:8371`), `tcc__mul` (`$81:8000`), `tcc__div`
(`$81:814F`) y `VBlank` (`$80:81AF`) salieron en la misma dirección en las cinco
compilaciones de esta unidad (cuatro del juego y la mini-ROM de la 15).
`WaitForVBlank` **no**: está en una sección que se coloca junto al código del
juego y se mueve con él (`$8E:D550`, `$8E:D43B`, `$8E:D98B`... según la
compilación; `$80:B1B8` en la mini-ROM). Las funciones del juego también se
mueven: mira siempre el `.sym` de la ROM que tienes abierta. Lo de la
columna «por dentro» está leído de los bytes de la ROM (`WaitForVBlank`,
`memset`, el principio de `VBlank`) o, para el resto, del fuente de la etiqueta
4.6.0. Ejemplo del SDK que usa `WaitForVBlank` y `nmiSet` desde C:
`tools/pvsneslib/snes-examples/graphics/Backgrounds/Mode1ContinuosScroll/Mode1ContinuosScroll.c` (`nmiSet`), y `hello_world/src/hello_world.c` (`WaitForVBlank` en el bucle).

## Cómo se trabaja esta unidad

1. Lee `demo_14_camara_anotado.asm`: el `camara.asm` que ya anotaste en la
   unidad 03, ahora con registros y banderas, y una **traza** a mano con dos
   valores de `x_pix`.
2. Haz el `ejercicio_14.md` (6 TODO): generar, anotar `nivel.asm`, explicar los
   dos `tcc__div` de `tile_del_bloque64`, el experimento de `guion[i]` (contar
   los `tcc__mul` y deshacerlo), leer un `switch` y parar Mesen en
   `camara_para`.
3. Compara con `solucion/` (`solucion_14.md`, `nivel_anotado.asm`, los `.asm`
   y los parches de los experimentos).

## Dónde está en el juego

| Qué | Dónde |
|---|---|
| Los `.asm` que genera tcc816 | `make src/<carpeta>/<archivo>.asm` en un clon (PLAN §6.4); copias en `asm_generado/` |
| Comparación con signo y booleano en X | `asm_generado/camara.asm` (`camara_para`) |
| Desplazamiento como bucle | `asm_generado/nivel.asm` (`nivel_columna_vram`, el `<< 5`) |
| `u8` que llama a `tcc__div` | `asm_generado/personajes.asm` (`tile_del_bloque64`), frente a `tile_del_bloque32` |
| Puntero que avanza frente a índice (`tcc__mul`) | `asm_generado/guion.asm` (`mantenidos`) y su comentario en `src/juego/guion.c` |
| `switch` y `PF_ENTERO` | `hud_debug_dibujar` en el `.asm` de `src/juego/hud_debug.c` |
| Ensamblador puro (directivas) | `data.asm`, `hdr.asm` (generado) |
| La única rutina del juego escrita a mano | `src/juego/dma_ejecutar.asm` (se lee en la unidad 15) |
| Las reglas de C que salen de todo esto | `CLAUDE.md` (reglas de tcc816), PLAN §6.6 |

## Trampas

- **El `.asm` se queda en `src/`**: `snes_rules` ensambla todo `src/*/*.asm` y
  un `camara.asm` olvidado choca con `camara.c`. Muévelo nada más generarlo.
- **Rutas relativas en PowerShell con .NET.** `cd` cambia la carpeta de
  PowerShell, pero `[IO.File]::ReadAllText("src/...")` resuelve contra la carpeta
  en la que arrancó el proceso: se puede acabar editando `E:\ProyectoSNES` en vez
  del clon (pasó al escribir esta unidad). Rutas absolutas siempre.
- **El valor devuelto no está en A**, está en `tcc__r0`. A veces coincide (el
  último `sta.b tcc__r0` deja el mismo valor en A), pero no es la regla.
- **Los `.asm` generados no tienen direcciones.** Para saber dónde acabó una
  función, el `.sym` de esa compilación; para la librería sirve cualquiera.
- **`sizeof` de una struct no es la suma de sus campos** (relleno a par):
  `TramoGuion` mide 6, no 5. Compruébalo en el `adc.w #` del `.asm`.
- **Mesen y las etiquetas**: si el depurador no muestra `camara_para`, importa
  el `.sym` a mano; y el breakpoint de dirección vale solo para ESA compilación.

## Ejercicio y criterio de aprobación

`ejercicio_14.md`. **Aprobación:** el `nivel.asm` anotado con las cinco piezas
identificadas; las respuestas de `tile_del_bloque64` (con la comprobación del
parche) y de `guion[i]` contadas en un `.asm` generado por ti, con el clon
limpio al terminar; y **una captura del depurador de Mesen parado en
`camara_para`** con los registros explicados y el valor de `tcc__r0` a la
salida igual a tu cuenta.

## Lo que viene

- **Unidad 15:** escribir una rutina en ensamblador y llamarla desde C (la
  mini-ROM `relleno.asm` y `numero` del HUD), con la convención que aquí se ha
  leído.
- **Unidad 16:** el depurador de Mesen a fondo (breakpoints de lectura y
  escritura, visores).
- **Unidad 17:** audio desde C con PVSnesLib (el driver de sonido también es
  ensamblador de la librería, pero se usa desde C).
