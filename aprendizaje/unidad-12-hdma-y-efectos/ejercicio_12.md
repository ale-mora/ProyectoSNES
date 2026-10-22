# Ejercicio 12 — Un cielo de atardecer y una escena para el publisher

Dos partes. La A es obligatoria (mini-ROM); la B es la ampliación (el juego
real, en tu clon). Lee antes el README de la unidad.

## Parte A — El cielo de atardecer (mini-ROM)

Archivo: `rom/src/cielos.c`. La tabla `cielo_dia` está hecha; `cielo_atardecer`
es una franja negra provisional. Hay que escribirla con la macro `FRANJA` de
`rom/src/cielos.h` (cada `FRANJA` es una entrada de 5 bytes de la tabla HDMA).

- **TODO 1.** 15 franjas de 8 líneas (líneas 0..119) que vayan de morado
  `(6, 2, 12)` a naranja `(28, 10, 6)`. Cada componente es de 5 bits (0..31).
  Calcula los intermedios repartiendo la diferencia en 14 saltos (o copia los de
  la solución, si lo que quieres practicar es solo el formato).
- **TODO 2.** Una entrada de **1 línea** blanca `(31, 31, 31)`. Antes de
  compilar, escribe en el comentario en qué fila de la imagen (0..223) esperas
  verla, sumando las cuentas de las entradas anteriores.
- **TODO 3.** 13 franjas de 8 líneas de naranja `(28, 10, 6)` a amarillo
  `(31, 24, 10)`, y el `0x00` final. Borra las dos `FRANJA(112, 0, 0, 0)`
  provisionales.

Comprobaciones antes de compilar:

1. Ninguna cuenta pasa de 127 (con el bit 7 a 1 el canal entraría en modo
   repetir y leería un color por línea: basura).
2. La tabla termina en `0x00`.
3. ¿Cuántos bytes mide la tabla? (Entradas × 5 + 1.) Compruébalo con
   `cielo_dia` en el `.sym`: está en `$7F:0000` y `cielo_atardecer` en
   `$7F:008D`; la diferencia, 141, es 28 × 5 + 1.

Compila desde `rom/` con la orden del README y busca `No warnings or errors
found.`. En Mesen, a los 6 s del ciclo (o con X) el cielo cambia al atardecer.

**Medir la línea blanca.** Pausa (Game → Pause) con el atardecer en pantalla y
haz una captura (o usa el Event Viewer de Mesen, Debug → Event Viewer: marca
las escrituras en `$2122` y mira en qué línea cae cada una). Anota la fila de
píxel de la imagen de 256×224 en la que se ve la línea blanca y compárala con
tu predicción del TODO 2. El README explica por qué el proyecto midió un
desfase de una línea en la tabla de la ventana del logo Nintendo; aquí, con la
tabla de color, comprueba si se repite y escribe en una línea por qué crees que
sí o que no.

**Preguntas** (respóndelas en tu cuaderno):

- a) Con el cielo de atardecer puesto, ¿qué color queda en el cielo cuando el
  ciclo apaga el HDMA a los 8 s? ¿Por qué ese y no otro?
- b) Durante el fundido de los 2 s, ¿cuánto avanza `VBL` mientras `LOOP` está
  parado? Calcúlalo antes de mirarlo (`FRAMES_PASO` = 2).
- c) ¿Por qué `setModeHdmaColor` puede leer `cielo_atardecer` aunque esté en la
  WRAM y no en la ROM?

Solución: `solucion/cielos.c` (cópiala sobre `rom/src/cielos.c` para compilarla
y vuelve a dejar el esqueleto si quieres repetir el ejercicio). Respuestas: a)
el amarillo de la última franja, porque el canal escribió ese color en la
CGRAM en la última entrada del frame y nadie lo cambia después; b) 64 VBlanks
(2 × 16 del `FADE_OUT` + 2 × 16 del `FADE_IN`); c) el HDMA lee por el bus de
la CPU cualquier dirección de 24 bits: el banco `$7F` de la WRAM tanto como la
ROM; `setModeHdmaColor` toma el banco del propio puntero.

## Parte B (ampliación) — Una escena para el logo del publisher

Hoy `src/pantallas/pantalla_publisher.c` hace `fade_in`, espera
`DURACION_PUBLISHER` (180 frames) y `fade_out`: 244 frames, dos de ellos
bloqueantes. Pásala al motor de escenas, como el logo Nintendo.

Trabaja en **tu clon** (nunca en `E:\ProyectoSNES`):

```
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u12
```

- **TODO 1.** Crea `src/escena/escena_publisher.h` y `.c` (con la cabecera
  estándar) con la tabla `ESCENA_PUBLISHER`, que dure **244 frames** (si dura
  otra cosa, el título deja de salir hacia el segundo 47): el logo se enciende
  en 32 frames mientras se enfoca (mosaico 15 → 0), queda quieto, se pixela al
  salir (mosaico 0 → 15 en 48 frames, el ritmo de `setMosaicEffect`) y se apaga
  (brillo 15 → 0 en los últimos 32).
- **TODO 2.** Escribe `tests/prueba_escena_publisher.c` con una función
  `pruebas_escena_publisher` que compruebe: tabla ordenada con el `FIN` el
  último, duración `32 + 180 + 32`, brillo 15 y mosaico 0 entre los fundidos,
  ninguna pista con saltos mayores que su paso, y al final brillo 0 (**apagada**)
  y luz 0. Añádela a `tests/pruebas.c` y `escena_publisher.c` a `PURO` en
  `tests/Makefile`.
- **TODO 3.** Cambia `pantalla_publisher.c`: `_iniciar` carga el logo,
  `efectos_preparar(0x02)` (el logo está en el fondo 1) y `escena_empezar`, sin
  `fade_in`; `_actualizar` llama a `escena_avanzar` y, al terminar,
  `efectos_terminar()` **y después** `secuencia_cambiar(PANTALLA_DEVS)`; y una
  función nueva `pantalla_publisher_vblank` que llama a `efectos_aplicar`.
- **TODO 4.** Declara `pantalla_publisher_vblank` en `src/sistema/pantallas.h`
  y añade su `case` en `secuencia_vblank` (`src/sistema/secuencia.c`).
- **TODO 5.** Como tocaste un `.h`: `make clean` y `make` (desde PowerShell, con
  el entorno del README). Y las pruebas:

```
$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests; .\tests\pruebas.exe
```

**Aprobación de la parte B:** `pruebas.exe` en verde (con la solución:
`378 pruebas, 0 fallos`, ver `capturas/salida_pruebas_ampliacion.txt`), el
juego compila con `No warnings or errors found.`, y en Mesen (captura cada
0,1 s desde el segundo 4 de la ROM) se ve el logo de Capcom enfocándose, quieto
y pixelándose mientras se apaga; la pantalla siguiente (el logo de los
desarrolladores, con su zoom de Mode 7) sale **nítida**: si saliera pixelada,
faltaría `efectos_terminar()`.

Solución: `solucion/juego/` (los cuatro archivos completos y
`ampliacion_publisher.patch`, el `git diff` del clon; se aplica en un clon
limpio con `git apply solucion/juego/ampliacion_publisher.patch` desde la raíz
del clon, después de copiar la ruta). Al terminar, deshaz los cambios del clon
o bórralo: el juego principal no cambia con este ejercicio.
