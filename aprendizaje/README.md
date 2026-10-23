# `aprendizaje/` — aprender a programar la SNES con este juego

Esta carpeta es el curso para quien hereda el proyecto: 27 unidades de
estudio que van de C en el PC hasta programar la SNES **en C con
PVSnesLib**, cada una anclada en el código real de *Tarou: A Dog's Journey*.
Las unidades 14 y 15 enseñan además a leer y escribir ensamblador 65816.
Hay 21 escritas (00–20) y 6 por escribir (21–26).

> **El juego cambió de género (2026-10-23).** Las unidades 00–20 se
> escribieron sobre la **Revisión 2**, el plataformas con el tutorial de
> movimiento, que queda congelado en el tag git **`plataformas-r2`** (commit
> `480f373`). El juego pasa a ser un beat 'em up de carril en profundidad
> ([`../docs/diseno-beat-em-up.md`](../docs/diseno-beat-em-up.md)), todavía
> sin código. Por eso toda práctica sobre el juego de las unidades 00–20 se
> hace en un clon fijado en ese tag (paso 3 de «Para el alumno»). Las
> unidades **01, 02, 10, 11 y 20 son históricas** (física de plataformas,
> colisión por rejilla, tutorial): su README lo avisa y dice qué parte del
> concepto se traslada al beat 'em up. Las demás enseñan hardware y motores
> que se quedan. Las 21–26, sobre el beat 'em up, se escribirán cuando
> exista su código (`PLAN.md` §3).

- **El plan** (formato de una unidad, dependencias, recetas verificadas,
  trampas conocidas y estado): [`PLAN.md`](PLAN.md).
- **La librería**: [`pvsneslib.md`](pvsneslib.md) relaciona cada función de
  PVSnesLib 4.6.0 que usa el curso con su unidad y con el archivo del juego.
  Su documentación oficial: <https://alekmaul.github.io/pvsneslib/>.
- **El vocabulario** común: [`../docs/glosario-snes.md`](../docs/glosario-snes.md)
  (cada entrada dice qué unidad la enseña).
- **La documentación técnica** del juego, que las unidades resumen y citan:
  [`../docs/README.md`](../docs/README.md).

## Las unidades (00–20 escritas el 2026-10-22; 21–26 por escribir)

El orden es el del grafo de [`PLAN.md`](PLAN.md) §4: cada unidad da por
sabido lo de sus prerrequisitos. «(tag)» = la práctica sobre el juego se
hace en un clon fijado en `plataformas-r2`; vale para todas las prácticas
sobre el juego de las unidades 00–20.

| Unidad | Tema | Práctica |
|---|---|---|
| [00](unidad-00-c-en-el-pc/) | C en el PC: lógica, funciones, arrays, structs, punteros, bits, preprocesador, varios archivos, `enum`/`switch`/`const` | PC (8 ejercicios) |
| [01](unidad-01-enteros-y-punto-fijo/) | Enteros de 16 bits, desbordamiento y punto fijo (histórica) | PC |
| [02](unidad-02-maquinas-de-estado/) | Máquinas de estado (histórica) | PC + pruebas del juego (tag) |
| [03](unidad-03-tuberia-de-compilacion/) | De `.c` a `.sfc`: la tubería de compilación | mini-ROM + lectura |
| [04](unidad-04-frame-y-vblank/) | El frame, el VBlank y el bucle principal | mini-ROM + juego |
| [05](unidad-05-mando/) | El mando | mini-ROM |
| [06](unidad-06-tiles-y-fondos/) | Tiles, paletas, tilemaps y fondos | mini-ROM |
| [07](unidad-07-scroll-y-camara/) | Scroll, cámara y parallax | mini-ROM + pruebas del juego |
| [08](unidad-08-sprites-y-oam/) | Sprites estáticos y la OAM | mini-ROM |
| [09](unidad-09-metasprites/) | Metasprites y sprites dinámicos | mini-ROM + juego |
| [10](unidad-10-fisica/) | Física de plataformas (histórica) | PC + juego (tag) |
| [11](unidad-11-animacion/) | Animación por estados (histórica) | PC + juego (tag) |
| [12](unidad-12-hdma-y-efectos/) | HDMA, efectos de la PPU y el motor de escenas | mini-ROM + juego |
| [13](unidad-13-memoria/) | Memoria: VRAM, CGRAM, OAM, WRAM y ROM por bancos | PC + juego |
| [14](unidad-14-asm-leer/) | Ensamblador 65816 I: leer lo que genera tcc816 | lectura + depurador |
| [15](unidad-15-asm-escribir/) | Ensamblador 65816 II: escribir una rutina y llamarla desde C | mini-ROM + juego |
| [16](unidad-16-mesen/) | Mesen: depurador, puntos de ruptura y visores | juego |
| [17](unidad-17-audio/) | Audio desde C con PVSnesLib (el juego aún no tiene audio) | mini-ROM |
| [18](unidad-18-herramientas/) | Herramientas: gfx4snes, scripts de Python y el Makefile | juego (TDD) |
| [19](unidad-19-del-arte-al-juego/) | Del arte al juego: Pixellab, celdas, paletas y perspectiva | PC + juego (TDD) |
| [20](unidad-20-colision/) | Colisión con el escenario y el nivel en Tiled (histórica) | PC + juego (tag) |
| 21 | Presupuesto de sprites: ranuras por línea, 512 tiles, VBlank, bandas de cine, parpadeo rotatorio (la prueba de carga) | **por escribir** |
| 22 | Belt-scroll: x/y/z, el carril, la sombra y el orden por profundidad (OAM y bit de prioridad) | **por escribir** |
| 23 | Combate: estados que se afectan, cajas de golpe y daño, parada de impacto | **por escribir** |
| 24 | Enemigos: IA con turnos de ataque y oleadas desde Tiled | **por escribir** |
| 25 | Matemática de color, ventanas por HDMA y los efectos de los apoyos (Mode 7, opcional) | **por escribir** |
| 26 | Relevo entre protagonistas y celda de VRAM compartida | **por escribir** |

## Para el alumno

1. Empieza por la unidad 00 y sigue el orden de `PLAN.md` §4.
2. En cada unidad: lee el `README.md`, ejecuta la demo, resuelve el
   ejercicio, compáralo con `solucion/` y cumple el criterio de aprobación.
   `capturas/` guarda la salida de referencia de lo que se ejecutó al
   escribir la unidad.
3. Las prácticas **sobre el juego real** se hacen en un clon, nunca en la
   rama principal. En las unidades 00–20, el clon se fija en el tag de la
   Revisión 2, el código contra el que se verificaron:

   ```text
   git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mi_unidad
   git -C E:\ProyectoSNES_trabajo\mi_unidad checkout -q plataformas-r2
   ```

   (o una rama propia creada desde el tag, `git switch -c mi_unidad
   plataformas-r2`). El clon compila con el SDK del repo (receta de
   `PLAN.md` §6.3).
4. Si un término no está definido en la unidad, búscalo en el glosario; si
   una función de la librería no está explicada, en `pvsneslib.md`.

## Para la sesión que revisa o reescribe una unidad

Pega esto al empezar, cambiando el número:

```text
Revisa la unidad NN de aprendizaje/ siguiendo aprendizaje/PLAN.md: su ficha
en §3, el formato de §2 y el procedimiento de §6 (definición de «hecho»,
convenciones, receta de mini-ROM, receta del .asm, comandos y trampas
conocidas). Antes de cambiar nada, relee los archivos de «Código del juego»
de la unidad; si algo cambió, manda el código y corrige la unidad y el plan.
Para las unidades 00–20, «el código» es el del tag plataformas-r2 (PLAN §6.1,
punto 7): reléelo en un clon fijado en ese tag, no en la rama principal, y no
reescribas una unidad histórica para enseñar el beat 'em up. Las 21–26 solo
se escriben cuando su código exista en la rama principal.
Trabaja los cambios del juego en un clon, ejecuta el criterio de aprobación
de verdad con la solución, guarda las capturas en la unidad, marca la tabla
de §7 y haz un commit sin push.
```
