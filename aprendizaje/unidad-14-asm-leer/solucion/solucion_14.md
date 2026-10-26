# Solución del ejercicio 14

Todo lo de aquí se generó y se contó en un clon (`E:\ProyectoSNES_trabajo\u14`)
el 2026-10-22, con PVSnesLib 4.6.0 y FastROM. Las cuentas están en
`../capturas/comprobaciones.txt`.

## TODO 1

Los cuatro `.asm` que genera `make` coinciden con los de `../asm_generado/`
(85, 43, 894 y 516 líneas).

## TODO 2 — `nivel_columna_vram`

Anotado línea a línea en `nivel_anotado.asm`. Resumen:

1. `lda 3 + 1,s`: el parámetro está en `4,s` porque `jsr.l` dejó 3 bytes de
   vuelta en `1,s..3,s` y no hay locales (tcc816 escribe siempre
   `3 + locales + 1`).
2. `and.w #32`.
3. `ldy.w #5` / `asl a` / `dey` / `bne -`: tcc816 escribe seguidos los
   desplazamientos de hasta 4 bits y a partir de 5 genera el bucle. 15
   instrucciones (34 ciclos) en vez de 5 (10 ciclos).
4. `lda 3 + 1,s` otra vez, `and.w #31`, `ora.b tcc__r0`.
5. `sta.b tcc__r0` y `rtl`: el valor devuelto va en `tcc__r0`.

En total, 26 instrucciones ejecutadas.

## TODO 3 — `tile_del_bloque64` (u8) frente a `tile_del_bloque32` (u16)

1. 64: **dos** `jsr.l tcc__div` (uno para `/ 2` y otro para `% 2`; la rutina
   deja el cociente en `tcc__r9` y el resto en X, y tcc816 no aprovecha la
   primera llamada para la segunda). 32: **ninguno**.
2. `n / 4` → `lsr.b tcc__r0` dos veces; `* 64` → bucle `ldy.w #6` de `asl`;
   `n % 4` → `and #3`; `* 4` → dos `asl a`.
3. En C, antes de operar, todo lo más pequeño que `int` se convierte a `int`
   (**promoción entera**). En tcc816 `int` es de 16 bits **con signo**, así que
   `n / 2` con `n` de tipo `u8` es una división de un `int` con signo. Y la
   división con signo de C **redondea hacia el cero**: `-3 / 2` es `-1`, pero
   `-3 >> 1` es `-2`. Un desplazamiento no vale para un número que podría ser
   negativo, así que tcc816 llama a la rutina con signo, `tcc__div`. Con `u16`
   (sin signo, del mismo tamaño que `int`, así que se queda sin signo) la
   división por una potencia de 2 sí es un desplazamiento. Que el `u8` no pueda
   ser negativo de verdad no le importa al compilador: mira el tipo, no los valores.
4. Con `personajes_u16.patch` (`(u16)n / 2`, `(u16)n % 2`) quedan **0**
   `tcc__div`: `lsr.b tcc__r0` y `and #1` (ver `personajes_u16.asm`).
5. La sonda `../asm_generado/divisiones.asm`:

   | C | Sale |
   |---|---|
   | `u8 / 2` | `jsr.l tcc__div` (con signo) |
   | `(u16)u8 / 2` | `lsr` |
   | `u16 / 2` | `lsr` |
   | `s16 / 2` | `jsr.l tcc__div` |
   | `u16 / u16` (divisor variable) | `jsr.l tcc__udiv` (sin signo) |
   | `u8 >> 1` | `cmp #$8000` + `ror` (desplazamiento aritmético de un `int`) |

## TODO 4 — `guion[i]`

1. `t++` es `adc.w #6`: para tcc816 `sizeof(TramoGuion)` es **6**, no 5 como
   dice el comentario de `guion.c`. Dos `u16` (4 bytes) y un `u8` (1) son 5, pero
   el compilador **rellena** la struct hasta un número par de bytes (un byte de
   relleno al final) para que, en un array, cada `u16` empiece en dirección par.
   `gcc` hace lo mismo en el PC (`sizeof` = 6). 6 sigue sin ser potencia de 2, así
   que la conclusión del comentario no cambia.
2. Con `guion_indice.patch`, la función pasa de 144 a 181 líneas.
3. **3** `jsr.l tcc__mul` dentro de `mantenidos`, y antes de cada uno
   `lda.w #6` → `tcc__r9`: es `i * 6`, el desplazamiento del tramo `i` dentro del
   array, recalculado en cada acceso (`desde`, `hasta` y `botones`).
4. Por llamada: la primera comparación (`cuadro >= desde`) se evalúa en los 13
   tramos (13 multiplicaciones); la segunda solo si la primera es cierta, y la
   lectura de `botones` solo si las dos lo son. Así que entre **13 y 39** por
   llamada, y con dos llamadas por frame, entre **26 y 78** multiplicaciones por
   frame. (El comentario de `guion.c` dice «72»: está en ese rango, pero no es
   un número fijo.) `tcc__mul` es un bucle de desplazar y sumar de la librería
   (unos cientos de ciclos por llamada).
5. Con el puntero, pasar al tramo siguiente es **sumar 6** al puntero
   (`adc.w #6`, una vez por vuelta), y cada campo está a una distancia fija del
   puntero (`desde` en +0, `hasta` en +2: `inc a` / `inc a`; `botones` en +4:
   `adc.w #4`). Ninguna multiplicación.

## TODO 5 — `switch` y `PF_ENTERO`

1. Una **cadena de comparaciones**: `lda.b tcc__r0` / `cmp #0` / `beq +` /
   `brl __local_35`, luego `cmp #1`, `cmp #2`... hasta `cmp #7`, y el `default`
   al final. tcc816 no genera tablas de saltos. Con 8 casos, llegar al último
   cuesta 8 comparaciones.
2. `PF_ENTERO(t->x_fp)` es `x_fp >> 4` de un `s16`: un desplazamiento
   **aritmético** (conserva el signo). El 65816 no tiene esa instrucción:
   `cmp #$8000` copia el bit 15 en el acarreo (C = 1 si el número es negativo) y
   `ror a` rota a la derecha metiendo ese acarreo por arriba. `lsr a` metería
   siempre un 0 y un número negativo se volvería enorme. (Que `PF_ENTERO` solo
   valga para valores ≥ 0 no es culpa del `ror`, que es correcto: es el `(u16)`
   de la macro en `src/sistema/puntofijo.h`, que convierte −3 en 65533; ver
   unidad 01.)

## TODO 6 — Mesen (para comprobar con tu captura)

En la primera parada (entrada en el nivel, `pantalla_nivel1_iniciar` →
`camara_para(X_INICIAL)`):

- `x_pix` en la pila (`4,s`, tras los 3 bytes de vuelta) = `$0008`:
  `X_INICIAL = NIVEL1_INICIO_X − ANCHO_CELDA / 2 = 56 − 48 = 8`.
- Tras `sbc.w #128`: `tcc__r0` = `$FFB8` = −72 en complemento a dos
  (8 + 48 − 128). El primer `if` lo cambia a 0; sale con `tcc__r0 = $0000`.
- `D = $0000` (la página directa del programa: `tcc__r0` está en `$00`, que en
  el banco 0 es la WRAM `$7E:0000`). `DB = $7E`: tcc816 lee y escribe las
  variables globales con direcciones de 16 bits (`sta.w`), que se completan con
  DB; las variables están en el banco $7E. `PB = $8x`: FastROM, el código se ve
  en los bancos `$80-$FF` (el `.sym` dice en cuál; `camara_para` estaba en `$8E`
  en la compilación de referencia). Banderas: `e = 0` (modo nativo), `m = 0` y
  `x = 0` (A, X e Y de 16 bits: tcc816 vive en 16 bits).
- Los valores exactos de `S`, `A`, `X` e `Y` al entrar dependen de quién llama
  y se interpretan, no se comparan: `S` apunta al primer hueco libre de la pila
  (por eso la vuelta está en `S+1..S+3`).
- Más adelante, con Tarou en `x_pix = 200`: 200 + 48 − 128 = 120 = `$0078`, que
  no toca ninguno de los dos recortes.
