# Ejercicio 20 (juego real) — Un segundo foso

**Qué vas a hacer:** añadir al tutorial un foso de 64 px **después del tronco**,
regenerar el nivel, ver cómo las pruebas detectan que el guion ya no llega a la
salida, y arreglar el guion con una traza de x/y por cuadro hasta que pase.
Después, verlo en la ROM de prueba que se juega sola.

Trabaja en una rama propia o en un clon (`git clone E:\ProyectoSNES
E:\ProyectoSNES_trabajo\mio`): los cambios no se confirman en la rama
principal. En un clon, `res/descargas/` no existe (git la ignora) y
`preparar_nivel.py` necesita la hoja de Pixellab del suelo: copia
`E:\ProyectoSNES\res\descargas\graficos\pixellab\suelo\*.png` a la misma ruta del
clon. Los comandos de compilar van en **PowerShell** (con el entorno de la
unidad 03).

## Por qué ahí

El foso va en las columnas de celda **107..110** (x 1712..1775), filas 11..13
(todo el suelo base). Comprueba tú por qué ese sitio no rompe nada más:

- el **tronco** ocupa x 1640..1671 y su dibujo son 4 columnas de parche
  pre-dibujadas: un foso que las tocara saldría con tierra dibujada debajo del
  tronco;
- el cartel **EXIT** está en x 1816: `poner_cartel` lo planta sobre el suelo que
  haya debajo (`suelo_bajo`); encima de un foso, el poste bajaría hasta el fondo;
- `tests/prueba_partida.c` (`prueba_salida`) empieza con la celda en x 1800
  (pies en 1836 y 1864): fuera del foso.

Y por qué **64 px**: los pies están a 28 px; caminando (1 px por cuadro) un
salto largo dura unos 48 cuadros, 48 px en el aire. Hay que despegar con el pie
de atrás aún en tierra (x + 36 ≤ 1711) y aterrizar con el de delante ya al otro
lado (x + 64 ≥ 1776): es un foso que se salta **caminando**, a diferencia del de
96 px, que exige correr.

## Pasos

1. **TODO 1 — Pintarlo en Tiled.** Instala Tiled (`winget install Tiled.Tiled`)
   y abre `res/nivel1/tutorial.tmj`. En la capa `logica`, borra (goma) las
   celdas de las columnas 107..110 en las filas 11, 12 y 13. Tiled muestra la
   celda bajo el ratón en la barra de estado (columna, fila desde 0). Guarda.
   Comprueba con `git diff res/nivel1/tutorial.tmj` que solo cambian 3 líneas
   de la capa y que el JSON sigue **sin espacio tras los dos puntos**
   (`"width":128`).

2. **TODO 2 — Regenerar.** Desde la raíz:

   ```
   python scripts/preparar_nivel.py
   ```

   **Sin `--crear`** (reescribiría el `.tmj` con el trazado inicial). Anota el
   número de «tiles distintos» que imprime (antes eran 260): ¿por qué sube un
   poco? (pista: bordes nuevos de la rejilla doble). Debe seguir por debajo de
   460.

3. **TODO 3 — Ver fallar las pruebas.**

   ```
   $env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
   ```

   Deben salir **4 fallos, todos en `prueba_recorrido.c`**. Guarda la salida.
   Explica por qué `prueba_colision.c` y `prueba_partida.c` siguen en verde
   aunque usen la misma rejilla (mira qué x usa cada prueba).

4. **TODO 4 — La traza.** Compila la herramienta de esta unidad (desde
   `aprendizaje/unidad-20-colision/`):

   ```
   gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../tests -I../../src -I../../res/nivel1 -o traza_20.exe traza_20.c ../../src/juego/colision.c ../../src/juego/tarou.c ../../src/juego/animaciones.c ../../src/juego/partida.c ../../src/juego/guion.c ../../src/juego/guion_tutorial.c
   .\traza_20.exe
   ```

   Sin argumentos imprime solo los cuadros en que cambia algo (estado, botones,
   apoyo) y termina diciendo dónde reaparece. Busca el cuadro en que pasa a
   `CAYENDO` sobre el foso nuevo. Después imprime un tramo entero, cuadro a
   cuadro, para elegir el cuadro del salto:

   ```
   .\traza_20.exe 2010 2030
   ```

   La columna `pies=` dice dónde están los dos pies. Busca el intervalo de
   cuadros en que el pie de atrás aún está en tierra (≤ 1711).

5. **TODO 5 — Ajustar el guion.** En `src/juego/guion_tutorial.c`, añade un
   tramo `{desde, desde + 30, BOTON_SALTAR}` (B mantenido 30 cuadros: salto
   largo, como los demás) con el comentario de qué obstáculo resuelve. Sube
   `GUION_TUTORIAL_TRAMOS` en `guion_tutorial.h` (y en las dos cabeceras de
   comentario). Vuelve a compilar la traza y repite hasta que diga
   `SALIDA en el cuadro N`. Si N no coincide con `GUION_TUTORIAL_FIN`,
   actualízalo (pregunta: ¿por qué saltar caminando no cambia el cuadro de
   llegada?).

6. **TODO 6 — Pruebas en verde.** `make -C tests; .\tests\pruebas.exe` debe dar
   `367 pruebas, 0 fallos`.

7. **TODO 7 — La ROM de prueba.** Como has tocado un `.h`, siempre con
   `make clean` delante:

   ```
   $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
   make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null
   Copy-Item ProyectoSNES.sfc ..\roms\foso2_guion.sfc
   .\scripts\capturar_mesen.ps1 -Rom ..\roms\foso2_guion.sfc -Duracion 46 -Cada 1 -Desde 30 -Espera 3
   ```

   (rutas relativas a la raíz de tu clon; ajústalas). En las capturas de los
   segundos ~33–35 Tarou salta el foso nuevo; hacia el 38–39 aparece «LEVEL 2 /
   COMING SOON». Después vuelve a compilar la ROM normal (`make clean; make`).
   No uses el equipo mientras captura.

8. **TODO 8 (ampliación) — Un cartel.** Añade en Tiled un objeto `cartel` (un
   punto en la capa `objetos`) antes del foso, con la propiedad `texto`
   (`JUMP|AGAIN`; `|` separa las dos líneas). ¿Qué cambia en
   `nivel1_datos.h`? ¿Qué prueba comprobaría que se pasa por él? ¿Dónde
   reaparece ahora Tarou si cae en el foso nuevo?

## Preguntas para el README de tu práctica

- ¿Por qué el guion no se ajusta «a ojo» en Mesen sino con la traza del PC?
- La ROM de prueba usa el `tarou_anim.h` real (desenvainar dura 9 × 5 = 45
  cuadros) y la traza la copia fija de `tests/` (4 × 5 = 20). ¿Por qué el
  recorrido llega igual? (Mira cuántos cuadros hay entre la R y la A del
  guion.)

## Criterio de aprobación

- La salida de `pruebas.exe` con 4 fallos (paso 3) y con 0 fallos (paso 6).
- La traza final terminando en `SALIDA en el cuadro ...`.
- Las capturas de la ROM de prueba saltando el foso y llegando a «COMING SOON».

## Solución

`solucion/foso2.diff` (el cambio entero: 3 líneas del `.tmj` y el guion) y los
archivos completos `solucion/tutorial.tmj`, `guion_tutorial.c` y
`guion_tutorial.h`. La salida real de cada paso está en `capturas/`:
`pruebas_con_foso2_sin_guion.txt` (4 fallos), `traza_con_foso2_sin_guion.txt`
(reaparece en el cuadro 2051), `traza_tramo_2010_2030.txt` (el tramo cuadro a
cuadro), `pruebas_solucion.txt` (0 fallos) y `traza_solucion.txt` (salida en el
cuadro 2270). Resumen: el nivel pasa de 260 a 264 tiles distintos; el salto va
en `{2020, 2050, BOTON_SALTAR}` (despega con los pies en 1706..1734 y aterriza
en el cuadro 2066 con el de delante en 1780); `GUION_TUTORIAL_TRAMOS` pasa a
14 y `GUION_TUTORIAL_FIN` sigue en 2270, porque en el aire se conserva la
velocidad de caminar.
