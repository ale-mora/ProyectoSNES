# Ejercicio 10 — Afinar el salto sin romper el tutorial

Sobre el juego real. **Trabaja en tu propia copia**, nunca en la rama
principal:

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio10
git -C E:\ProyectoSNES_trabajo\mio10 checkout -q plataformas-r2
cd E:\ProyectoSNES_trabajo\mio10
```

(el tag `plataformas-r2` es la Revisión 2, el código de plataformas que enseña
esta unidad; o una rama propia creada desde ese tag,
`git switch -c mio10 plataformas-r2`; al terminar, `git restore` deshace todo).
Las pruebas se ejecutan así, desde la raíz de la copia:

```
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

Antes de tocar nada, comprueba el punto de partida: `367 pruebas, 0 fallos`.

## Parte A — Un salto flojo: ¿por qué falla el nivel?

- **TODO 1.** En `src/juego/tarou.h`, cambia `VEL_SALTO` de `(-96)` a
  `(-80)`. Nada más.
- **TODO 2.** Antes de compilar, **predice** con la demo: `demo_10.exe` ya
  trae la línea de −80 (altura, frame de aterrizaje, «muro» y «foso»).
- **TODO 3.** Compila y ejecuta las pruebas. Deben fallar **seis**
  comprobaciones (salida real en `capturas/pruebas_parteA_salto_flojo.txt`):

  ```
  FALLO prueba_tarou.c:181: PF_ENTERO(y_minimo) >= 73 && PF_ENTERO(y_minimo) <= 77
  FALLO prueba_colision.c:181: PF_ENTERO(t.x_fp) > 640
  FALLO prueba_recorrido.c:66: tronco_roto == 1
  FALLO prueba_recorrido.c:67: e == PARTIDA_SALIDA
  FALLO prueba_recorrido.c:68: cuadro == GUION_TUTORIAL_FIN
  FALLO prueba_recorrido.c:69: p.reaparicion == controles[NIVEL1_CONTROLES - 1]
  367 pruebas, 6 fallos
  ```

- **TODO 4.** Escribe, en tres o cuatro frases, **por qué** falla cada grupo.
  Pistas: la altura máxima con −80 (la suma de la unidad: `g·n(n−1)/2` con
  `n = 80/4`); el alto del muro (`prueba_muro`, en `tests/prueba_colision.c`:
  el muro va de x 592 a 639 y mide 48 px); qué hace el guion del recorrido
  cuando el salto del cuadro 592 no sube al muro; y por qué el foso **no**
  falla (demo: «foso corriendo: SI»).
- **TODO 5.** Deshaz el cambio (`git restore src/juego/tarou.h`).

## Parte B — Más rápido, a la misma altura

Un salto de −96 con gravedad 4 tarda 24 frames en subir: algo «flotante».
Queremos uno más vivo: **`GRAVEDAD 6`** y **`VEL_SALTO (-120)`**.

- **TODO 6.** Con la demo, comprueba la pareja antes de tocar el juego:
  altura, `y` mínima, frame del apogeo y del aterrizaje, muro y foso.
- **TODO 7.** Cambia las dos constantes en `src/juego/tarou.h` y **sus
  comentarios** (hoy dicen «0,25 px» y «~72 px de altura»: un comentario que
  miente es peor que ninguno).
- **TODO 8.** Ejecuta las pruebas. Falla **una**: `prueba_apogeo_y_altura`.
  Arréglala con los números nuevos: cuántos frames hasta `vy >= 0`
  (`120 / 6`), y el rango de la `y` mínima (144 − 71,25 → 72). Revisa
  también los comentarios con cuentas de las pruebas de salto (`-96 + 4`,
  «92/16 px», «Aterriza en el frame 47»): que digan la verdad.
- **TODO 9.** Pruebas en verde: `367 pruebas, 0 fallos`. Fíjate en que
  `prueba_colision` y `prueba_recorrido` **siguen** en verde: con 71,25 px el
  muro se salta y con 40 frames en el aire corriendo se recorren 80 px (el foso
  pide 68). El guion del recorrido mantiene B 40 frames contra el muro; con
  este salto le sobra.
- **TODO 10.** Compila la ROM que se juega sola (la del guion) en tu copia:

  ```
  $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
  make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null
  ```

  `make clean` es obligatorio: has tocado un `.h` y `snes_rules` no lo sigue.
  Debe decir `No warnings or errors found.`.
- **TODO 11.** Mírala en Mesen con el HUD. La ROM de prueba arranca directa en
  el nivel y la juega el guion; SELECT (tecla E) enciende el HUD. Con el
  script de capturas, desde la raíz de **tu copia** (la ROM se llama
  `ProyectoSNES.sfc`; cópiala aparte primero):

  ```
  .\scripts\capturar_mesen.ps1 -Rom RUTA\tu_rom_guion.sfc -Duracion 14 -Cada 0.1 -Desde 9 -Espera 3 -Guion "1,1.3,Select"
  ```

  El salto del muro empieza en el cuadro 592 del nivel (unos 10 s; el momento
  exacto depende del arranque, de ahí la ventana de 9 a 14 s con 10 capturas
  por segundo). Busca la captura en que Tarou está más alto: la fila `Y` debe
  estar cerca de **72** (era 75). Recuerda que cada fila del HUD se refresca
  cada 8 frames: el valor exacto lo da el PC; Mesen lo corrobora. Al terminar,
  deja que siga: el guion debe llegar a «LEVEL 2 / COMING SOON» (unos 38 s).

## Solución

- `solucion/ejercicio_10_parteA.patch`: el cambio de la parte A (para
  reproducir los fallos).
- `solucion/ejercicio_10_parteB.patch`: constantes, comentarios y la prueba
  ajustada. Se aplica en tu copia con `git apply RUTA\ejercicio_10_parteB.patch`.
- `capturas/pruebas_solucion.txt`: `367 pruebas, 0 fallos` con la parte B.
- `capturas/compilacion_rom_guion.txt`: la ROM de prueba compilada sin avisos.

Respuesta del TODO 4 (léela después de escribir la tuya): con −80 la subida
es `4·20·19/2 = 760` dieciseisavos = **47,5 px**, medio píxel menos que el
muro de 48. Los pies nunca cruzan el borde de arriba del muro, así que Tarou
vuelve a caer a su lado: falla `prueba_muro` (no pasa de x 640). El guion del
recorrido salta en el mismo sitio y se queda atascado delante del muro hasta
que se acaba la tabla: nunca rompe el tronco, nunca llega a la salida (el
bucle termina en `GUION_TUTORIAL_FIN + 300` sin `PARTIDA_SALIDA`) y su último
punto de control no es el último cartel. `prueba_apogeo_y_altura` falla porque
la `y` mínima es 96, no 73..77. El foso no falla porque depende del tiempo en
el aire y de la velocidad: 39 frames a 2 px son 78 px, más que los 68 que
pide.

## Aprobación

- `367 pruebas, 0 fallos` con `GRAVEDAD 6` y `VEL_SALTO -120`.
- La explicación del TODO 4.
- Una captura del HUD de la ROM de prueba con Tarou en lo alto del salto del
  muro y `Y` cerca de 72, y el guion terminando el nivel.
