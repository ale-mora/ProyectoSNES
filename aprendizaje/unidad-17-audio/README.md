# Unidad 17 — Audio desde C con PVSnesLib

**Objetivo:** poner música y efectos de sonido en una ROM escribiendo solo C:
arrancar el chip de sonido, meter la música en la ROM con `smconv`, convertir
un efecto con `snesbrr`, y controlar desde el bucle del juego qué suena, cuándo
se pausa y a qué volumen.

**Prerrequisitos:** 04 (el frame, el VBlank y el bucle principal) y 05 (el
mando). **Práctica:** mini-ROM. **Tiempo orientativo:** una tarde.

> **El juego aún no aplica esta unidad.** Hoy la ROM del proyecto no suena:
> `src/audio/` es un *stub* (→ glosario: Stub) que solo documenta el contrato
> previsto. Esta unidad enseña lo necesario para escribirlo; al final,
> «Dónde está en el juego» relaciona cada función del contrato con las de la
> librería.

Aquí no se escribe ni se lee ensamblador del procesador de sonido: el SDK trae
un programa de audio ya hecho y todo se maneja con funciones de C.

## Conceptos

### Por qué el audio va aparte: el SPC700

La SNES tiene **dos procesadores**. La **CPU** (el 65816) ejecuta nuestro C.
El **SPC700** es un segundo procesador, de 8 bits, dedicado al sonido, con su
propia memoria: **64 KB de RAM de audio** que la CPU **no puede leer ni
escribir directamente** (→ glosario: SNES / Super Nintendo). Junto a él está
el **DSP** (*digital signal processor*, procesador de señal): el circuito que
de verdad fabrica el sonido. Tiene **8 voces**: cada voz toca una **muestra**
(*sample*, un sonido grabado: una nota de piano, un golpe de tambor) a la
altura y el volumen que se le pidan, y el DSP mezcla las 8 y saca el
resultado por los altavoces.

¿Qué gana la consola con un procesador aparte? Que **el sonido no depende del
frame**. El SPC700 ejecuta su propio programa (la partitura: «en este instante,
la voz 3 toca esta muestra con este tono») a su ritmo, sin esperar a la CPU.
Si el juego se retrasa un frame (unidad 04: se pierde un VBlank), la música
no se entrecorta. La CPU solo **da órdenes**: «carga esta canción», «tócala»,
«para», «dispara este efecto».

Consecuencia práctica: **antes de sonar, el SPC700 necesita un programa**. Al
encender la consola solo tiene un cargador mínimo grabado de fábrica (la *IPL
ROM*, 64 bytes) que sabe una cosa: recibir bytes de la CPU y copiarlos a su
RAM. Así que el primer paso de cualquier juego con sonido es enviarle el
programa de audio. A ese programa se le llama **driver de sonido**.

### Los puertos de comunicación (`$2140-$2143`)

La CPU y el SPC700 se hablan por **cuatro puertos** de un byte, que la CPU ve
en las direcciones `$2140`, `$2141`, `$2142` y `$2143` (en `sound.h` se llaman
`REG_APU00` .. `REG_APU03`). Un **puerto** es un registro compartido: lo que
la CPU escribe en `$2140` lo lee el SPC700 en su lado, y lo que el SPC700
escribe allí lo lee la CPU. Son en realidad dos buzones por dirección, uno
para cada sentido.

```
   CPU 65816 (nuestro C)                       SPC700 + DSP (8 voces)
 ┌───────────────────────┐   $2140 ◄──►   ┌───────────────────────────┐
 │ spcPlay, spcEffect... │   $2141 ◄──►   │ driver snesmod (en su RAM)│
 │ escriben un mensaje   │   $2142 ◄──►   │ lee el mensaje y obedece  │
 │ en los puertos        │   $2143 ◄──►   │ 64 KB: driver, canción,   │
 └───────────────────────┘                │ muestras, efectos         │
                                          └───────────────────────────┘
```

Cuatro bytes no dan para mucho, así que todo es un **protocolo**: la CPU
escribe una orden y sus datos, el driver contesta «recibido» por otro puerto, y
la CPU pasa al siguiente mensaje. Las funciones `spc*` de PVSnesLib hacen ese
baile; nosotros no tocamos los puertos. (Si algún día lo haces a mano, la
cabecera avisa: escríbelos **de 8 en 8 bits**, porque una escritura de 16 bits
en `$2140-$2141` puede estropear `$2143` en algunas consolas.)

### El driver del SDK: snesmod

PVSnesLib trae como driver **snesmod** (de mukunda, adaptado al C de la
librería). Tiene dos partes:

- **El programa del SPC700**, que `spcBoot()` copia a la RAM de audio usando
  el cargador de fábrica. Tarda un poco y desactiva las interrupciones
  mientras dura: por eso se llama **una vez, lo primero** de `main`.
- **Las funciones de C** que llamamos desde la CPU (`spcLoad`, `spcPlay`...).

Una idea importante: casi todas esas funciones **no hablan con el SPC700 en
el momento**: dejan un mensaje en una **cola** (una lista de órdenes
pendientes en la RAM de la CPU). Quien vacía la cola es **`spcProcess()`**,
que entrega mensajes por los puertos durante unas pocas líneas del frame y
vuelve. Si el bucle no llama a `spcProcess()`, las órdenes se quedan en la
cola y **no suena nada nuevo**. Por eso va en el bucle principal, una vez por
frame, justo antes de `WaitForVBlank()` (unidad 04), como en todos los ejemplos
del SDK. Lo dice la propia cabecera de `spcPlay`: *«this simply queues a
message»*.

`spcProcess()` además alimenta los **efectos BRR en streaming** (más abajo):
el driver los recibe a trozos, frame a frame. La cabecera lo deja claro:
*«this function MUST be called every frame if you are using streamed sounds»*.

### El módulo `.it`: la música como partitura

Grabar una canción entera ocuparía megas. La música de SNES se guarda como un
**módulo**: unas pocas muestras cortas (los instrumentos) más la **partitura**
(qué muestra suena en cada voz, en qué nota y cuándo). El formato que acepta
snesmod es **`.it` (Impulse Tracker)**: un **tracker** es un programa de
composición que guarda la canción así, en **patrones** (bloques de
compases) que se encadenan. Se compone en OpenMPT o Schism Tracker; en este
proyecto **no se compone**: se usan `.it` ya hechos.

Las reglas que un `.it` debe cumplir están en
`tools/pvsneslib/pvsneslib/pvsneslib_snesmod.txt`: **8 canales como máximo**
(las 8 voces del DSP), en modo instrumento, sin muestras estéreo, y todas las
muestras deben caber en **58 KB** de RAM de audio después de comprimirlas (los
64 KB menos el driver). Un `.it` que no las cumple puede fallar al convertirse
o sonar mal (`docs/diseno-secuencia-arranque.md` §8 cuenta cuáles de los
descargados pasan).

### El soundbank y `smconv`

Un **soundbank** es un archivo con todos los módulos del juego ya convertidos
al formato del driver, que vive en la ROM y del que `spcLoad(n)` copia el
módulo `n` al SPC700. Lo fabrica la herramienta **`smconv`** a partir de la
lista de `.it` (→ glosario: Soundbank / módulo .it).

`smconv -s -o res/soundbank -V -b 5 res/overworld.it` genera tres archivos:

| Archivo | Qué es |
|---|---|
| `res/soundbank.bnk` | los módulos convertidos, en binario (18 700 bytes con `overworld.it`) |
| `res/soundbank.asm` | `.BANK 5` / `.SECTION "SOUNDBANK"` / `SOUNDBANK__: .incbin "res/soundbank.bnk"`: lo mete en la ROM en el banco 5 |
| `res/soundbank.h` | un `#define` por módulo: `MOD_OVERWORLD 0` y `MOD_OVERWORLD_SIZE 17441` |

Opciones: `-s` modo soundbank (varios `.it` a la vez), `-o` la base del nombre
de salida, `-V` informe detallado (cuánta RAM del SPC700 usa cada módulo; sale
en el `.log` de la compilación) y `-b 5` el **banco** de la ROM donde empieza
(unidad 03: un banco son 32 KB). El soundbank necesita **bancos para él solo**:
el driver lo lee como un bloque seguido desde el principio del banco. Si hay un
`.it` de efectos (abajo), va **el primero** de la lista y se añade `-f`.

El informe de `-V` con `overworld.it` (real, de `capturas/compilacion_solucion.txt`
y el `.log`):

```
Pattern data: [ 4778 bytes]   Module Length: [12/200]
 Sample data: [12663 bytes]        Patterns: [11/64]
       Total: [17441 bytes]   *40516 bytes free*
```

Es decir: la canción usa 17 441 bytes de los 58 KB de la RAM de audio, y
quedan 40 516 libres para efectos.

El `Makefile` no llama a `smconv` a mano: basta declarar **antes del `include`
de `snes_rules`** dos variables, y el SDK hace el resto:

```make
AUDIOFILES := res/overworld.it        # los .it, en orden: el primero es el módulo 0
export SOUNDBANK := res/soundbank     # base del nombre de salida
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules
SMCONVFLAGS := -s -o $(SOUNDBANK) -V -b 5
musics: $(SOUNDBANK).obj
```

Si `AUDIOFILES` va **después** del `include`, `snes_rules` ya ha hecho su
lista de fuentes y el soundbank no entra en la ROM.

### BRR y `snesbrr`: los efectos

El DSP no toca audio sin comprimir: sus muestras van en **BRR** (*bit rate
reduction*), el formato de compresión del chip. Cada **bloque BRR** son **9
bytes**: uno de cabecera y 8 con **16 muestras de 4 bits**. Una muestra de 16
bits (2 bytes por valor) se queda en 9/32 de su tamaño.

La herramienta **`snesbrr`** convierte un `.wav` (mono, 16 bits) a `.brr`:
`snesbrr -e entrada.wav salida.brr`. `snes_rules` ya trae la regla de patrón
`%.brr: %.wav`, así que en el `Makefile` basta con pedir el `.brr`. Con el
salto de Mario (cifras reales de esta unidad):

```
mariojump.wav   mono, 16 bits, 12 000 Hz, 6682 muestras = 0,56 s   13 408 bytes
mariojump.brr   418 bloques de 9 bytes (+ 9)                       3 771 bytes
snesbrr informa: BRR Pitch Rate: 0x0600
```

El **tono** (*pitch*) dice a qué velocidad lee el DSP la muestra: `0x1000` es
32 kHz, la frecuencia nativa del DSP, así que `0x0600` son 12 kHz, la del
`.wav`. Por eso, al registrarlo, el tono es 6 (`spcSetSoundEntry` cuenta en
pasos de 2 kHz: 6 × 2 = 12 kHz). Con otro tono el efecto suena más agudo y
rápido, o más grave y lento.

### Dos maneras de hacer efectos con snesmod

| | Efectos de un `.it` | Efectos BRR en streaming |
|---|---|---|
| De dónde salen | instrumentos de un `.it` de efectos, el **primero** de `AUDIOFILES` (con `-f`) | un `.wav` convertido con `snesbrr`, incrustado en `data.asm` |
| Preparar | `spcLoad(0)` y `spcLoadEffect(i)` para cada uno | `spcAllocateSoundRegion(n)` y `spcSetSoundEntry(...)` |
| Disparar | `spcEffect(tono, i, volumen*16+pan)` | `spcPlaySound(i)` |
| Ejemplo del SDK | `snes-examples/audio/effects` y `effectsandmusic` | `snes-examples/audio/tada` y `systems/games/likemario` |

La mini-ROM usa la **segunda**, que es la que encaja con `mariojump.brr`: el
efecto no vive en el soundbank sino en la ROM normal, y el driver lo recibe a
trozos por los puertos (eso es el *streaming*) en la **región de sonido**: un
espacio de la RAM de audio que reserva `spcAllocateSoundRegion(n)`, en bloques
de 256 bytes (39 bloques ≈ 10 KB, como LikeMario). La primera manera se
estudia leyendo `effectsandmusic.c` (ver «Demos»).

## La librería: PVSnesLib

Firmas copiadas de `tools/pvsneslib/pvsneslib/include/snes/sound.h` (versión
4.6.0). Página de la cabecera:
<https://alekmaul.github.io/pvsneslib/sound_8h.html>. Todas tocan el SPC700 a
través de los puertos `$2140-$2143` (directamente o encolando un mensaje que
entrega `spcProcess`).

| Firma | Qué hace |
|---|---|
| `void spcBoot(void);` | copia el driver snesmod al SPC700 por el cargador de fábrica. Una vez, al principio; tarda y desactiva interrupciones |
| `void spcSetBank(u8 *bank);` | dice dónde empieza el soundbank: `spcSetBank(&SOUNDBANK__)` |
| `void spcAllocateSoundRegion(u8 size);` | reserva `size` × 256 bytes de RAM de audio para efectos BRR. **También para la música**: llamarla antes de `spcLoad`/`spcPlay` |
| `void spcLoad(u16 musIndex);` | copia el módulo `musIndex` (`MOD_OVERWORLD`) del soundbank al SPC700. Puede tardar (son kilobytes por cuatro puertos): fuera del juego en marcha, en la carga de una pantalla |
| `void spcPlay(u8 startPos);` | encola «toca el módulo cargado desde la posición `startPos`» (0 = el principio) |
| `void spcStop(void);` | detiene el módulo. Volver a tocarlo con `spcPlay(0)` empieza **desde el principio** |
| `void spcPauseMusic(void);` | detiene el módulo **guardando la posición** |
| `void spcResumeMusic(void);` | sigue desde la posición guardada (exige un `spcPauseMusic` antes) |
| `void spcSetModuleVolume(u8 vol);` | volumen de la música, 0..255 |
| `void spcFadeModuleVolume(u16 vol, u16 fadespeed);` | lleva el volumen hacia `vol` poco a poco (`fadespeed` cada 32 ms): un fundido de la música |
| `void spcProcess(void);` | entrega los mensajes encolados y alimenta los efectos en streaming. **Una vez por frame** |
| `void spcFlush(void);` | espera a que la cola se vacíe (sincroniza) |
| `void spcLoadEffect(u16 sfxIndex);` | carga el efecto `sfxIndex` del `.it` de efectos |
| `void spcEffect(u16 pitch, u16 sfxIndex, u8 volpan);` | dispara un efecto cargado con `spcLoadEffect`; `volpan` = volumen (0..15) × 16 + panorámica (0..15) |
| `void spcSetSoundEntry(u8 vol, u8 panning, u8 pitch, u16 length, u8 *sampleaddr, brrsamples *ptr);` | registra un efecto BRR: volumen 0..15, panorámica 0..15 (8 = centro), tono 1..6 (× 2 kHz), longitud en bytes, dirección en la ROM y la ficha `brrsamples` (8 bytes) que rellena |
| `void spcPlaySound(u8 sndIndex);` | dispara un efecto BRR registrado. **El índice 0 es el ÚLTIMO registrado**, el 1 el penúltimo... |
| `void spcPlaySoundV(u8 sndIndex, u16 volume);` | lo mismo con otro volumen (0..15) |

Otras funciones que usa la mini-ROM (vistas en las unidades 04 y 05):
`consoleDrawText(u16 x, u16 y, char *fmt, ...)` (**textfont.h**,
<https://alekmaul.github.io/pvsneslib/textfont_8h.html>), `padsDown(0)` (una
**macro** de input.h, `pad_keysdown[0]`,
<https://alekmaul.github.io/pvsneslib/input_8h.html>), `WaitForVBlank()`
(<https://alekmaul.github.io/pvsneslib/interrupt_8h.html>), `setMode`,
`setScreenOn` (<https://alekmaul.github.io/pvsneslib/video_8h.html>) y
`bgSetGfxPtr`, `bgSetMapPtr`, `bgSetDisable`
(<https://alekmaul.github.io/pvsneslib/background_8h.html>).

Ejemplos del SDK equivalentes: `tools/pvsneslib/snes-examples/audio/`
(`music`, `music2` con pausa, `effects`, `effectsandmusic`, `tada` con BRR) y
`tools/pvsneslib/snes-examples/systems/games/likemario/src/LikeMario.c`
(música + efecto BRR en un juego, función `main`).

## Cómo funciona: de los archivos al altavoz

**En el PC (`make`):**

```
res/overworld.it ──smconv──► res/soundbank.bnk + .asm (banco 5) + soundbank.h
res/mariojump.wav ─snesbrr─► res/mariojump.brr ──.incbin en data.asm──┐
res/pvsneslibfont.bmp ─gfx4snes─► .pic/.pal ──.incbin en data.asm─────┤
src/audio17.c (#include "res/soundbank.h") ──tcc816──────────────────┴─► audio17.sfc
```

`.incbin` es la directiva del ensamblador que pega un archivo binario tal cual dentro de una sección de
la ROM. Como `make` no ve esos `.incbin`, el `Makefile` de la mini-ROM declara
`data.obj: res/pvsneslibfont.pic res/mariojump.brr` (la misma idea que
`data.obj: $(ASSETS)` del juego).

**En la consola (`main` de la solución):**

```
spcBoot()                    el driver viaja al SPC700                 (una vez)
spcSetBank(&SOUNDBANK__)     dónde está el soundbank en la ROM
spcAllocateSoundRegion(39)   ~10 KB de RAM de audio para efectos
spcLoad(MOD_OVERWORLD)       la canción viaja al SPC700                (tarda)
spcSetSoundEntry(...)        se registra el salto: efecto 0
spcPlay(0); spcSetModuleVolume(128)      dos mensajes en la cola
bucle, 60 veces por segundo:
    padsDown(0)              ¿qué botón bajó?  A → spcPlaySound(0)
                                               Start → spcPauseMusic / spcResumeMusic
                                               ↑ ↓ → spcSetModuleVolume(volumen)
    spcProcess()             la cola sale por los puertos; el efecto avanza
    WaitForVBlank()
```

Mientras tanto el SPC700 toca la canción por su cuenta: si el bucle tardara
dos frames en dar la vuelta, la música seguiría igual; solo se retrasaría la
respuesta a los botones.

### Cuánto ocupa (medido)

| Qué | Bytes | De dónde sale la cifra |
|---|---|---|
| Soundbank en la ROM (banco 5) | 18 700 | el banco 5 tiene 14 068 libres de 32 768 |
| La canción en la RAM de audio | 17 441 de 57 957 | informe de `smconv -V` |
| La muestra del salto | 3 771 | `res/mariojump.brr` |
| Código de snesmod (driver + funciones C), más el poco C de los TODO | ~7 800 | banco 0: 16 295 libres sin llamadas `spc*`, 8 502 con ellas |

La última fila sale de comparar el resumen de compilación del ejercicio sin
resolver (`capturas/compilacion_ejercicio.txt`) con el de la solución: el
enlazador **descarta lo que nadie usa**. Sin una referencia a `SOUNDBANK__` el
banco 5 queda vacío (100 % libre), y sin llamadas `spc*` el driver no entra en
la ROM.

## Demos (para leer y oír)

El SDK trae los ejemplos ya compilados. Ábrelos en Mesen y lee su `.c`:

| ROM | Qué enseña |
|---|---|
| `tools/pvsneslib/snes-examples/audio/music/music.sfc` | lo mínimo: `spcBoot`, `spcSetBank`, `spcLoad`, `spcPlay`, `spcProcess` |
| `tools/pvsneslib/snes-examples/audio/music2/music2.sfc` | `spcPauseMusic` (A) y `spcResumeMusic` (B) |
| `tools/pvsneslib/snes-examples/audio/effectsandmusic/effectsandmusic.sfc` | efectos de un `.it` (`spcLoadEffect`/`spcEffect`) más música; recarga los efectos tras cada `spcLoad` |
| `tools/pvsneslib/snes-examples/audio/tada/tada.sfc` | un efecto BRR en streaming, sin música |
| `tools/pvsneslib/snes-examples/bin/likemario.sfc` | música y salto con sonido en un juego de plataformas |

Mapeo de teclado de Mesen en este equipo (`docs/mesen-y-depuracion.md` §2):
cruceta = flechas, A = tecla S, B = tecla A, Start = tecla D.

## Trampas

- **Olvidar `spcProcess()`.** La ROM compila y arranca, pero la música no
  empieza y los efectos no suenan: los mensajes se quedan en la cola. Va en el
  bucle, una vez por frame, **antes** de `WaitForVBlank()`, no en el código del
  VBlank: gasta líneas del frame, y el VBlank del juego ya va justo (37 líneas,
  unidad 04).
- **`spcAllocateSoundRegion` después de `spcPlay`.** La cabecera avisa: esa
  función **detiene el módulo**. Se llama antes de cargar y tocar la música.
- **El índice de `spcPlaySound` va al revés.** `spcPlaySound(0)` es el
  **último** efecto registrado con `spcSetSoundEntry`, no el primero.
- **Parar no es pausar.** `spcStop()` + `spcPlay(0)` empieza la canción desde
  el principio; para seguir donde estaba hacen falta `spcPauseMusic()` y
  `spcResumeMusic()`.
- **El volumen es un `u8`.** Un `u8` solo guarda 0..255: `volumen + 16` con
  `volumen = 248` guardado en un `u8` da 8, no 264 (el resultado «da la
  vuelta», se desborda). Hay que comparar antes de sumar.
- **Un `u8` en `consoleDrawText` con `%d` sale con basura en el byte alto.**
  La primera versión de la mini-ROM mostraba `VOLUME: -16256`, `12160`...: el
  byte bajo era el bueno (128 = `$80`, 144, 160) y el alto, basura
  (-16256 = `$C080`). tcc816 **no promociona** un `u8` a 16 bits en los
  argumentos `...` de una función variádica: lo empuja como **un byte**
  (`sep #$20` / `pha`) y `%d` lee **dos**; el segundo es lo que haya al lado en
  la pila. Comprobado compilando un `f("%d", c)` suelto con `816-tcc`
  (`capturas/tcc_u8_variadico.txt`): con `u8`, 1 byte y `add sp, #5`; con
  `(u16)c`, `pei` (2 bytes) y `add sp, #6`. Remedio: `(u16)volumen`. En gcc no
  pasa (C promociona `u8` a `int` en `...`).
- **Fondo magenta.** El color 0 de `pvsneslibfont.pal` es `$7C1F`, el magenta
  que el `.bmp` usa como color clave, y el color 0 de la CGRAM es el fondo de la
  pantalla (donde no hay nada dibujado). `setPaletteColor(0, RGB5(0, 0, 0))`
  justo después de `consoleInitText` lo deja negro (es una macro de tres
  sentencias: entre llaves).
- **`?:` con cadenas como argumento variádico** (`consoleDrawText(x, y, "%s",
  pausada ? "PAUSED" : "PLAYING")`) hace fallar a tcc816 (`PLAN.md` §6.6): dos
  llamadas en un `if`.
- **`AUDIOFILES` y `SOUNDBANK` antes del `include` de `snes_rules`**, y el `.it`
  de efectos (si lo hay) el primero de la lista.
- **El soundbank quiere bancos propios** (`-b 5`): no se mezcla con otras
  secciones en ese banco. Con músicas grandes ocupa varios bancos seguidos.
- **`spcLoad` tarda.** Copia kilobytes por cuatro puertos de un byte: se hace
  al cargar una pantalla (con la pantalla en negro), no en mitad del juego.
- **`spcBoot()` va lo primero** de `main`, antes de configurar el vídeo: tarda
  y desactiva las interrupciones mientras dura.
- **Mesen y el sonido.** Las capturas de pantalla no demuestran que suene: la
  aprobación es **oírlo** (con el volumen del PC encendido y Mesen sin
  silenciar).

## Ejercicio

En `rom/src/audio17.c` está la mini-ROM **sin sonido**: compila tal cual,
enseña los controles y el estado (`MUSIC: PLAYING`, `VOLUME: 128`), y no suena
nada. Completa los **TODO 1..9**, en orden:

| TODO | Qué escribir |
|---|---|
| 1 | arrancar el SPC700 |
| 2 | el soundbank (`&SOUNDBANK__`) y la región de efectos (`REGION_SONIDO`) |
| 3 | cargar `MOD_OVERWORLD` |
| 4 | registrar el salto con `spcSetSoundEntry` (los valores están en el comentario) |
| 5 | tocar la música desde el principio y fijar su volumen |
| 6 | A: disparar el efecto |
| 7 | Start: pausa/reanudar con la variable `pausada` |
| 8a, 8b | arriba/abajo: volumen de 16 en 16, recortado a 0..255 |
| 9 | entregar los mensajes al SPC700 cada frame |

Compila desde PowerShell, en `rom/`:

```
cd E:\ProyectoSNES\aprendizaje\unidad-17-audio\rom
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
& "E:\ProyectoSNES\tools\mesen\Mesen.exe" "E:\ProyectoSNES\aprendizaje\unidad-17-audio\rom\audio17.sfc"
```

Si algo falla, la causa está en `rom/audio17.log` (no en la pantalla: unidad
03). Después compara con `solucion/rom/src/audio17.c`; la carpeta
`solucion/rom/` es una mini-ROM completa que compila igual (`make` dentro de
ella).

Para ir más allá (sin solución): con `spcFadeModuleVolume(0, 4)` haz que
Select apague la música con un fundido en vez de cortarla.

## Criterio de aprobación

1. `make` en `rom/` termina con **`No warnings or errors found.`** y el resumen
   muestra el **banco 5 ocupado** (14 068 bytes libres): señal de que el
   soundbank entró en la ROM. Referencias: `capturas/compilacion_ejercicio.txt`
   (sin resolver: banco 5 al 100 % libre) y `capturas/compilacion_solucion.txt`.
2. En Mesen, **se oye**: la música suena sola al arrancar; A dispara el salto
   (también con la música en pausa); Start la pausa y la reanuda **en el mismo
   punto**, con `MUSIC:` cambiando entre `PLAYING` y `PAUSED`; arriba/abajo
   cambian `VOLUME:` de 16 en 16 entre 0 y 255 y la música sube y baja (a 0 no
   se oye, y el salto sí, porque su volumen es el suyo, no el del módulo).

## Dónde está en el juego

Todavía en ningún sitio: es el siguiente subsistema. Lo que hay:

| Qué | Dónde |
|---|---|
| El contrato previsto (cuatro funciones, comentadas) | `src/audio/audio.h` |
| El stub que solo enlaza (`audio_pendiente`) | `src/audio/audio.c` |
| Las líneas de `AUDIOFILES` y `SOUNDBANK`, comentadas, **antes** del `include` | `Makefile`, bloque «Audio (se activa en la tarea 3 del plan...)» |
| Formatos que sirven, `smconv`/`snesbrr` y límites | `docs/diseno-secuencia-arranque.md` §5 («Formatos que acepta el hardware/toolchain») |
| `.it` descargados y cuáles pasan `smconv` | `docs/diseno-secuencia-arranque.md` §8, `res/descargas/audio/candidatos_it/` |
| Por qué `spcProcess` obliga a `WaitForVBlank` en cada vuelta | `docs/diseno-secuencia-arranque.md` §4 («Bucle principal») |

El contrato de `audio.h`, traducido a lo que has escrito en la mini-ROM:

| Función prevista | Qué llamaría | Dónde se llamaría |
|---|---|---|
| `void audio_iniciar(void);` | `spcBoot()`, `spcSetBank(&SOUNDBANK__)`, `spcAllocateSoundRegion(n)` | `src/sistema/main.c`, antes de `secuencia_iniciar` |
| `void audio_musica(u8 id);` | `spcStop()`, `spcLoad(id)`, `spcPlay(0)` (y volver a registrar los efectos) | el `_iniciar` de cada pantalla, con la pantalla apagada (`secuencia_cambiar` ya llama a `setScreenOff`) |
| `void audio_efecto(u8 id);` | `spcPlaySound(id)` (BRR) o `spcEffect(...)` (efectos de `.it`) | `src/pantallas/pantalla_nivel1.c`: salto, espada |
| `void audio_procesar(void);` | `spcProcess()` | el bucle de `src/sistema/main.c`, entre `secuencia_actualizar()` y `WaitForVBlank()`: **no** en `secuencia_vblank()`, que gasta el presupuesto del VBlank |

Dos decisiones que tendrá que tomar quien lo implemente, y que esta unidad ya
deja medidas: los efectos del juego (salto, espada) encajan con los **BRR en
streaming** (cortos, en la ROM normal, sin ocupar el soundbank), y el soundbank
necesita **bancos propios**, que en la ROM de 4 MB del proyecto sobran (unidad
13: quedan más de 3 MB libres).

## Lo que viene

- **Unidad 13:** dónde vive cada byte: bancos, secciones `superfree` y por qué
  el soundbank pide un banco entero.
- **Unidad 18:** el `Makefile` y las herramientas de assets, `smconv` y
  `snesbrr` incluidas en la misma familia que `gfx4snes`.
