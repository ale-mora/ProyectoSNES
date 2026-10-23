# Unidad 04 — El frame, el VBlank y el bucle principal

**Objetivo:** entender el reloj del juego: un bucle que da 60 vueltas por
segundo sincronizado con la pantalla, qué se puede hacer en cada momento de
la vuelta y cómo se mide cuánto de ese tiempo gasta el juego.

**Prerrequisitos:** unidad 03 (la tubería; compilar una mini-ROM) y, a
través de ella, la 00. **Práctica:** mini-ROM + sobre el juego real (medir
con el HUD cuánto frame usa el nivel). **Tiempo orientativo:** dos tardes.

Hasta ahora `main` terminaba en un `while (1) { WaitForVBlank(); }` que «no
hacía nada». Esta unidad explica qué hace exactamente esa línea, por qué es
el corazón de todo juego de SNES y cómo la usa el juego para que Tarou se
mueva a 60 FPS sin romper la imagen.

## Conceptos

### Frame y línea del haz

La PPU (→ glosario: SNES; el chip que dibuja) no guarda la imagen entera en
ningún sitio: la **compone línea a línea** mientras la tele la muestra, de
arriba abajo, como un haz que barre la pantalla. Cada línea horizontal es una
**línea del haz** (*scanline*; → glosario: Línea del haz). En NTSC hay **262**
por imagen, numeradas 0..261, de las que se ven **224**. Una imagen completa
es un **frame** (→ glosario: Frame), y en NTSC se dibujan **60 por segundo**:
un frame dura 16,7 ms y una línea, unos 63,6 µs.

```
línea   0  ┌──────────────────────────────┐
           │                              │
           │   área visible: la PPU        │  224 líneas: la PPU está LEYENDO
           │   lee VRAM, CGRAM y OAM       │  la VRAM, la CGRAM y la OAM
           │   y dibuja                     │
línea 224  │                              │
línea 225  ├──────────────────────────────┤ ← empieza el VBlank: NMI
           │   VBlank (37 líneas)          │  la PPU no dibuja: se puede
línea 261  │                              │  escribir en VRAM, CGRAM y OAM
           └──────────────────────────────┘
  (la 0 del frame siguiente aún no dibuja nada: el juego la cuenta dentro del VBlank)
```

### VBlank

El **VBlank** (*vertical blank*, → glosario: VBlank / NMI) es el tramo del
final de cada frame en que la PPU no dibuja, mientras el haz «vuelve» arriba.
Va de la línea 225 a la 261 y el juego le suma la 0 del frame siguiente
(`desde_vblank()` en `src/pantallas/pantalla_nivel1.c:112`): **37 líneas**,
unos 2,3 ms.

### Qué solo se puede hacer en el VBlank

Mientras la PPU dibuja está leyendo la **VRAM** (tiles y mapas), la **CGRAM**
(paletas) y la **OAM** (sprites) (→ glosario: VRAM, CGRAM, OAM; se ven en las
unidades 06 y 08). Si la CPU escribe en ellas a la vez, **la escritura se
pierde sin aviso** (o rompe la imagen). Regla: **VRAM, CGRAM y OAM se escriben
solo en el VBlank**. Todo lo demás (la lógica del juego, la WRAM) se puede
hacer en cualquier momento.

### Forced blank: la excepción

El registro `INIDISP` (`$2100`) tiene un bit (el 7) que **apaga la pantalla**:
la PPU deja de dibujar y saca negro. Con la pantalla así (**forced blank**,
→ glosario: Forced blank) la VRAM se puede escribir **en cualquier momento**
y tanto como se quiera. Es lo que hace el juego cuando tiene que cargar mucho
de golpe: al entrar en una pantalla y al **reaparecer** Tarou tras caer a un
foso (4 KB de mapa más los tiles de Tarou: no caben en un VBlank). Ojo: el
final de un `fade_out` (brillo 0) **no** es forced blank (ver «La trampa de
`fade_out`»).

### Interrupción y NMI

Una **interrupción** es una señal del hardware que hace que la CPU deje lo
que está haciendo, ejecute una rutina concreta y luego siga donde estaba. La
**NMI** (*non-maskable interrupt*) es la que dispara la PPU **al empezar el
VBlank**, una vez por frame. La rutina que la atiende la pone el SDK (su
dirección está en la cabecera de la ROM: `NMI VBlank`, unidad 03). La NMI
solo llega si está habilitada en `NMITIMEN` (`$4200`); el arranque del SDK la
habilita antes de `main`.

### `WaitForVBlank()`: el metrónomo

`WaitForVBlank()` pone a 1 una bandera (`vblank_flag`) y **duerme la CPU**
(instrucción `wai`) hasta que la rutina de la NMI la pone a 0. Así, cada
vuelta del bucle

```c
while (1)
{
    secuencia_actualizar();   /* el trabajo del frame */
    WaitForVBlank();          /* esperar al VBlank    */
    secuencia_vblank();       /* lo que va justo detrás del VBlank */
}
```

dura **exactamente un frame**, siempre que el trabajo termine antes de que
llegue el VBlank siguiente. El bucle no «va rápido» ni «va lento»: va al
ritmo de la pantalla. (`src/sistema/main.c:62-67`.)

### Contar tiempo en frames: `SEGUNDOS(n)`

Como cada vuelta es un frame, el tiempo se cuenta en vueltas. El juego no
usa relojes: usa contadores que suben 1 por frame y las macros de
`src/sistema/pantallas.h:54-55`:

```c
#define FPS 60
#define SEGUNDOS(s) ((u16)((s) * FPS))
```

`DURACION_NINTENDO SEGUNDOS(3)` son 180 frames. `SEGUNDOS(3)` es una
constante: el compilador hace la multiplicación (unidad 03, ejercicio), así
que en la ROM no cuesta nada. Para el segundero de la mini-ROM no se divide
`frames / 60` (una división de variables llama a una rutina lenta, unidad 01):
se lleva un segundo contador que vuelve a 0 al llegar a `SEGUNDOS(1)`.

### Frame perdido (lag)

Si el trabajo de una vuelta **no** termina antes del VBlank, la NMI llega
cuando la CPU todavía no ha llamado a `WaitForVBlank()`. Cuando por fin lo
llama, la CPU duerme hasta el **siguiente** VBlank: esa vuelta ha durado dos
frames. Es un **frame perdido** (*lag frame*): el juego se ve a tirones y todo
lo que se mide en frames (un salto, un fundido) dura más en segundos.

```
        frame n                       frame n+1                     frame n+2
 ├── lógica ──────────┤ VBl │├── lógica (larga) ────────── VBl ──┤ espera │ VBl │├──
                       ▲                                    ▲              ▲
                       vuelve WaitForVBlank           la NMI llega y la     aquí vuelve
                                                       lógica aún sigue      WaitForVBlank
```

Dos contadores lo delatan: el de **vueltas del bucle** (el juego:
`frames_en_pantalla`, la `F` del HUD) y el de **VBlanks** del SDK
(`snes_vblank_count`, la `VB`), que sube en cada NMI pase lo que pase. A 60
FPS suben igual; si `F` sube menos que `VB`, hay frames perdidos. El SDK
además cuenta los perdidos en `lag_frame_counter`.

### El presupuesto del VBlank

37 líneas es poco. El VBlank empieza con el trabajo del SDK (copiar la OAM,
leer el mando: unidad 05) y después el juego hace el suyo. El HUD de
depuración del nivel lo mide en su fila `V a b` (→ glosario: Presupuesto del
VBlank): `a` es la línea del VBlank (contada desde su inicio) en que **entra**
`pantalla_nivel1_vblank()` y `b` en la que **sale**. Ejemplo de
`CLAUDE.md` (medida del 2026-10-21, FastROM): **`V5 7`** en un frame normal y
**`V5 17..20`** en los frames en que sube medio cuadro de Tarou. Es decir: el
SDK gasta 5 líneas, el juego 2 en un frame normal y unas 12–15 cuando copia
1,5 KB de tiles. Otras medidas de los documentos dan cifras parecidas pero no
idénticas (`docs/mesen-y-depuracion.md` §3: `V5 8` y `V5 19..23`;
`docs/hardware-snes-por-modulo.md` §8: «hacia la línea 23 del VBlank»): varían
según el momento y la versión del código; lo que importa es que `b` quede por
debajo de **37**. Si pasa de 37, lo que se escriba después cae con la PPU ya
dibujando la línea 1 **y se pierde**: el 2026-10-20 el HUD mostró `V5 46` y
Tarou salía a trozos.

### La lista de DMA (la idea)

El **DMA** (→ glosario: DMA / HDMA) es una máquina de la consola que copia
bloques de memoria a la VRAM mucho más deprisa que la CPU byte a byte. Para
gastar lo mínimo del VBlank, el juego separa **preparar** de **copiar**:

```
durante el frame (cualquier línea)          en el VBlank (37 líneas)
──────────────────────────────────          ─────────────────────────────
tarou_sprite_dibujar:  "copia 1,5 KB de      dma_lista_ejecutar(): recorre
   tiles de ROM banco $8x a VRAM 0x0400"  ─►    la lista y dispara un DMA
fondo_nivel_actualizar: "copia la columna       por entrada, sin pensar
   37 del mapa a VRAM 0x6C25"                   nada más
hud_debug_dibujar: "copia la fila 3 del HUD"
        (dma_lista_anadir: origen, destino, bytes, banco, modo)
```

Cada entrada son 8 bytes (`DmaItem` en `src/juego/dma_lista.h`). Durante el
frame, los módulos solo **apuntan** (`dma_lista_anadir`); en el VBlank,
`dma_lista_ejecutar()` (escrita en ensamblador, unidad 15) las dispara de un
tirón. Cómo se programa un DMA se ve en la unidad 06; la lista entera, en la
09 y la 20.

### La trampa de `fade_out`: acaba en `INIDISP = $00`, con la pantalla encendida

Un **fundido** (→ glosario: Fade) sube o baja el brillo de `INIDISP` paso a
paso, esperando un VBlank (o varios) entre paso y paso. `fade_out` termina
con **brillo 0 y la pantalla ENCENDIDA**: la PPU sigue dibujando, solo que en
negro (escribe `$00` en `INIDISP`: brillo 0 con el bit 7 a 0). Por eso, tras
un `fade_out`, la VRAM **sigue sin poder escribirse fuera
del VBlank**. El fallo real (2026-10-21): al volver del nivel al título, el
título copiaba los tiles de Tarou con la pantalla en negro pero encendida; la
copia se perdía y se veía a Tarou con trozos del nivel. El remedio está en
`src/sistema/secuencia.c:50`: `secuencia_cambiar()` llama a
**`setScreenOff()` antes de cada `_iniciar`**, así toda pantalla carga en
forced blank y se enciende ella misma con su fundido de entrada.

## La librería: PVSnesLib

Firmas copiadas de `tools/pvsneslib/pvsneslib/include/snes/*.h` (PVSnesLib
4.6.0). El comportamiento interno se ha leído en el código fuente del SDK
(`vblank.asm`, `videos.asm`, `consoles.asm` del repositorio de PVSnesLib),
porque `tools/` solo trae la librería ya ensamblada.

| Función / variable | Cabecera | Qué hace | Hardware |
|---|---|---|---|
| `void WaitForVBlank(void);` | [interrupt.h](https://alekmaul.github.io/pvsneslib/interrupt_8h.html) | pone `vblank_flag = 1` y repite `wai` hasta que la rutina de la NMI la pone a 0. Si la NMI está deshabilitada, no vuelve nunca | ninguno (espera la NMI) |
| `void nmiSet(void (*vblankRoutine)(void));` | interrupt.h | cambia la función que la rutina de la NMI llama en **cada** VBlank (`nmi_handler`), y de paso habilita la NMI y la lectura automática del mando | `NMITIMEN $4200` |
| `extern u8 vblank_flag;` | interrupt.h | 1 mientras el bucle espera en `WaitForVBlank`; dentro de una rutina propia de NMI, 0 significa «frame perdido» | — |
| `extern u16 lag_frame_counter;` | interrupt.h | sube en cada VBlank que llega con el bucle sin terminar | — |
| `extern u16 snes_vblank_count;` | [console.h](https://alekmaul.github.io/pvsneslib/console_8h.html) | sube 1 en cada VBlank, siempre (la `VB` del HUD) | — |
| `void setScreenOff(void);` | [video.h](https://alekmaul.github.io/pvsneslib/video_8h.html) | forced blank: escribe `$80` en `INIDISP` | `INIDISP $2100` |
| `void setScreenOn(void);` | video.h | **espera un VBlank** y escribe `$0F` (pantalla encendida, brillo 15) | `INIDISP $2100` |
| `void setBrightness(u8 level);` | video.h | brillo 1..15 (`INIDISP = level & $0F`); **con 0 escribe `$80`: forced blank**, no «brillo 0» (comprobado en el código de la 4.6.0 de la mini-ROM: `lda #$80` si el nivel es 0, `and #$0F` si no) | `INIDISP $2100` |
| `void setFadeEffectEx(u8 mode, u8 framesNumber);` | video.h | fundido `FADE_IN` (0→15) o `FADE_OUT` (15→0) en 16 pasos de `framesNumber` VBlanks cada uno: **bloquea** (llama a `WaitForVBlank` dentro) y el `FADE_OUT` acaba en `INIDISP = $00`: brillo 0 con la pantalla **encendida** | `INIDISP $2100` |

`fade_in(frames)` y `fade_out(frames)` de `src/sistema/comun.c:172-180` son
`setFadeEffectEx` con nombre propio; `FRAMES_FADE 2` (`pantallas.h:68`) da
2 × 16 = 32 frames por fundido.

### Qué hace la rutina de la NMI del SDK, en orden

1. Si el bucle estaba esperando (`vblank_flag` ≠ 0): copia el búfer de la OAM
   a la PPU por DMA (544 bytes; unidad 08).
2. Llama a `nmi_handler` (en **todos** los VBlanks, también en los perdidos).
3. Suma 1 a `snes_vblank_count`.
4. Si el bucle estaba esperando: pone `vblank_flag = 0` (eso despierta a
   `WaitForVBlank`) y lee el mando (unidad 05). Si no: suma 1 a
   `lag_frame_counter` y **no** lee el mando.

Cuando `WaitForVBlank()` vuelve, todo eso ya ha pasado: por eso el juego
entra en su `_vblank` hacia la línea 5 del VBlank (la `a` de `V5`).

### `nmiSet` y la advertencia de `consoleVblank`

Antes de `main`, el arranque llama a `consoleInit()`, que instala como
`nmi_handler` una rutina de la librería, **`consoleVblank`**, que copia a la
VRAM el mapa de texto cuando `consoleDrawText` lo ha cambiado. La
documentación de `nmiSet` avisa: *«This function will override the default
nmi_handler. If you are using consoleDrawText(), you will need to call
consoleVblank() inside vblankRoutine.»* Si instalas tu propia rutina y no
llamas a `consoleVblank`, **el texto deja de actualizarse**. Detalle de la
4.6.0: `consoleVblank` existe en la librería pero **ninguna cabecera la
declara**; hay que escribir `extern void consoleVblank(void);`. La mini-ROM lo
hace con `-DNMI_PROPIA` (comprobado: compila y enlaza).

Dos cosas más de una rutina propia: se ejecuta en **cada** VBlank, también en
los frames perdidos, cuando el bucle puede estar a medias de rellenar un búfer
(por eso el ejemplo del SDK mira `vblank_flag` antes de copiar nada); y tiene
que ser corta, porque gasta del mismo presupuesto de 37 líneas.

**El juego no usa `nmiSet`.** Su trabajo de VBlank va en el propio bucle,
justo después de `WaitForVBlank()` (`secuencia_vblank()`): solo se ejecuta en
frames buenos, ve los datos del frame ya terminados y es C normal. El precio
es que empieza unas líneas más tarde (tras el trabajo del SDK).

Ejemplos del SDK: `tools/pvsneslib/snes-examples/systems/timer/src/timer.c`
(contar VBlanks; base de la mini-ROM) y
`tools/pvsneslib/snes-examples/graphics/Backgrounds/Mode1ContinuosScroll/Mode1ContinuosScroll.c`
(una rutina propia, `myconsoleVblank`, instalada con `nmiSet`, que mira
`vblank_flag` antes de copiar).

## La práctica, paso a paso

### Paso 1 — La mini-ROM

`rom/src/contador.c` muestra cinco filas: `FRAMES` (vueltas del bucle),
`SECONDS` (segundero sin divisiones), `VBLANKS` (`snes_vblank_count`), `LAG`
(`lag_frame_counter`) y `LINE` (la línea del haz al terminar el trabajo del
frame, leída como en el juego). Para no gastar de más, reescribe **una fila
por frame** por turnos, como el HUD del juego: cada número se refresca cada 5
frames. Compílala con la orden de la unidad 03, desde `rom/`:

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
```

Resultado: «No warnings or errors found.» (`capturas/compilacion_rom.txt`).
Ábrela en Mesen. Qué debe verse:

- `FRAMES` y `VBLANKS` suben al mismo ritmo (60 por segundo); `SECONDS`
  avanza 1 por segundo; `LAG` se queda en 0. `VBLANKS` va unos pocos por
  delante de `FRAMES` desde el principio y la diferencia no crece: son los
  VBlanks que pasaron durante el arranque, antes del bucle. Por esa misma
  razón la mini-ROM pone `lag_frame_counter = 0` justo después de
  `setScreenOn()`: el SDK cuenta como «perdido» cualquier VBlank que llega sin
  que el bucle esté esperando, y durante la preparación de la pantalla eso
  pasa (lo avisa `interrupt.h`: el contador no distingue un frame perdido de
  una carga con la pantalla apagada).
- `LINE` es la línea en que el bucle termina su trabajo. El trabajo empieza
  justo después del VBlank (línea ~226), así que el número sale **dentro del
  VBlank** (225..261) o, si se pasa de la 261, «dado la vuelta» (0, 1, 2...).
  Anota el valor que veas.

Si `VBLANKS` sube más deprisa que `FRAMES`, Mesen no está al 100 % o el
bucle pierde frames: mira `LAG`. (Ojo: en Mesen, `Emulation.EmulationSpeed`
estuvo en 75 en este equipo y todo iba un 25 % más lento; ver
`docs/mesen-y-depuracion.md` §1. `snes_vblank_count` no lo nota: cuenta VBlanks
emulados, no segundos reales.)

### Paso 2 — La rutina propia de NMI (opcional)

```
$env:CFLAGS="-DNMI_PROPIA"; make clean; make; $env:CFLAGS=$null
```

Aparece `NMI n` junto a `LINE`: las veces que corrió `mi_vblank`. Sube igual
que `VBLANKS`. Después, comenta la línea `consoleVblank();` de `mi_vblank`,
recompila y mira: el texto se congela (los números se siguen calculando,
pero nadie copia el mapa a la VRAM). Deja la línea como estaba.

### Paso 3 — Medir el juego con el HUD

Con la ROM de prueba (unidad 03, `-DGUION_PRUEBA`) el nivel se juega solo.
Para medir sin tocar teclas, el ejercicio te pide encender el HUD desde el
primer frame en esa ROM (parte B). Las filas que importan:

```
CAM 0  F 1234        F: frames_en_pantalla (vueltas del bucle en el nivel)
LIN 65 120 170       línea del haz tras la lógica, tras el sprite y al final del frame
V5 7  VB 3738        V: líneas del VBlank al entrar y salir de _vblank; VB: snes_vblank_count
```

Cómo leer `LIN` sin equivocarse: el frame del juego empieza en el VBlank
(línea 225), así que lo gastado hasta la línea `L` son **`desde_vblank(L)`**
líneas (`pantalla_nivel1.c:112`): `L − 225` si `L ≥ 225` y `37 + L` si
`L < 225`. La parte del frame usada es **`desde_vblank(L) / 262`**. Ejemplo:
`LIN 65 120 170` → la lógica acabó a 37 + 65 = 102 líneas (39 %), el sprite y
las columnas a 157 (60 %) y el frame entero a 207 (79 %).

Un número en 225..261 es **ambiguo**: o es un frame baratísimo que acabó aún
dentro del VBlank, o uno que se pasó y dio la vuelta entera (casi 262 líneas
más). `LIN` solo no lo distingue: lo decide `F` frente a `VB` (si no se
pierden frames, era barato). Cifras de referencia de
`docs/mesen-y-depuracion.md` §3: en el tutorial (Revisión 2, tag
`plataformas-r2`) la lógica acaba hacia la línea
70–120 y el final del frame, con el HUD encendido, hacia 150–200.

**Por qué `F` puede avanzar menos que `VB`:** `F` cuenta vueltas del bucle y
`VB` cuenta VBlanks. Cada frame perdido es una vuelta que duró dos VBlanks.
Hasta el 2026-10-21 el HUD encendido costaba 50–150 líneas por fila y el
tutorial bajaba a ~47 FPS; hoy una fila cuesta 24–63 líneas y hay un **tope**
(`HUD_LINEA_MAX 150`, `src/juego/hud_debug.c:88`): si el haz pasó de la 150,
esa fila espera al frame siguiente. Con el HUD encendido se pierden unos 6
frames en todo el recorrido. Las filas se escriben por turnos (una por
frame): un número puede tener hasta 7 frames de retraso respecto a otro.

**La velocidad real, sin HUD:** el recorrido de la ROM de prueba son **2238
cuadros**; a 60 FPS, **37,3 s** desde que empieza el nivel. Si en la captura
el nivel tarda bastante más, se pierden frames (o Mesen no va al 100 %).

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| El bucle: `secuencia_actualizar(); WaitForVBlank(); secuencia_vblank();` | `src/sistema/main.c:62-67` |
| `setScreenOn(); setBrightness(0);` al arrancar: la primera pantalla hace su fundido desde negro | `src/sistema/main.c:45-46` |
| `secuencia_vblank()`: solo el logo Nintendo y el nivel tienen `_vblank` | `src/sistema/secuencia.c:108` |
| `setScreenOff()` antes de cada `_iniciar` (la trampa de `fade_out`) | `src/sistema/secuencia.c:50` |
| `FPS`, `SEGUNDOS`, las duraciones y `FRAMES_FADE` (con la nota de los 16 pasos) | `src/sistema/pantallas.h:54-70` |
| `leer_linea()` (`$2137`/`$213D`), `desde_vblank()` y los medidores `diag_linea[0..2]` | `src/pantallas/pantalla_nivel1.c:100-120`, `:217`, `:229`, `:233` |
| `pantalla_nivel1_vblank()`: medir, `dma_lista_ejecutar()`, medir | `src/pantallas/pantalla_nivel1.c:236` |
| `reaparecer()`: `fade_out` → `setScreenOff()` → reescribir el anillo y los tiles de Tarou → `fade_in` | `src/pantallas/pantalla_nivel1.c:132-150` |
| `_iniciar`: el bucle `do { ... dma_lista_ejecutar(); } while (!tarou_sprite_al_dia(&tarou));` que carga con la pantalla apagada | `src/pantallas/pantalla_nivel1.c:172-177` |
| La lista de DMA: `DmaItem`, `dma_lista_anadir`, `dma_lista_ejecutar` | `src/juego/dma_lista.h` |
| El HUD: filas `LIN` y `V a b  VB`, el tope `HUD_LINEA_MAX` | `src/juego/hud_debug.c:88`, `:223`, `:276-290` |
| El frame paso a paso (qué pasa en cada tramo de los 16,7 ms) | `docs/hardware-snes-por-modulo.md` §8 |
| Cómo usar `F`/`VB`, `LIN` y `V` para medir | `docs/mesen-y-depuracion.md` §3 |

## Trampas

- **El final de `fade_out` no es pantalla apagada.** Deja la PPU dibujando
  negro; las copias a la VRAM fuera del VBlank se pierden. Para cargar,
  `setScreenOff()` (forced blank). En el juego lo hace `secuencia_cambiar`.
- **Pasarse de la línea 37 del VBlank** pierde las escrituras sin aviso. La
  columna `V` del HUD lo vigila; Tarou a trozos (2026-10-20) fue eso.
- **`LIN` puede salir 225..261 o «dado la vuelta».** No es un fallo: la
  lógica empieza dentro del VBlank. Léelo con `desde_vblank()`, y si sale
  225..261 mira `F` frente a `VB` antes de concluir nada.
- **`setBrightness(0)` no es brillo 0: es forced blank** (`$80`); y el final
  de `fade_out` no es forced blank (`$00`). Para cargar la VRAM con calma,
  `setScreenOff()` explícito.
- **Una fila del HUD que no se refresca** en un frame cargado es el tope
  `HUD_LINEA_MAX`, no un fallo.
- **`nmiSet` sin `consoleVblank`** congela el texto de `consoleDrawText`.
  Y `consoleVblank` no está declarada: `extern void consoleVblank(void);`.
- **`WaitForVBlank` con la NMI deshabilitada no vuelve nunca.**
- **Mesen al 75 %** parece un juego lento: compara `F`/`VB` con el reloj
  (`docs/mesen-y-depuracion.md` §1).
- **Las `static` no empiezan en 0** en este proyecto: la mini-ROM las pone a
  mano antes del bucle (regla de `CLAUDE.md`; unidad 13).

## Ejercicio y criterio de aprobación

El ejercicio está en **`ejercicio_04.md`**: (A) hacer que el contador de
frames de la mini-ROM parpadee cada medio segundo; (B) encender el HUD desde
el primer frame en la ROM de prueba del juego y medir con él. Soluciones:
`solucion/contador.c` (A) y `solucion/hud_encendido_en_rom_de_prueba.patch`
(B; comprobada en un clon: 367 pruebas, 0 fallos, compila sin avisos;
`capturas/practica_juego.txt`).

**Aprobación:**

1. Captura de Mesen de la mini-ROM con el contador de frames visible y otra
   con él oculto (parte A), `LAG 0` y `SECONDS` avanzando.
2. Una nota con: el valor de `LIN` (los tres números) y de `V a b` leídos en
   una captura del nivel con el HUD encendido; el porcentaje de frame que usa
   ese frame, calculado con `desde_vblank(L) / 262` (si `L` cae en 225..261,
   di cuál de las dos lecturas es y por qué, mirando `F` y `VB`); dos lecturas de `F` y `VB` separadas
   unos segundos y la explicación de por qué `F` puede avanzar menos que `VB`.

## Lo que viene

- **Unidad 05:** el mando, que el SDK lee en esa misma rutina de la NMI.
- **Unidad 06:** el DMA de verdad: cargar tiles, paletas y mapas.
- **Unidad 12:** fundidos, HDMA y el motor de escenas, que también vive en el
  `_vblank` (el del logo Nintendo).
