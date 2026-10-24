# Ejercicio 08 — Un segundo sprite con otra paleta, y romperlo a propósito

`rom/src/sprites.c` ya pone a Bárbara (entrada 0 de la OAM, tiles 0..255,
paleta de sprites 0), la anima cada 8 frames y la mueve con la cruceta. Tú
añades al lobo samurái y después lo rompes de dos maneras para ver dos
límites del hardware.

## Parte A — El lobo (TODO 1 y 2)

1. **TODO 1** (en `main`): copia a la VRAM los `BYTES_LOBO` bytes de
   `lobo_til` (sus dos primeros cuadros) empezando en el tile `TILE_LOBO`
   (256), y sus 16 colores a la paleta de sprites `PALETA_LOBO` (1). Dos
   cuentas que tienes que hacer tú: cuántas **palabras** de VRAM ocupa un tile
   de 4bpp, y en qué entrada de la CGRAM empiezan las paletas de sprites.
2. **TODO 2** (en `dibujar`): pon al lobo en la entrada `OAM_LOBO` con
   `oamSet`, en (`LOBO_X`, `LOBO_Y`), con el volteo horizontal (para que mire
   a Bárbara), la paleta `PALETA_LOBO` y el tile de su cuadro: el primer tile
   de su tira **más** `tile_del_bloque64(cuadro_lobo)`. Después, `oamSetEx`
   para hacerlo grande (64×64) y visible. Recuerda: el primer parámetro de las
   funciones `oam*` es la entrada **× 4**.

Compila (PowerShell, dentro de `rom/`) y míralo:

```
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
cd E:\ProyectoSNES
.\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES\aprendizaje\unidad-08-sprites-y-oam\rom\u08_sprites.sfc -Duracion 6 -Cada 1 -Espera 3 -Guion "2,4,Left"
```

Debe verse: Bárbara a la derecha y el lobo a la izquierda, mirándose, cada
uno con sus colores, los dos animados; entre los segundos 2 y 4 Bárbara se
acerca al lobo (1 px por frame: 120 px en 2 s).

3. **La fórmula, en Mesen.** Abre **Debug → Tile Viewer** (sprites, 4bpp,
   paleta de sprites 0 y luego 1): comprueba que el cuadro 1 de Bárbara
   empieza en el tile 8, el 2 en el 128 y el 3 en el 136, y que el lobo
   empieza en el 256. En **Debug → Sprite Viewer**, selecciona la entrada 0 y
   mira cómo cambia su tile al animarse: 0, 8, 128, 136.

## Parte B — El cruce de tablas (tile 192)

Compila con el lobo en el tile 192 sin tocar el código (el `#ifndef` de
`TILE_LOBO` lo permite):

```
make clean; $env:CFLAGS="-DTILE_LOBO=192"; make; $env:CFLAGS=$null
```

Antes de ejecutarlo, **predice** qué se verá. El lobo es un sprite de 64×64:
8 filas de 8 tiles. Su primera fila empieza en el tile 192; ¿en qué tile
empieza su fila 4, según la PPU? (README, «Las dos tablas de 256».) Y la
copia de sus 128 tiles, ¿qué tiles de Bárbara pisa?

Después, en Mesen: **el lobo casi desaparece**. Quedan la punta de las
orejas y la punta de la espada arriba, y abajo un fragmento marrón suelto; el
resto, tiles vacíos. Es la cuenta de antes llevada hasta el final, con dos
efectos que se suman:

1. **La copia parte al lobo entre las dos tablas.** Los 128 tiles copiados
   son las 8 filas de 16 tiles de su hoja (sus cuadros 0 y 1, uno al lado del
   otro). Desde el tile 192, sus filas 0..3 caen en los tiles 192..255 (tabla
   0) y sus filas 4..7 en los 256..319 (tabla 1). El cuerpo del lobo está en
   la mitad de abajo de su cuadro de 64×64: en la tabla 1, **que el sprite no
   lee nunca** (su tile, 192 o 200, tiene el bit 8 a 0).
2. **El sprite da la vuelta dentro de la tabla 0.** Sus filas 0..3 leen
   192..255: la mitad de arriba del lobo, casi vacía (orejas y espada). Sus
   filas 4..7 leen `(192 + 64) & 255` = 0..63 (8..71 en el cuadro 1): la
   mitad de arriba de los cuadros 0 y 1 de Bárbara, también casi vacía; solo
   asoma la punta de su coleta, pintada con la paleta del lobo (el fragmento
   marrón).

Y Bárbara pierde el cuerpo en la mitad de su animación: en los cuadros 2 y 3
su mitad de abajo son los tiles 192..255, que la copia pisó con la mitad de
arriba del lobo. Se ve parpadear: dos cuadros entera, dos cuadros con solo el
pelo arriba y, a sus pies, la espada y las orejas del lobo pintadas con la
paleta de Bárbara (la 0: es su entrada de la OAM la que elige la paleta, no
los tiles).

La predicción «la mitad de abajo del lobo hecha de trozos de Bárbara» es
correcta en los tiles pero engaña a la vista: los trozos que lee son la parte
vacía (de arriba) de sus cuadros. `capturas/simulacion_cruce192.png` lo
reproduce en el PC leyendo los `.pic` como lo haría la PPU
(`capturas/simular_vram_sprites.py 192 salida.png`: lobo cuadros 0 y 1,
Bárbara cuadros 0..3); con 256 (`simulacion_tile256.png`) sale todo bien.
Haz una captura en la que se vea y, en el **Tile Viewer** de Mesen, mira
dónde quedó cada mitad del lobo (tiles 192..255 y 256..319).

## Parte C — Esconder mal un sprite (y = 240)

```
make clean; $env:CFLAGS="-DLOBO_Y=240"; make; $env:CFLAGS=$null
```

Un sprite en y = 240 debería estar 16 px por debajo de la pantalla. ¿Por qué
se ve el lobo **arriba**, cortado? (README, «Esconder un sprite».) Captura.
Vuelve a compilar normal (`make clean; make`) al terminar.

## Aprobación

- Captura de la parte A con los dos sprites, cada uno con su paleta.
- La comprobación de la fórmula en el Tile Viewer (tiles 0, 8, 128, 136 y
  256), con captura.
- Captura del cruce de tablas (parte B) y tu explicación de qué tile lee la
  PPU en cada fila del lobo.
- (Recomendada) captura de la parte C.

Solución: `solucion/sprites.c` (cópiala sobre `rom/src/sprites.c`).
