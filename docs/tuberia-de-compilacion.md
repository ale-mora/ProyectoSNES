# Tubería de compilación: de un `.c` a una ROM que arranca

**Para quién:** quien tenga que mantener el `Makefile`, entender un error de
compilación o explicar por qué un `.h` cambiado no se recompila.
**Qué es:** el camino completo que recorre el código y los gráficos hasta
`ProyectoSNES.sfc`, herramienta por herramienta y regla por regla, y cómo se
lee lo que sale por pantalla. Términos: `docs/glosario-snes.md`.

## 1. El camino de un archivo C

```
src/juego/tarou.c
   │  816-tcc   (compilador de C para el 65816; CFLAGS: -I del SDK, -I$(CURDIR), -Isrc, -Ires/personajes/tarou)
   ▼
src/juego/tarou.ps        ensamblador "crudo" que genera el compilador
   │  816-opt   (optimizador: quita redundancias del ensamblador generado)
   ▼
src/juego/tarou.asm       ensamblador optimizado, legible: ÚTIL PARA APRENDER 65816
   │  wla-65816 -d -s -x   (ensamblador: texto → código objeto)
   ▼
src/juego/tarou.obj       objeto con secciones (.text, .rodata, .bss) y etiquetas
   │
   │   + todos los demás .obj de src/**  + data.obj (los assets)  + los .obj de la librería
   ▼
linkfile                  lista de objetos que escribe la regla %.sfc
   │  wlalink -d -s -v -A -c -L <lib> linkfile ProyectoSNES.sfc
   ▼
ProyectoSNES.sfc          la ROM (16 bancos de 32 KB = 524 288 bytes)
ProyectoSNES.sym          etiquetas → direcciones (para el depurador de Mesen)
ProyectoSNES.log          todo lo que dijeron las herramientas + espacio libre por banco
```

Definiciones:

- **Compilador (`816-tcc`)**: traduce C a ensamblador del 65816, la CPU de la
  SNES. Es una variante de _Tiny C Compiler_: no optimiza casi nada, no tiene
  `float`, y las divisiones y multiplicaciones llaman a rutinas lentas de la
  librería (por eso en código por frame se evitan `/` y `%`).
- **Ensamblador (`wla-65816`)**: convierte el texto en ensamblador en bytes
  del procesador, organizados en **secciones** (bloques con nombre: `.text`
  código, `.rodata` constantes, `.bss` variables sin inicializar).
- **Enlazador (`wlalink`)**: coloca cada sección en un **banco** de la ROM
  (32 KB en LoROM), resuelve las etiquetas (`jsr tarou_actualizar` → una
  dirección) y escribe el `.sfc`. Con `-d` descarta las secciones que nadie
  referencia: por eso un stub o un asset no usado no gasta ROM.
- **`.sfc`**: el archivo de ROM que entiende cualquier emulador o cartucho
  flash. Los primeros 32 KB son el banco 0; la cabecera (nombre, mapa, vectores
  de interrupción) va en los últimos bytes de ese banco y la genera `hdr.asm`.

## 2. El camino de un gráfico

```
res/personajes/tarou/tarou_reposo.png     PNG indexado (≤ 16 colores, índice 0 = transparente)
   │  gfx4snes -s 32 -o 16 -u 16 -p -T -X 64 -Y 64 -P 3 -i tarou_reposo.png
   ▼
tarou_reposo.pic          los tiles (bloques de 32x32 empaquetados en filas de 128 px)
tarou_reposo.pal          la paleta: 16 colores de 15 bits (2 bytes cada uno)
tarou_reposo_meta.inc     tablas C `t_metasprite tarou_reposo_metasprites[n]` (por -T)
tarou_reposo.inc          declaraciones extern para C
   │
   │  data.asm: .section ".rodata_tarou_reposo" superfree / tarou_reposo_til: .incbin "...pic" / .ends
   ▼
data.obj  ─────────────► wlalink ─────────────► la ROM
```

Opciones de `gfx4snes` que usa el Makefile:

| Opción | Significa |
|---|---|
| `-s N` | tamaño del bloque: 8 (fondos), 32 (piezas de metasprite), 64 (sprites de 64×64) |
| `-o N` / `-u N` | colores que escribe en el `.pal` / colores por tile (16 = 4bpp, 4 = 2bpp, 256 = 8bpp) |
| `-p` | genera el `.pal` |
| `-m` | genera el mapa (`.map`) de un fondo; `-e N` pone la paleta N en las entradas del mapa |
| `-y` | mapa en páginas de 32×32 tiles (formato `SC_64x32`, para scroll); **exige alto múltiplo de 256** |
| `-T -X 64 -Y 64 -P 3` | tabla metasprite de 64×64 px con prioridad 3 |
| `-n` | tabla HDMA con un color por línea (`cielo_grad_data.as`) |
| `-t bmp` | la entrada es BMP (las fuentes) |

**Numeración de bloques** (verificada): con `-s 32`, el bloque `n` empieza en el
tile `(n/4)·64 + (n%4)·4`; con `-s 64`, en `(n/2)·128 + (n%2)·8`. Las tablas
metasprite numeran los bloques por filas de la imagen de origen (una tira de N
cuadros tiene 2N bloques por fila: el cuadro 0 de una tira de 4 usa los
bloques 0, 1, 8 y 9).

## 3. El `Makefile`, regla por regla

```make
export ROMNAME  := ProyectoSNES      # nombre de los archivos de salida
export ROMTITLE := TAROU DEMO        # nombre dentro de la cabecera de la ROM (21 caracteres máx.)
export ROMSIZE  := 09                # byte de tamaño de la cabecera: 09 = 512 KB (08 = 256 KB)
export ROMBANKS := 16                # cuántas secciones de 32 KB reparte el enlazador
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules   # las reglas genéricas del SDK (ver §4)
CFLAGS += -Isrc                      # #include "sistema/comun.h" desde cualquier .c
CFLAGS += -Ires/personajes/tarou     # tarou_anim.h generado por el pipeline

all: buildWithSummary                # lo que hace `make` a secas (resumen + exit 1 si hubo errores)
buildActual: bitmaps $(OBJS) $(ROMNAME).sfc   # primero convertir gráficos, luego compilar y enlazar
```

Variables y reglas propias, en orden de aparición:

| Regla / variable | Qué hace | Por qué está |
|---|---|---|
| `res/fuente/ark_consola.pic: ...bmp` | fuente de consola 4bpp (`-s 8 -o 2 -u 16 -p -e 0 -t bmp`) | la consola de texto de PVSnesLib |
| `res/fuente/ark_consola_2bpp.pic` | la misma fuente a 4 colores (`-u 4`) | BG3 en Mode 1 solo puede mostrar 2bpp |
| `res/personajes/lobo_samurai.pic`, `barbara_samurai.pic` | sprites de 64×64 (`-s 64 -o 16 -u 16 -p`) | Bárbara en intro/título; el lobo, futuro enemigo |
| `res/nintendo|publisher|intro/escena_3|devs/*.pic` | fondos de 16 colores con mapa (`-e 1 -m`) | pantallas 1–3 y escena 3 |
| `res/titulo/fondo_lejos.pic` (`-e 2`), `fondo_cerca.pic` (`-y`) | capas del título | lejana en paleta 2; cercana de 512 px en páginas de 32×32 |
| `res/intro/escena_1|2.pic` | imágenes de 256 colores (`-o 256 -u 256`) | Mode 3 |
| `res/titulo/cielo_grad_data.as` | tabla HDMA (`-n -t bmp`) | el degradado del cielo, una línea por color |
| `TAROU_ANIMS`, `TAROU_PICS`, regla patrón `res/personajes/tarou/%.pic` | las 13 tiras de Tarou más `tarou_reposo_estatico` → metasprites (`-s 32 -T -X 64 -Y 64 -P 3`) | un solo patrón para las 14 tiras |
| `res/nivel1/fondo_lejos.pic` | capa lejana del nivel (`-y -e 2`) | 512 px en páginas, paleta 2 |
| `ASSETS := ...` | la lista de todas las salidas de gfx4snes | `bitmaps: $(ASSETS)` las fuerza antes de compilar |
| `data.obj: $(ASSETS)` | `data.asm` se reensambla si cambia cualquier asset | `make` no ve los `.incbin`; sin esta línea los gráficos viejos se quedaban en la ROM |
| `tarou_sprite.obj: $(TAROU_PICS)`, `personajes.obj: …reposo_estatico.pic`, `animaciones.obj: tarou_anim.h` | los `.c` que incluyen archivos **generados** se recompilan cuando se regeneran | `snes_rules` no ve esos `#include`; sin estas líneas, tras rehacer el arte de Tarou el código seguía con las tablas viejas hasta un `make clean` |
| `cleanGfxRes` | borra las salidas de gfx4snes dentro de `res/` (el `cleanGfx` del SDK solo mira la raíz) | para que `make clean` deje `res/` como en git |
| `clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes` | limpieza total | lo ejecuta `run.bat` siempre |

## 4. `snes_rules`: lo que el SDK hace por nosotros

Es un Makefile incluido desde `tools/pvsneslib/devkitsnes/snes_rules`. Lo
importante:

- **Encuentra los fuentes solo**: `CFILES = src/*.c src/*/*.c src/*/*/*.c` (y
  lo mismo para `.asm` y `.dbg`); `OBJS` son esos nombres con `.obj`. Por eso
  basta con crear un `.c` en `src/juego/` para que entre en la ROM.
- **Genera `hdr.asm`** desde `hdr.asm.in` sustituyendo `ROMTITLE`, `ROMSIZE`,
  `ROMBANKS`, etc. (`AUTOHDR = 1`).
- **Reglas de patrón**: `%.ps: %.c` (816-tcc), `%.asm: %.ps` (816-opt),
  `%.obj: %.asm` (wla-65816), `%.sfc: $(OFILES)` (escribe `linkfile`, enlaza,
  limpia el `.sym`).
- **`buildWithSummary`**: ejecuta `make buildActual` volcando TODO a
  `ProyectoSNES.log`, imprime las líneas con `warning|error` y las de espacio
  libre, y devuelve error si hubo "error" en el log. Por eso, ante un fallo,
  **la causa real está en `ProyectoSNES.log`**, no en la pantalla.
- **Lo que NO hace**: no rastrea cabeceras. Si cambias `pantallas.h`, los `.c`
  que lo incluyen no se recompilan y los objetos viejos conservan los valores
  antiguos. Regla del proyecto: `make clean` después de tocar cualquier `.h`.

## 5. Tres formas de compilar

| Desde | Comando | Qué hace |
|---|---|---|
| PowerShell (lo que usa Claude Code) | `$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make` | compila lo que cambió. `make clean` antes si se tocaron cabeceras |
| CMD / doble clic | `run.bat` | `make clean` + `make` + abre Mesen; se para con `[ERROR]` |
| VS Code | tarea "Build SNES ROM" (`Ctrl+Shift+B`) o "Clean Project" | las mismas órdenes por `bash.exe` de MSYS2 |

La variable `PVSNESLIB_HOME` tiene que apuntar al SDK **en formato MSYS**
(`/e/ProyectoSNES/tools/pvsneslib`), porque `make` es el de MSYS2 aunque las
herramientas sean `.exe` nativos. `tools/setup_env.ps1` lo deja fijo en el
usuario; `run.bat` lo fija solo para su sesión.

## 6. Leer el resultado

Al final de `make` (o en el `.log`):

```
ROM bank 0 (10 bytes (0.03%) free)        ← el banco 0 está lleno (código + cabecera)
...
ROM bank 7 (32768 bytes (100.00%) free)   ← bancos vacíos
RAM slot 0 bank 126 (24445 bytes free)    ← WRAM de trabajo (variables)
ROM: 238927 bytes (45.57%) free of total 524288.
RAM: 154372 bytes (94.22%) free of total 163840.
```

- Una sección `superfree` de `data.asm` que no quepa en ningún banco da un
  error del enlazador: dividir el asset (tiles en una sección, mapa en otra).
- `DISCARD: ... Section ".rodata_tarou_atacar" was discarded` no es un error:
  nadie la referencia todavía (`wlalink -d`).
- `_TRY_PUT_LABEL: Label "SECTIONSTART_.bss" was defined more than once` sale
  con `-d` en todos los objetos con `.rodata`/`.bss`; es ruido de la
  información de depuración, no un error.
- `error: ICE 2` es un **error interno del compilador** `tcc816`: construcción
  de C que no soporta. Visto con expresiones `?:` de cadenas como argumentos
  variádicos; se reescribe con `if/else`.

## 7. Las pruebas en PC: otra tubería, mismo código

```
src/juego/tarou.c + tests/prueba_tarou.c + ...
   │  gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I. -I../src     (tests/Makefile)
   ▼
tests/pruebas.exe   → "191 pruebas, 0 fallos"
```

`-DPRUEBAS_PC` hace que `sistema/tipos.h` tome `u8/u16/s16` de `<stdint.h>`
en vez de `<snes.h>`, y `-I.` hace que `animaciones.c` encuentre la copia fija
`tests/tarou_anim.h`. La lógica pura compila con gcc **y** con tcc816: si una
construcción no funciona en uno de los dos, no vale.

Uso desde PowerShell: `$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe`
(`gcc` es el de WinLibs/MinGW que ya está en el PATH del usuario).

## 8. Los scripts de assets también son parte de la tubería

Antes de `make`, cuando cambia el arte:

| Script | Cuándo se ejecuta | Salida versionada |
|---|---|---|
| `python scripts/componer_escenas.py` | al cambiar logos, escenas, título o el logotipo | `res/<pantalla>/*.png`, `cielo.bmp` |
| `python scripts/preparar_personaje.py tarou` | al descargar arte nuevo de Pixellab (o para regenerar marcadores) | `res/personajes/tarou/*.png`, `tarou_anim.h` |
| `python scripts/preparar_fondo_nivel.py res/titulo/fondo_lejos.png res/nivel1/fondo_lejos.png --borrar 71,22,185,76` | si cambia la capa lejana del título | `res/nivel1/fondo_lejos.png` |
| `python -m unittest scripts/pruebas/prueba_preparar_personaje.py` (y `..._fondo_nivel.py`) | tras tocar los scripts | — |

Estas salidas sí se versionan (son pequeñas y el resultado del trabajo
manual); las de `gfx4snes` no (las regenera `make`).
