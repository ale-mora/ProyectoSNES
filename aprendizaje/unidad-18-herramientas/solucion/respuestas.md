# Respuestas del ejercicio 18

La solución completa está en `guardia.patch` (aplicable con `git apply` desde
la raíz de un clon limpio; la tira PNG, que es binaria, va aparte) y en
`archivos/`, con la misma ruta que en el repositorio:
`Makefile`, `data.asm`, `scripts/preparar_personaje.py`,
`scripts/pruebas/prueba_preparar_personaje.py`, `src/juego/tarou_sprite.c`,
`res/personajes/tarou/tarou_anim.h` y `res/personajes/tarou/tarou_guardia.png`.
Se comprobó en un clon el 2026-10-22: 4 fallos en rojo → 36 pruebas de scripts
en verde, `No warnings or errors found.` (ROM normal y `-DGUION_PRUEBA`), 367
pruebas de PC, `tarou_guardia_til` en `$85:F000`.

Para aplicarla: `git apply ruta\guardia.patch`, copiar
`archivos/res/personajes/tarou/tarou_guardia.png` a `res/personajes/tarou/`, y
`make clean` + `make`.

**1. ¿Por qué `.db :tarou_guardia_til`?** El puntero que devuelve
`tarou_sprite_tiles` es de **16 bits** (los punteros de tcc816): da la
dirección dentro del banco, pero no **el banco**. El DMA que sube el cuadro a
la VRAM necesita la dirección completa de 24 bits (banco + 16 bits, unidad
13), y `tarou_sprite_dibujar` saca el banco de `tarou_til_bancos[dibujo_estado]`.
`:etiqueta` es el banco de la etiqueta en WLA-DX, que solo el enlazador
conoce. Sin la línea, el índice 13 leería el byte siguiente de la ROM y el DMA
copiaría basura de otro banco: Tarou en guardia saldría con tiles ajenos.

**2. ¿Por qué un `make` sin `clean` ya lo rehace todo?** Porque al añadir
`guardia` a `TAROU_ANIMS`, `tarou_guardia.pic` entra en `TAROU_PICS` y, con
ella, en `ASSETS`. El `.pic` es nuevo, así que es más reciente que
`data.obj` (regla `data.obj: $(ASSETS)`) y que `tarou_sprite.obj` (regla
`src/juego/tarou_sprite.obj: $(TAROU_PICS)`), y los dos se rehacen; y
`animaciones.obj` depende de `tarou_anim.h`, que el script reescribió. Se pide
`make clean` igual porque `snes_rules` no sigue los `#include` de las
cabeceras del código: si algún `.c` que no está en esas reglas usara algo que
cambió, conservaría el valor viejo sin avisar. Con un `clean` no hay que
pensar quién depende de qué.

**3. Si guardia fuera un estado.** Además de todo lo anterior:
`src/juego/animaciones.h` (`ESTADO_GUARDIA` en el `enum Estado`, antes de
`ESTADO_CANTIDAD`, que pasaría a 14), `src/juego/animaciones.c` (su fila en
`ANIMACIONES` con `TAROU_GUARDIA_CUADROS`, frames por cuadro y bucle, y su
nombre en `NOMBRES_ESTADO`, que sale en el HUD), `src/juego/tarou.c` (las
transiciones: cuándo se entra y se sale de la guardia, y qué hace el mando en
ella), `tests/tarou_anim.h` (la copia fija necesita `TAROU_GUARDIA_CUADROS`,
o las pruebas de PC no compilan), `tests/prueba_animacion.c` (comprueba
`ESTADO_CANTIDAD == 13`) y probablemente `tests/prueba_tarou.c` y
`tests/prueba_recorrido.c` (un estado nuevo en el camino del guion cambia el
recorrido). El orden de `tarou_til_bancos` y de los `switch` seguiría siendo
el del `enum`. Por eso aquí se eligió un **dibujo** y no un estado: la lógica
y sus 367 pruebas no cambian.

**4. ¿Por qué 4 KB?** `gfx4snes -s 32` empaqueta los bloques de 32×32 en filas
de 128 px (4 bloques), como la hoja de sprites de la VRAM (unidad 09). Un
cuadro de 96×64 son 3 bloques arriba y 3 abajo; cada fila de bloques se
rellena hasta 4, así que salen 8 bloques = 8 × 16 tiles = 128 tiles × 32 bytes
= 4096 bytes (el log dice `saving 128 tiles`). A la VRAM solo suben las 6
piezas que usa la tabla (3 KB): `metasprite_copias` copia por bloques.

**5. ¿Y si se usara `t->cuadro`?** En `PARADO`, la lógica cuenta los cuadros
del **reposo** (9, de 0 a 8). La tabla `tarou_guardia_metasprites` tiene un
solo elemento: `tarou_guardia_metasprites[1]` y siguientes leen fuera del
array (C no lo comprueba, unidad 00) y darían un puntero basura a
`metasprite_copias`. Además, cada cambio de cuadro dispara una subida de 3 KB
a la VRAM aunque el dibujo fuera el mismo. Con el cuadro fijo a 0, la guardia
se sube una vez y no vuelve a gastar DMA mientras Tarou siga quieto.
