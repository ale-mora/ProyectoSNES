# El hardware de la SNES, módulo por módulo

**Para quién:** quien quiera saber qué chip y qué registros hay detrás de cada
función de PVSnesLib que usa el código, y dónde vive cada cosa en memoria.
**Qué es:** la SNES en tres chips (CPU, PPU, SPC700) y cuatro memorias (ROM,
WRAM, VRAM, CGRAM) más la OAM, explicados a partir de los módulos de `src/`.
Las direcciones de registro van en hexadecimal (`$2100`); las de VRAM, en
palabras de 2 bytes como las escribe el código. Términos: `docs/glosario-snes.md`.

## 1. La máquina en una página

```
                 ┌──────────────── cartucho ────────────────┐
                 │ ROM 512 KB = 16 bancos LoROM de 32 KB    │  código + data.asm
                 └───────────────────┬──────────────────────┘
                                     │ bus
┌──────────────────┐   ┌─────────────┴──────────────┐   ┌──────────────────────┐
│ CPU 65816        │   │ WRAM 128 KB ($7E/$7F)      │   │ SPC700 + DSP (audio) │
│ 3,58 MHz, 16 bits│◄─►│ variables, pila, tablas    │   │ 64 KB propios        │
│ ejecuta nuestro C│   └────────────────────────────┘   │ puertos $2140-$2143  │
└───────┬──────────┘                                    └──────────────────────┘
        │ registros $2100-$21FF (PPU) y $4200-$43FF (CPU: NMI, DMA, mando)
        ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│ PPU  dibuja 256x224 a 60 Hz:  hasta 4 fondos (BG) + 128 sprites (OBJ)        │
│   VRAM 64 KB  tiles y tilemaps  │  CGRAM 512 B  256 colores  │  OAM 544 B    │
└──────────────────────────────────────────────────────────────────────────────┘
```

- **CPU 65816**: ejecuta lo que compila `tcc816`. 16 bits (por eso `u16`),
  sin multiplicación ni división en hardware (rutinas lentas), sin coma
  flotante (punto fijo). Cada frame tiene ~59 000 ciclos para nuestra lógica.
- **PPU**: la "tarjeta gráfica". La CPU no dibuja píxeles: carga tiles y
  mapas en la VRAM, colores en la CGRAM y sprites en la OAM, y la PPU compone
  la imagen línea a línea. Solo se puede escribir en VRAM/CGRAM/OAM sin romper
  la imagen durante el **VBlank** (líneas 225..261 de cada frame).
- **SPC700**: un segundo ordenador para el sonido, con su propio programa y
  RAM. Hoy solo lo toca el stub `audio/`.
- **NMI**: la interrupción que la PPU dispara al empezar el VBlank. El SDK la
  atiende: copia la OAM, lee el mando y despierta a `WaitForVBlank()`.

## 2. Qué toca cada módulo

| Módulo | Chip / memoria | Registros (a través de qué función del SDK) |
|---|---|---|
| `sistema/main.c` | PPU, OAM, CPU | `INIDISP $2100` brillo y pantalla (`setBrightness`, `setScreenOn`); `oamInit` limpia la OAM; `WaitForVBlank` espera la NMI (`NMITIMEN $4200` la habilita el SDK) |
| `sistema/secuencia.c` | ninguno | lógica pura |
| `sistema/comun.c` | PPU, VRAM, CGRAM | `BGMODE $2105` (`setMode`), `BGnSC $2107-$210A` base de mapas y `BGnNBA $210B/$210C` base de tiles (`bgSetMapPtr`, `bgSetGfxPtr`), `TM $212C` capas encendidas (`bgSetEnable/Disable`), `VMAIN/VMADD/VMDATA $2115-$2118` por DMA (`bgInitTileSet`, `bgInitMapSet`, `consoleInitText`), `CGADD/CGDATA $2121/$2122` (paletas), brillo con pasos (`setFadeEffectEx`) |
| `pantallas/pantalla_intro.c` | PPU | además, `MOSAIC $2106` (`setMosaicEffect`, `REG_MOSAIC = 0`) y Mode 3 (`BGMODE`) |
| `pantallas/pantalla_titulo.c`, `pantalla_nivel1.c` | PPU, HDMA | `BGnHOFS/BGnVOFS $210D-$2114` scroll (`bgSetScroll`, dos escrituras de 8 bits); `COLDATA $2132` color de fondo por línea vía **HDMA** canal 6 (`setModeHdmaColor` programa `$43x0-$43x7` y `HDMAEN $420C`; `setModeHdmaReset` lo apaga); `SLHV $2137` / `OPVCT $213D` lectura de la línea del haz (medidor `LIN`) |
| `pantallas/personajes.c` | OAM, VRAM, CGRAM | `OBSEL $2101` tamaño y base de sprites (`oamInitGfxAttr`), `OAMADD/OAMDATA $2102-$2104` por la copia de la OAM en la NMI (`oamSet`, `oamSetEx`, `oamSetXY` escriben en un buffer en RAM), `dmaCopyVram`, `dmaCopyCGram` |
| `juego/tarou_sprite.c` | OAM, VRAM, CGRAM | `oamInitGfxAttr` (`OBSEL`: tiles desde 0x0000, tamaños 16/32), `oamSet`/`oamSetEx`/`oamSetXY` (el buffer de la OAM, una pieza de 32×32 por entrada), `dmaCopyCGram` (paleta 0). Los tiles del cuadro van por la lista de DMA, a una de las dos celdas de la VRAM (tiles 0..127 o 128..255), medio cuadro por VBlank |
| `juego/dma_ejecutar.asm` | VRAM, DMA | **la única escritura a la VRAM del nivel en marcha**, con los registros a mano: por cada entrada de la lista, `VMAIN $2115` (`0x80` tiles, `0x81` columnas: avanzar 32 palabras = bajar una fila del mapa), `VMADD $2116`, canal 0 (`$4300 = 0x01` dos registros, `$4301 = 0x18` destino `VMDATA`, `$4302-$4304` origen de 24 bits, `$4305` bytes), `MDMAEN $420B`; deja `VMAIN = 0x80`. ~60 ciclos por entrada: en C eran 100-200 |
| `juego/dma_lista.c`, `juego/fondo_nivel.c`, `juego/hud_debug.c` | ninguno (preparan) | durante el frame apuntan en la lista qué copiar: `fondo_nivel.c` las columnas que entran (solo si la cámara cambió de columna) con su banco (`nivel1_cerca_col_banco`); `hud_debug.c` la fila que preparó (64 bytes desde el banco $7E). El HUD además enciende y apaga BG2 (`TM`); sus tiles llevan el **bit de prioridad** (`0x2000`): con `BG3_MODE1_PRIORITY_HIGH`, solo así BG3 va delante de la capa cercana. Tiles y paleta de la capa cercana, con `bgInitTileSet` al cargar |
| `pantallas/pantalla_nivel1.c` (reaparecer) | PPU | `setScreenOff()` = **forced blank** (`INIDISP` bit 7): con la pantalla apagada la VRAM se puede escribir en cualquier momento, no solo en el VBlank; así se reescriben de golpe los 4 KB del anillo antes del fundido de entrada |
| `juego/mando.c` | mando | `JOY1L/H $4218/$4219`: la autolectura del mando que la consola hace al inicio del VBlank; el SDK la lee en la NMI y `padsCurrent/padsDown` devuelven bits. En la ROM de prueba (`-DGUION_PRUEBA`) los botones del juego salen de una tabla (`guion.c`) |
| `juego/tarou.c`, `camara.c`, `nivel.c`, `animaciones.c`, `metasprite.c`, `colision.c`, `partida.c`, `guion.c` | CPU, WRAM, ROM | **nada de PPU**: aritmética de 16 bits en WRAM; `colision.c` lee la rejilla de colisión directamente de la ROM (`nivel1_colision`) |
| `data.asm` | ROM | secciones `superfree` que el enlazador reparte por bancos |
| `audio/` (stub) | SPC700 | en el futuro, `APUIO0-3 $2140-$2143` por `spcBoot/spcLoad/spcPlay/spcProcess` |

## 3. Mapa de la VRAM (64 KB = 0x8000 palabras)

Lo fija `sistema/comun.h` y lo respetan todas las pantallas:

```
palabras   bytes      qué hay                                   quién lo escribe
0x0000     0x0000  ┌ tiles de sprites (hasta 512 tiles, 16 KB) ┐  intro/título: Tarou 2 cuadros de 96x64 (6 KB, tiles 0..191) + Bárbara 4 cuadros desde 0x1000 (tile 256)
0x1000     0x2000  │                                           │  nivel: dos celdas de 3x2 bloques (tiles 0..127 y 128..255): la que se muestra y la que recibe el cuadro siguiente
0x2000     0x4000  ├ VRAM_FONDO_A: tiles del fondo principal / capa cercana (logos, escena 3, arbustos del título, suelo del nivel)
0x4000     0x8000  ├ VRAM_FONDO_B: tiles de la capa lejana (título y nivel: montañas)
0x6000     0xC000  ├ VRAM_FUENTE: 96 tiles de la fuente (4bpp o 2bpp según la pantalla)
0x6800     0xD000  ├ VRAM_MAPA_TEXTO: mapa 32x32 de la capa de texto (consola y HUD)
0x6C00     0xD800  ├ VRAM_MAPA_A: mapa 64x32 de la capa cercana (512 px, para scroll; en el nivel, un ANILLO: el tutorial mide 2048 px y se copia por columnas)
0x7400     0xE800  ├ VRAM_MAPA_B: mapa del fondo principal (32x32) o de la lejana (64x32, hasta 0x7BFF)
0x8000     0x10000 └ fin
Mode 3 (intro): la imagen de 256 colores ocupa 0x0000-0x3FFF (tiles de 64 bytes) y su mapa va en VRAM_MAPA_B.
```

Un **tile** de 8×8 a 16 colores (4bpp) son 32 bytes; a 4 colores (2bpp), 16;
a 256 (8bpp), 64. Un **tilemap** de 32×32 celdas son 2 KB (una palabra por
celda: tile, paleta, prioridad, volteretas); uno de 64×32, 4 KB.

## 4. Mapa de la CGRAM (256 colores de 15 bits)

| Entradas | Paleta | Quién la usa |
|---|---|---|
| 0..15 | fondos 0 | la fuente de la consola (4bpp) o la de 2bpp (colores 0..3) |
| 16..31 | fondos 1 | logos, escena 3, capa cercana (arbustos del título; suelo y seto del nivel) |
| 32..47 | fondos 2 | capa lejana del título y del nivel |
| 112..127 | fondos 7 | el texto en Mode 3 (las escenas de 256 colores dejan libres estas 16) |
| 0..255 | — | en Mode 3, `fondo256_cargar` sobrescribe las 256 entradas: después hay que recargar las paletas de texto y sprites |
| 128..143 | sprites 0 | Tarou (`tarou_pal`), en intro, título y nivel |
| 144..159 | sprites 1 | Bárbara |

El color 0 de cada paleta es **transparente** en sprites y en las capas
superiores; en la capa de más abajo se ve el color de fondo (`COLDATA`), que
en título y nivel pinta el HDMA línea a línea.

## 5. Mapa de la OAM (128 sprites)

| Entrada | Quién | Tamaño |
|---|---|---|
| 0..5 | Tarou: las hasta 6 piezas de 32×32 del metasprite (celda de 96×64) (intro/título estático; nivel por el motor dinámico) | pequeño (32×32 con `OBJ_SIZE32_L64`; grande de 32 con `OBJ_SIZE16_L32` en el nivel) |
| 6 | Bárbara (intro y título) | grande, 64×64 |
| 7..127 | libres (enemigos y objetos en el futuro) | — |

Cada entrada guarda x, y, el tile inicial, la paleta, la prioridad y los
bits de volteo. Límites: 128 sprites, 32 por línea, 34 "ranuras" de 8 px por
línea, y 512 tiles direccionables (por eso Tarou tiene que ser dinámico).

## 6. Mapa de la ROM (16 bancos de 32 KB)

Del `ProyectoSNES.log` del 2026-10-20 (tutorial, arte real): bancos 0..13
llenos, banco 14 al 56 %, banco 15 vacío (47 289 bytes libres de 524 288).
En la Revisión 1, con los marcadores, quedaban 238 927.

- **Banco 0**: la cabecera (`hdr.asm`), los vectores de interrupción y el
  código que el enlazador coloca primero (nuestro `.text` y la librería).
- **Bancos 1..8**: el resto del código y las secciones `superfree` de
  `data.asm`: cada asset entero en un banco (ninguna sección puede cruzar los
  32 KB). Las 13 animaciones de Tarou (3 KB por cuadro de 96×64) son el bloque
  mayor; la capa cercana del tutorial (tiles + mapa por columnas, ~25 KB) va
  en una sola sección, y su banco se guarda aparte en 1 byte
  (`nivel1_cerca_col_banco: .db :nivel1_cerca_col`) porque el DMA lo necesita.
- `wlalink -d` descarta las secciones que nadie referencia: los stubs y el
  lobo samurái no ocupan ROM.

Un detalle importante para los junior: `tcc816` coloca las **tablas `const`**
de C en la WRAM, no en la ROM (`ANIMACIONES` está en `$7F00E4`, se ve en el
`.sym`): se copian al arrancar. Los datos grandes deben ir en `data.asm` con
`.incbin`, que sí se quedan en la ROM.

## 7. Mapa de la WRAM (128 KB)

| Zona | Qué hay |
|---|---|
| `$7E0000-$7E1FFF` (8 KB, también visible en `$0000-$1FFF`) | la página cero y la pila del 65816; variables rápidas del SDK |
| `$7E2000-$7EFFFF` | variables globales y estáticas de nuestros `.c` (`.bss`), el buffer de la OAM, la consola |
| `$7F0000-$7FFFFF` | tablas `const` copiadas por tcc816, buffers grandes |

El `.log` dice cuánto queda: en la Revisión 1, 154 372 bytes libres de 163 840.
Toda variable global de nuestro código está aquí; por eso la lógica pura usa
`u8/u16/s16` y no `int` de 32 bits: cada byte y cada ciclo cuentan.

## 8. El frame, paso a paso (lo que pasa en 16,7 ms)

```
línea 0           la PPU empieza a dibujar la línea 0 (área visible 0..224)
                  │  nuestra lógica del frame corre aquí (en el tutorial, con colisión y
                  │  sin HUD, acaba hacia la línea 100-130: "LIN")
                  │  mientras, HDMA escribe COLDATA en cada línea (el cielo)
línea 225         VBlank: la PPU descansa; la consola autolee el mando ($4218)
                  │  NMI: el SDK copia la OAM (DMA), lee pad_keys, cuenta snes_vblank_count
                  │  WaitForVBlank() vuelve → secuencia_vblank(): dma_lista_ejecutar: de un
                  │  tirón, medio cuadro de Tarou (≤ 1,5 KB), las dos columnas del mapa
                  │  (128 B) y una fila del HUD (64 B). Acaba hacia la línea 23 del
                  │  VBlank (la 248); el límite es la 37 (la 0 del frame siguiente)
línea 262 → 0     siguiente frame
```

**DMA** (`$43x0-$43x7`, `MDMAEN $420B`): copia bloques de memoria a VRAM/CGRAM/OAM
a ~2,7 MB/s, mucho más rápido que la CPU; solo durante el VBlank. **HDMA**
(`HDMAEN $420C`): la misma máquina, pero una escritura pequeña en cada línea
mientras la PPU dibuja: así cambia el color del cielo línea a línea sin gastar
CPU.
