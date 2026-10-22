# Ejercicio 11 — Un gesto de espada que no se haga eterno

El problema existe de verdad: con el arte de Pixellab, desenvainar y envainar
(9 cuadros × 5 frames) duran 0,75 s cada uno y despertar (10 × 8), 1,33 s, y
mientras tanto el mando no responde. Hay que darles un ritmo natural sin
romper ni las pruebas ni la ROM que se juega sola.

Trabaja en tu propia copia (nunca en la rama principal):

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio11
```

## Parte A — Decidir con la demo (PC)

- **TODO 1.** Compila y ejecuta `demo_11_animacion.c`. Anota la duración de
  desenvainar, envainar, echarse y despertar en frames y segundos.
- **TODO 2.** Elige frames por cuadro nuevos para desenvainar, envainar y
  despertar. Criterio: un gesto de espada de menos de medio segundo (30
  frames) y un despertar de menos de un segundo. Cámbialos en la tabla `ANIMS`
  de la demo (columnas 4 y 5) y vuelve a ejecutarla hasta que te convenzan.
- **TODO 3.** Calcula a mano, con los cuadros de la **copia fija**
  (`tests/tarou_anim.h`: desenvainar 4, despertar 4), cuántos frames durarán
  en las pruebas. Ese es el número que va a `FRAMES_DESENVAINAR` y
  `FRAMES_DESPERTAR`.

## Parte B — Cambiarlo en el juego

- **TODO 4.** En `src/juego/animaciones.c`, cambia los frames por cuadro de
  `ESTADO_DESENVAINANDO`, `ESTADO_ENVAINANDO` y `ESTADO_DESPERTANDO`, con un
  comentario que diga la duración nueva con el arte (y la vieja).
- **TODO 5.** Ejecuta las pruebas **sin tocarlas**:

  ```
  $env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
  ```

  Con 3, 3 y 5 fallan 9 comprobaciones de `prueba_desenvainar` y
  `prueba_despertar` (`capturas/pruebas_antes_de_ajustar.txt`). Lee cada una y
  di por qué falla (pista: esperan 20 y 32 frames). ¿Por qué `prueba_envainar`
  no falla aunque también use `FRAMES_DESENVAINAR`? (Espera esos frames y
  comprueba que ya está en `PARADO`: si el gesto es más corto, también lo
  está.)
- **TODO 6.** En `tests/prueba_tarou.c`, ajusta `FRAMES_DESENVAINAR` y
  `FRAMES_DESPERTAR` con los cuadros de la copia fija y tus ritmos
  (`4 * 3` y `4 * 5` en la solución) y sus comentarios. Pruebas en verde:
  `367 pruebas, 0 fallos`.

## Parte C — Lo que las pruebas no ven

- **TODO 7.** Ejecuta `pruebas_con_arte_real.ps1 -Clon E:\ProyectoSNES_trabajo\mio11`.
  La segunda compilación usa los cuadros del arte: fallan 16 comprobaciones
  que dependen de la copia fija (es normal); **no** debe aparecer
  `prueba_recorrido`.
- **TODO 8.** Experimento: pon desenvainar a **12** frames por cuadro, ajusta
  `FRAMES_DESENVAINAR` a `4 * 12` y repite TODO 5 y TODO 7. Las pruebas de
  siempre pasan; con el arte real falla `prueba_recorrido`
  (`capturas/trampa_desenvainar_12.txt`). Explica por qué con los números
  del guion (`src/juego/guion_tutorial.c`: R en el cuadro 1772, A en el 1872)
  y vuelve a tus valores.

## Parte D — Verlo en Mesen

- **TODO 9.** Compila la ROM que se juega sola y guárdala aparte:

  ```
  $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
  make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null
  ```

  (`make clean` porque `animaciones.obj` no se rehace solo si cambias solo el
  header generado; aquí cambiaste un `.c`, pero la ROM de prueba pide de todos
  modos una compilación limpia). Debe decir `No warnings or errors found.`.
- **TODO 10.** Captura el desenvainado ante el tronco (cuadro 1772, unos 29,5 s
  después de entrar en el nivel; con el HUD, la fila `EST` dice
  `DESENVAINANDO`):

  ```
  .\scripts\capturar_mesen.ps1 -Rom RUTA\rom_guion.sfc -Duracion 33 -Cada 0.1 -Desde 28.5 -Espera 3 -Guion "1,1.3,Select"
  ```

  Debe durar unas 5 capturas (0,45 s) en vez de 8.
- **TODO 11.** Despertar no sale en el guion (Tarou nunca está 10 s quieto).
  Con la ROM normal (`make clean; make`): entra en el nivel con Start, no
  toques nada 10 s para que se duerma y pulsa Derecha:

  ```
  .\scripts\capturar_mesen.ps1 -Rom RUTA\rom_normal.sfc -Duracion 73 -Cada 0.1 -Desde 69.5 -Espera 4 -Guion "55,55.3,Start;70,70.3,Right"
  ```

  (el título aparece hacia el segundo 47; el sueño empieza unos 10 s después
  de entrar). Despertar debe ocupar unas 8 capturas (0,83 s), no 13. Durante
  la tanda no uses el equipo: el foco de Mesen salta y se pierden teclas.

## Aprobación

- `367 pruebas, 0 fallos` con los ritmos nuevos.
- `pruebas_con_arte_real.ps1` sin `prueba_recorrido` entre los fallos, y la
  explicación del TODO 8.
- Capturas de Mesen: desenvainar ante el tronco y despertar con el ritmo
  nuevo.

Al terminar, `git restore` en tu copia o deja los cambios en una rama propia.

## Solución

`solucion/ejercicio_11.patch` (3, 3 y 5 frames por cuadro; `4 * 3` y `4 * 5`
en las pruebas). Se aplica en una copia limpia con
`git apply RUTA\ejercicio_11.patch`. Respuesta del TODO 8: con 12, el arte
real tarda `9 × 12 = 108` frames en desenvainar y Tarou vuelve a `PARADO` en
el cuadro 1772 + 108 = 1880; la A del guion llega en el 1872, en mitad del
gesto, y `manejar_espada` ignora el mando. Sin ataque no se rompe el tronco y
el guion choca contra él hasta acabarse. La copia fija (4 cuadros) tarda 48
frames y llega de sobra: por eso `make -C tests` no lo ve.
