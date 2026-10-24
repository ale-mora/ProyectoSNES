# Ejercicio 05 — Contar cuántas veces se pulsa B

Copia `rom/` a una carpeta tuya (por ejemplo `E:\ProyectoSNES_trabajo\mi_u05\`)
y trabaja sobre la copia. Objetivo: una fila nueva, `B PRESSES n`, que sube 1
cada vez que B **baja**, no mientras está mantenido.

Restricciones de siempre: sin `int` (salvo `main`), las `static`
inicializadas a mano, y `!= 0` al mirar un bit alto.

- **TODO 1.** Una variable `static u16 pulsaciones_b;` con su comentario,
  puesta a 0 junto a las demás antes del bucle.
- **TODO 2.** En el bucle, justo después de `leer_mando()`, súmale 1 cuando B
  esté entre los **recién pulsados** (`pulsados`, que viene de `padsDown`).
  Antes de escribirlo, responde: ¿qué contaría si usaras `mantenidos`
  (`padsCurrent`) y mantuvieras B un segundo?
- **TODO 3.** Sube `FILAS` de 5 a 6 para que haya un turno más.
- **TODO 4.** Añade al `switch` un `case 5:` que dibuje
  `"B PRESSES %5u"` en la fila `FILA_HELD + 13`, columna 1. Comprueba que el
  `default` sigue dibujando la fila `PAD` (ahora le toca al turno 4).

## Comprobar

1. Compila en tu copia: «No warnings or errors found.».
2. Ábrela en Mesen. Pulsa B (tecla **A** del teclado) tres veces, una de ellas
   manteniéndola un par de segundos: tiene que poner `B PRESSES 3`.
3. Compila la versión que se juega sola y comprueba sin tocar nada:
   ```
   $env:CFLAGS="-DGUION_MANDO"; make clean; make; $env:CFLAGS=$null
   ```
   A partir del frame 400 (unos 7 s) debe quedarse en `B PRESSES 3`: el
   guion mantiene B 60 frames (60–119) y da dos toques (150 y 300).

## Pregunta extra

Si el bucle perdiera un frame justo cuando se pulsa B (unidad 04), ¿se
perdería la pulsación? Pista: en qué frames calcula el SDK `pad_keysdown`
(README, «Cómo lo lee PVSnesLib»).

## Solución

`solucion/botones.c` (el archivo completo). Respuestas:

- Con `mantenidos`, mantener B un segundo sumaría 60.
- No se pierde: en un frame perdido la NMI no lee el mando, así que
  `pad_keys` y `pad_keysold` no cambian; el cambio se detecta en el siguiente
  frame bueno, que es la siguiente vuelta del bucle. Llega un frame tarde,
  pero llega una vez.
