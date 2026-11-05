# Ejercicio 25 (juego real, con TDD) — Un destello al recibir un golpe

**Qué vas a hacer:** cuando la protagonista recibe un golpe, la pantalla se aclara unos
frames: se **suma** un gris (el color fijo) a los fondos y al cielo, con un brillo que baja
de 16 a 4. La regla va en un módulo puro con su prueba escrita **primero**; luego la pantalla
escribe los tres registros en el VBlank sin estropear la sombra ni la pausa, y lo ves en
Mesen.

Trabaja en un **clon** de la rama actual (nunca en el árbol principal):

```text
git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u25
```

(El clon sale del commit `6845b5f`. Si `git` marca archivos por los saltos de línea CRLF/LF,
mira solo tus cambios con `git diff --ignore-cr-at-eol`.) Los comandos de `make` van en
**PowerShell**. Para las pruebas basta `$env:PATH="C:\msys64\usr\bin;"+$env:PATH`; para la
ROM, el entorno de la unidad 03 con el SDK del repo principal (el clon no trae `tools/`):

```text
$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH
```

## Punto de partida

```text
make -C tests ; .\tests\pruebas.exe        →   2477 pruebas, 0 fallos
```

## La regla

1. Si la vida de este frame es **menor** que la del anterior, empieza un destello de
   `DESTELLO_FRAMES` (4) frames. Si llega otro golpe a mitad, **empieza de nuevo**.
2. El brillo de cada frame es `frames que quedan · 4`, con tope `DESTELLO_MAXIMO` (16): con 4
   frames sale **16, 12, 8, 4** y luego 0.
3. Si la vida **sube** (al reiniciar la fase con CONTINUE vuelve a 24) **no** es un golpe.
4. En pausa, GAME OVER o STAGE CLEAR no hay destello (el mundo no avanza).

## Pasos

1. **TODO 1 — La prueba primero.** Crea `tests/prueba_destello.c` con una función
   `pruebas_destello(void)` (cabecera estándar y comentarios, como `prueba_estado_fase.c`).
   Comprueba, con `COMPROBAR`: que sin golpes siempre da 0 (10 frames); que un golpe de 24 a
   22 da 16, 12, 8, 4 y luego 0 dos veces; que un golpe a mitad (dos frames después) vuelve a
   dar 16; que subir la vida de 2 a 24 da 0 y el golpe siguiente sí da 16. Regístrala en
   `tests/pruebas.c` (declaración y llamada, junto a `pruebas_ambiente`) y añade
   `../src/juego/destello.c` a la lista `PURO` de `tests/Makefile`.

2. **TODO 2 — Verlo fallar.** Copia los esqueletos:

   ```text
   copy E:\ProyectoSNES\aprendizaje\unidad-25-matematica-de-color\ejercicio_25_destello.h src\juego\destello.h
   copy E:\ProyectoSNES\aprendizaje\unidad-25-matematica-de-color\ejercicio_25_destello.c src\juego\destello.c
   make -C tests ; .\tests\pruebas.exe
   ```

   Deben salir **6 fallos**, todos de `prueba_destello.c`: las cuatro del primer destello y
   las dos de «empieza de nuevo». ¿Por qué las de «sin golpes da 0» **no** fallan? (El
   esqueleto siempre devuelve 0: esas pruebas pasan por casualidad. Una prueba que pasa con
   un esqueleto vacío solo vale junto a las que fallan.)

3. **TODO 3–6 — Implementarlo** en `destello.h` / `destello.c` siguiendo los TODO del
   esqueleto. Sin `int`, sin multiplicar (`frames << 2`), sin tablas. Deben quedar
   **2497 pruebas, 0 fallos** (2477 + las 20 de tu prueba, si la escribes como la de la
   solución).

4. **TODO 7 — La pantalla.** En `src/pantallas/pantalla_fase1.c`:
   - incluye `juego/destello.h` y añade `static Destello destello;`, `static u8 brillo;` y
     `static u8 brillo_puesto;` (el que hay puesto en la PPU, para escribir solo cuando
     cambia);
   - en `pantalla_fase1_iniciar`, después de `mundo_iniciar`: `destello_iniciar(&destello,
     cuerpo.vida)` y los dos brillos a 0 (**las `static` no empiezan a 0** en la SNES);
   - en `pantalla_fase1_actualizar`, justo antes de «Lo que va a la pantalla»: `brillo = 0`
     y, si `estado_fase_mundo_avanza(&partida)`, `brillo = destello_actualizar(&destello,
     cuerpo.vida)`;
   - al final de `pantalla_fase1_vblank`, **después** del bloque de la media luz: si no hay
     media luz (`!atenuada`) y `brillo != brillo_puesto`, apunta el nuevo y escribe:
     - con brillo: `REG_COLDATA = 0xE0 | brillo` (gris en los tres canales),
       `REG_CGWSEL = 0x00` (operando: el color fijo), `REG_CGADSUB = 0x23` (sumar, sin
       mitad: BG1, BG2 y fondo);
     - sin brillo: **vuelve** a `REG_COLDATA = 0xE0`, `REG_CGWSEL = 0x02`,
       `REG_CGADSUB = 0x50` (lo de `protagonista_escena_cargar`).

5. **TODO 8 — Compilar.** Has tocado un `.h` nuevo: `make clean` delante.

   ```text
   make clean ; make
   make clean ; $env:CFLAGS="-DGUION_FASE1 -DDESTELLO_FRAMES=63" ; make ; $env:CFLAGS=$null
   copy ProyectoSNES.sfc E:\ProyectoSNES_trabajo\u25_destello.sfc
   make clean
   ```

   Las dos con `No warnings or errors found.` La segunda es solo para **verlo**: un destello
   de 4 frames dura 67 ms y una captura por segundo casi nunca lo pilla; con 63 frames dura
   un segundo (el brillo se queda en 16 casi todo ese tiempo por el tope).

6. **TODO 9 — Mesen.** Desde el repo principal, sin tocar el PC durante la captura:

   ```text
   .\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\u25_destello.sfc -Duracion 42 -Desde 26 -Cada 1 -Espera 3
   ```

   La ROM se juega sola; el ninja de la segunda zona alcanza a la protagonista hacia los
   30–31 s (la vida pasa de 24 a 22). Busca la captura clara y compárala con la anterior y
   con la siguiente. Mira, en la clara: los fondos y el cielo, aclarados; los **cuerpos**
   (paletas 0–3) y el **texto** (BG3), igual que antes; y la **sombra**, ahora negra y opaca.
   Explica por qué (el modo `0x23` no lleva el bit 4: la paleta 4 dejó de mezclarse).

7. **TODO 10 (ampliación) — Preguntas.**
   - Pausa justo durante un destello largo y reanuda. ¿Queda la pantalla bien? Sigue el
     código: la pausa escribe sus registros; al reanudar, la media luz vuelve a `0x02`/`0x50`
     y el bloque del destello ve `brillo (0) != brillo_puesto` y escribe lo mismo otra vez.
   - Quita `REG_COLDATA = 0xE0` del caso «sin brillo». ¿Qué cambia después del primer
     golpe? (Pista: el humo sobre el cielo. Ver «Trampas» en el README.)
   - ¿Por qué el destello no se hace con `setBrightness`? (Pista: el brillo de la pantalla
     solo **baja** de 15; un destello **sube**.)

## Criterio de aprobación

- La salida del TODO 2 con **6 fallos** y la del TODO 3 con **2497 pruebas, 0 fallos**.
- Las dos ROM con `No warnings or errors found.`
- Tres capturas de Mesen: antes del golpe, el destello y después, con los cuerpos y el texto
  iguales en las tres.

## Solución

`solucion/destello.diff` (el cambio entero, `git diff --ignore-cr-at-eol` con los archivos
nuevos) y los archivos completos `destello.h`, `destello.c` y `prueba_destello.c`. Para
aplicar el diff en un clon: `git apply --ignore-whitespace solucion\destello.diff`. Las
salidas reales de cada paso están en `capturas/` (`pruebas_rojo.txt`, `pruebas_verde.txt`,
`compilacion.txt` y las tres `mesen_*.png`).
