# Unidad 03 — De `.c` a `.sfc`: la tubería de compilación

**Objetivo:** saber qué hace cada herramienta que convierte el C del juego en
una ROM de Super Nintendo, leer lo que dicen al terminar y leer el
ensamblador que genera el compilador.

**Prerrequisitos:** unidad 00 (C: funciones, `#define`, `#include`, guarda de
inclusión, `extern`). **Práctica:** mini-ROM + lectura. **Tiempo
orientativo:** dos tardes (una para la tubería y la mini-ROM, otra para el
ejercicio del ensamblador).

En la unidad 00 todo se compilaba con `gcc` y se ejecutaba en el PC. Aquí el
mismo C sale del PC: lo traduce otro compilador, para otra CPU, y el
resultado es un archivo que entiende la consola (o el emulador Mesen). No hace
falta saber todavía qué hace cada función de la SNES: esta unidad trata del
**camino**, no del destino. El detalle de referencia está en
`docs/tuberia-de-compilacion.md`; aquí se resume y se practica.

## Conceptos

### CPU, instrucción y ensamblador (el lenguaje)

La **CPU** de la SNES es un **65816** (→ glosario: SNES): un procesador de 16
bits que ejecuta **instrucciones**, órdenes muy pequeñas codificadas en uno a
cuatro bytes («carga este número en el registro A», «súmale 48», «salta si es
negativo»). Un **registro** es una casilla de memoria dentro de la propia CPU;
el 65816 tiene pocos (A, X, Y y algunos más; la unidad 14 los ve todos). El
**lenguaje ensamblador** es esas mismas instrucciones escritas con nombres
cortos (`lda`, `adc`, `bmi`) en vez de bytes. Una línea de C suele dar varias
de ensamblador.

### Compilador, ensamblador y enlazador (las herramientas)

Tres programas, uno detrás de otro:

1. El **compilador** traduce C a ensamblador. El de la SNES es **`816-tcc`**
   (también llamado `tcc816`), una variante de *Tiny C Compiler*: casi no
   optimiza, no tiene coma flotante y, como el 65816 no sabe multiplicar ni
   dividir, convierte cada `*` y `/` entre variables en una llamada a una
   rutina lenta (por eso el juego evita `/` y `%` en el código de cada frame;
   unidad 01). Detrás va **`816-opt`**, un optimizador que limpia el
   ensamblador generado.
2. El **ensamblador** traduce el texto en ensamblador a bytes de la CPU. El
   de la SNES es **`wla-65816`** (del paquete WLA-DX).
3. El **enlazador** (*linker*) junta todos los trozos, decide en qué
   dirección de la ROM va cada uno, sustituye cada nombre por su dirección y
   escribe la ROM. Es **`wlalink`**.

Todos están en `tools/pvsneslib/devkitsnes/bin/` (los `.exe`) y
`tools/pvsneslib/devkitsnes/tools/` (`816-opt`, `gfx4snes`...). Nadie los
llama a mano: los llama `make` (más abajo).

### Código objeto, sección y etiqueta

El ensamblador no produce todavía la ROM: produce un **archivo objeto**
(`.obj`), bytes de código y datos que aún no saben en qué dirección van a
vivir. Dentro, los bytes están agrupados en **secciones**: bloques con nombre
que el enlazador coloca como una pieza (`.text` o similar para el código,
`.rodata` para constantes, `.bss` para variables sin valor inicial). Una
**etiqueta** es un nombre que señala una dirección: `camara_para:` marca
dónde empieza esa función. Cuando el código de otro archivo dice
`jsl camara_para`, es el enlazador quien pone ahí la dirección de verdad.

Una sección `SUPERFREE` (la que usa `tcc816` para cada función y `data.asm`
para cada gráfico) puede ir en el banco que el enlazador quiera, pero
**entera en uno**: nunca a caballo de dos (unidad 13).

### Banco de ROM, LoROM y FastROM

La CPU de la SNES ve direcciones de **24 bits**: un **banco** de 8 bits
(`$00..$FF`) y 16 bits dentro de él. El `$` delante significa hexadecimal
(unidad 00): `$80` es 128. En el mapa **LoROM** (→ glosario: Banco / LoROM)
la ROM se corta en trozos de **32 KB** y cada trozo se ve en la mitad alta
(`$8000-$FFFF`) de un banco. Por eso «banco de ROM» y «trozo de 32 KB» son
aquí la misma cosa.

**FastROM** (→ glosario: FastROM) es la otra mitad del mapa: los mismos
bancos vistos a partir de `$80` en vez de `$00`. Tiene dos efectos: el código
que corre en `$80-$FF` va a 3,58 MHz en vez de 2,68 MHz, y el enlazador puede
numerar 128 bancos (`$80-$FF`) sin pisar la WRAM, que ocupa los bancos
`$7E/$7F` (unidad 13). Para que `tcc816` genere código pensado para esos
bancos se le pasa la opción `-F`; `snes_rules` lo hace solo cuando el
`Makefile` dice `FASTROM := 1`.

### Cabecera de la ROM

Los últimos bytes del primer banco son la **cabecera**: el nombre del juego,
el mapa de memoria, el tamaño y los **vectores** (las direcciones a las que
salta la CPU al encenderse y al recibir cada interrupción). La genera
`hdr.asm`, que a su vez sale de las variables del `Makefile`:

| Variable | Juego | Significado |
|---|---|---|
| `ROMTITLE` | `TAROU DEMO` | nombre dentro de la cabecera (21 caracteres como máximo) |
| `ROMSIZE` | `0C` | tamaño en la cabecera: 2^n KB → `0C` = 12 → 2^12 KB = **4 MB** (`08` = 256 KB) |
| `ROMBANKS` | `128` | cuántos bancos de 32 KB reparte el enlazador: 128 × 32 KB = 4 MB |
| `FASTROM` | `1` | bancos en `$80-$FF`, CPU a 3,58 MHz, `tcc816 -F`, librería `LoROM_FastROM` |

En `hdr.asm` se ve, por ejemplo, `RESET tcc__start` (al encender, saltar al
arranque del SDK) y `NMI VBlank` (la rutina del VBlank, unidad 04).

### `Makefile`: regla, objetivo, dependencia y receta

**`make`** es un programa que decide qué hay que volver a construir mirando
**fechas de archivos**. Lee un `Makefile` hecho de **reglas**:

```make
objetivo: dependencia1 dependencia2
	receta (la orden que crea el objetivo)
```

El **objetivo** es el archivo que se quiere tener; las **dependencias**, los
archivos de los que sale; la **receta**, la orden que lo fabrica. Si el
objetivo no existe, o alguna dependencia es **más nueva** que él, `make`
ejecuta la receta. Si no, no hace nada. Una **regla de patrón** usa `%` como
comodín: `%.obj: %.asm` vale para todos los `.asm`.

### Archivo intermedio

Para ir de `hola.c` a `hola.obj`, `make` encadena tres reglas de patrón:
`.c → .ps → .asm → .obj`. Los `.ps` y `.asm` de en medio son **archivos
intermedios**: `make` los crea, los usa y **los borra** al terminar (es una
regla de GNU make para los archivos que solo existen por una cadena de
patrones). Por eso tras compilar no hay ningún `camara.asm` en `src/juego/`.
Si se pide el `.asm` **como objetivo**, `make` lo deja (receta de
`PLAN.md` §6.4, la del ejercicio).

### El `.log` y el `.sym`

- **`<ROMNAME>.log`**: todo lo que dijeron las herramientas, en orden. En
  pantalla, `make` solo imprime un **resumen** (las líneas con `warning` o
  `error`, o «No warnings or errors found.», y el espacio libre). Ante
  cualquier duda, **la verdad está en el `.log`**.
- **`<ROMNAME>.sym`**: la lista de etiquetas con su dirección final de 24
  bits. Mesen la usa para poner nombres de función en el depurador
  (unidad 16).

## Cómo funciona

### El camino de un `.c`

```
src/hola.c
   │  816-tcc -F        compilador: C → ensamblador "crudo"
   ▼
src/hola.ps             (intermedio: make lo borra)
   │  816-opt           optimizador
   ▼
src/hola.asm            (intermedio: make lo borra)
   │  wla-65816         ensamblador: texto → código objeto con secciones
   ▼
src/hola.obj
   │   + hdr.obj (la cabecera) + crt0_snes.obj, libc.obj, libm.obj, libtcc.obj (el SDK)
   │  wlalink -d        enlazador: coloca las secciones en bancos, resuelve etiquetas
   ▼
hola_mundo.sfc  (la ROM)   hola_mundo.sym (etiquetas)   hola_mundo.log (todo lo dicho)
```

El juego hace lo mismo con sus 36 `.c` de `src/*/`, más `data.asm` (los
gráficos, unidad 06) y `dma_ejecutar.asm` (ensamblador escrito a mano,
unidad 15). `wlalink -d` **descarta** las secciones que nadie usa: por eso en
el `.log` del juego salen líneas como
`DISCARD: ... Section ".barbara_pendientetext_0x0" was discarded.` (un *stub*)
o `".guion_mandotext_0x1" was discarded` (el guion de mando solo se usa en la
ROM de prueba, unidad 05). No son errores.

### Qué pasa al encender la consola

El `main()` del juego no es lo primero que se ejecuta. Lo que viene del SDK:

```
encendido → la CPU lee el vector RESET de la cabecera → tcc__start (crt0_snes)
   1. CPU en modo nativo de 16 bits, pila en $1FFF, interrupciones cortadas
   2. Snes_Init: pantalla apagada (INIDISP = $8F) y los registros de la PPU a cero
   3. copia la sección glob.data de la ROM a la WRAM: así es como las tablas
      `const` de C acaban en RAM (unidad 13)
   4. consoleInit(): deja lista la rutina del VBlank (unidad 04) y la lectura
      automática del mando (unidad 05)
   5. jsr.l main  → nuestro main()
```

(Leído en el código fuente del SDK, `crt0_snes.asm` y `consoles.asm` del
repositorio de PVSnesLib; la carpeta `tools/pvsneslib` solo trae los `.obj`
ya ensamblados, pero sus nombres se ven en la ROM: `tcc__start` está en
`$80:839A` tanto en la mini-ROM como en el juego.) Si `main` terminara, la CPU
se pararía (instrucción `stp`): por eso todo programa de SNES acaba en un
`while (1)`.

## La librería: PVSnesLib

PVSnesLib (versión **4.6.0**, la de `tools/pvsneslib/`; índice de la
documentación: <https://alekmaul.github.io/pvsneslib/files.html>) no es solo
una biblioteca de funciones: es también la tubería. Lo que aporta en esta
unidad:

| Pieza | Dónde | Qué hace |
|---|---|---|
| `snes_rules` | `tools/pvsneslib/devkitsnes/snes_rules` | el `Makefile` genérico que incluye cada proyecto: busca solo los fuentes (`src/*.c`, `src/*/*.c`, `src/*/*/*.c` y lo mismo con `.asm`), define las reglas de patrón `%.ps: %.c` (`816-tcc`, con `-F` si `FASTROM`), `%.asm: %.ps` (`816-opt`), `%.obj: %.asm` (`wla-65816 -d -s -x`) y `%.sfc` (escribe `linkfile` con todos los objetos más los del SDK y llama a `wlalink`), genera `hdr.asm` desde `devkitsnes/include/hdr.asm.in` y ofrece `buildWithSummary` |
| `buildWithSummary` | en `snes_rules` | ejecuta `make buildActual` volcándolo todo al `.log`, imprime las líneas con `warning|error` (o «No warnings or errors found.») y las de espacio libre, y sale con error si el `.log` contiene «error» |
| `crt0_snes.obj` y `libc.obj` | `pvsneslib/lib/LoROM_FastROM/` | el arranque (`tcc__start`), la rutina del VBlank y todas las funciones de la librería, ya ensambladas; el enlazador solo mete las que se usan |

Y las funciones que usa la mini-ROM (firmas copiadas de
`tools/pvsneslib/pvsneslib/include/snes/*.h`):

| Función | Cabecera | Qué hace | Hardware |
|---|---|---|---|
| `void consoleInit(void);` | [console.h](https://alekmaul.github.io/pvsneslib/console_8h.html) | inicialización genérica; **la llama el arranque del SDK antes de `main`**, no la llama nuestro código | la rutina del VBlank y la lectura del mando (unidad 04) |
| `void consoleSetTextMapPtr(u16 offsetfont);` | [textfont.h](https://alekmaul.github.io/pvsneslib/textfont_8h.html) | dónde (dirección de VRAM, en palabras) escribirá la consola el mapa de texto | ninguno todavía |
| `void consoleSetTextGfxPtr(u16 vramfont);` | textfont.h | dónde copiará la consola los tiles de la fuente | ninguno todavía |
| `void consoleInitDefaultText(u8 palnum);` | textfont.h | copia la fuente que trae la librería y su paleta | VRAM y CGRAM (por DMA) |
| `void consoleDrawText(u16 x, u16 y, char *fmt, ...);` | textfont.h | escribe un texto (admite `%d`, `%u`, `%x` como `printf`) en un búfer en RAM; la copia a la VRAM la hace la rutina del VBlank | VRAM, en el VBlank |
| `void bgSetGfxPtr(u8 bgNumber, u16 address);` | [background.h](https://alekmaul.github.io/pvsneslib/background_8h.html) | de qué dirección de VRAM saca los tiles un fondo | `BG12NBA $210B` |
| `void bgSetMapPtr(u8 bgNumber, u16 address, u8 mapSize);` | background.h | dónde está el mapa de un fondo y su tamaño (`SC_32x32`...) | `BG1SC $2107`... |
| `void bgSetDisable(u8 bgNumber);` | background.h | apaga un fondo | `TM $212C` |
| `void setMode(u8 mode, u8 size);` | [video.h](https://alekmaul.github.io/pvsneslib/video_8h.html) | modo de vídeo (`BG_MODE1`...) | `BGMODE $2105` |
| `void setScreenOn(void);` | video.h | espera un VBlank y enciende la pantalla a brillo 15 | `INIDISP $2100` |
| `void WaitForVBlank(void);` | [interrupt.h](https://alekmaul.github.io/pvsneslib/interrupt_8h.html) | duerme hasta el siguiente VBlank (unidad 04) | ninguno (espera la interrupción) |

Qué es un tile, un mapa o un fondo se ve en la unidad 06; qué es el VBlank,
en la 04. Para esta unidad basta con saber que estas llamadas preparan una
capa de texto. Ejemplo del SDK equivalente:
`tools/pvsneslib/snes-examples/hello_world/src/hello_world.c` (la mini-ROM
parte de él).

## La práctica, paso a paso

La mini-ROM está en `rom/`: `Makefile`, `src/hola.c` y `src/textos.h`. Es el
`hello_world` del SDK con dos cambios: textos propios y `FASTROM := 1`, como
el juego.

### Paso 1 — Compilar

Desde **PowerShell** (el Bash de Claude Code no puede ejecutar `wla-65816`,
que escribe un archivo temporal en la raíz de la unidad), dentro de `rom/`:

```
cd E:\ProyectoSNES\aprendizaje\unidad-03-tuberia-de-compilacion\rom
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
```

`PVSNESLIB_HOME` va en formato MSYS (`/e/...`, con barras normales) porque
`make` es el de MSYS2 (`C:\msys64\usr\bin`) aunque las herramientas sean
`.exe` de Windows; `snes_rules` se niega a seguir si ve una `\`. La salida
(completa en `capturas/compilacion_rom.txt`):

```
LoROM FastROM compilation is enabled
--- COMPILATION SUMMARY ---
No warnings or errors found.
---------------------------
ROM bank 0 (20379 bytes (62.19%) free)
ROM bank 1 (28550 bytes (87.13%) free)
ROM bank 2 (32768 bytes (100.00%) free)
...
ROM: 245537 bytes (93.66%) free of total 262144.
RAM: 155987 bytes (95.21%) free of total 163840.
```

Sin `ROMSIZE`/`ROMBANKS` en su `Makefile`, la mini-ROM toma los valores por
defecto de `snes_rules`: `08` y 8 bancos, 256 KB (262 144 bytes). El código
de la librería y el nuestro ocupan unos 16 KB del banco 0 y algo del 1.

### Paso 2 — Mirar qué quedó

```
Get-ChildItem rom, rom\src
```

Quedan `hola_mundo.sfc`, `.sym`, `.symfull`, `.log`, `hdr.asm`, `hdr.obj`,
`linkfile` y `src/hola.obj`, pero **ni `hola.ps` ni `hola.asm`**: eran
intermedios. Abre `hola_mundo.log` y busca las cuatro órdenes:
`Compiling to .ps ... hola.c` (con la línea completa de `816-tcc ... -F`),
`Assembling ... hola.ps` (`816-opt`), `Doing obj files from asm`
(`wla-65816`) y `Linking ... hola_mundo.sfc` (`wlalink`). Debajo verás líneas
`_TRY_PUT_LABEL: Label "SECTIONSTART_.rodata" was defined more than once.`:
ruido de la información de depuración, no un error.

### Paso 3 — Verla en Mesen

```
& "E:\ProyectoSNES\tools\mesen\Mesen.exe" "E:\ProyectoSNES\aprendizaje\unidad-03-tuberia-de-compilacion\rom\hola_mundo.sfc"
```

Tiene que verse, en blanco sobre negro: `HELLO, SNES!` (fila 10),
`UNIT 03: FROM .C TO .SFC` (fila 13) y `BUILT WITH PVSNESLIB` (fila 16).

### Paso 4 — La trampa de las cabeceras, en vivo

`snes_rules` sabe que `hola.obj` sale de `hola.c`, pero **no sabe que
`hola.c` incluye `textos.h`**: no rastrea las cabeceras. Compruébalo:

1. Cambia en `src/textos.h` el texto de `TEXTO_SALUDO` a `"HELLO, WORLD!"`.
2. `make` (sin `clean`). El resumen dice «No warnings or errors found.»
   pero en el `.log` **no aparece** `Compiling to .ps ... hola.c`: solo se
   reensambla `hdr.asm` (se regenera en cada `make`, así que siempre es
   nuevo) y se vuelve a enlazar el `hola.obj` viejo.
3. Busca el texto dentro de la ROM:
   `grep -c "HELLO, WORLD" hola_mundo.sfc` → `0`; con `"HELLO, SNES"` → `1`.
   La ROM sigue con el texto viejo.
4. `make clean` y `make`: ahora sí aparece `Compiling to .ps ... hola.c` y el
   `grep` da la vuelta (0 y 1).
5. Deja `textos.h` como estaba, `make clean` y `make`.

(`grep` viene con MSYS2, que ya está en el `PATH` de la orden de compilar.
Resultado real en `capturas/experimento_cabecera.txt`.) Esta es la regla del
proyecto: **después de tocar cualquier `.h`, `make clean` antes de `make`**.
En el juego ya costó un fallo: unos tiempos de fundido de `pantallas.h` que
no cambiaban.

### Paso 5 — Bancos en el `.sym`

En `hola_mundo.sym`, `main` está en `0080a74e`: banco `$80`, dirección
`$A74E`. En el `.sym` del juego (resumen en
`capturas/compilacion_juego_resumen.txt`) `main` está en `008ed147` y
`camara_para` en `008ecbc7`: banco `$8E`. Las dos ROM empiezan en `$80`
porque son FastROM; el juego tiene tanto código y gráficos que el enlazador
ha repartido sus secciones por los bancos `$80..$8E`. Las variables, en
cambio, están en la WRAM: `frames_en_pantalla` en `007e26e9` (banco `$7E`).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| `ROMNAME`, `ROMTITLE`, `ROMSIZE 0C`, `ROMBANKS 128`, `FASTROM 1` con su explicación | `Makefile:6-32` |
| `include ${PVSNESLIB_HOME}/devkitsnes/snes_rules` y los `CFLAGS += -Isrc -Ires/personajes/tarou -Ires/nivel1` que van **después** (las recetas expanden `CFLAGS` al ejecutarse) | `Makefile:35-43` |
| `all: buildWithSummary` y `buildActual: bitmaps $(OBJS) $(ROMNAME).sfc`: primero los gráficos, luego compilar y enlazar | `Makefile:50-51` |
| `data.obj: $(ASSETS)` y las dependencias de los `.obj` que incluyen archivos **generados** (`tarou_sprite.obj`, `animaciones.obj`, `pantalla_nivel1.obj`): el remedio a mano de que `snes_rules` no rastree inclusiones | `Makefile:142-152` |
| El `.asm` que genera `tcc816` para `camara_para` | `src/juego/camara.c` → `camara.asm` de esta carpeta (ejercicio) |
| El `.log` del juego: 15 bancos usados de 128, 3 713 537 bytes libres (88,54 %) | `ProyectoSNES.log` (tras compilar; extracto en `capturas/compilacion_juego_resumen.txt`) |
| Las tres formas de compilar (PowerShell, `run.bat`, VS Code) y la ROM de prueba con `CFLAGS` | `docs/tuberia-de-compilacion.md` §5 |
| El segundo camino del mismo C: `gcc` para las pruebas de PC (`-DPRUEBAS_PC`) | `tests/Makefile`, `docs/tuberia-de-compilacion.md` §7 |

En el `.log` del juego el banco 0 sale con `10 bytes (0.03%) free` y los
bancos 1..13 con 0..5 bytes libres: el enlazador los llena como un puzle con
secciones de todos los tamaños, y por eso el banco 14 queda a medias
(10 723 bytes libres) y del 15 al 127 vacíos. Compilar el juego desde cero
(`make` tras clonar, con todos los gráficos) tardó **87 s** en este equipo;
`run.bat` siempre hace `make clean` antes, así que siempre tarda eso.

## Trampas

- **`make` no rastrea los `.h`** (paso 4). Tras tocar una cabecera:
  `make clean`. El resumen no avisa de nada.
- **El resumen no es el `.log`.** Si algo no cuadra, abre el `.log`.
- **Un `.asm` generado para estudiar se queda en `src/`** si se pide como
  objetivo. `snes_rules` ensambla **todo** `src/*/*.asm`: un `camara.asm`
  olvidado junto a `camara.c` da símbolos duplicados. Cópialo fuera y bórralo
  enseguida (PLAN.md §6.4).
- **La otra cara:** un `.asm` escrito a mano no puede llamarse como ningún
  `.c` (`make` lo sobrescribiría con su intermedio). Por eso la rutina del
  juego se llama `dma_ejecutar.asm` y no `dma_lista.asm` (unidad 15).
- **`error: ICE 2`** de `816-tcc` es un fallo interno del compilador ante una
  construcción que no soporta: en este proyecto, `?:` con cadenas como
  argumento de una función variádica. Se reescribe con `if/else`.
- **`PVSNESLIB_HOME` con `\`**: `snes_rules` se para con un error. Va con
  barras normales y la letra de unidad en minúscula: `/e/...`.
- **`DISCARD` y `_TRY_PUT_LABEL`** en el `.log` no son errores (ver arriba).

## Ejercicio y criterio de aprobación

El ejercicio está en **`ejercicio_03_leer_asm.md`**: generar el ensamblador
de `src/juego/camara.c` en tu propio clon, anotarlo línea a línea y
responder tres preguntas. La solución anotada está en
`solucion/camara_anotado.asm` (mírala después, no antes).

**Aprobación:**

1. La mini-ROM compila con «No warnings or errors found.» y en Mesen se ven
   las tres líneas del paso 3.
2. El paso 4 hecho: el `grep` de la ROM da 1/0 tras el `make` sin `clean` y
   0/1 tras `make clean` + `make`.
3. Tu `camara.asm` anotado identifica: **la carga del parámetro** desde la
   pila (`lda 3 + __camara_para_locals + 1,s`), **la suma de la media celda**
   (`adc.w #48`), **la resta de la media pantalla** (`sbc.w #128`), **los dos
   recortes** (por abajo con `bvc`/`bmi` y `stz`; por arriba comparando con
   `#1792`, `ANCHO_NIVEL − ANCHO_PANTALLA`) y **el retorno** (el valor en
   `tcc__r0` y `rtl`).

## Lo que viene

- **Unidad 04:** qué hace `WaitForVBlank()` y por qué el bucle de `main`
  da exactamente 60 vueltas por segundo.
- **Unidad 13:** bancos, secciones `superfree` y `.incbin` a fondo, y cómo
  se leen el `.log` y el `.sym` para encontrar memoria.
- **Unidad 14:** el ensamblador del 65816 en serio (registros, modos de
  direccionamiento, la pila); el `camara.asm` de esta unidad vuelve allí.
