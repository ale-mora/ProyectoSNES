# Ejercicio 04 — Un contador que parpadea y una medida del juego real

Dos partes. La A es sobre la mini-ROM de `rom/`; la B, sobre el juego, en un
clon tuyo.

## Parte A — El contador de frames parpadea cada medio segundo

Copia `rom/` a una carpeta tuya (por ejemplo
`E:\ProyectoSNES_trabajo\mi_u04\`) y trabaja sobre la copia: así `rom/` queda
como la demo. Objetivo: la fila `FRAMES` se ve medio segundo, desaparece medio
segundo, se ve otra vez... mientras las demás filas siguen igual.

Restricciones (las del código de la SNES de este proyecto): sin `int`, sin
divisiones de variables en el bucle, y las `static` inicializadas a mano.

- **TODO 1.** Define `MEDIO_SEGUNDO` a partir de `SEGUNDOS(1)`. ¿Hay alguna
  división en la ROM si escribes `SEGUNDOS(1) / 2`? (Unidad 03, ejercicio.)
- **TODO 2.** Dos variables `static`: un contador de frames del parpadeo
  (`u8`) y una bandera `visible` (`u8`, 0 o 1). Ponlas a su valor inicial
  junto a las demás, antes del bucle.
- **TODO 3.** En cada vuelta del bucle, suma 1 al contador; al llegar a
  `MEDIO_SEGUNDO`, vuelve a 0 y alterna `visible` (pista: `^ 1`, unidad 00).
- **TODO 4.** En el `case 0` del `switch`: si `visible`, dibuja la fila como
  ahora; si no, dibuja espacios en la misma posición (los suficientes para
  tapar `FRAMES  65535`). Usa `if/else`, **no** `visible ? "..." : "..."`
  como argumento de `consoleDrawText`: `tcc816` da `error: ICE 2` con `?:` de
  cadenas en una función variádica (unidad 03, trampas).

Compila en tu copia («No warnings or errors found.») y ábrela en Mesen.

**Pregunta:** la fila se redibuja solo cuando le toca el turno (1 de cada 5
frames). ¿Cuántos frames, como mucho, tarda en desaparecer desde que
`visible` pasa a 0? ¿Se nota? ¿Cómo lo arreglarías si se notara?

## Parte B — Medir el nivel con el HUD encendido

La ROM de prueba (`-DGUION_PRUEBA`) se juega sola, pero el HUD solo se
enciende con SELECT, y enviar teclas a Mesen desde un guion no es fiable
(`docs/mesen-y-depuracion.md` §5). Haz que la ROM de prueba arranque con el
HUD encendido:

1. Clona el proyecto:
   `git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio`.
2. En `src/pantallas/pantalla_nivel1.c`, en `pantalla_nivel1_iniciar()`,
   justo después de `hud_debug_iniciar();`, añade una llamada a
   `hud_debug_alternar();` **solo** cuando se compila con `GUION_PRUEBA`
   (`#ifdef`, unidad 00), con su comentario.
3. ¿Cambia eso el recorrido? Razónalo: el guion cuenta **vueltas del bucle**
   (`cuadro_guion++` en `mando_leer`), no VBlanks. Compruébalo: pruebas de
   PC en tu clon (`$env:PATH="C:\msys64\usr\bin;"+$env:PATH; make -C tests;
   .\tests\pruebas.exe` → `367 pruebas, 0 fallos`).
4. Compila la ROM de prueba en tu clon:
   `make clean; $env:CFLAGS="-DGUION_PRUEBA"; make; $env:CFLAGS=$null`.
5. Captúrala sin enviar ninguna tecla (¡sin `Select`, que apagaría el HUD!),
   desde la raíz de `E:\ProyectoSNES`:
   ```
   .\scripts\capturar_mesen.ps1 -Rom E:\ProyectoSNES_trabajo\mio\ProyectoSNES.sfc -Duracion 46 -Cada 2 -Espera 3
   ```
   y no uses el equipo mientras corre.
6. En dos capturas separadas unos segundos, apunta `F`, `VB`, los tres
   números de `LIN` y `V a b`.

**Escribe la nota** que pide la aprobación del README: el porcentaje de frame
(`desde_vblank(L) / 262` con el último número de `LIN`; README, paso 3), cuánto
subieron `F` y `VB` entre las dos capturas y por qué pueden no subir igual.

Al terminar, deshaz el cambio en tu clon (`git restore .`) o bórralo.

## Soluciones

- Parte A: `solucion/contador.c` (el archivo completo).
- Parte B: `solucion/hud_encendido_en_rom_de_prueba.patch` (se aplica en la
  raíz de un clon con `git apply`).
- Respuesta a la pregunta de A: hasta 4 frames (si `visible` cambia justo
  después del turno de la fila, espera a la siguiente vuelta del turno). A 60
  FPS son 67 ms y no se nota; si se notara, se dibujaría la fila en el mismo
  frame en que cambia `visible`, fuera del turno.
