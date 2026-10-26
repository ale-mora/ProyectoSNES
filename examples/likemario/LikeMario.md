# LikeMario — estudio técnico del ejemplo de plataformas de PVSnesLib

> Notas de estudio sobre `tools/pvsneslib/snes-examples/systems/games/likemario/`, el ejemplo
> oficial de PVSnesLib que sirve de referencia para el motor de nuestro propio plataformas
> (`ProyectoSNES`). Este documento es para consulta personal — no forma parte del build del SDK
> ni de nuestro repositorio (la carpeta `tools/` está en `.gitignore`).

## 1. Qué es y por qué lo estudiamos

`LikeMario` es un plataformas mínimo de scroll horizontal: Mario camina, salta, cae, y la
cámara lo sigue sobre un nivel más grande que la pantalla. No tiene enemigos ni meta —
es deliberadamente pequeño para mostrar, sin ruido, **los tres motores de PVSnesLib
trabajando juntos**: el motor de mapas (`map.h`), el motor de entidades (`object.h`) y el
motor de sprites dinámicos (`sprite.h`), más audio vía SPC700. Es la plantilla más cercana
a lo que queremos construir (un plataformas lineal estilo Donkey Kong Country), así que la
usamos como mapa de referencia en vez de diseñar el motor desde cero.

## 2. Panorama técnico general

### 2.1 Arquitectura en capas

```
main()                     ← el único punto de entrada, se ejecuta una vez y luego entra en loop
 ├─ inicializa audio         (spcBoot, spcSetBank, spcLoad, spcSetSoundEntry)
 ├─ inicializa gráficos      (bgSetGfxPtr/bgSetMapPtr, bgInitTileSet)
 ├─ inicializa sprites       (oamInitDynamicSprite)
 ├─ inicializa el motor de objetos y registra el "tipo" Mario
 │    objInitFunctions(0, marioinit, marioupdate, NULL)
 ├─ carga el nivel completo  (objLoadObjects + mapLoad)
 └─ while(1):                ← el frame loop, 60 veces por segundo
      mapUpdate()             actualiza qué parte del mapa se ve
      objUpdateAll()          llama marioupdate() (y el de cualquier otro objeto vivo)
      spcProcess()            mantiene el audio streameando
      WaitForVBlank()         sincroniza con el hardware
      mapVblank()             sube los tiles nuevos a VRAM
      oamVramQueueUpdate()    sube los sprites nuevos a VRAM
```

**Principio de diseño clave**: `main()` no contiene lógica de juego, solo orquestación.
Toda la física, animación y reacción a input vive en las funciones registradas por tipo de
objeto (`marioinit`/`marioupdate`). Si mañana agregamos un enemigo, no tocamos `main()` —
registramos un tipo de objeto nuevo con su propio par init/update.

### 2.2 El pipeline de assets es parte de la arquitectura, no un detalle aparte

Ningún dato de nivel, sprite o música está escrito a mano en C. Todo se **autora en una
herramienta externa** (Tiled para el nivel, un editor de imagen indexada para los sprites,
un tracker para la música) y se **convierte en tiempo de build** a binarios que el
ensamblador empaqueta y el linker une al ejecutable final:

```
  fuente editable          herramienta          binario SNES        wrapper para C
 ───────────────────      ─────────────        ───────────────      ──────────────
  tiles.png            →  gfx4snes         →   tiles.pic/.pal/.map
  mario_sprite.bmp      →  gfx4snes         →   mario_sprite.pic/.pal
  mariofont.bmp         →  gfx4snes         →   mariofont.pic/.pal/.map     →  .inc (extern) +
  map_1_1.tmx           →  tmx2snes         →   BG1.m16, map_1_1.b16/.o16/.t16   .as (.incbin)
  overworld.it          →  smconv           →   soundbank.bnk/.asm/.obj    →  soundbank.h
  mariojump.wav          →  snesbrr (vía make)→  mariojump.brr
```

La sección 4 explica exactamente cómo cada binario llega a ser un símbolo `extern`
utilizable desde `LikeMario.c`.

## 3. Inventario de archivos

### 3.1 Control de build

| Archivo | Rol | Generado por |
|---|---|---|
| `Makefile` | Declara `ROMNAME`, `AUDIOFILES`, `SOUNDBANK`, e incluye `snes_rules` (reglas genéricas del SDK). Define las reglas propias de conversión de assets (`bitmaps`, `musics`) que no vienen de fábrica. | escrito a mano |
| `hdr.asm` / `hdr.obj` | Header ROM + tabla de vectores de interrupción (NMI→VBlank, RESET→arranque). Igual que en nuestro propio proyecto, se regenera en cada `make` a partir de las variables del Makefile. | `snes_rules`, auto-generado |
| `linkfile` | Lista exacta de `.obj` que `wlalink` combina, en orden: `src/LikeMario.obj`, `soundbank.obj`, `data.obj`, `hdr.obj`, más el runtime de PVSnesLib (`crt0_snes`, `libc`, `libm`, `libtcc`). | regla `%.sfc:` en `snes_rules` |
| `likemario.sfc` / `.sym` / `.symfull` | ROM final y sus tablas de símbolos (para depurar en Mesen con nombres en vez de direcciones). | build completo |

### 3.2 Código fuente

| Archivo | Rol |
|---|---|
| `src/LikeMario.c` | Toda la lógica del juego. Ver desglose función por función en la sección 5. |
| `src/LikeMario.obj` | Objeto compilado de lo anterior por `816-tcc`. |

### 3.3 Nivel — autoría en Tiled y su conversión

| Archivo | Rol | Vínculo |
|---|---|---|
| `map_1_1.tmx` | El nivel, editado visualmente en Tiled. Tiene una capa de tiles llamada **"BG1"**, una capa de objetos **"Entities"** con el spawn de **"Mario"**, y cada tile del tileset lleva propiedades personalizadas `attribute` (máscara `T_SOLID`/`T_LADDER`/etc.), `palette` y `priority`. | fuente editable |
| `map_1_1.png` | Imagen del tileset tal como la ve Tiled (referencia visual, no se usa en el build). | fuente editable |
| `BG1.m16` | El mapa real del nivel (qué bloque va en cada celda). **Nombrado por la capa "BG1" del .tmx, no por el nombre del archivo .tmx** — ver nota en sección 6. Esto es lo que carga `mapLoad()` como `mapmario`. | salida de `tmx2snes -i map_1_1.tmx -m tiles.map` |
| `map_1_1.b16` | Atributos de colisión por bloque de 16×16 (sólido/escalera/pincho/plataforma), extraídos de la propiedad `attribute` de cada tile en Tiled. Es `tilesetatt` en `data.asm`. | salida de `tmx2snes` |
| `map_1_1.o16` | Tabla de objetos del nivel (posición/tipo de cada entidad a spawnear — aquí solo Mario). Es `objmario`, lo que carga `objLoadObjects()`. | salida de `tmx2snes` |
| `map_1_1.t16` | Paleta y prioridad por bloque, de las propiedades `palette`/`priority` de Tiled. Es `tilesetdef`. | salida de `tmx2snes` |

### 3.4 Tileset del nivel

| Archivo | Rol |
|---|---|
| `tiles.png` | Arte fuente del tileset, en paleta indexada. |
| `tiles.pic` / `tiles.pal` / `tiles.map` | Tiles, paleta y mapa de tiles-a-bloques, convertidos por `gfx4snes -s 8 -o 16 -u 16 -p -m -i tiles.png` (bloques de salida de 16px hechos de tiles fuente de 8px, con reducción/deduplicación). |
| `tiles.inc` | Header generado: declara `extern unsigned char tiles_til, tiles_tilend, tiles_pal, tiles_palend` y `extern unsigned short tiles_map, tiles_mapend` — los símbolos que `LikeMario.c` importa con `#include "tiles.inc"`. |
| `tiles_data.as` | Ensamblador generado: usa `.incbin` para insertar los binarios `.pic`/`.map`/`.pal` exactamente entre las etiquetas que `tiles.inc` declaró como `extern`. Es el puente binario↔símbolo. |

### 3.5 Sprite del jugador

| Archivo | Rol |
|---|---|
| `mario_sprite.bmp` | Arte fuente del sprite de Mario (hoja de animación), paleta indexada. |
| `mario_sprite.pic` / `.pal` | Convertidos por `gfx4snes -s 16 -o 16 -u 16 -p -t bmp -i mario_sprite.bmp` (bloques de 16×16, sin reducción de mapa porque es un sprite, no un fondo). |
| `mario_sprite.inc` | Declara `extern mario_sprite_til/_tilend/_pal/_palend`. |
| `mario_sprite_data.as` | `.incbin` de `mario_sprite.pic`/`.pal` en esas etiquetas. |

### 3.6 Fuente de texto del HUD

| Archivo | Rol |
|---|---|
| `mariofont.bmp` | Bitmap de la fuente usada para "MARIOx00 WORLD TIME". |
| `mariofont.pic` / `.pal` / `.map` | Convertidos con `-e 1 -R` (sin reducción de tiles — una fuente necesita cada carácter intacto, no deduplicado). |
| `mariofont.inc` / `mariofont_data.as` | Mismo patrón: declara + `.incbin`. |

### 3.7 Audio

| Archivo | Rol |
|---|---|
| `overworld.it` / `overworld.mid` | Música fuente en formato tracker (Impulse Tracker). |
| `soundbank.asm` / `.bnk` / `.obj` | Generados por `smconv -s -o soundbank -V -b 5` a partir de `AUDIOFILES := overworld.it`. Empaquetan la música para el driver SPC700. |
| `soundbank.h` | Define `MOD_OVERWORLD = 0` (índice de la canción) — es lo que usa `spcLoad(MOD_OVERWORLD)` en `main()`. |
| `mariojump.wav` / `.brr` | Efecto de sonido del salto, comprimido a BRR (formato nativo de la SPC700) por la regla `mariojump.brr` del Makefile. |

### 3.8 El archivo que amarra todo: `data.asm`

```asm
.section ".rodata1" superfree
  .include "tiles_data.as"          ; tiles del nivel
  .include "mario_sprite_data.as"   ; sprite de Mario
  .include "mariofont_data.as"      ; fuente del HUD
  jumpsnd: .incbin "mariojump.brr"  ; SFX de salto
  jumpsndend:
.ends

.section ".rodata2" superfree
  mapmario:    .incbin "BG1.m16"       ; el mapa del nivel
  objmario:    .incbin "map_1_1.o16"   ; tabla de objetos
  tilesetatt:  .incbin "map_1_1.b16"   ; atributos de colisión
  tilesetdef:  .incbin "map_1_1.t16"   ; paleta/prioridad
.ends
```

`data.asm` no tiene lógica — es puro pegamento: define las etiquetas (`mapmario`,
`objmario`, etc.) que `LikeMario.c` declara como `extern` y las llena con el contenido
binario de los archivos convertidos. Se ensambla a `data.obj`, que el `linkfile` incluye
junto al objeto compilado de C.

## 4. El flujo completo, de principio a fin

```
map_1_1.tmx ──tmx2snes──> BG1.m16, map_1_1.{b16,o16,t16}
tiles.png   ──gfx4snes──> tiles.{pic,pal,map}          ──┐
mario_sprite.bmp ─gfx4snes─> mario_sprite.{pic,pal}       ├─ .incbin en data.asm ──┐
mariofont.bmp   ──gfx4snes──> mariofont.{pic,pal,map}     │                        │
mariojump.wav   ──snesbrr───> mariojump.brr             ──┘                        │
overworld.it    ──smconv────> soundbank.{bnk,asm,h}                                │
                                                                                     │
data.asm ──(wla-65816)──> data.obj ─────────────────────────────────────────────────┤
src/LikeMario.c ──(816-tcc + wla-65816)──> src/LikeMario.obj                        │
                                                                                     ▼
                                              linkfile ──(wlalink)──> likemario.sfc
```

`LikeMario.c` nunca abre un archivo `.tmx` ni `.bmp` — solo ve símbolos `extern` (`mapmario`,
`tiles_til`, `mario_sprite_pal`...) que ya están resueltos por el linker a direcciones
concretas en la ROM. Esa indirección es lo que permite rediseñar el nivel en Tiled sin
tocar una sola línea de C.

## 5. `LikeMario.c` función por función

- **`main()`** (línea 229) — arranca audio, gráficos, sprites y el motor de objetos; registra
  el tipo `0` como Mario; carga nivel y objetos; enciende pantalla; entra al loop de frame
  descrito en 2.1.
- **`marioinit(xp, yp, type, minx, maxx)`** (línea 51) — se llama una sola vez, cuando
  `objLoadObjects` instancia a Mario desde `map_1_1.o16`. Reserva el slot con `objNew`,
  guarda punteros directos a los campos internos de posición/velocidad del objeto
  (`marioox`, `marioxv`...) para no tener que indexar `objbuffers[]` cada frame, y configura
  el sprite y la paleta inicial.
- **`marioupdate(idx)`** (línea 140) — se llama **una vez por frame** desde `objUpdateAll()`.
  Orden exacto, y por qué importa:
  1. Lee input (`padsCurrent`) y ajusta velocidad/estado según lo presionado — todavía no
     mueve nada.
  2. `objCollidMap(idx)` — resuelve física y colisión contra el mapa (gravedad, `tilestand`).
  3. Según `marioobj->action`, delega a `mariowalk`/`mariofall`/`mariojump` — estas solo
     deciden **animación**, la física ya se resolvió en el paso 2.
  4. `objUpdateXY(idx)` — recién aquí la velocidad se aplica a la posición.
  5. Convierte posición de mundo a posición de pantalla y dibuja (`oamDynamic16Draw`).
  6. `mapUpdateCamera(...)` — la cámara sigue a Mario, siempre al final.
- **`mariowalk(idx)`** (línea 90) — anima el ciclo de caminata cada 3 frames, y decide si el
  estado pasa a `ACT_FALL`/`ACT_STAND` según la velocidad.
- **`mariofall(idx)`** (línea 111) — cuando la velocidad vertical llega a 0, vuelve a
  `ACT_STAND`.
- **`mariojump(idx)`** (línea 124) — cambia el sprite al frame de salto; en cuanto la
  velocidad vertical deja de ser negativa, pasa a `ACT_FALL`.

## 6. Constantes de física — qué "siente" cada una

```c
#define GRAVITY 48
#define JUMPVALUE (GRAVITY * 20)
#define MARIO_MAXACCEL 0x0140   // velocidad horizontal máxima
#define MARIO_ACCEL    0x0038   // cuánto acelera por frame al mantener izq/der
#define MARIO_JUMPING  0x0394   // impulso vertical de salto normal
#define MARIO_HIJUMPING 0x0594  // impulso si se salta manteniendo arriba
```

`GRAVITY` se pasa a `objInitGravity` — no aparece explícito en este archivo pero es el valor
que el motor de objetos suma a `yvel` cada frame. Subir `MARIO_ACCEL` hace el control más
"snappy"; bajar `MARIO_MAXACCEL` hace el andar más lento; la relación entre `GRAVITY` y
`MARIO_JUMPING` define la altura y duración del arco de salto — es el mismo trío de
constantes que vamos a tener que tunear para el moveset de nuestro propio jugador.

## 7. Tabla comparativa — los 33 archivos, de un vistazo

| Archivo | Categoría | Tipo | Qué hace | Producido por |
|---|---|---|---|---|
| `Makefile` | Build | Config manual | Declara `ROMNAME`, `AUDIOFILES`, `SOUNDBANK`; incluye `snes_rules`; agrega reglas propias de conversión de assets | escrito a mano |
| `hdr.asm` | Build | Auto-generado | Header ROM + tabla de vectores de interrupción | `snes_rules`, en cada `make` |
| `hdr.obj` | Build | Binario intermedio | `hdr.asm` ensamblado | `wla-65816` |
| `linkfile` | Build | Auto-generado | Lista ordenada de `.obj` a enlazar | `snes_rules` |
| `likemario.sfc` | Build | Salida final | La ROM jugable | `wlalink` |
| `likemario.sym` / `.symfull` | Build | Salida final | Tablas de símbolos para depurar en Mesen | `wlalink` |
| `src/LikeMario.c` | Código | Fuente editable | Toda la lógica del juego (`main`, `marioinit`, `marioupdate`...) | escrito a mano |
| `src/LikeMario.obj` | Código | Binario intermedio | `LikeMario.c` compilado | `816-tcc` |
| `map_1_1.tmx` | Nivel | Fuente editable | Diseño del nivel: capa de tiles "BG1", capa de objetos "Entities"/Mario, propiedades `attribute`/`palette`/`priority` por tile | Tiled |
| `map_1_1.png` | Nivel | Fuente editable | Imagen del tileset, referencia visual dentro de Tiled | arte fuente |
| `BG1.m16` | Nivel | Binario convertido | El mapa real del nivel (`mapmario`) — nombrado por la capa "BG1", no por el `.tmx` | `tmx2snes` |
| `map_1_1.b16` | Nivel | Binario convertido | Atributos de colisión por bloque (`tilesetatt`) | `tmx2snes` |
| `map_1_1.o16` | Nivel | Binario convertido | Tabla de objetos a spawnear (`objmario`) | `tmx2snes` |
| `map_1_1.t16` | Nivel | Binario convertido | Paleta + prioridad por bloque (`tilesetdef`) | `tmx2snes` |
| `tiles.png` | Tileset | Fuente editable | Arte del tileset en paleta indexada | arte fuente |
| `tiles.pic` / `.pal` / `.map` | Tileset | Binario convertido | Tiles, paleta y mapa de tiles-a-bloques de 16px | `gfx4snes` |
| `tiles.inc` | Tileset | Wrapper C | Declara `extern tiles_til/_map/_pal` para `LikeMario.c` | `gfx4snes` (junto con `.pic`) |
| `tiles_data.as` | Tileset | Wrapper asm | `.incbin` de los binarios en esas mismas etiquetas | `gfx4snes` (junto con `.pic`) |
| `mario_sprite.bmp` | Sprite | Fuente editable | Hoja de animación de Mario, paleta indexada | arte fuente |
| `mario_sprite.pic` / `.pal` | Sprite | Binario convertido | Tiles + paleta del sprite | `gfx4snes` |
| `mario_sprite.inc` / `_data.as` | Sprite | Wrapper C/asm | `extern` + `.incbin` del sprite | `gfx4snes` |
| `mariofont.bmp` | Fuente HUD | Fuente editable | Bitmap de la tipografía del HUD | arte fuente |
| `mariofont.pic` / `.pal` / `.map` | Fuente HUD | Binario convertido | Tiles/paleta/mapa de caracteres (sin deduplicar, `-R`) | `gfx4snes` |
| `mariofont.inc` / `_data.as` | Fuente HUD | Wrapper C/asm | `extern` + `.incbin` de la fuente | `gfx4snes` |
| `overworld.it` | Audio | Fuente editable | Música del nivel, formato tracker | compuesto en tracker (IT) |
| `overworld.mid` | Audio | Fuente editable | Versión MIDI de referencia | — |
| `soundbank.asm` / `.bnk` / `.obj` | Audio | Binario convertido | Paquete de música para el driver SPC700 | `smconv` |
| `soundbank.h` | Audio | Wrapper C | Define `MOD_OVERWORLD` (índice de canción) | `smconv` |
| `mariojump.wav` | Audio | Fuente editable | SFX de salto, sin comprimir | grabado/generado |
| `mariojump.brr` | Audio | Binario convertido | SFX comprimido para SPC700 | `snesbrr`, vía regla del Makefile |
| `data.asm` | Pegamento | Escrito a mano | Define `mapmario`/`objmario`/`tilesetatt`/`tilesetdef`/`jumpsnd` e inserta ahí los binarios con `.incbin` | escrito a mano |
| `data.obj` | Pegamento | Binario intermedio | `data.asm` ensamblado | `wla-65816` |

**Cómo leerla**: la columna *Tipo* es la comparación más útil — "Fuente editable" es lo único
que un humano toca; "Binario convertido" es generado y desechable (se regenera con `make`);
"Wrapper C/asm" es el puente que declaramos en la sección 4; "Salida final"/"Binario
intermedio" son productos del build que tampoco se editan a mano. De los 33 archivos, solo
**5** se editan directamente: `Makefile`, `LikeMario.c`, `map_1_1.tmx`, los `.bmp` de arte, y
`overworld.it`/`mariojump.wav`. Todo lo demás es máquina generando máquina.

## 8. Una nota honesta: no todo el nombrado es consistente

`tmx2snes -i map_1_1.tmx -m tiles.map` produce **cuatro** archivos, pero no todos se llaman
`map_1_1.*`: el mapa de tiles (`.m16`) se nombra según el **nombre de la capa dentro del
.tmx** (acá la capa se llama literalmente "BG1", de ahí `BG1.m16`), mientras que los otros
tres (`.b16`/`.o16`/`.t16`, que no son "por capa" sino globales del nivel) sí toman el
nombre del archivo `.tmx`. Si renombras una capa en Tiled, el nombre del `.m16` cambia con
ella — vale la pena no asumir que siempre va a coincidir con el nombre del archivo `.tmx`.
