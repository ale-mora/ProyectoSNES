# Solución del ejercicio 15

Archivos:

| Archivo | Qué es | Dónde va en el clon |
|---|---|---|
| `hud_numero.asm` | TODO 1–7 resueltos | `src/juego/hud_numero.asm` |
| `colision_rapida.asm` | la ampliación: `colision_apoyan` en asm | `src/juego/colision_rapida.asm` |
| `colision.c.patch` | `colision_rejilla` global + la versión C solo con `PRUEBAS_PC` | `git apply` en la raíz del clon |

(`../andamio_hud_debug.c.patch` no es solución: es el andamio del enunciado.)

Verificado en el clon `E:\ProyectoSNES_trabajo\u14` el 2026-10-22
(`../capturas/verificacion_juego.txt`): los dos parches pasan `git apply --check`
sobre el clon limpio; `367 pruebas, 0 fallos`; las ROM normal, con
`-DGUION_PRUEBA` y con `-DGUION_PRUEBA -DHUD_NUMERO_ASM` dan
`No warnings or errors found.`; `comprobar_hud_numero.py` da 0 fallos en 65 536
valores. Lo que solo Mesen puede ver (la fila `ASM 14/14`, el recorrido con
`colision_apoyan` en asm y los ciclos) queda para la tanda de capturas.

## TODO 1–7: las decisiones

- **El valor en A todo el rato.** La versión de C guarda `v`, `cifra`,
  `empezado`, `k` y el puntero en la pila y los recarga en cada operación
  (genera `hud_debug.asm` y lo verás: una vuelta de `while (v >= *pot) { v -= *pot;
  cifra++; }` son **43 instrucciones**, contadas en el `.asm` del 2026-10-22: recarga
  el puntero `pot` de la pila dos veces, lo lee dos veces con `[tcc__r0]`, calcula el
  booleano de la comparación en X y suma 1 a `cifra` con `sep`/`rep`). Aquí la vuelta
  de la resta son **6**:
  `cmp.l` / `bcc` / `sec` / `sbc.l` / `inc.b` / `bra`.
- **El puntero no se mueve: Y es el desplazamiento.** `sta [tcc__r0],y` escribe
  en `p + Y`; al final, `p + Y` es lo que se devuelve. En C, `*p++` copia el
  puntero de 4 bytes a otro registro, le suma 2 y lo vuelve a guardar.
- **`cmp.l hud_potencias,x`**: la tabla está en la ROM, en otro banco que la
  WRAM a la que apunta DB ($7E), así que se lee con dirección LARGA (`.l`). La
  `POTENCIAS` de C también está en la ROM (el `.sym` la pone en `$0E:C7EF`: una
  sección `.rodata` que tcc816 escribe tras `.BASE $00`, por eso sin el `$80`), y
  la versión en C la lee con un puntero largo (`lda.b [tcc__r0]`).
- **−32768**: `eor #$FFFF` + `inc a` da `$8000`, que como número sin signo es
  32768, y la resta de potencias funciona igual que en C (`(u16)(-valor)`).
- **`tcc__r1`/`tcc__r2` se pueden usar**: en `hud_debug.asm`, quien llama a
  `numero` guarda todo lo suyo en la pila y después de la llamada solo lee
  `tcc__r0`/`tcc__r0h` (el valor devuelto). Es la regla de tcc816: los `tcc__rN`
  son de usar y tirar entre llamadas.

Cuentas del simulador (instrucciones por llamada, `../capturas/salida_comprobar_hud_numero.txt`):
0 → 69, 1234 → 126, 32767 → 205, −32768 → 213 (el máximo, 279, es para valores
con muchos nueves). Los ciclos de verdad, en el profiler.

## Ampliación: `colision_apoyan`

- `colision_en` ya no la llama el juego (solo `tests/prueba_colision.c`): por eso
  la ampliación cambia de objetivo.
- `#define rejilla colision_rejilla` deja el resto de `colision.c` sin tocar.
- En el `.asm`, `cmp.w #224` + `bcs` cubre a la vez `y >= 224` e `y < 0`: una
  `s16` negativa, mirada sin signo, es ≥ $8000. Es el mismo truco que el
  `(u16)(x - bloque_x) < bloque_ancho` de `colision.c`.
- La subrutina `_pie` usa `jsr`/`rts` (corta, mismo banco): C no la ve y no
  necesita la convención de tcc816; devuelve el resultado en la bandera C.
- La celda es un byte: `sep #$20` solo alrededor de `lda [tcc__r0],y` y las dos
  comparaciones.
- El `.asm` que genera tcc816 para la versión en C son 247 líneas; la de mano,
  unas 50.

## TODO 11: `*p++ = 0` frente a `memset`

Medido en `../capturas/relleno_c_generado.asm`: una vuelta de `limpiar_c` son
**40 instrucciones** (11 de comprobar `n != 0` calculando un booleano en X, 18 de
copiar el puntero de 4 bytes, sumarle 1 y escribir el byte con `sep`/`rep`, 11
de restar 1 a `n` con `adc.w #65535`). `memset` son **3** (`dey` /
`sta [$00],y` / `bne`), leídas de los bytes de la ROM. Y `relleno_vram_c` frente
a `relleno_vram`: **30 instrucciones** por palabra frente a **3**.
