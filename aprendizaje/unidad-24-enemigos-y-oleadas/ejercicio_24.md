# Ejercicio 24 (juego real, con TDD) — Una quinta zona al final del nivel

**Qué vas a hacer:** añadir una quinta zona de bloqueo de cámara en el último punto posible
del nivel (cámara 1792), pero **escribiendo primero la prueba que falla**. Después
comprobarás que el testigo de ataque tiene una prueba que lo vigila (rompiéndolo a
propósito), compilarás la ROM normal y la autojugable, y repetirás la réplica del guion.

Trabaja en un **clon** de la rama actual (nunca en el árbol principal):

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u24
git -C E:\ProyectoSNES_trabajo\u24 checkout -q 641d7c0
```

(El clon sale del commit `641d7c0`. `git` puede marcar todos los archivos como modificados
por los saltos de línea CRLF/LF; para ver solo tus cambios usa
`git diff --ignore-cr-at-eol`.) Los comandos de `make` van en **PowerShell**. Para las
pruebas basta `$env:PATH="C:\msys64\usr\bin;"+$env:PATH`. Para compilar la ROM, el entorno
de la unidad 03, con `PVSNESLIB_HOME` apuntando al SDK del repo principal (el clon no
trae `tools/`):

```text
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
```

## Punto de partida

```text
make -C tests ; .\tests\pruebas.exe        →   560 pruebas, 0 fallos
```

## Por qué 1792

`ZONA_CAMARA` guarda posiciones de la **cámara**, y la cámara no pasa de `CAMARA_MAX`
= `ANCHO_NIVEL − ANCHO_PANTALLA` = 2048 − 256 = **1792** (`camara.c`). Un punto mayor no
bloquearía nunca (la cámara no llega: la demo `capturas/hallazgo_margenes.txt` lo muestra
con 1800). 1792 es el último punto con sentido: el borde derecho del nivel.

## Pasos

1. **TODO 1 — La prueba primero** (`tests/prueba_oleadas.c`, al final de
   `pruebas_oleadas`). Escribe un bloque que, partiendo de `oleadas_iniciar(&o)`:
   - pase las cuatro primeras zonas (`oleadas_actualizar` con el `oleadas_limite` de cada
     una, y `oleadas_vencer`);
   - compruebe que ahora `o.actual == 4`, no está bloqueada y `oleadas_limite(&o) == 1792`;
   - con `camara_avanzar(camara, 2000, oleadas_limite(&o))` en un bucle (la cámara
     empieza en 1700), compruebe que la cámara llega **exacta** a 1792;
   - compruebe que `oleadas_actualizar` bloquea y avisa (devuelve 1) en ese punto;
   - venza la zona y compruebe `o.actual == OLEADAS_ZONAS`, `OLEADAS_ZONAS == 5` y
     `oleadas_limite(&o) == 0xFFFF`.

2. **TODO 2 — Verlo fallar.**

   ```
   make -C tests ; .\tests\pruebas.exe
   ```

   Deben salir **3 fallos**, todos de `prueba_oleadas.c` (el límite sigue siendo `0xFFFF`,
   no bloquea, y `OLEADAS_ZONAS` vale 4). Guarda la salida. Observa cuál de tus
   comprobaciones **no** falla (la de «la cámara llega a 1792») y explica por qué: sin
   zona, el límite es `0xFFFF`, pero `camara_avanzar` ya recorta a `CAMARA_MAX`.

3. **TODO 3 — Implementarlo** (dos líneas):
   `OLEADAS_ZONAS` pasa a 5 en `src/juego/oleadas.h`, y `ZONA_CAMARA` gana el 1792 en
   `src/juego/oleadas.c`. Vuelve a las pruebas: ahora fallan **otras 3**, las de la
   prueba vieja de «las cuatro zonas, y después ya no hay límite» (líneas ~52–54). Léelas:
   ¿qué regla antigua codifican? Esa prueba no estaba mal: describía el juego de
   entonces. Se actualiza (una zona más antes del «sin límite»), no se borra. Debe quedar
   en **568 pruebas, 0 fallos** (560 + 6 comprobaciones de tu bloque + 2 del TODO 4).

4. **TODO 4 — Una prueba para el testigo** (`tests/prueba_ninja.c`, al final de
   `pruebas_ninja`). Copia en una prueba el bloque de IA de `pantalla_fase1.c:249-263`
   (turnos, `ninja_actualizar`, `ninja_limitar_x`, `ninja_intentar_atacar`) con Tarou
   quieto en x 384, y 200 y la cámara en 256, una oleada de la zona 2 (tres ninjas) y 900
   cuadros. Comprueba dos cosas: que el máximo de ninjas atacando a la vez es **exactamente**
   `NINJA_TURNOS` (llegan al tope y no lo pasan), y que **cada ranura** dio al menos un
   tajo (nadie se queda sin turno). Debe pasar a la primera: es una prueba de
   **caracterización**, fija el comportamiento de hoy.

5. **TODO 5 — Rompe el testigo a propósito.** Borra la línea `turnos++;` de **tu copia** del
   bloque en la prueba y vuelve a ejecutar. Debe fallar `maximo == NINJA_TURNOS` (el máximo
   sube a 3). Restaura la línea. Esa es la utilidad de la prueba: si alguien toca el bucle
   de la pantalla o la regla de `turno_libre`, la prueba lo ve sin abrir Mesen. (Pregunta:
   la prueba **no** vigila el código de `pantalla_fase1.c`, solo su copia. ¿Qué pasaría si
   alguien cambiara la pantalla y no la prueba? ¿Cómo lo evitarías? Pista: extraer el bucle a
   una función de `ninja.c` que la pantalla y la prueba llamen las dos.)

6. **TODO 6 — Compilar las dos ROM.** Has tocado un `.h`: `make clean` siempre delante
   (`snes_rules` no sigue las cabeceras):

   ```
   make clean ; make
   make clean ; $env:CFLAGS="-DGUION_FASE1" ; make ; $env:CFLAGS=$null
   make clean
   ```

   Ambas deben terminar con `No warnings or errors found.` No abras Mesen: solo se
   comprueba que compilan. (El `make clean` final deja el árbol limpio.)

7. **TODO 7 — La réplica del guion.** Desde `aprendizaje/unidad-24-enemigos-y-oleadas/`
   (la copia de esta carpeta **dentro del clon**, o ajusta las rutas `../../` para que
   apunten al clon), genera la tabla y compila `traza_24_guion.c` como dice su cabecera.
   La salida debe ser **idéntica** a `capturas/salida_traza_24_guion.txt`. ¿Por qué no
   cambia nada si la lógica de zonas cambió? (Respuesta: la tabla de teclas acaba en el
   cuadro 1320, con la cámara en 738, mucho antes de la zona 4.)

8. **TODO 8 (ampliación) — Preguntas de diseño.**
   - La quinta oleada nace por la derecha a `camara + 256 + 40` = 2088, **más allá del
     final del nivel** (2048). ¿Se ve? ¿Por qué no pasa nada? (Pista: ¿dónde dibuja la
     pantalla un sprite con x de pantalla 296?)
   - Cambia `NINJA_TURNOS` a 1 y a 3 y ejecuta las pruebas. ¿Cuáles siguen en verde y por
     qué? Compara con la parte C de la demo.
   - **(Diseño, no existe todavía)** ¿Qué propiedades tendría el objeto de Tiled `oleada`
     de esta quinta zona para que el script `preparar_zona.py` generara las dos tablas
     (`ZONA_CAMARA` y la regla «zona + 1») que hoy son constantes? Escríbelas, pero no
     intentes ejecutarlo: el script no existe.

## Preguntas para el README de tu práctica

- ¿Por qué la prueba roja es útil aunque «ya sepas» que va a fallar?
- La prueba del TODO 4 pasó a la primera. ¿Qué la distingue de una prueba que no prueba
  nada? (La mutación del TODO 5.)
- ¿Qué se pierde si la ROM autojugable se queda sin repetirse tras cambiar la IA? (Mira
  `traza_24_guion.c`.)

## Criterio de aprobación

- La salida con **3 fallos** (TODO 2) y la de **568 pruebas, 0 fallos** (TODO 3).
- La salida del TODO 5 con `maximo == NINJA_TURNOS` fallando.
- Las dos ROM con `No warnings or errors found.`
- La traza igual que la de referencia.

## Solución

`solucion/quinta_zona.diff` (el cambio entero, hecho con `git diff --ignore-cr-at-eol`) y
los cuatro archivos completos (`oleadas.h`, `oleadas.c`, `prueba_oleadas.c`,
`prueba_ninja.c`). Para aplicar el diff en un clon con CRLF:
`git apply --ignore-whitespace solucion\quinta_zona.diff`. La salida real de cada paso
está en `capturas/`: `pruebas_rojo_quinta_zona.txt` (TODO 2), 
`pruebas_tras_implementar_sin_arreglar_la_vieja.txt` (TODO 3 a medias),
`pruebas_solucion.txt` (568, 0 fallos), `pruebas_mutacion_sin_turnos_mas_mas.txt`
(TODO 5), `compilacion.txt` (TODO 6) y `salida_traza_24_guion.txt` (TODO 7).

Respuesta medida del TODO 8 (`capturas/turnos_1_y_3.txt`): con `NINJA_TURNOS` en 1 y en 3
las 568 pruebas siguen en verde, porque las que miran los turnos comparan con la constante y
no con un 2 escrito a mano; lo que cambia es el comportamiento (demo, parte C), no las pruebas.
El 2 de la spec (§6) es una decisión de diseño y ninguna prueba la protege.

Los números de estos pasos son los medidos con la solución: 568 = 560 + 6 + 2. (Las
comprobaciones del bloque del TODO 1 son 6 y las del TODO 4, 2; si las escribes de otra
forma, tu total cambiará, y lo importante es que no haya fallos.)
