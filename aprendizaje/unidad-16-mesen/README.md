# Unidad 16 — Mesen: depurador, puntos de ruptura y visores

**Objetivo:** diagnosticar un fallo gráfico o de física **sin adivinar**:
elegir la herramienta de Mesen que enseña la causa, y dejar una captura que lo
demuestre.

**Prerrequisitos:** 05 (mando, guion de mando), 08 (sprites, OAM, las dos
tablas de 256 tiles), 13 (direcciones de 24 bits, `.sym`, WRAM, `struct Tarou`
en memoria). **Práctica:** sobre el juego real (ROMs con un fallo provocado,
ya compiladas). **Tiempo orientativo:** dos tardes.

Esta unidad resume `docs/mesen-y-depuracion.md` con ejercicios. El emulador es
MesenCE 2.2.1 (`tools/mesen/Mesen.exe`). Las rutas de menú son las de Mesen 2;
si alguna difiere en tu versión, busca la herramienta por su nombre en el menú
**Debug**.

## Antes de empezar: las ROMs de la unidad

Todas están compiladas desde el código del 2026-10-22 (rama
`demo-secuencia-arranque`) en un clon, cada una con su `.sym` al lado y el
mismo nombre (Mesen busca las etiquetas así; si no las carga, en el depurador:
File → Import Labels → el `.sym`). Están en `E:\ProyectoSNES_trabajo\roms\`:

| ROM | Qué es | Cómo se hizo (parches en `solucion/`) |
|---|---|---|
| `u16_normal.sfc` | el juego tal cual | `make` |
| `u16_guion_hud.sfc` | ROM de prueba: arranca en el nivel, se juega sola y con el HUD encendido desde el primer frame | `-DGUION_PRUEBA -DHUD_AL_EMPEZAR` (+ `variante_hud_al_empezar.patch`: llama a `hud_debug_alternar()`, el mismo camino que SELECT) |
| `u16_caso1_barbara192.sfc` | Bárbara en el tile 192 | `caso1_barbara192.patch` (`TILE_BASE_BARBARA 192`, `VRAM_BARBARA 0x0C00`) |
| `u16_caso2_pies_flotan.sfc` | Tarou dibujado 2 px más arriba (sin hundir los pies) | `caso2_pies_flotan.patch` (`HUNDIR_PIES (-2)`), con `-DGUION_PRUEBA` |
| `u16_caso3_hud_sin_prioridad.sfc` | el HUD sin el bit de prioridad | `caso3_hud_sin_prioridad.patch` (`TILE(c)` sin `0x2000`), `-DGUION_PRUEBA -DHUD_AL_EMPEZAR` |

Para rehacerlas: clon fijado en el tag de la Revisión 2 (`git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u16`
y `git -C E:\ProyectoSNES_trabajo\u16 checkout -q plataformas-r2`: los parches y la ROM de prueba son del tutorial),
`git apply` del parche, `make clean`, `$env:CFLAGS="..."; make; $env:CFLAGS=$null`
con el entorno de la unidad 03, y copiar `.sfc` y `.sym` juntos.

## Conceptos

### Punto de ruptura (*breakpoint*)

Un **punto de ruptura** es una condición que **pausa** la emulación en el
instante exacto en que se cumple, con todo (registros, memoria, imagen) tal
como estaba. Tres clases:

- **de ejecución:** la CPU va a ejecutar la instrucción de esa dirección (el
  principio de una función: `camara_para`);
- **de lectura:** alguien **lee** esa dirección (o un rango: la rejilla de
  colisión);
- **de escritura:** alguien **escribe** en ella (un campo de una variable:
  `tarou.en_suelo`). Es la más útil para «¿quién me cambia esto?».

En Mesen se ponen en el **Debugger** (Debug → Debugger): clic en el margen de
una línea del código (ejecución) o en la lista de breakpoints, «Add», con el
tipo de memoria, la dirección o el rango y las casillas Read / Write / Execute.

### Paso a paso y paso a frame

Pausado, el depurador avanza **una instrucción** (*Step Into*), saltándose las
llamadas (*Step Over*) o hasta que la función vuelve (*Step Out*). En la
ventana del juego, Game → Pause y **avanzar un frame** (*Step Frame*, o su
tecla) enseña un fallo que dura un frame.

### Etiquetas del `.sym`

El `.sym` (unidad 13) da nombre a las direcciones: el depurador enseña
`jsl camara_para` en vez de `jsl $8ECFC0`. Las funciones y variables `static`
salen con nombre largo (`tccs_src/pantallas/pantalla_nivel1.asm_tarou`): que
Mesen las importe o no con la `/` en el nombre **no está comprobado**; por eso
la demo de esta unidad las busca en el texto del `.sym` y da la dirección
lista para escribirla.

### Los visores

| Visor (Debug →) | Enseña | Para qué aquí |
|---|---|---|
| **Tilemap Viewer** | cada fondo como lo ve la PPU, con su scroll; al pasar el ratón, la entrada del mapa (tile, paleta, **prioridad**, volteo) | por qué un texto queda detrás de otra capa |
| **Tile Viewer** | los tiles crudos de la VRAM en 2/4/8bpp | qué hay en el tile 192, o en las dos celdas de Tarou |
| **Sprite Viewer** | las 128 entradas de la OAM: x, y, tile, paleta, tamaño, prioridad, volteo | las 6 piezas de Tarou, su `y` |
| **Palette Viewer** | las 256 entradas de la CGRAM | una paleta pisada |
| **Memory Tools** | cualquier memoria (WRAM, VRAM, CGRAM, OAM, ROM), editable | ver `tarou` cambiar (unidad 13) |
| **Event Viewer** | cada escritura en un registro, en su línea y su ciclo del frame | el HDMA del cielo (una escritura en `$2122` por franja), los DMA del VBlank |
| **Profiler** | ciclos por función | complementa la fila `LIN` del HUD |
| **Trace Logger** | cada instrucción ejecutada | seguir la NMI del SDK |

### Las dos herramientas propias: el HUD y las capturas con guion

- **HUD de depuración** (SELECT en el nivel, `src/juego/hud_debug.c`): x, y,
  velocidades, estado, cámara, `LIN` (línea del haz al final de cada tramo del
  frame) y `V a b` (líneas del VBlank al entrar y salir de `_vblank`; el
  límite es 37). Una fila por frame, por turnos: un dato puede ir hasta 6
  frames por detrás de otro, y si el haz pasó de la línea 150 la fila espera
  (`HUD_LINEA_MAX`): no es un fallo (→ glosario: HUD de depuración).
- **`scripts/capturar_mesen.ps1`**: lanza Mesen al 100 % de velocidad, envía
  teclas según un guion de tiempos (`-Guion "desde,hasta,Tecla;..."`) y guarda
  capturas de la ventana del juego cada `-Cada` segundos desde `-Desde`
  (`docs/mesen-y-depuracion.md` §5). **Sus límites:** captura solo la ventana
  del juego (no las del depurador: esas, a mano); la primera captura con
  `-Desde` puede ser de un instante anterior; y las **teclas se pierden** si
  el foco de Mesen salta a su barra de menús, lo que pasa al usar el equipo
  durante la tanda. Lo fiable para el nivel es la **ROM de prueba** (guion de
  mando dentro de la ROM, `-DGUION_PRUEBA`): no necesita teclas.

### Medir antes de mirar

El quinto caso de esta unidad (Tarou a trozos, 2026-10-20) no se encontró
mirando, sino **midiendo**: la línea del VBlank al entrar y al salir de
`_vblank` (`V5 46`, con 37 de límite). Mirar dice *qué* se ve; medir dice
*cuánto* tiempo, *dónde* y *quién*. Regla de la unidad: antes de tocar código,
deja una captura o un número que señale la causa.

## La librería: PVSnesLib

| Función (cabecera) | Qué hace | Hardware |
|---|---|---|
| `void consoleMesenBreakpoint(void);` ([console.h](https://alekmaul.github.io/pvsneslib/console_8h.html)) | ejecuta la instrucción `WDM`, que no hace nada en la consola y en la que Mesen se para si en el depurador está marcado «Break on... WDM»: un breakpoint escrito en el código | CPU (opcode `WDM`) |
| `void consoleNocashMessage(char *fmt, ...);` ([console.h](https://alekmaul.github.io/pvsneslib/console_8h.html)) | escribe un texto en la ventana de mensajes de depuración de no$sns (un `printf` para el emulador) | registro de depuración del emulador |
| `extern u16 snes_vblank_count;` ([console.h](https://alekmaul.github.io/pvsneslib/console_8h.html)) | VBlanks desde el arranque (la `VB` del HUD) | contador de la NMI |

Ejemplos del SDK: `tools/pvsneslib/snes-examples/systems/breakpoints`
(`consoleMesenBreakpoint` entre cada paso de la inicialización) y
`systems/debug` (`consoleNocashMessage` con un contador). El juego no usa
ninguna de las dos: prefiere el HUD (se ve en la captura) y el `.sym`.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| El script de capturas, su guion de teclas, `-Desde`, `AllowBackgroundInput` | `scripts/capturar_mesen.ps1`; `docs/mesen-y-depuracion.md` §5 |
| La ROM de prueba: el mando sale de una tabla | `src/juego/mando.c` (`#ifdef GUION_PRUEBA`), `src/juego/guion.c`, `src/juego/guion_tutorial.c` |
| El HUD: `TILE(c)` con el bit de prioridad `0x2000`, `HUD_LINEA_MAX`, `hud_debug_alternar` | `src/juego/hud_debug.c:85-88`, `:189` |
| El límite de las dos tablas de tiles: bloque «LÍMITE», `TILE_BASE_BARBARA 256` | `src/pantallas/personajes.c:17-24`, `:83-84` |
| `HUNDIR_PIES` (2 px de dibujo + 2 del arte) | `src/juego/tarou_sprite.c:93`, `:205` |
| La columna `V` del HUD y el fallo de Tarou a trozos | `docs/diseno-tutorial-movimiento.md` §8; `docs/mesen-y-depuracion.md` §3 y §6 |
| La tabla de fallos conocidos | `docs/mesen-y-depuracion.md` §6 |

## Trampas

- **Mesen al 75 %:** si `Emulation.EmulationSpeed` no está en 100, todo va
  lento y parece un fallo del juego. El HUD lo delata (`F` y `VB` contra el
  reloj). El script de capturas fuerza el 100 %.
- **No usar el equipo durante una tanda de capturas con teclas.**
- **Las direcciones son de UNA compilación:** usa el `.sym` de la ROM que
  tienes abierta (por eso cada ROM de `roms\` lleva el suyo).
- **El HUD va por turnos** (hasta 6 frames de retraso entre filas) y una fila
  puede saltarse un frame (`HUD_LINEA_MAX`): para un instante exacto, el
  depurador.
- **La primera captura con `-Desde`** puede enseñar un instante anterior.
- **Un sprite grande no cruza la frontera de los 256 tiles** (unidad 08): se
  corrompe sin avisar.
- **Lo escrito en la VRAM después de la línea 37 del VBlank se pierde** sin
  aviso: se ve como sprites a trozos o columnas que faltan; la columna `V` lo
  mide.

## Cómo se trabaja esta unidad

1. **La demo** (`demo_16_sym.c`): busca etiquetas en un `.sym` y suma
   desplazamientos de campo, para escribir direcciones en el depurador.

   ```
   gcc -std=c99 -Wall -Wextra -o demo_16.exe demo_16_sym.c
   .\demo_16.exe E:\ProyectoSNES_trabajo\roms\u16_guion_hud.sym camara_para
   .\demo_16.exe E:\ProyectoSNES_trabajo\roms\u16_guion_hud.sym pantalla_nivel1.asm_tarou 11
   ```

   Salida de referencia: `capturas/salida_demo_16_sym.txt` (`camara_para` en
   `$8E:CFC0`; `tarou + 11`, el campo `en_suelo`, en `$7E:263C`).
2. **El ejercicio** (`ejercicio_16.md`): primero tres prácticas de
   herramientas (un breakpoint de ejecución, uno de escritura y el Event
   Viewer), después los cuatro casos reales del proyecto y el quinto, el de
   medir.
3. Compara con `solucion/casos_16.md` **después**.

## Ejercicio y criterio de aprobación

**Aprobación:** los cuatro casos de `ejercicio_16.md` resueltos, cada uno con
**la captura que lo demuestra** (la que dice cada caso) y una línea con la
causa; y el quinto, con la captura del HUD donde se leen `V5 7` (frame normal)
y `V5 17..20` (frame en que sube medio cuadro de Tarou). Las capturas de las
herramientas del depurador se hacen a mano (Mesen abierto, sin el script);
las del juego, con `capturar_mesen.ps1` y los comandos que da cada caso.

## Lo que viene

- **Unidad 14:** leer en el depurador el ensamblador que genera tcc816: el
  breakpoint en `camara_para` de este ejercicio, instrucción a instrucción.
- **Unidad 20:** la colisión: depurar con la traza de x/y por cuadro y la ROM
  de prueba.
